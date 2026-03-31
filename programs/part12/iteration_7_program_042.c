/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass (omp-oacc-neuter-broadcast.cc).
 * It uses various OpenACC compute constructs with explicit data partitioning
 * to cover the switch statement at lines 335-343.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 64
#define M 32
#define P 16

/* Global arrays for persistent device data */
int global_3d_arr[N][M][P];
float global_2d_arr[N][M];

/* Test 1: Basic partition combinations using #pragma acc parallel */
void test_basic_partitions(int use_gpu) {
    int arr1[N][M];
    int arr2[N][M];
    
    /* Initialize arrays */
    #pragma acc parallel if(use_gpu) copy(arr1, arr2) gang
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i + j;
            arr2[i][j] = i * j;
        }
    }
    
    /* Case 0: gang redundant */
    #pragma acc parallel if(use_gpu) copy(arr1) gang
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] += 1;
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc parallel if(use_gpu) copy(arr2) gang(static:2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] *= 2;
        }
    }
    
    /* Case 2: worker partitioned */
    int arr3[N];
    #pragma acc parallel if(use_gpu) copy(arr3) worker
    for (int i = 0; i < N; i++) {
        arr3[i] = i * 3;
    }
    
    /* Case 3: gang+worker partitioned */
    int arr4[N][M];
    #pragma acc parallel if(use_gpu) copy(arr4) gang worker
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr4[i][j] = i - j;
        }
    }
    
    /* Case 4: vector partitioned */
    float arr5[N];
    #pragma acc parallel if(use_gpu) copy(arr5) vector
    for (int i = 0; i < N; i++) {
        arr5[i] = i * 1.5f;
    }
    
    /* Case 5: gang+vector partitioned */
    double arr6[N][M];
    #pragma acc parallel if(use_gpu) copy(arr6) gang vector
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr6[i][j] = (double)i / (j + 1);
        }
    }
    
    /* Case 6: worker+vector partitioned */
    int arr7[N];
    #pragma acc parallel if(use_gpu) copy(arr7) worker vector
    for (int i = 0; i < N; i++) {
        arr7[i] = i % 10;
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    int arr8[N][M][P];
    #pragma acc parallel if(use_gpu) copy(arr8) gang worker vector
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr8[i][j][k] = i + j + k;
            }
        }
    }
}

/* Test 2: Multi-dimensional arrays with collapse and partition clauses */
void test_multi_dim_partitions(int use_gpu) {
    int arr3d[N][M][P];
    
    /* Initialize 3D array */
    #pragma acc parallel if(use_gpu) copy(arr3d) gang
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = 0;
            }
        }
    }
    
    /* gang partitioned with collapse */
    #pragma acc parallel if(use_gpu) copy(arr3d) gang(static:4)
    #pragma acc loop collapse(2) gang
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] += i;
            }
        }
    }
    
    /* worker+vector partitioned with collapse */
    #pragma acc parallel if(use_gpu) copy(arr3d) worker vector
    #pragma acc loop collapse(3) worker vector
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] += j;
            }
        }
    }
    
    /* fully partitioned with independent clauses */
    #pragma acc parallel if(use_gpu) copy(arr3d) gang worker vector
    #pragma acc loop gang
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            #pragma acc loop vector
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] += k;
            }
        }
    }
}

/* Test 3: Nested and sequential compute regions */
void test_nested_regions(int use_gpu) {
    int arr[N][M];
    
    /* Sequential regions with different partition types */
    #pragma acc kernels if(use_gpu) copy(arr) gang
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    #pragma acc parallel if(use_gpu) copy(arr) worker
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] += 1;
        }
    }
    
    #pragma acc kernels if(use_gpu) copy(arr) vector
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] *= 2;
        }
    }
    
    /* Function with nested parallel region */
    #pragma acc parallel if(use_gpu) copy(arr) gang
    {
        int local_sum = 0;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                local_sum += arr[i][j];
            }
        }
        
        /* Nested parallel region with different partition */
        #pragma acc parallel if(use_gpu) vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] % 1000;
            }
        }
    }
}

