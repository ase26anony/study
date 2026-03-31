/* test_omp_acc_partitions.c
 * Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test.c -o test.o
 * Or for debugging: gcc -O1 -fopenacc -fdump-tree-all -c test.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 25

/* Routine with explicit partition type */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

#pragma acc routine worker
void acc_routine_worker(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

/* Test 1: Basic partition types in parallel regions */
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
    #pragma acc parallel copy(arr1[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] += 1;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc parallel copy(arr2[0:N][0:M]) gang(static:1)
    {
        #pragma acc loop gang(static:1)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2[i][j] *= 2;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    int arr3[N];
    #pragma acc parallel copy(arr3[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr3[i] = i * 3;
        }
    }
    
    /* Case 3: gang+worker partitioned */
    int arr4[N][M];
    #pragma acc parallel copy(arr4[0:N][0:M]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr4[i][j] = i + j * 2;
            }
        }
    }
}

/* Test 2: Vector partitions and multi-dimensional arrays */
void test_vector_partitions(int argc) {
    int arr3d[10][20][30];
    
    /* Initialize 3D array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                arr3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                for (int k = 0; k < 30; k++) {
                    arr3d[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel copy(arr3d[0:10][0:20][0:30]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                for (int k = 0; k < 30; k++) {
                    arr3d[i][j][k] *= 2;
                }
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    int arr2d[50][100];
    #pragma acc parallel copy(arr2d[0:50][0:100]) worker vector
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < 50; i++) {
            for (int j = 0; j < 100; j++) {
                arr2d[i][j] = i * j;
            }
        }
    }
}

/* Test 3: Fully partitioned and complex cases */
void test_fully_partitioned(int argc) {
    int arr_full[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr_full[i][j][k] = 1;
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(arr_full[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr_full[i][j][k] += i + j + k;
                }
            }
        }
    }
    
    /* Nested regions with different partitions */
    if (argc > 1) {
        #pragma acc parallel if(argc > 2) gang copy(arr_full[0:N][0:M][0:P])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                /* Inner region with different partition */
                #pragma acc kernels worker
                {
                    #pragma acc loop worker
                    for (int j = 0; j < M; j++) {
                        for (int k = 0; k < P; k++) {
                            arr_full[i][j][k] *= 3;
                        }
                    }
                }
            }
        }
    }
}

/* Test 4: Device data environments with partitions */
void test_device_data_partitions(int argc) {
    int persistent_arr[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            persistent_arr[i][j] = i * 10 + j;
        }
    }
    
    /* Enter data with gang partition */
    #pragma acc enter data copyin(persistent_arr[0:N][0:M]) gang
    
    /* Compute region with worker partition on present data */
    #pragma acc parallel present(persistent_arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                persistent_arr[i][j] += 5;
            }
        }
    }
    
    /* Another region with vector partition */
    #pragma acc parallel present(persistent_arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                persistent_arr[i][j] *= 2;
            }
        }
    }
    
    /* Exit data */
    #pragma acc exit data copyout(persistent_arr[0:N][0:M])
}

/* Test 5: Routine directives with partitions */
void test_routine_partitions(int argc) {
    int routine_arr[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        routine_arr[i] = i;
    }
    
    /* Call gang routine from vector region */
    #pragma acc parallel copy(routine_arr[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            if (i % 2 == 0) {
                acc_routine_gang(&routine_arr[i], 1);
            } else {
                acc_routine_worker(&routine_arr[i], 1);
            }
        }
    }
    
    /* Mixed partition kernels region */
    int arr_mixed[N][M];
    #pragma acc kernels copy(arr_mixed[0:N][0:M]) gang worker
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr_mixed[i][j] = routine_arr[i] + j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Use argc to prevent dead code elimination */
    int test_selector = argc;
    
    /* Execute tests based on selector to ensure all code paths are considered */
    if (test_selector >= 1) {
        test_basic_partitions(argc);
    }
    
    if (test_selector >= 2) {
        test_vector_partitions(argc);
    }
    
    if (test_selector >= 3) {
        test_fully_partitioned(argc);
    }
    
    if (test_selector >= 4) {
        test_device_data_partitions(argc);
    }
    
    if (test_selector >= 5) {
        test_routine_partitions(argc);
    }
    
    /* Additional complex case combining multiple partition types */
    int final_arr[5][10][15];
    #pragma acc parallel copy(final_arr[0:5][0:10][0:15]) gang worker vector
    {
        #pragma acc loop gang
        for (int i = 0; i < 5; i++) {
            #pragma acc loop worker vector collapse(2)
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 15; k++) {
                    final_arr[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
    
    printf("Test completed. Check compiler intermediate outputs for partition string mappings.\n");
    
    /* Simple validation */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 15; k++) {
                sum += final_arr[i][j][k];
            }
        }
    }
    printf("Validation sum: %d\n", sum);
    
    return 0;
}
