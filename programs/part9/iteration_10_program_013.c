/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partition codes (0-7)
 * to cover the switch statement in omp-oacc-neuter-broadcast.cc
 * lines 335-343.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Volatile variables to prevent constant propagation */
volatile int v_N = N;
volatile int v_M = M;
volatile int v_P = P;

/* Global checksum accumulator */
static int global_checksum = 0;

/* Function to update checksum in a way that's hard to optimize away */
static void update_checksum(int value) {
    global_checksum = (global_checksum * 31 + value) & 0xFFFFFF;
}

/* Case 0: gang redundant */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Default gang redundancy - no explicit partition clause */
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 2 + 1;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 200 + j * 3;
        }
    }
    
    /* Explicit gang partition */
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] + i - j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 300 + j * 5;
        }
    }
    
    /* Explicit worker partition */
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 3 - 7;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 400 + j * 7;
        }
    }
    
    /* Both gang and worker partition */
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = (arr[i][j] << 1) | 1;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 500 + j * 11;
        }
    }
    
    /* Explicit vector partition */
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] ^ 0x55;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 600 + j * 13;
        }
    }
    
    /* Gang and vector partition */
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] + (i << 4) + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 700 + j * 17;
        }
    }
    
    /* Worker and vector partition */
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] - (i * j);
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 800 + j * 19;
        }
    }
    
    /* Fully partitioned: gang, worker, and vector */
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = (arr[i][j] * 2) / 3;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Test with OpenACC kernels construct for variety */
__attribute__((noinline, used))
int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 900 + j * 23;
        }
    }
    
    /* Use kernels with gang partitioned data */
    #pragma acc kernels copy(gang: arr[0:n][0:m])
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] | 0xAA;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Test with OpenMP offloading for broader coverage */
__attribute__((noinline, used))
int test_omp_offload(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 1000 + j * 29;
        }
    }
    
    /* OpenMP target with distribute and teams */
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute
        for (int i = 0; i < n; i++) {
            #pragma omp parallel for
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] ^ (i + j);
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Test with data region and multiple compute constructs */
__attribute__((noinline, used))
int test_data_region(int n, int m) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 1100 + j * 31;
        }
    }
    
    /* Structured data region */
    #pragma acc data copy(arr[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] += 1;
                }
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] *= 2;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Test with conditional parallel regions to force neutering analysis */
__attribute__((noinline, used))
int test_conditional_regions(int n, int m, int flag) {
    int arr[N][M];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 1200 + j * 37;
        }
    }
    
    /* Conditional parallel region - may be neutered */
    if (flag > 0) {
        #pragma acc parallel copy(arr[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = arr[i][j] - 100;
                }
            }
        }
    } else {
        #pragma acc parallel copy(worker: arr[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = arr[i][j] + 100;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr[i][j];
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

/* Test with 3D array for more complex data sections */
__attribute__((noinline, used))
int test_3d_array(int n, int m, int p) {
    int arr[N][M][P];
    int local_sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                arr[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    /* 3D array with gang,worker,vector partition */
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m][0:p])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                #pragma acc loop vector
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] = arr[i][j][k] / 2;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                local_sum += arr[i][j][k];
            }
        }
    }
    
    update_checksum(local_sum);
    return local_sum & 0xFF;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use volatile bounds to prevent constant propagation */
    int n = v_N;
    int m = v_M;
    int p = v_P;
    
    /* Use command line argument to vary control flow */
    int flag = (argc > 1) ? atoi(argv[1]) : 1;
    
    printf("Testing OpenACC/OpenMP partition codes...\n");
    
    /* Execute all test cases to cover all partition codes */
    result |= test_gang_redundant(n, m);
    result |= test_gang_partitioned(n, m);
    result |= test_worker_partitioned(n, m);
    result |= test_gang_worker_partitioned(n, m);
    result |= test_vector_partitioned(n, m);
    result |= test_gang_vector_partitioned(n, m);
    result |= test_worker_vector_partitioned(n, m);
    result |= test_fully_partitioned(n, m);
    
    /* Additional tests for broader coverage */
    result |= test_kernels_partition(n, m);
    result |= test_omp_offload(n, m);
    result |= test_data_region(n, m);
    result |= test_conditional_regions(n, m, flag);
    result |= test_3d_array(n/2, m/2, p);
    
    /* Final checksum output to ensure all code executed */
    printf("Result: %d (checksum: %d)\n", result & 0xFF, global_checksum);
    
    return 0;
}
