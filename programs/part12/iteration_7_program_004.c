/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass (lines 335-343 of
 * omp-oacc-neuter-broadcast.cc). It uses various OpenACC compute constructs
 * with explicit data partitioning across gang, worker, and vector dimensions.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 16
#define P 8

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
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
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
    #pragma acc parallel copy(arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += i + j + k;
                }
            }
        }
    }
}

/* Test 4: gang+worker partitioned */
void test_gang_worker_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= (i * j * k) % 7;
                }
            }
        }
    }
}

/* Test 5: vector partitioned */
void test_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = arr[i][j][k] / 2;
                }
            }
        }
    }
}

/* Test 6: gang+vector partitioned */
void test_gang_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += k * 3;
                }
            }
        }
    }
}

/* Test 7: worker+vector partitioned */
void test_worker_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = (arr[i][j][k] + 1) % 100;
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
                    arr[i][j][k] = arr[i][j][k] * 2 + 1;
                }
            }
        }
    }
}

/* Test 9: Nested and sequential compute regions with different partitions */
void test_nested_regions(int arr[N][M][P]) {
    int condition = 1;
    
    /* First region with gang partitioning */
    #pragma acc parallel if(condition) copy(arr) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 5;
                }
            }
        }
    }
    
    /* Second region with worker partitioning */
    #pragma acc kernels copy(arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 2;
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
    
    #pragma acc parallel copy(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            /* Call vector-partitioned routine */
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                acc_routine_vector(&arr[i][j][0], P);
            }
        }
    }
}

/* Test 11: Persistent device data with partition clauses */
void test_persistent_data(int arr[N][M][P]) {
    /* Establish device data region with gang partitioning */
    #pragma acc enter data copyin(arr[0:N][0:M][0:P]) gang
    
    /* Compute region using present data with worker partitioning */
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
                    arr[i][j][k] /= 2;
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
                arr[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
}

/* Verify array contents (simplified check) */
int verify_array(int arr[N][M][P], int test_id) {
    int errors = 0;
    /* Simple check: ensure no negative values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                if (arr[i][j][k] < 0) {
                    errors++;
                }
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    int array1[N][M][P];
    int array2[N][M][P];
    
    /* Use argc to prevent dead code elimination */
    int test_mask = (argc > 1) ? atoi(argv[1]) : 0xFFF;
    
    printf("Testing OpenACC partition combinations to trigger neuter-broadcast pass...\n");
    
    /* Test 1: gang redundant */
    if (test_mask & 0x001) {
        init_array(array1);
        test_gang_redundant(array1);
        printf("Test 1 (gang redundant): %d errors\n", verify_array(array1, 1));
    }
    
    /* Test 2: gang partitioned */
    if (test_mask & 0x002) {
        init_array(array2);
        test_gang_partitioned(array2);
        printf("Test 2 (gang partitioned): %d errors\n", verify_array(array2, 2));
    }
    
    /* Test 3: worker partitioned */
    if (test_mask & 0x004) {
        init_array(array1);
        test_worker_partitioned(array1);
        printf("Test 3 (worker partitioned): %d errors\n", verify_array(array1, 3));
    }
    
    /* Test 4: gang+worker partitioned */
    if (test_mask & 0x008) {
        init_array(array2);
        test_gang_worker_partitioned(array2);
        printf("Test 4 (gang+worker partitioned): %d errors\n", verify_array(array2, 4));
    }
    
    /* Test 5: vector partitioned */
    if (test_mask & 0x010) {
        init_array(array1);
        test_vector_partitioned(array1);
        printf("Test 5 (vector partitioned): %d errors\n", verify_array(array1, 5));
    }
    
    /* Test 6: gang+vector partitioned */
    if (test_mask & 0x020) {
        init_array(array2);
        test_gang_vector_partitioned(array2);
        printf("Test 6 (gang+vector partitioned): %d errors\n", verify_array(array2, 6));
    }
    
    /* Test 7: worker+vector partitioned */
    if (test_mask & 0x040) {
        init_array(array1);
        test_worker_vector_partitioned(array1);
        printf("Test 7 (worker+vector partitioned): %d errors\n", verify_array(array1, 7));
    }
    
    /* Test 8: fully partitioned */
    if (test_mask & 0x080) {
        init_array(array2);
        test_fully_partitioned(array2);
        printf("Test 8 (fully partitioned): %d errors\n", verify_array(array2, 8));
    }
    
    /* Test 9: nested regions */
    if (test_mask & 0x100) {
        init_array(array1);
        test_nested_regions(array1);
        printf("Test 9 (nested regions): %d errors\n", verify_array(array1, 9));
    }
    
    /* Test 10: routine directives */
    if (test_mask & 0x200) {
        init_array(array2);
        test_routine_with_partition(array2);
        printf("Test 10 (routine directives): %d errors\n", verify_array(array2, 10));
    }
    
    /* Test 11: persistent data */
    if (test_mask & 0x400) {
        init_array(array1);
        test_persistent_data(array1);
        printf("Test 11 (persistent data): %d errors\n", verify_array(array1, 11));
    }
    
    printf("All tests completed.\n");
    return 0;
}
