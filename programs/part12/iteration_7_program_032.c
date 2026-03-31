/* test_openacc_partitions.c
 * Designed to trigger partition string mapping in GCC's omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=0 -c test_openacc_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 100
#define M 50
#define P 25

/* Routine with explicit partition specification */
#pragma acc routine vec gang
void increment_element(int *arr, int idx, int value) {
    arr[idx] += value;
}

/* Test function for gang redundant partitioning */
void test_gang_redundant(int argc) {
    int arr1[N][M][P];
    
    /* Initialize array */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            for (int k = 0; k < P; k++)
                arr1[i][j][k] = i + j + k;
    
    /* Case 0: gang redundant */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr1[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Verify results */
    int errors = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            for (int k = 0; k < P; k++)
                if (arr1[i][j][k] != i + j + k + 1)
                    errors++;
    
    if (argc > 1 && errors > 0)
        printf("Gang redundant test: %d errors\n", errors);
}

/* Test function for gang partitioned */
void test_gang_partitioned(int argc) {
    int arr2[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr2[i][j] = i * j;
    
    /* Case 1: gang partitioned */
    #pragma acc kernels create(arr2[0:N][0:M]) gang(static:1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2[i][j] *= 2;
            }
        }
    }
    
    /* Use conditional to prevent dead code elimination */
    if (argc > 2) {
        #pragma acc parallel if(argc > 2) copy(arr2[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    arr2[i][j] += 1;
                }
            }
        }
    }
}

/* Test function for worker partitioned */
void test_worker_partitioned(int argc) {
    int arr3[N];
    
    /* Case 2: worker partitioned */
    #pragma acc enter data copyin(arr3[0:N])
    
    #pragma acc parallel present(arr3[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr3[i] = i * 3;
        }
    }
    
    #pragma acc exit data copyout(arr3[0:N])
    
    /* Nested region with different partition */
    if (argc > 3) {
        #pragma acc parallel if(argc > 3) copy(arr3[0:N]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                increment_element(arr3, i, 1);
            }
        }
    }
}

/* Test function for gang+worker partitioned */
void test_gang_worker_partitioned(int argc) {
    int arr4[N][M][P];
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(arr4[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr4[i][j][k] = i * j * k;
                }
            }
        }
    }
}

/* Test function for vector partitioned */
void test_vector_partitioned(int argc) {
    float arr5[N][M];
    
    /* Case 4: vector partitioned */
    #pragma acc kernels copy(arr5[0:N][0:M]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                arr5[i][j] = (float)i / (j + 1);
            }
        }
    }
    
    /* Sequential regions with different partitions */
    if (argc > 4) {
        #pragma acc parallel if(argc > 4) copy(arr5[0:N][0:M]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    arr5[i][j] += 0.5f;
                }
            }
        }
    }
}

/* Test function for gang+vector partitioned */
void test_gang_vector_partitioned(int argc) {
    double arr6[N][M];
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel copy(arr6[0:N][0:M]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr6[i][j] = (double)(i + j) * 1.5;
            }
        }
    }
}

/* Test function for worker+vector partitioned */
void test_worker_vector_partitioned(int argc) {
    int arr7[N][M][P];
    
    /* Case 6: worker+vector partitioned */
    #pragma acc enter data create(arr7[0:N][0:M][0:P]) worker
    
    #pragma acc parallel present(arr7[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr7[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
    
    #pragma acc exit data copyout(arr7[0:N][0:M][0:P])
}

/* Test function for fully partitioned */
void test_fully_partitioned(int argc) {
    int arr8[N][M][P];
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(arr8[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr8[i][j][k] = 1;
                }
            }
        }
    }
    
    /* Nested parallel region with different partition */
    if (argc > 5) {
        #pragma acc parallel if(argc > 5) copy(arr8[0:N][0:M][0:P]) 
        {
            /* Mixed partitioning within same region */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr8[i][j][k] += arr8[i][j][k];
                    }
                }
            }
        }
    }
}

/* Combined test with multiple partition types in sequence */
void test_mixed_partitions(int argc) {
    int mixed[N][M];
    
    /* Sequence of different partition types */
    #pragma acc parallel copy(mixed[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                mixed[i][j] = 0;
            }
        }
    }
    
    #pragma acc parallel copy(mixed[0:N][0:M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                mixed[i][j] += i;
            }
        }
    }
    
    #pragma acc parallel copy(mixed[0:N][0:M]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                mixed[i][j] += j;
            }
        }
    }
    
    /* Fully partitioned update */
    #pragma acc parallel copy(mixed[0:N][0:M]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                mixed[i][j] *= 2;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Execute all test functions with argc-based conditions
     * to prevent dead code elimination */
    
    test_gang_redundant(argc);
    test_gang_partitioned(argc);
    test_worker_partitioned(argc);
    test_gang_worker_partitioned(argc);
    test_vector_partitioned(argc);
    test_gang_vector_partitioned(argc);
    test_worker_vector_partitioned(argc);
    test_fully_partitioned(argc);
    test_mixed_partitions(argc);
    
    printf("All OpenACC partition tests completed (argc=%d)\n", argc);
    
    return 0;
}
