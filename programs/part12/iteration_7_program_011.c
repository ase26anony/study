/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition string mapping logic
 * in GCC's OpenACC neuter-broadcast pass (lines 335-343 of omp-oacc-neuter-broadcast.cc).
 * It uses various OpenACC compute constructs with explicit data partitioning
 * across gang, worker, and vector dimensions.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Test function with routine directive */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

/* Another routine with different partition */
#pragma acc routine worker
void acc_routine_worker(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

/* Test 1: Basic partition combinations on multi-dimensional array */
void test_basic_partitions(int argc) {
    int arr3d[10][20][30];
    
    /* Initialize */
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            for (int k = 0; k < 30; k++)
                arr3d[i][j][k] = i + j + k;
    
    /* Case 0: gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (int j = 0; j < 20; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 30; k++) {
                        arr3d[i][j][k] += 1;
                    }
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (argc > 2) {
        #pragma acc kernels copy(arr3d[0:10][0:20][0:30]) gang(static:2)
        {
            #pragma acc loop gang
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 30; k++) {
                        arr3d[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 3) {
        #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 30; k++) {
                        arr3d[i][j][k] -= 1;
                    }
                }
            }
        }
    }
}

/* Test 2: Combined partitions with collapse */
void test_combined_partitions(int argc) {
    int arr2d[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr2d[i][j] = i * j;
    
    /* Case 3: gang+worker partitioned */
    if (argc > 4) {
        #pragma acc parallel loop collapse(2) gang worker copy(arr2d[0:N][0:M])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] += i + j;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (argc > 5) {
        #pragma acc kernels loop vector copy(arr2d[0:N][0:M])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] *= 3;
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (argc > 6) {
        #pragma acc parallel loop gang vector copy(arr2d[0:N][0:M])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] /= 2;
            }
        }
    }
}

/* Test 3: Nested regions and routine calls */
void test_nested_routines(int argc) {
    int arr1d[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++)
        arr1d[i] = i;
    
    /* Case 6: worker+vector partitioned with routine call */
    if (argc > 7) {
        #pragma acc parallel copy(arr1d[0:N]) worker vector
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                arr1d[i] += 5;
            }
            
            /* Call routine with different partition */
            #pragma acc routine seq
            for (int i = 0; i < N; i += 2) {
                arr1d[i] *= 2;
            }
        }
    }
    
    /* Nested conditional region */
    if (argc > 8) {
        #pragma acc parallel if(argc > 10) gang copy(arr1d[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                if (i % 3 == 0) {
                    #pragma acc kernels worker
                    {
                        acc_routine_worker(&arr1d[i], 1);
                    }
                }
            }
        }
    }
}

/* Test 4: Device data environment with partitions */
void test_device_data_env(int argc) {
    int persistent_arr[P][P];
    
    /* Initialize */
    for (int i = 0; i < P; i++)
        for (int j = 0; j < P; j++)
            persistent_arr[i][j] = i * 100 + j;
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (argc > 9) {
        /* Establish device data region with partition */
        #pragma acc enter data copyin(persistent_arr[0:P][0:P]) gang
        
        /* Multiple compute regions with different partitions */
        #pragma acc parallel present(persistent_arr) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < P; i++) {
                for (int j = 0; j < P; j++) {
                    persistent_arr[i][j] += 10;
                }
            }
        }
        
        #pragma acc kernels present(persistent_arr) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < P; i++) {
                for (int j = 0; j < P; j++) {
                    persistent_arr[i][j] *= 2;
                }
            }
        }
        
        /* Fully partitioned access */
        #pragma acc parallel loop collapse(2) gang worker vector present(persistent_arr)
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < P; j++) {
                persistent_arr[i][j] -= 5;
            }
        }
        
        /* Exit data region */
        #pragma acc exit data copyout(persistent_arr[0:P][0:P])
    }
}

/* Test 5: Complex multi-region scenario */
void test_complex_scenario(int argc) {
    int complex_arr[5][10][15];
    
    /* Initialize */
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 10; j++)
            for (int k = 0; k < 15; k++)
                complex_arr[i][j][k] = i * j * k;
    
    /* Mixed partition types in sequence */
    if (argc > 11) {
        /* gang partitioned */
        #pragma acc parallel copy(complex_arr[0:5][0:10][0:15]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 10; j++) {
                    for (int k = 0; k < 15; k++) {
                        complex_arr[i][j][k] += 1;
                    }
                }
            }
        }
        
        /* worker partitioned */
        #pragma acc kernels copy(complex_arr[0:5][0:10][0:15]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 10; j++) {
                    for (int k = 0; k < 15; k++) {
                        complex_arr[i][j][k] *= 2;
                    }
                }
            }
        }
        
        /* vector partitioned with routine call */
        #pragma acc parallel copy(complex_arr[0:5][0:10][0:15]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < 5; i++) {
                acc_routine_gang(&complex_arr[i][0][0], 10 * 15);
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition string mapping...\n");
    
    /* Execute all tests with argc-based conditions to prevent dead code elimination */
    test_basic_partitions(argc);
    test_combined_partitions(argc);
    test_nested_routines(argc);
    test_device_data_env(argc);
    test_complex_scenario(argc);
    
    /* Simple validation */
    int check_arr[10] = {0};
    #pragma acc parallel copy(check_arr[0:10]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            check_arr[i] = i * 2;
        }
    }
    
    /* Verify on host */
    int valid = 1;
    for (int i = 0; i < 10; i++) {
        if (check_arr[i] != i * 2) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        printf("Basic OpenACC execution successful.\n");
    } else {
        printf("Validation failed.\n");
    }
    
    return 0;
}
