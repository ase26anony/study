/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * The test exercises all 8 partition code cases (0-7) in GCC's
 * omp-oacc-neuter-broadcast.cc by creating OpenACC compute constructs
 * with different data clause partitioning combinations.
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

/* Helper to ensure side effects */
static int checksum = 0;

/* Function to update checksum in a way compiler can't eliminate */
static void update_checksum(int value) {
    checksum = (checksum * 31 + value) & 0xFFFFFF;
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
static int test_gang_redundant(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j;
                if (i == j) result += arr[i][j];
            }
        }
    }
    
    /* Use volatile to prevent optimization */
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
static int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 200 + j;
                if (i + j == n/2) result += arr[i][j];
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
static int test_worker_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 300 + j;
                if (j % 3 == 0) result += arr[i][j];
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
static int test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 400 + j;
                if ((i + j) % 5 == 0) result += arr[i][j];
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
static int test_vector_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 500 + j;
                if (i % 4 == 0) result += arr[i][j];
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
static int test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 600 + j;
                if (j % 6 == 0) result += arr[i][j];
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
static int test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 700 + j;
                if ((i * j) % 7 == 0) result += arr[i][j];
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
static int test_fully_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 800 + j;
                if (i == n/2 || j == m/2) result += arr[i][j];
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Test with OpenACC kernels construct for variety */
__attribute__((noinline, used))
static int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 900 + j;
                result += arr[i][j] % 11;
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Test with OpenMP target for broader coverage */
__attribute__((noinline, used))
static int test_omp_target(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute parallel for collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1000 + j;
                result += (i + j) % 13;
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Test with data region containing multiple constructs */
__attribute__((noinline, used))
static int test_data_region(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc data copy(arr[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1100 + j;
                }
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    result += arr[i][j] % 17;
                }
            }
        }
    }
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Test with unstructured data directives */
__attribute__((noinline, used))
static int test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * m * sizeof(int));
    int result = 0;
    
    #pragma acc enter data create(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n*m; i++) {
            arr[i] = i * 2;
        }
    }
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n*m; i++) {
            result += arr[i] % 19;
        }
    }
    
    #pragma acc exit data copyout(arr[0:n*m])
    
    free(arr);
    
    volatile int use_result = result;
    update_checksum(use_result);
    return use_result;
}

/* Main function with conditional execution to force neutering analysis */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use command line args to create runtime variability */
    int use_all = 1;
    if (argc > 1) {
        use_all = atoi(argv[1]) & 1;
    }
    
    /* Get sizes from volatile variables */
    int n = v_N;
    int m = v_M;
    int p = v_P;
    
    /* Always execute base cases */
    total += test_gang_redundant(n, m);
    total += test_gang_partitioned(n, m);
    total += test_worker_partitioned(n, m);
    total += test_gang_worker_partitioned(n, m);
    
    /* Conditional execution to force compiler to handle control flow */
    if (use_all) {
        total += test_vector_partitioned(n, m);
        total += test_gang_vector_partitioned(n, m);
        total += test_worker_vector_partitioned(n, m);
        total += test_fully_partitioned(n, m);
        
        /* Additional tests for broader coverage */
        total += test_kernels_partition(n, m);
        
        if (argc > 2) {
            total += test_omp_target(n, m);
        }
        
        total += test_data_region(n, m);
        total += test_unstructured_data(n, m);
    } else {
        /* Alternative path with different partition combinations */
        total += test_vector_partitioned(m, p);
        total += test_gang_vector_partitioned(p, n);
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Result: %d (checksum: %d)\n", total & 0xFF, checksum & 0xFF);
    
    return 0;
}
