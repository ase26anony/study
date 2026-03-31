/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's omp-oacc-neuter-broadcast.cc (lines 335-343).
 * It uses various OpenACC constructs with explicit data partitioning
 * to ensure all switch cases are exercised during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i * 2;
    }
}

/* OpenACC routine with vector partitioning */
#pragma acc routine vec vector
void acc_routine_vector(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] -= i;
    }
}

/* Test 1: gang redundant partitioning */
void test_gang_redundant(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang
            for (int j = 0; j < M; j++) {
                #pragma acc loop gang
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 2: gang partitioned */
void test_gang_partitioned(int arr[N][M][P]) {
    #pragma acc kernels create(arr[0:N][0:M][0:P]) gang(static:2)
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
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop worker
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 3;
                }
            }
        }
    }
}

/* Test 4: gang+worker partitioned */
void test_gang_worker_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop worker
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += i + j + k;
                }
            }
        }
    }
}

/* Test 5: vector partitioned */
void test_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] ^= 0xFF;
                }
            }
        }
    }
}

/* Test 6: gang+vector partitioned */
void test_gang_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] |= 0x01;
                }
            }
        }
    }
}

/* Test 7: worker+vector partitioned */
void test_worker_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] &= 0xFE;
                }
            }
        }
    }
}

/* Test 8: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = arr[i][j][k] * 3 / 2;
                }
            }
        }
    }
}

/* Test 9: Nested and sequential compute regions with different partitions */
void test_nested_regions(int arr[N][M][P]) {
    int condition = 1;
    
    /* First region with gang partitioning */
    #pragma acc parallel if(condition) copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 10;
                }
            }
        }
    }
    
    /* Second region with worker partitioning */
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 5;
                }
            }
        }
    }
}

/* Test 10: Routine directives with partition types */
void test_routine_with_partition(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Call gang-partitioned routine */
            acc_routine_gang(&arr[i][0][0], M * P);
        }
    }
    
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            /* Call vector-partitioned routine */
            acc_routine_vector(&arr[i][0][0], M * P);
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
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += i * j * k;
                }
            }
        }
    }
    
    /* Another compute region with vector partitioning */
    #pragma acc kernels present(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 100;
                }
            }
        }
    }
    
    /* Exit data region */
    #pragma acc exit data copyout(arr[0:N][0:M][0:P])
}

/* Initialize array with test data */
void init_array(int arr[N][M][P]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
}

/* Verify array contents (simplified check) */
int verify_array(int arr[N][M][P], int expected_base) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                /* Just check that values are non-zero after transformations */
                if (arr[i][j][k] == 0) {
                    errors++;
                }
            }
        }
    }
    return errors;
}

int main(int argc, char **argv) {
    int arr[N][M][P];
    
    /* Initialize with deterministic values */
    init_array(arr);
    
    /* Use argc to create conditional execution paths
     * This prevents dead code elimination */
    
    if (argc > 1) {
        /* Test all partition types */
        test_gang_redundant(arr);
        printf("Test 1 (gang redundant) completed\n");
    }
    
    if (argc > 2) {
        test_gang_partitioned(arr);
        printf("Test 2 (gang partitioned) completed\n");
    }
    
    if (argc > 3) {
        test_worker_partitioned(arr);
        printf("Test 3 (worker partitioned) completed\n");
    }
    
    if (argc > 4) {
        test_gang_worker_partitioned(arr);
        printf("Test 4 (gang+worker partitioned) completed\n");
    }
    
    if (argc > 5) {
        test_vector_partitioned(arr);
        printf("Test 5 (vector partitioned) completed\n");
    }
    
    if (argc > 6) {
        test_gang_vector_partitioned(arr);
        printf("Test 6 (gang+vector partitioned) completed\n");
    }
    
    if (argc > 7) {
        test_worker_vector_partitioned(arr);
        printf("Test 7 (worker+vector partitioned) completed\n");
    }
    
    if (argc > 8) {
        test_fully_partitioned(arr);
        printf("Test 8 (fully partitioned) completed\n");
    }
    
    if (argc > 9) {
        test_nested_regions(arr);
        printf("Test 9 (nested regions) completed\n");
    }
    
    if (argc > 10) {
        test_routine_with_partition(arr);
        printf("Test 10 (routine with partition) completed\n");
    }
    
    if (argc > 11) {
        test_persistent_data(arr);
        printf("Test 11 (persistent data) completed\n");
    }
    
    /* Final verification */
    int errors = verify_array(arr, 0);
    if (errors > 0) {
        printf("Found %d zero values in array\n", errors);
    } else {
        printf("All array elements are non-zero\n");
    }
    
    return 0;
}
