/* test_openacc_partitions.c - Designed to trigger partition string mapping in omp-oacc-neuter-broadcast.cc */

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
void test_mixed_regions(int arr[N][M][P]);
void test_nested_partitions(int arr[N][M][P]);
void test_device_data_env(int arr[N][M][P]);

/* Routines with explicit partition directives */
#pragma acc routine seq
void init_element(int *elem, int value) {
    *elem = value;
}

#pragma acc routine gang
void process_gang(int *elem, int factor) {
    *elem *= factor;
}

#pragma acc routine worker
void process_worker(int *elem, int offset) {
    *elem += offset;
}

#pragma acc routine vector
void process_vector(int *elem, int divisor) {
    *elem /= divisor;
}

/* Test 1: Gang redundant partitioning */
void test_gang_redundant(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
}

/* Test 2: Gang partitioned */
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

/* Test 3: Worker partitioned */
void test_worker_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    process_worker(&arr[i][j][k], 10);
                }
            }
        }
    }
}

/* Test 4: Gang+worker partitioned */
void test_gang_worker_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = arr[i][j][k] * 2;
                }
            }
        }
    }
}

/* Test 5: Vector partitioned */
void test_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    process_vector(&arr[i][j][k], 2);
                }
            }
        }
    }
}

/* Test 6: Gang+vector partitioned */
void test_gang_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = arr[i][j][k] - 5;
                }
            }
        }
    }
}

/* Test 7: Worker+vector partitioned */
void test_worker_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = arr[i][j][k] + 100;
                }
            }
        }
    }
}

/* Test 8: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = arr[i][j][k] % 256;
                }
            }
        }
    }
}

/* Test 9: Mixed regions with conditional execution */
void test_mixed_regions(int arr[N][M][P]) {
    int condition = 1;
    
    /* First region with gang partitioning */
    #pragma acc parallel if(condition) copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    process_gang(&arr[i][j][k], 3);
                }
            }
        }
    }
    
    /* Second region with different partitioning */
    #pragma acc kernels if(condition) copy(arr[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 20;
                }
            }
        }
    }
}

/* Test 10: Nested partitions */
void test_nested_partitions(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Nested parallel region with different partitioning */
            #pragma acc parallel present(arr) worker
            {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr[i][j][k] = arr[i][j][k] * arr[i][j][k];
                    }
                }
            }
        }
    }
}

/* Test 11: Device data environment with partitions */
void test_device_data_env(int arr[N][M][P]) {
    /* Establish device data with gang partitioning */
    #pragma acc enter data copyin(arr[0:N][0:M][0:P]) gang
    
    /* Multiple compute regions accessing partitioned data */
    #pragma acc parallel present(arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 50;
                }
            }
        }
    }
    
    #pragma acc kernels present(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 25;
                }
            }
        }
    }
    
    /* Exit data with same partitioning */
    #pragma acc exit data copyout(arr[0:N][0:M][0:P]) gang
}

int main(int argc, char *argv[]) {
    int arr[N][M][P];
    int test_case = 0;
    
    /* Use argc to prevent dead code elimination */
    if (argc > 1) {
        test_case = atoi(argv[1]) % 12;
    }
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = 1;
            }
        }
    }
    
    /* Execute different test cases based on input */
    switch (test_case) {
        case 0:
            test_gang_redundant(arr);
            break;
        case 1:
            test_gang_partitioned(arr);
            break;
        case 2:
            test_worker_partitioned(arr);
            break;
        case 3:
            test_gang_worker_partitioned(arr);
            break;
        case 4:
            test_vector_partitioned(arr);
            break;
        case 5:
            test_gang_vector_partitioned(arr);
            break;
        case 6:
            test_worker_vector_partitioned(arr);
            break;
        case 7:
            test_fully_partitioned(arr);
            break;
        case 8:
            test_mixed_regions(arr);
            break;
        case 9:
            test_nested_partitions(arr);
            break;
        case 10:
            test_device_data_env(arr);
            break;
        case 11:
            /* Execute all tests sequentially */
            test_gang_redundant(arr);
            test_gang_partitioned(arr);
            test_worker_partitioned(arr);
            test_gang_worker_partitioned(arr);
            test_vector_partitioned(arr);
            test_gang_vector_partitioned(arr);
            test_worker_vector_partitioned(arr);
            test_fully_partitioned(arr);
            test_mixed_regions(arr);
            test_nested_partitions(arr);
            test_device_data_env(arr);
            break;
    }
    
    /* Verify results (simple checksum) */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                sum += arr[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d (test case: %d)\n", sum, test_case);
    
    return 0;
}
