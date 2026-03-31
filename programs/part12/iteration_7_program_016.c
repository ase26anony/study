/* test_omp_acc_partitions.c
 * Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Function with routine directive */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i * 2;
    }
}

#pragma acc routine worker
void acc_routine_worker(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] -= i;
    }
}

/* Test 1: Basic partition combinations */
void test_basic_partitions(int argc) {
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
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr1[i][j] += 1;
                }
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (argc > 2) {
        #pragma acc kernels copy(arr2[0:N][0:M]) gang(static:2)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr2[i][j] *= 2;
                }
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (argc > 3) {
        int arr3[N];
        #pragma acc parallel copy(arr3[0:N]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                arr3[i] = i * 3;
            }
        }
    }
}

/* Test 2: Multi-dimensional with collapse */
void test_multi_dim_collapse(int argc) {
    int arr3d[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    if (argc > 4) {
        #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) gang worker
        {
            #pragma acc loop collapse(3) gang worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr3d[i][j][k] += i * j * k;
                    }
                }
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (argc > 5) {
        #pragma acc kernels copy(arr3d[0:N][0:M][0:P]) vector
        {
            #pragma acc loop collapse(2) vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr3d[i][j][k] -= k;
                    }
                }
            }
        }
    }
}

/* Test 3: Combined partitions */
void test_combined_partitions(int argc) {
    int arr4[N][M];
    int arr5[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr5[i] = i * 10;
        for (int j = 0; j < M; j++) {
            arr4[i][j] = i * j * 10;
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (argc > 6) {
        #pragma acc parallel copy(arr4[0:N][0:M]) gang vector
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr4[i][j] += arr5[i];
                }
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    if (argc > 7) {
        #pragma acc kernels copy(arr5[0:N]) worker vector
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                arr5[i] = arr5[i] % 100;
            }
        }
    }
}

/* Test 4: Nested and sequential regions */
void test_nested_sequential(int argc) {
    int arr6[N][M][P];
    int arr7[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr7[i] = i * 5;
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr6[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Sequential regions with different partitions */
    if (argc > 8) {
        /* First region: gang partitioned */
        #pragma acc parallel copy(arr6[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr6[i][j][k] += arr7[i];
                    }
                }
            }
        }
        
        /* Second region: worker partitioned */
        #pragma acc kernels copy(arr7[0:N]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                arr7[i] += i * 2;
            }
        }
        
        /* Nested-like structure using function calls */
        #pragma acc parallel copy(arr6[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                /* Call routine with different partition */
                #pragma acc parallel vector
                {
                    #pragma acc loop vector
                    for (int j = 0; j < M; j++) {
                        for (int k = 0; k < P; k++) {
                            arr6[i][j][k] *= 2;
                        }
                    }
                }
            }
        }
    }
}

/* Test 5: Device data environments with partitions */
void test_device_data_env(int argc) {
    int arr8[N][M];
    int arr9[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr9[i] = i * 7;
        for (int j = 0; j < M; j++) {
            arr8[i][j] = i * 11 + j * 3;
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (argc > 9) {
        /* Enter data with gang partition */
        #pragma acc enter data copyin(arr8[0:N][0:M]) gang
        
        /* Enter data with worker partition */
        #pragma acc enter data copyin(arr9[0:N]) worker
        
        /* Compute region with full partition */
        #pragma acc parallel present(arr8, arr9) gang worker vector
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr8[i][j] += arr9[i] * j;
                }
            }
        }
        
        /* Another region with vector partition */
        #pragma acc kernels present(arr9) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                arr9[i] = arr9[i] % 50;
            }
        }
        
        /* Exit data */
        #pragma acc exit data copyout(arr8[0:N][0:M])
        #pragma acc exit data copyout(arr9[0:N])
    }
}

/* Test 6: Routine directives with partitions */
void test_routine_partitions(int argc) {
    int arr10[N];
    int arr11[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr10[i] = i * 3;
        arr11[i] = i * 4;
    }
    
    if (argc > 10) {
        /* Call gang routine in gang region */
        #pragma acc parallel copy(arr10[0:N]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i += 4) {
                acc_routine_gang(&arr10[i], 4);
            }
        }
        
        /* Call worker routine in worker region */
        #pragma acc kernels copy(arr11[0:N]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i += 8) {
                acc_routine_worker(&arr11[i], 8);
            }
        }
        
        /* Mixed partitions with routine calls */
        #pragma acc parallel copy(arr10[0:N], arr11[0:N]) gang vector
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i += 16) {
                #pragma acc loop vector
                for (int j = 0; j < 16; j++) {
                    int idx = i + j;
                    if (idx < N) {
                        arr10[idx] += arr11[idx];
                    }
                }
            }
        }
    }
}

/* Main function with conditional execution paths */
int main(int argc, char **argv) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Each test is conditionally executed based on argc
     * This prevents dead code elimination while ensuring
     * all partition combinations are compiled */
    
    test_basic_partitions(argc);
    test_multi_dim_collapse(argc);
    test_combined_partitions(argc);
    test_nested_sequential(argc);
    test_device_data_env(argc);
    test_routine_partitions(argc);
    
    /* Final validation region with all partition types */
    int final_arr[N][M];
    
    /* Initialize final array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            final_arr[i][j] = i * M + j;
        }
    }
    
    /* Use all partition types in one final complex region */
    if (argc > 11) {
        /* gang redundant */
        #pragma acc parallel copy(final_arr[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    final_arr[i][j] += 1000;
                }
            }
        }
        
        /* gang partitioned */
        #pragma acc kernels copy(final_arr[0:N][0:M]) gang(static:4)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    final_arr[i][j] += 2000;
                }
            }
        }
        
        /* worker partitioned */
        #pragma acc parallel copy(final_arr[0:N][0:M]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    final_arr[i][j] += 3000;
                }
            }
        }
        
        /* gang+worker partitioned */
        #pragma acc kernels copy(final_arr[0:N][0:M]) gang worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    final_arr[i][j] += 4000;
                }
            }
        }
        
        /* vector partitioned */
        #pragma acc parallel copy(final_arr[0:N][0:M]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    final_arr[i][j] += 5000;
                }
            }
        }
        
        /* gang+vector partitioned */
        #pragma acc kernels copy(final_arr[0:N][0:M]) gang vector
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    final_arr[i][j] += 6000;
                }
            }
        }
        
        /* worker+vector partitioned */
        #pragma acc parallel copy(final_arr[0:N][0:M]) worker vector
        {
            #pragma acc loop worker vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    final_arr[i][j] += 7000;
                }
            }
        }
        
        /* fully partitioned */
        #pragma acc kernels copy(final_arr[0:N][0:M]) gang worker vector
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    final_arr[i][j] += 8000;
                }
            }
        }
    }
    
    /* Simple validation */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += final_arr[i][j];
        }
    }
    
    printf("Final sum: %d\n", sum);
    printf("Test completed.\n");
    
    return 0;
}
