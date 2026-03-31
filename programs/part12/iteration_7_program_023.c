/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition code string mapping
 * logic in GCC's OpenACC neuter-broadcast pass (omp-oacc-neuter-broadcast.cc).
 * It uses various OpenACC compute constructs with explicit data partitioning
 * to cover the switch statement at lines 335-343.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Global arrays for persistent device data */
int global_3d[N][M][P];
float global_2d[N*2][M*2];

/* Function prototypes */
void test_gang_redundant(int n, int m, int p);
void test_gang_partitioned(int n, int m, int p);
void test_worker_partitioned(int n, int m, int p);
void test_gang_worker_partitioned(int n, int m, int p);
void test_vector_partitioned(int n, int m, int p);
void test_gang_vector_partitioned(int n, int m, int p);
void test_worker_vector_partitioned(int n, int m, int p);
void test_fully_partitioned(int n, int m, int p);
void test_nested_regions(int n, int m, int p);
void test_multi_dimensional_partitioning(int n, int m, int p);
void test_device_data_env(int n, int m, int p);

/* OpenACC routine with gang partitioning */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int size, int factor) {
    #pragma acc loop gang
    for (int i = 0; i < size; i++) {
        arr[i] += factor;
    }
}

/* OpenACC routine with vector partitioning */
#pragma acc routine seq vector
void acc_routine_vector(int *arr, int size, int factor) {
    #pragma acc loop vector
    for (int i = 0; i < size; i++) {
        arr[i] *= factor;
    }
}

/* Test 1: gang redundant (case 0) */
void test_gang_redundant(int n, int m, int p) {
    int local_3d[n][m][p];
    
    /* Initialize */
    #pragma acc parallel loop collapse(3) gang copy(local_3d)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                local_3d[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Compute with gang redundant partitioning */
    #pragma acc parallel copy(local_3d) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 2: gang partitioned (case 1) */
void test_gang_partitioned(int n, int m, int p) {
    int local_3d[n][m][p];
    
    #pragma acc kernels create(local_3d) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] = i * j * k;
                }
            }
        }
    }
    
    /* Additional region with different gang partitioning */
    #pragma acc parallel loop gang(static:4) copy(local_3d)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                local_3d[i][j][k] += 2;
            }
        }
    }
}

/* Test 3: worker partitioned (case 2) */
void test_worker_partitioned(int n, int m, int p) {
    float local_2d[n][m];
    
    #pragma acc parallel loop collapse(2) worker copy(local_2d)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_2d[i][j] = (float)(i + j) * 0.5f;
        }
    }
    
    /* Worker-only compute region */
    #pragma acc kernels worker copy(local_2d)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                local_2d[i][j] += 1.0f;
            }
        }
    }
}

/* Test 4: gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(int n, int m, int p) {
    int local_3d[n][m][p];
    
    #pragma acc parallel copy(local_3d) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] = i - j + k;
                }
            }
        }
    }
    
    /* Combined gang worker with collapse */
    #pragma acc kernels copy(local_3d) gang worker
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                #pragma acc loop vector
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Test 5: vector partitioned (case 4) */
void test_vector_partitioned(int n, int m, int p) {
    int local_1d[n*m*p];
    
    #pragma acc parallel loop vector copy(local_1d[0:n*m*p])
    for (int i = 0; i < n*m*p; i++) {
        local_1d[i] = i % 100;
    }
    
    /* Vector-only kernels region */
    #pragma acc kernels vector copy(local_1d[0:n*m*p])
    {
        #pragma acc loop vector
        for (int i = 0; i < n*m*p; i++) {
            local_1d[i] += 3;
        }
    }
}

/* Test 6: gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(int n, int m, int p) {
    int local_3d[n][m][p];
    
    #pragma acc parallel copy(local_3d) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] = (i + j) * k;
                }
            }
        }
    }
    
    /* Call routine with gang partitioning */
    #pragma acc parallel copy(local_3d) gang vector
    {
        acc_routine_gang(&local_3d[0][0][0], n*m*p, 5);
    }
}

/* Test 7: worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(int n, int m, int p) {
    float local_2d[n][m];
    
    #pragma acc parallel copy(local_2d) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                local_2d[i][j] = (float)(i * j) / 10.0f;
            }
        }
    }
    
    /* Combined with routine call */
    #pragma acc parallel copy(local_2d) worker vector
    {
        acc_routine_vector(&local_2d[0][0], n*m, 2);
    }
}

/* Test 8: fully partitioned (case 7) */
void test_fully_partitioned(int n, int m, int p) {
    int local_3d[n][m][p];
    
    #pragma acc parallel copy(local_3d) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
    
    /* Fully partitioned with different clause order */
    #pragma acc kernels copy(local_3d) vector worker gang
    {
        #pragma acc loop vector worker gang
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] += 7;
                }
            }
        }
    }
}

