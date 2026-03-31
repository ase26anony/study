/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass (lines 335-343 of
 * omp-oacc-neuter-broadcast.cc). It uses various OpenACC compute constructs
 * with explicit data partitioning across gang, worker, and vector dimensions.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Global arrays to allow data persistence across functions */
int global_3d_arr[N][M][P];
float global_2d_arr[N*2][M*2];

/* Routine with explicit partition type */
#pragma acc routine vec gang
void acc_routine_gang_vector(int *arr, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        arr[i] += i * 2;
    }
}

/* Another routine with worker partitioning */
#pragma acc routine worker
void acc_routine_worker(float *arr, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 1.5f;
    }
}

/* Test function 1: Basic partition combinations using parallel construct */
void test_basic_partitions(int use_alt_path) {
    int arr1[N][M];
    int arr2[N][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i + j;
            arr2[i][j] = i * j;
        }
    }
    
    /* Case 0: gang redundant */
    #pragma acc parallel if(use_alt_path > 0) copy(arr1[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr1[i][j] += 1;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc parallel copy(arr2[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2[i][j] *= 2;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    int arr3[N*M];
    #pragma acc parallel copy(arr3[0:N*M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N*M; i++) {
            arr3[i] = i % 256;
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] -= arr2[i][j];
            }
        }
    }
}

/* Test function 2: Vector partitioning with multi-dimensional arrays */
void test_vector_partitions(int seed) {
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(global_3d_arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    global_3d_arr[i][j][k] = i * 10000 + j * 100 + k + seed;
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc kernels copy(global_3d_arr[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    global_3d_arr[i][j][k] += (i + j + k) * 2;
                }
            }
        }
    }
}

/* Test function 3: Combined partitioning and nested regions */
void test_combined_partitions(int toggle) {
    float local_arr[N*2][M*2];
    
    /* Initialize */
    for (int i = 0; i < N*2; i++) {
        for (int j = 0; j < M*2; j++) {
            local_arr[i][j] = (i * 1.0f) / (j + 1.0f);
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel if(toggle == 0) copy(local_arr[0:N*2][0:M*2]) worker vector
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < N*2; i++) {
            for (int j = 0; j < M*2; j++) {
                local_arr[i][j] = local_arr[i][j] * 0.5f + 1.0f;
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc kernels copy(local_arr[0:N*2][0:M*2]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < N*2; i++) {
            for (int j = 0; j < M*2; j++) {
                local_arr[i][j] = local_arr[i][j] * 2.0f - 1.0f;
            }
        }
    }
    
    /* Nested region with different partition */
    #pragma acc parallel copy(local_arr[0:N*2][0:M*2]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N*2; i += 2) {
            /* Inner parallel region with vector partition */
            #pragma acc parallel vector present(local_arr)
            {
                #pragma acc loop vector
                for (int j = 0; j < M*2; j++) {
                    local_arr[i][j] += local_arr[i+1][j];
                }
            }
        }
    }
}

/* Test function 4: Device data environment with partitioning */
void test_device_data_env(int use_persistent) {
    int persistent_arr[N*M*2];
    
    /* Initialize on host */
    for (int i = 0; i < N*M*2; i++) {
        persistent_arr[i] = i * 3;
    }
    
    /* Enter data with gang partition */
    #pragma acc enter data copyin(persistent_arr[0:N*M*2]) gang
    
    /* Compute with worker partition on device data */
    #pragma acc parallel present(persistent_arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N*M*2; i++) {
            persistent_arr[i] += (i % 10);
        }
    }
    
    /* Another compute with vector partition */
    #pragma acc kernels present(persistent_arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N*M*2; i++) {
            persistent_arr[i] *= 2;
        }
    }
    
    /* Call routine with gang vector partition */
    #pragma acc parallel present(persistent_arr) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < 4; i++) {
            acc_routine_gang_vector(&persistent_arr[i * (N*M*2)/4], (N*M*2)/4);
        }
    }
    
    /* Exit data */
    #pragma acc exit data copyout(persistent_arr[0:N*M*2])
    
    /* Verify on host */
    int sum = 0;
    for (int i = 0; i < N*M*2; i++) {
        sum += persistent_arr[i];
    }
    printf("Persistent array sum: %d\n", sum);
}

/* Test function 5: Mixed constructs and routine calls */
void test_mixed_routines(int variant) {
    float dynamic_arr[N*4];
    
    for (int i = 0; i < N*4; i++) {
        dynamic_arr[i] = (float)i / 100.0f;
    }
    
    /* Use routine with worker partition inside parallel region */
    #pragma acc parallel copy(dynamic_arr[0:N*4]) gang worker
    {
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            acc_routine_worker(&dynamic_arr[g * (N*2)], N*2);
        }
    }
    
    /* Follow with vector-partitioned kernels */
    #pragma acc kernels copy(dynamic_arr[0:N*4]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N*4; i++) {
            dynamic_arr[i] += 0.01f * (i % 7);
        }
    }
    
    /* Final fully partitioned region */
    if (variant > 0) {
        #pragma acc parallel copy(dynamic_arr[0:N*4]) gang worker vector
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N*4; i++) {
                dynamic_arr[i] = dynamic_arr[i] * 0.9f;
            }
        }
    }
}

int main(int argc, char **argv) {
    int test_path = (argc > 1) ? atoi(argv[1]) : 0;
    
    printf("Testing OpenACC partition mapping (targeting neuter-broadcast pass)\n");
    
    /* Execute different test paths based on arguments to prevent dead code elimination */
    switch (test_path % 4) {
        case 0:
            test_basic_partitions(argc);
            test_vector_partitions(argc);
            break;
        case 1:
            test_combined_partitions(argc);
            test_device_data_env(argc);
            break;
        case 2:
            test_mixed_routines(argc);
            test_basic_partitions(argc + 1);
            break;
        case 3:
            test_vector_partitions(argc * 2);
            test_combined_partitions(argc);
            test_mixed_routines(argc % 2);
            break;
    }
    
    /* Final validation on host */
    int final_check = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                final_check += global_3d_arr[i][j][k] % 100;
            }
        }
    }
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}
