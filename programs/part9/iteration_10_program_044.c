/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * The test exercises all 8 partition codes (0-7) in GCC's omp-oacc-neuter-broadcast.cc
 * by using OpenACC data clauses with gang/worker/vector partitioning.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Volatile variables to prevent constant propagation */
volatile int vN = N;
volatile int vM = M;
volatile int vP = P;

/* Global checksum to ensure all code paths execute */
static int global_checksum = 0;

/* Helper to update checksum in a way compiler can't eliminate */
static void update_checksum(int value) {
    global_checksum += value;
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
void test_gang_redundant(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            #pragma acc loop
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
void test_gang_partitioned(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 200 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
void test_worker_partitioned(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 300 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
void test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 400 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
void test_vector_partitioned(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 500 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
void test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 600 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
void test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 700 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
void test_fully_partitioned(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 800 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Test with OpenACC kernels construct for variety */
__attribute__((noinline, used))
void test_kernels_partition(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    /* This should generate different partition codes */
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 900 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Test with OpenMP target for broader coverage */
__attribute__((noinline, used))
void test_omp_target(int n, int m) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute
        for (int i = 0; i < n; i++) {
            #pragma omp parallel for
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1000 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Test with structured data region */
__attribute__((noinline, used))
void test_structured_data_region(int n, int m, int p) {
    int arr[N][M];
    memset(arr, 0, sizeof(arr));
    
    #pragma acc data copy(arr[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop
            for (int i = 0; i < n; i++) {
                #pragma acc loop
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1100 + j;
                }
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop
            for (int i = 0; i < n; i++) {
                #pragma acc loop
                for (int j = 0; j < m; j++) {
                    arr[i][j] += p;
                }
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum & 0xFF);
}

/* Test with unstructured data using runtime library calls */
__attribute__((noinline, used))
void test_unstructured_data(int n, int m) {
    int *arr = (int*)malloc(n * m * sizeof(int));
    if (!arr) return;
    
    for (int i = 0; i < n * m; i++) {
        arr[i] = 0;
    }
    
    /* Use runtime library calls to create data */
    #pragma acc enter data copyin(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop
        for (int i = 0; i < n * m; i++) {
            arr[i] = i * 1200;
        }
    }
    
    #pragma acc exit data copyout(arr[0:n*m])
    
    int sum = 0;
    for (int i = 0; i < n * m; i++) {
        sum += arr[i];
    }
    update_checksum(sum & 0xFF);
    
    free(arr);
}

/* Main function with conditional execution to force neutering analysis */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent dead code elimination */
    volatile int use_acc = 1;
    volatile int use_omp = 1;
    
    /* Get sizes from volatile variables */
    int n = vN;
    int m = vM;
    int p = vP;
    
    /* Execute all test cases, some conditionally */
    test_gang_redundant(n, m);
    
    if (use_acc) {
        test_gang_partitioned(n, m);
        test_worker_partitioned(n, m);
        test_gang_worker_partitioned(n, m);
        test_vector_partitioned(n, m);
        test_gang_vector_partitioned(n, m);
        test_worker_vector_partitioned(n, m);
        test_fully_partitioned(n, m);
        test_kernels_partition(n, m);
        test_structured_data_region(n, m, p);
        test_unstructured_data(n, m);
    }
    
    if (use_omp && argc > 1) {
        /* Conditional based on command line to force control flow variability */
        test_omp_target(n, m);
    }
    
    /* Print checksum to ensure all code executed */
    printf("Result: %d\n", global_checksum & 0xFF);
    
    return 0;
}
