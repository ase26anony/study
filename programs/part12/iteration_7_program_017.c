/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's omp-oacc-neuter-broadcast.cc (lines 335-343).
 * It uses various OpenACC compute constructs with explicit data partitioning
 * to ensure all switch cases are exercised during compilation.
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
void test_device_data_env(int arr[N][M][P]);

/* OpenACC routine with explicit partition */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] += 1;
    }
}

#pragma acc routine worker
void acc_routine_worker(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] *= 2;
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
                    arr[i][j][k] = i * M * P + j * P + k;
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
                    arr[i][j][k] += 1;
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
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] *= 2;
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
                    arr[i][j][k] -= 5;
                }
            }
        }
    }
}

/* Test 5: vector partitioned */
void test_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 10;
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
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] /= 2;
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
                    arr[i][j][k] = arr[i][j][k] % 100;
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
                    arr[i][j][k] = 0;
                }
            }
        }
    }
}

/* Test 9: Nested regions with different partition types */
void test_nested_regions(int arr[N][M][P]) {
    int condition = 1;
    
    #pragma acc parallel if(condition) copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc kernels worker
            {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr[i][j][k] += i + j + k;
                    }
                }
            }
        }
    }
}

/* Test 10: Device data environment with partition clauses */
void test_device_data_env(int arr[N][M][P]) {
    #pragma acc enter data copyin(arr[0:N][0:M][0:P]) gang
    
    #pragma acc parallel present(arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] *= 3;
                }
            }
        }
    }
    
    #pragma acc parallel present(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 1;
                }
            }
        }
    }
    
    #pragma acc exit data copyout(arr[0:N][0:M][0:P])
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int arr[N][M][P];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = 0;
            }
        }
    }
    
    /* Use argc to create conditional execution paths */
    if (argc > 1) {
        test_gang_redundant(arr);
    }
    if (argc > 2) {
        test_gang_partitioned(arr);
    }
    if (argc > 3) {
        test_worker_partitioned(arr);
    }
    if (argc > 4) {
        test_gang_worker_partitioned(arr);
    }
    if (argc > 5) {
        test_vector_partitioned(arr);
    }
    if (argc > 6) {
        test_gang_vector_partitioned(arr);
    }
    if (argc > 7) {
        test_worker_vector_partitioned(arr);
    }
    if (argc > 8) {
        test_fully_partitioned(arr);
    }
    if (argc > 9) {
        test_nested_regions(arr);
    }
    if (argc > 10) {
        test_device_data_env(arr);
    }
    
    /* Simple validation to ensure computations aren't optimized away */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                sum += arr[i][j][k];
            }
        }
    }
    
    printf("Final array sum: %d\n", sum);
    
    return 0;
}
