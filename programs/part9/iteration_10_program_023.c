/* test_offload_partition.c
 * This test requires GCC configured with offloading support.
 * Compile with: gcc -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage test_offload_partition.c -o test_offload_partition_executable
 * Run with: ./test_offload_partition_executable
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

volatile int v_N = N;
volatile int v_M = M;
volatile int v_P = P;

/* Function prototypes with attributes to prevent optimization */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m);
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m);
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m);
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m);
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m);
__attribute__((noinline, used))
int test_openmp_offload(int n, int m);
__attribute__((noinline, used))
int test_acc_data_region(int n, int m);
__attribute__((noinline, used))
int test_unstructured_data(int n, int m);

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array with volatile bounds to prevent optimization */
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
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 200 + j;
        }
    }
    
    /* Explicit gang partition */
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 3 - 2;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 300 + j;
        }
    }
    
    /* Explicit worker partition */
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] / 2 + 5;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 400 + j;
        }
    }
    
    /* Gang and worker partition */
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
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
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 500 + j;
        }
    }
    
    /* Vector partition */
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 7 % 256;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 600 + j;
        }
    }
    
    /* Gang and vector partition */
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = (arr[i][j] << 1) | 0x1;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 700 + j;
        }
    }
    
    /* Worker and vector partition */
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] ^ 0xAA;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 800 + j;
        }
    }
    
    /* Fully partitioned: gang, worker, and vector */
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] + arr[i][j] / 2;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test with OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
int test_openmp_offload(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 900 + j;
        }
    }
    
    /* OpenMP target with distribute and teams clauses */
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = arr[i][j] * 2 - 3;
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test with structured data region containing multiple compute constructs */
__attribute__((noinline, used))
int test_acc_data_region(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 1000 + j;
        }
    }
    
    /* Data region with multiple parallel constructs inside */
    #pragma acc data copy(arr[0:n][0:m])
    {
        /* Conditional parallel region to create control flow variability */
        if (n > 0) {
            #pragma acc parallel present(arr)
            {
                #pragma acc loop gang
                for (int i = 0; i < n; i++) {
                    #pragma acc loop worker vector
                    for (int j = 0; j < m; j++) {
                        arr[i][j] = arr[i][j] + 10;
                    }
                }
            }
        }
        
        /* Another parallel region with different partition */
        #pragma acc parallel present(arr)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = arr[i][j] - 5;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test with kernels construct for variety */
__attribute__((noinline, used))
int test_kernels_construct(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 1100 + j;
        }
    }
    
    /* Kernels construct with gang partitioned data */
    #pragma acc kernels copy(gang: arr[0:n][0:m])
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] % 100;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Use volatile to prevent constant propagation */
    int n = v_N;
    int m = v_M;
    
    /* Execute all test functions to cover all partition cases */
    checksum ^= test_gang_redundant(n, m);
    checksum ^= test_gang_partitioned(n, m);
    checksum ^= test_worker_partitioned(n, m);
    checksum ^= test_gang_worker_partitioned(n, m);
    checksum ^= test_vector_partitioned(n, m);
    checksum ^= test_gang_vector_partitioned(n, m);
    checksum ^= test_worker_vector_partitioned(n, m);
    checksum ^= test_fully_partitioned(n, m);
    
    /* Additional tests to trigger more compiler paths */
    if (argc > 1) {
        checksum ^= test_openmp_offload(n, m);
        checksum ^= test_acc_data_region(n, m);
        checksum ^= test_kernels_construct(n, m);
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    return 0;
}
