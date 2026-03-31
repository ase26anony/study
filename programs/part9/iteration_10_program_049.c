/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning scenarios
 * (gang redundant, gang partitioned, worker partitioned, etc.)
 * to trigger coverage of the partition code to string mapping function
 * in omp-oacc-neuter-broadcast.cc lines 335-343.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100
#define DIM 50

/* Volatile variables to prevent constant propagation */
volatile int v_N = DIM;
volatile int v_M = DIM;

/* Helper to ensure side effects */
static int checksum = 0;

/* Function 0: gang redundant (default) */
__attribute__((noinline, used))
int test_gang_redundant(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma acc parallel copy(arr[0:N][0:M])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            #pragma acc loop gang worker vector
            for (j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function 1: gang partitioned */
__attribute__((noinline, used))
int test_gang_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma acc parallel copy(gang: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 1;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function 2: worker partitioned */
__attribute__((noinline, used))
int test_worker_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma acc parallel copy(worker: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 2;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_gang_worker_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma acc parallel copy(gang, worker: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 3;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function 4: vector partitioned */
__attribute__((noinline, used))
int test_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma acc parallel copy(vector: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 4;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_gang_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma acc parallel copy(gang, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 5;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_worker_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma acc parallel copy(worker, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 6;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function 7: fully partitioned */
__attribute__((noinline, used))
int test_fully_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            #pragma acc loop gang worker vector
            for (j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 7;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function with kernels construct and mixed partitioning */
__attribute__((noinline, used))
int test_kernels_mixed(int N, int M) {
    int arr1[SIZE][SIZE], arr2[SIZE][SIZE];
    int i, j;
    
    #pragma acc data copy(arr1[0:N][0:M]) create(arr2[0:N][0:M])
    {
        #pragma acc kernels copy(gang: arr1[0:N][0:M]) copy(worker: arr2[0:N][0:M])
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                #pragma acc loop worker
                for (j = 0; j < M; j++) {
                    arr1[i][j] = i * j;
                    arr2[i][j] = i + j;
                }
            }
        }
        
        /* Nested conditional region */
        if (N > 10) {
            #pragma acc kernels copy(vector: arr1[0:N/2][0:M/2])
            {
                #pragma acc loop vector
                for (i = 0; i < N/2; i++) {
                    #pragma acc loop vector
                    for (j = 0; j < M/2; j++) {
                        arr1[i][j] *= 2;
                    }
                }
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr1[i][j] + arr2[i][j];
    return sum & 0xFF;
}

/* OpenMP target offloading version */
__attribute__((noinline, used))
int test_omp_target(int N, int M) {
    int arr[SIZE][SIZE];
    int i, j;
    
    #pragma omp target map(tofrom: arr[0:N][0:M])
    #pragma omp teams distribute parallel for collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr[i][j] = i * 200 + j + 100;
        }
    }
    
    int sum = 0;
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Function with unstructured data and runtime API calls */
__attribute__((noinline, used))
int test_unstructured_data(int N, int M) {
    int *arr = (int*)malloc(N * M * sizeof(int));
    int i, j;
    
    /* Initialize on host */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            arr[i*M + j] = i * M + j;
    
    /* Unstructured data region using runtime API */
    #pragma acc enter data copyin(arr[0:N*M])
    
    #pragma acc parallel present(arr[0:N*M])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N*M; i++) {
            arr[i] = arr[i] * 3 + 1;
        }
    }
    
    #pragma acc exit data copyout(arr[0:N*M])
    
    int sum = 0;
    for (i = 0; i < N*M; i++)
        sum += arr[i];
    
    free(arr);
    return sum & 0xFF;
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int N = v_N;
    int M = v_M;
    int result = 0;
    
    /* Always execute all 8 partition cases */
    result ^= test_gang_redundant(N, M);
    result ^= test_gang_partitioned(N, M);
    result ^= test_worker_partitioned(N, M);
    result ^= test_gang_worker_partitioned(N, M);
    result ^= test_vector_partitioned(N, M);
    result ^= test_gang_vector_partitioned(N, M);
    result ^= test_worker_vector_partitioned(N, M);
    result ^= test_fully_partitioned(N, M);
    
    /* Conditional execution based on volatile to force neutering analysis */
    if (N > 0) {
        result ^= test_kernels_mixed(N, M);
    }
    
    if (M > 0) {
        result ^= test_omp_target(N/2, M/2);
    }
    
    if (argc > 1) {
        result ^= test_unstructured_data(N, M);
    }
    
    printf("Result: %d\n", result & 0xFF);
    return 0;
}
