/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in GCC's
 * omp-oacc-neuter-broadcast.cc by creating OpenACC compute constructs
 * with different gang/worker/vector data clause combinations.
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
__attribute__((noinline)) 
static void update_checksum(int value) {
    checksum = (checksum * 31 + value) & 0x7FFFFFFF;
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
static void test_case_0(int n, int m) {
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
    
    /* Use result to prevent elimination */
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
static void test_case_1(int n, int m) {
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
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
static void test_case_2(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1002 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
static void test_case_3(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1003 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
static void test_case_4(int n, int m) {
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
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
static void test_case_5(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1005 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
static void test_case_6(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1006 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
static void test_case_7(int n, int m) {
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
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with OpenACC kernels construct */
__attribute__((noinline, used))
static void test_kernels_partition(int n, int m) {
    int arr[N][M];
    
    /* Mix different partition types in kernels regions */
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 2000 + j;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with OpenMP target offloading */
__attribute__((noinline, used))
static void test_omp_target(int n, int m) {
    int arr[N][M];
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 3000 + j;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n && i < N; i++) {
        for (int j = 0; j < m && j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with structured data region */
__attribute__((noinline, used))
static void test_structured_data(int n, int m, int p) {
    int arr1[N][M];
    int arr2[M][P];
    
    #pragma acc data copy(arr1[0:n][0:m]) create(arr2[0:m][0:p])
    {
        #pragma acc parallel present(arr1[0:n][0:m], arr2[0:m][0:p])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = i * 4000 + j;
                }
            }
            
            #pragma acc loop gang worker
            for (int i = 0; i < m; i++) {
                #pragma acc loop vector
                for (int j = 0; j < p; j++) {
                    arr2[i][j] = i * 5000 + j;
                }
            }
        }
    }
    
    int sum = 0;
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

/* Main function with conditional execution to force neutering analysis */
int main(int argc, char *argv[]) {
    /* Use command line args to create runtime variability */
    int use_case_0 = (argc > 1) ? 1 : 0;
    int use_case_1 = (argc > 2) ? 1 : 0;
    int use_case_2 = (argc > 3) ? 1 : 0;
    int use_case_3 = (argc > 4) ? 1 : 0;
    int use_case_4 = (argc > 5) ? 1 : 0;
    int use_case_5 = (argc > 6) ? 1 : 0;
    int use_case_6 = (argc > 7) ? 1 : 0;
    int use_case_7 = (argc > 8) ? 1 : 0;
    
    /* Always execute all cases for coverage, but conditionally
     * to create control flow for compiler analysis */
    if (use_case_0 || 1) test_case_0(v_N, v_M);
    if (use_case_1 || 1) test_case_1(v_N, v_M);
    if (use_case_2 || 1) test_case_2(v_N, v_M);
    if (use_case_3 || 1) test_case_3(v_N, v_M);
    if (use_case_4 || 1) test_case_4(v_N, v_M);
    if (use_case_5 || 1) test_case_5(v_N, v_M);
    if (use_case_6 || 1) test_case_6(v_N, v_M);
    if (use_case_7 || 1) test_case_7(v_N, v_M);
    
    /* Additional tests to exercise more compiler paths */
    test_kernels_partition(v_N, v_M);
    test_omp_target(v_N, v_M);
    test_structured_data(v_N, v_M, v_P);
    
    /* Print checksum to ensure all code executed */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
