/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning scenarios
 * (gang redundant, gang partitioned, worker partitioned, etc.)
 * to trigger coverage of the partition code to string mapping function
 * in omp-oacc-neuter-broadcast.cc lines 335-343.
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

/* Helper to ensure side effects */
static int checksum = 0;

/* Function to update checksum in a way compiler can't optimize away */
static void update_checksum(int value) {
    checksum = (checksum * 31 + value) & 0x7FFFFFFF;
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
static void test_gang_redundant(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1000 + j;
            }
        }
    }
    
    /* Use volatile to force computation */
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
static void test_gang_partitioned(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1001 + j;
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
static void test_worker_partitioned(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1002 + j;
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
static void test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1003 + j;
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
static void test_vector_partitioned(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1004 + j;
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
static void test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1005 + j;
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
static void test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1006 + j;
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 7: fully partitioned (gang+worker+vector) */
__attribute__((noinline, used))
static void test_fully_partitioned(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1007 + j;
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with OpenACC kernels construct for variety */
__attribute__((noinline, used))
static void test_kernels_partition(int n, int m) {
    int arr[N][M];
    
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 2000 + j;
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with OpenACC data region containing multiple compute constructs */
__attribute__((noinline, used))
static void test_data_region(int n, int m, int p) {
    int arr1[N][M];
    int arr2[M][P];
    
    #pragma acc data copy(arr1[0:n][0:m]) create(arr2[0:m][0:p])
    {
        #pragma acc parallel present(arr1[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = i * 3000 + j;
                }
            }
        }
        
        #pragma acc parallel present(arr2[0:m][0:p])
        {
            #pragma acc loop gang
            for (int i = 0; i < m; i++) {
                #pragma acc loop worker
                for (int j = 0; j < p; j++) {
                    arr2[i][j] = i * 4000 + j;
                }
            }
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr1[i][j];
        }
    }
    for (int i = 0; i < m && i < M; i++) {
        for (int j = 0; j < p && j < P; j++) {
            sum += arr2[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with OpenMP target for broader coverage */
__attribute__((noinline, used))
static void test_omp_target(int n, int m) {
    int arr[N][M];
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 5000 + j;
        }
    }
    
    volatile int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with unstructured data directives */
__attribute__((noinline, used))
static void test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * m * sizeof(int));
    
    #pragma acc enter data create(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n*m; i++) {
            arr[i] = i * 6000;
        }
    }
    
    #pragma acc exit data copyout(arr[0:n*m])
    
    volatile int sum = 0;
    for (int i = 0; i < n*m; i++) {
        sum += arr[i];
    }
    
    free(arr);
    update_checksum(sum);
}

/* Main function with conditional execution to create control flow variability */
int main(int argc, char *argv[]) {
    /* Use command line args to create runtime variability */
    int use_all = 1;
    if (argc > 1) {
        use_all = atoi(argv[1]) != 0;
    }
    
    /* Get sizes from volatile variables */
    int n = vN;
    int m = vM;
    int p = vP;
    
    /* Always execute the 8 core partitioning tests */
    test_gang_redundant(n, m);
    test_gang_partitioned(n, m);
    test_worker_partitioned(n, m);
    test_gang_worker_partitioned(n, m);
    test_vector_partitioned(n, m);
    test_gang_vector_partitioned(n, m);
    test_worker_vector_partitioned(n, m);
    test_fully_partitioned(n, m);
    
    /* Conditionally execute additional tests */
    if (use_all) {
        test_kernels_partition(n, m);
        test_data_region(n, m, p);
        test_omp_target(n, m);
        test_unstructured_data(n, m);
    } else {
        /* Alternative path with different partitioning */
        #pragma acc parallel copy(gang: arr[0:10][0:10])
        {
            #pragma acc loop gang
            for (int i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (int j = 0; j < 10; j++) {
                    volatile int dummy = i + j;
                }
            }
        }
    }
    
    /* Print final checksum to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
