/* test-omp-oacc-partitions.c
 * 
 * This program is designed to trigger the partition string mapping logic
 * in GCC's OpenACC neuter-broadcast pass (omp-oacc-neuter-broadcast.cc).
 * It exercises various data partition combinations through OpenACC directives
 * to cover the switch statement at lines 335-343.
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
void test_mixed_regions(int arr[N][M][P]);
void test_device_data_env(int arr[N][M][P]);

/* ACC routine with gang partitioning */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i * 2;
    }
}

/* ACC routine with vector partitioning */
#pragma acc routine vec vector
void acc_routine_vector(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i * 3;
    }
}

/* Test 1: gang redundant (case 0) */
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

/* Test 2: gang partitioned (case 1) */
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

/* Test 3: worker partitioned (case 2) */
void test_worker_partitioned(int arr[N][M][P]) {
    int condition = 1;
    #pragma acc parallel if(condition) copy(arr[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] -= 5;
                }
            }
        }
    }
}

/* Test 4: gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += i + j;
                }
            }
        }
    }
}

/* Test 5: vector partitioned (case 4) */
void test_vector_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += k * 2;
                }
            }
        }
    }
}

/* Test 6: gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel loop collapse(2) gang vector copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            #pragma acc loop seq
            for (int k = 0; k < P; k++) {
                arr[i][j][k] += i * j;
            }
        }
    }
}

/* Test 7: worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(int arr[N][M][P]) {
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += j * k;
                }
            }
        }
    }
}

/* Test 8: fully partitioned (case 7) */
void test_fully_partitioned(int arr[N][M][P]) {
    #pragma acc kernels copy(arr[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += i + j + k;
                }
            }
        }
    }
}

/* Test 9: Mixed regions with routine calls */
void test_mixed_regions(int arr[N][M][P]) {
    /* Nested region structure */
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Call gang-partitioned routine */
            acc_routine_gang(&arr[i][0][0], M*P);
            
            #pragma acc kernels worker
            {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    /* Call vector-partitioned routine */
                    acc_routine_vector(&arr[i][j][0], P);
                }
            }
        }
    }
}

/* Test 10: Device data environment with partitions */
void test_device_data_env(int arr[N][M][P]) {
    /* Establish device data with gang partition */
    #pragma acc enter data copyin(arr[0:N][0:M][0:P]) gang
    
    /* Compute region with worker partition */
    #pragma acc parallel present(arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 100;
                }
            }
        }
    }
    
    /* Another region with vector partition */
    #pragma acc kernels present(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] *= 2;
                }
            }
        }
    }
    
    /* Exit data environment */
    #pragma acc exit data copyout(arr[0:N][0:M][0:P])
}

int main(int argc, char **argv) {
    /* Multi-dimensional array */
    int arr[N][M][P];
    
    /* Initialize with test data */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * 10000 + j * 100 + k;
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
        test_mixed_regions(arr);
        test_device_data_env(arr);
    } else {
        /* Minimal execution path */
        test_gang_redundant(arr);
        test_fully_partitioned(arr);
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
    
    printf("Final checksum: %d\n", sum);
    
    return 0;
}
