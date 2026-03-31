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

/* Function prototypes */
void test_gang_redundant(int arr[N][M][P]);
void test_gang_partitioned(int arr[N][M][P]);
void test_worker_partitioned(int arr[N][M][P]);
void test_gang_worker_partitioned(int arr[N][M][P]);
void test_vector_partitioned(int arr[N][M][P]);
void test_gang_vector_partitioned(int arr[N][M][P]);
void test_worker_vector_partitioned(int arr[N][M][P]);
void test_fully_partitioned(int arr[N][M][P]);
void test_nested_regions(int arr[N][M][P]);
void test_routine_with_partition(int arr[N][M][P]);
void test_persistent_data(int arr[N][M][P]);

/* OpenACC routine with gang partitioning */
#pragma acc routine seq
void acc_increment_element(int *element) {
    *element += 1;
}

#pragma acc routine gang
void acc_gang_increment(int *element) {
    *element += 2;
}

/* Test 1: gang redundant partitioning */
void test_gang_redundant(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += i + j + k;
                }
            }
        }
    }
}

/* Test 2: gang partitioned */
void test_gang_partitioned(int arr[N][M][P]) {
    #pragma acc kernels create(arr[0:N][0:M][0:P]) gang(static:4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Test 3: worker partitioned */
void test_worker_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 1;
                }
            }
        }
    }
}

/* Test 4: gang+worker partitioned */
void test_gang_worker_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = arr[i][j][k] / 2;
                }
            }
        }
    }
}

/* Test 5: vector partitioned */
void test_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += k;
                }
            }
        }
    }
}

/* Test 6: gang+vector partitioned */
void test_gang_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector collapse(3)
        for (int i = 0; i < N; i++) {
            for ( int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += j;
                }
            }
        }
    }
}

/* Test 7: worker+vector partitioned */
void test_worker_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += i;
                }
            }
        }
    }
}

/* Test 8: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = arr[i][j][k] % 100;
                }
            }
        }
    }
}

/* Test 9: Nested and sequential compute regions */
void test_nested_regions(int arr[N][M][P]) {
    int condition = 1;
    
    /* First region with gang partitioning */
    #pragma acc parallel if(condition) copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr[i][0][0] = i;
        }
    }
    
    /* Second region with worker partitioning */
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            arr[0][j][0] = j;
        }
    }
    
    /* Third region with vector partitioning */
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int k = 0; k < P; k++) {
            arr[0][0][k] = k;
        }
    }
}

/* Test 10: Routine with partition specification */
void test_routine_with_partition(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    /* Call routine with gang partitioning */
                    acc_gang_increment(&arr[i][j][k]);
                    
                    /* Call seq routine */
                    acc_increment_element(&arr[i][j][k]);
                }
            }
        }
    }
}

/* Test 11: Persistent device data with partition clauses */
void test_persistent_data(int arr[N][M][P]) {
    /* Establish device data region with gang partitioning */
    #pragma acc enter data copyin(arr[0:N][0:M][0:P]) gang
    
    /* Compute region with worker partitioning using present data */
    #pragma acc parallel present(arr) worker
    {
        #pragma acc loop worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 10;
                }
            }
        }
    }
    
    /* Another compute region with vector partitioning */
    #pragma acc kernels present(arr) vector
    {
        #pragma acc loop vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] *= 3;
                }
            }
        }
    }
    
    /* Exit data region */
    #pragma acc exit data copyout(arr[0:N][0:M][0:P])
}

int main(int argc, char *argv[]) {
    /* Initialize 3D array */
    int (*arr)[M][P] = malloc(N * sizeof(*arr));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with test data */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
    
    /* Use argc to create conditional execution paths
     * This prevents dead code elimination */
    if (argc > 1) {
        /* Execute all partition tests */
        test_gang_redundant(arr);
        test_gang_partitioned(arr);
        test_worker_partitioned(arr);
        test_gang_worker_partitioned(arr);
        test_vector_partitioned(arr);
        test_gang_vector_partitioned(arr);
        test_worker_vector_partitioned(arr);
        test_fully_partitioned(arr);
        test_nested_regions(arr);
        test_routine_with_partition(arr);
        test_persistent_data(arr);
    } else {
        /* Execute a subset of tests */
        test_gang_redundant(arr);
        test_fully_partitioned(arr);
        test_nested_regions(arr);
    }
    
    /* Verify results on host (simple checksum) */
    long long sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                sum += arr[i][j][k];
            }
        }
    }
    
    printf("Array checksum: %lld\n", sum);
    
    free(arr);
    return 0;
}