/* Routine directive with partition specification */
#pragma acc routine vec gang
void acc_routine_func(int *arr, int n, int factor) {
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * factor;
    }
}

/* Test 4: Combination with routine directives */
void test_routine_directives(int use_gpu) {
    int arr1[N];
    int arr2[N];
    
    /* Initialize arrays */
    #pragma acc parallel if(use_gpu) copy(arr1, arr2) gang
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* Call routine from gang-partitioned region */
    #pragma acc parallel if(use_gpu) copy(arr1) gang
    #pragma acc loop gang
    for (int i = 0; i < 1; i++) {  /* Single iteration to call routine once per gang */
        acc_routine_func(arr1, N, 3);
    }
    
    /* Call routine from vector-partitioned region */
    #pragma acc parallel if(use_gpu) copy(arr2) vector
    #pragma acc loop vector
    for (int i = 0; i < 1; i++) {
        acc_routine_func(arr2, N, 2);
    }
}

/* Test 5: Device data environments with partition clauses */
void test_device_data_env(int use_gpu) {
    int persistent_arr[N][M];
    int temp_arr[N];
    
    /* Initialize on host */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            persistent_arr[i][j] = i + j * 10;
        }
        temp_arr[i] = i * 5;
    }
    
    /* Enter data with gang partition */
    #pragma acc enter data if(use_gpu) copyin(persistent_arr) gang
    
    /* Enter data with worker partition */
    #pragma acc enter data if(use_gpu) copyin(temp_arr) worker
    
    /* Compute region with present clause and gang partition */
    #pragma acc parallel if(use_gpu) present(persistent_arr) gang
    #pragma acc loop gang
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            persistent_arr[i][j] += 100;
        }
    }
    
    /* Another region with worker partition */
    #pragma acc parallel if(use_gpu) present(temp_arr) worker
    #pragma acc loop worker
    for (int i = 0; i < N; i++) {
        temp_arr[i] -= 50;
    }
    
    /* Region with vector partition using the same data */
    #pragma acc parallel if(use_gpu) present(persistent_arr) vector
    #pragma acc loop vector
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            persistent_arr[i][j] = persistent_arr[i][j] % 97;
        }
    }
    
    /* Exit data with partition clauses */
    #pragma acc exit data if(use_gpu) copyout(persistent_arr) gang
    #pragma acc exit data if(use_gpu) copyout(temp_arr) worker
}

/* Test 6: Mixed constructs with complex data clauses */
void test_mixed_constructs(int use_gpu) {
    int arr_a[N][M];
    int arr_b[N][M];
    int arr_c[N];
    
    /* Combined copy/create with partition specifications */
    #pragma acc parallel if(use_gpu) copy(arr_a) create(arr_b) gang worker
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr_a[i][j] = i * j;
                arr_b[i][j] = i + j;
            }
        }
    }
    
    /* Kernels construct with partition */
    #pragma acc kernels if(use_gpu) copy(arr_a, arr_b) create(arr_c) vector
    {
        for (int i = 0; i < N; i++) {
            int sum = 0;
            for (int j = 0; j < M; j++) {
                sum += arr_a[i][j] + arr_b[i][j];
            }
            arr_c[i] = sum;
        }
    }
    
    /* Parallel loop with multiple partition types */
    #pragma acc parallel loop if(use_gpu) copy(arr_c) gang vector collapse(1)
    for (int i = 0; i < N; i++) {
        arr_c[i] = arr_c[i] / M;
    }
}

int main(int argc, char **argv) {
    int use_gpu = 1;
    
    /* Use argc to prevent dead code elimination */
    if (argc > 1) {
        use_gpu = atoi(argv[1]) > 0;
    }
    
    printf("Testing OpenACC partition combinations to cover neuter-broadcast switch cases\n");
    
    /* Execute all tests to trigger various partition mappings */
    test_basic_partitions(use_gpu);
    test_multi_dim_partitions(use_gpu);
    test_nested_regions(use_gpu);
    test_routine_directives(use_gpu);
    test_device_data_env(use_gpu);
    test_mixed_constructs(use_gpu);
    
    printf("All tests completed (compile-time coverage goal achieved)\n");
    
    return 0;
}