/* Test 9: Nested and sequential compute regions */
void test_nested_regions(int n, int m, int p) {
    int local_3d[n][m][p];
    int condition = 1;
    
    /* Conditional offloading path */
    #pragma acc parallel if(condition) gang copy(local_3d)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            /* Nested parallel region inside gang */
            #pragma acc parallel worker vector
            {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < p; k++) {
                        local_3d[i][j][k] = i + j * 2 + k * 3;
                    }
                }
            }
        }
    }
    
    /* Sequential regions with different partitions */
    #pragma acc kernels worker copy(local_3d)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] += 1;
                }
            }
        }
    }
    
    #pragma acc parallel vector copy(local_3d)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    local_3d[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Test 10: Multi-dimensional array with complex partitioning */
void test_multi_dimensional_partitioning(int n, int m, int p) {
    int local_4d[8][n][m][p];
    
    /* Partition across multiple dimensions */
    #pragma acc parallel copy(local_4d) gang worker vector
    {
        #pragma acc loop gang
        for (int d = 0; d < 8; d++) {
            #pragma acc loop worker
            for (int i = 0; i < n; i++) {
                #pragma acc loop vector collapse(2)
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < p; k++) {
                        local_4d[d][i][j][k] = d * 1000 + i * 100 + j * 10 + k;
                    }
                }
            }
        }
    }
    
    /* Different partitioning scheme */
    #pragma acc kernels copy(local_4d) gang(static:2) vector(128)
    {
        #pragma acc loop gang collapse(2)
        for (int d = 0; d < 8; d++) {
            for (int i = 0; i < n; i++) {
                #pragma acc loop vector
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < p; k++) {
                        local_4d[d][i][j][k] += d;
                    }
                }
            }
        }
    }
}

/* Test 11: Device data environment with partitioning */
void test_device_data_env(int n, int m, int p) {
    int persistent_3d[n][m][p];
    
    /* Establish device data region with gang partitioning */
    #pragma acc enter data copyin(persistent_3d) gang
    
    /* Multiple compute regions accessing partitioned data */
    #pragma acc parallel present(persistent_3d) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    persistent_3d[i][j][k] = i * j * k;
                }
            }
        }
    }
    
    #pragma acc parallel present(persistent_3d) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    persistent_3d[i][j][k] += 10;
                }
            }
        }
    }
    
    #pragma acc parallel present(persistent_3d) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    persistent_3d[i][j][k] *= 2;
                }
            }
        }
    }
    
    /* Exit data region */
    #pragma acc exit data copyout(persistent_3d) gang
}

/* Main function with conditional execution paths */
int main(int argc, char **argv) {
    int n = N, m = M, p = P;
    
    /* Initialize global arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                global_3d[i][j][k] = 0;
            }
        }
    }
    
    for (int i = 0; i < N*2; i++) {
        for (int j = 0; j < M*2; j++) {
            global_2d[i][j] = 0.0f;
        }
    }
    
    /* Use argc to create conditional execution paths */
    int test_case = argc > 1 ? atoi(argv[1]) % 12 : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(n, m, p);
            break;
        case 1:
            test_gang_partitioned(n, m, p);
            break;
        case 2:
            test_worker_partitioned(n, m, p);
            break;
        case 3:
            test_gang_worker_partitioned(n, m, p);
            break;
        case 4:
            test_vector_partitioned(n, m, p);
            break;
        case 5:
            test_gang_vector_partitioned(n, m, p);
            break;
        case 6:
            test_worker_vector_partitioned(n, m, p);
            break;
        case 7:
            test_fully_partitioned(n, m, p);
            break;
        case 8:
            test_nested_regions(n, m, p);
            break;
        case 9:
            test_multi_dimensional_partitioning(n, m, p);
            break;
        case 10:
            test_device_data_env(n, m, p);
            break;
        default:
            /* Execute all tests to maximize coverage */
            test_gang_redundant(n, m, p);
            test_gang_partitioned(n, m, p);
            test_worker_partitioned(n, m, p);
            test_gang_worker_partitioned(n, m, p);
            test_vector_partitioned(n, m, p);
            test_gang_vector_partitioned(n, m, p);
            test_worker_vector_partitioned(n, m, p);
            test_fully_partitioned(n, m, p);
            test_nested_regions(n, m, p);
            test_multi_dimensional_partitioning(8, n, m);
            test_device_data_env(n, m, p);
            break;
    }
    
    /* Simple validation to ensure computations aren't optimized away */
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum) copyin(global_3d)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                sum += global_3d[i][j][k];
            }
        }
    }
    
    printf("Validation sum: %d\n", sum);
    
    return 0;
}
