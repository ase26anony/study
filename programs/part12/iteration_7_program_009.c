/* test_openacc_partitions.c
 * Designed to trigger partition string mapping in GCC's omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_openacc_partitions.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

#define N 10
#define M 20
#define P 30

/* Routine with explicit partition specification */
#pragma acc routine vec gang
void increment_element(int *arr, int i, int j, int k) {
    arr[i*M*P + j*P + k] += 1;
}

/* Function with nested compute regions */
void test_nested_partitions(int argc) {
    int arr1[N][M][P];
    int arr2[N][M][P];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr1[i][j][k] = i + j + k;
                arr2[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Test case 0: gang redundant */
    if (argc > 1) {
        #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr1[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    
    /* Test case 1: gang partitioned */
    #pragma acc kernels create(arr2[0:N][0:M][0:P]) gang(static:1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr2[i][j][k] += arr1[i][j][k];
                }
            }
        }
    }
    
    /* Test case 2: worker partitioned */
    int arr3[N][M];
    #pragma acc enter data copyin(arr3[0:N][0:M]) worker
    
    #pragma acc parallel present(arr3) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr3[i][j] = i * 100 + j;
            }
        }
    }
    
    #pragma acc exit data copyout(arr3[0:N][0:M]) worker
}

/* Function with multi-dimensional array and collapse clause */
void test_collapsed_partitions(int argc) {
    int arr4[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr4[i][j][k] = 1;
            }
        }
    }
    
    /* Test case 3: gang+worker partitioned */
    #pragma acc parallel loop collapse(2) gang worker copy(arr4[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr4[i][j][k] += i + j + k;
            }
        }
    }
    
    /* Test case 4: vector partitioned */
    int arr5[N][M];
    #pragma acc enter data create(arr5[0:N][0:M]) vector
    
    if (argc > 2) {
        #pragma acc parallel loop vector present(arr5)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr5[i][j] = arr4[i][j][0] % 100;
            }
        }
    }
    
    #pragma acc exit data copyout(arr5[0:N][0:M]) vector
}

/* Function with routine calls and complex partitioning */
void test_routine_partitions(int argc) {
    int arr6[N][M][P];
    int arr7[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr6[i][j][k] = i * j * k;
                arr7[i][j][k] = 0;
            }
        }
    }
    
    /* Test case 5: gang+vector partitioned */
    #pragma acc parallel copy(arr6[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    /* Call routine with gang partition */
                    increment_element(&arr6[0][0][0], i, j, k);
                }
            }
        }
    }
    
    /* Test case 6: worker+vector partitioned */
    #pragma acc kernels create(arr7[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr7[i][j][k] = arr6[i][j][k] * 2;
                }
            }
        }
    }
}

/* Function testing fully partitioned case */
void test_fully_partitioned(int argc) {
    int arr8[N][M][P];
    int arr9[N][M][P];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr8[i][j][k] = i + j * 2 + k * 3;
                arr9[i][j][k] = 0;
            }
        }
    }
    
    /* Test case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(arr8[0:N][0:M][0:P]) copyout(arr9[0:N][0:M][0:P]) \
        gang worker vector
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr9[i][j][k] = arr8[i][j][k] * 3;
                }
            }
        }
    }
    
    /* Nested region with different partition */
    if (argc > 3) {
        #pragma acc parallel present(arr8, arr9) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc parallel worker
                {
                    #pragma acc loop worker
                    for (int j = 0; j < M; j++) {
                        for (int k = 0; k < P; k++) {
                            arr9[i][j][k] += arr8[i][j][k];
                        }
                    }
                }
            }
        }
    }
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute different test functions based on argc
     * to prevent dead code elimination */
    if (argc > 0) {
        test_nested_partitions(argc);
    }
    
    if (argc > 1) {
        test_collapsed_partitions(argc);
    }
    
    if (argc > 2) {
        test_routine_partitions(argc);
    }
    
    if (argc > 3) {
        test_fully_partitioned(argc);
    }
    
    /* Additional test with mixed partition types in sequence */
    int arr10[5][10][15];
    
    #pragma acc enter data copyin(arr10[0:5][0:10][0:15]) gang
    #pragma acc parallel present(arr10) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 15; k++) {
                    arr10[i][j][k] = i * j * k;
                }
            }
        }
    }
    #pragma acc exit data copyout(arr10[0:5][0:10][0:15]) gang
    
    /* Validate results (simplified) */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 15; k++) {
                sum += arr10[i][j][k];
            }
        }
    }
    
    printf("Final sum: %d\n", sum);
    printf("Test completed.\n");
    
    return 0;
}
