/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * The test exercises all 8 partition code cases (0-7) in GCC's
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

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
void test_case_0(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                arr[i][j] = i * j;
            }
        }
    }
    /* Use result to prevent elimination */
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
void test_case_1(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker
            for (int j = 0; j < m; ++j) {
                arr[i][j] = i + j;
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
void test_case_2(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker
            for (int j = 0; j < m; ++j) {
                arr[i][j] = i - j;
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
void test_case_3(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker
            for (int j = 0; j < m; ++j) {
                arr[i][j] = i | j;
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
void test_case_4(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; ++j) {
                arr[i][j] = i & j;
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
void test_case_5(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; ++j) {
                arr[i][j] = i ^ j;
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
void test_case_6(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; ++j) {
                arr[i][j] = i * 2 + j;
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
void test_case_7(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; ++j) {
                arr[i][j] = i * 3 - j;
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Test with OpenACC kernels construct */
__attribute__((noinline, used))
void test_kernels_partition(int n, int m) {
    int arr[N][M];
    /* Mix of partition types in kernels */
    #pragma acc kernels create(gang, worker: arr[0:n][0:m]) copyout(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker
            for (int j = 0; j < m; ++j) {
                arr[i][j] = (i + j) * 2;
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Test with OpenMP target offloading */
__attribute__((noinline, used))
void test_omp_target(int n, int m) {
    int arr[N][M];
    #pragma omp target teams distribute parallel for map(tofrom: arr[0:n][0:m])
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            arr[i][j] = i * j * 3;
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            checksum += arr[i][j];
}

/* Test with structured data region */
__attribute__((noinline, used))
void test_structured_data(int n, int m, int p) {
    int arr[N][M][P];
    #pragma acc data copy(arr[0:n][0:m][0:p])
    {
        #pragma acc parallel present(arr[0:n][0:m][0:p])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; ++i) {
                #pragma acc loop worker
                for (int j = 0; j < m; ++j) {
                    #pragma acc loop vector
                    for (int k = 0; k < p; ++k) {
                        arr[i][j][k] = i + j + k;
                    }
                }
            }
        }
        
        /* Second compute construct in same data region */
        #pragma acc parallel present(arr[0:n][0:m][0:p])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; ++i) {
                #pragma acc loop worker
                for (int j = 0; j < m; ++j) {
                    #pragma acc loop vector
                    for (int k = 0; k < p; ++k) {
                        arr[i][j][k] *= 2;
                    }
                }
            }
        }
    }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            for (int k = 0; k < p; ++k)
                checksum += arr[i][j][k];
}

/* Test with unstructured data and runtime library calls */
#ifdef _OPENACC
#include <openacc.h>
__attribute__((noinline, used))
void test_unstructured_data(int n, int m) {
    int *arr = (int*)malloc(n * m * sizeof(int));
    int *dev_arr = (int*)acc_create(arr, n * m * sizeof(int));
    
    #pragma acc parallel present(dev_arr[0:n*m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; ++i) {
            #pragma acc loop worker
            for (int j = 0; j < m; ++j) {
                dev_arr[i * m + j] = i * m + j;
            }
        }
    }
    
    acc_copyout(arr, n * m * sizeof(int));
    for (int i = 0; i < n * m; ++i)
        checksum += arr[i];
    
    free(arr);
}
#endif

/* Conditional execution to force control flow variability */
void run_conditional_tests(int cond, int n, int m) {
    if (cond & 1) {
        test_case_0(n, m);
    }
    if (cond & 2) {
        test_case_1(n, m);
    }
    if (cond & 4) {
        test_case_2(n, m);
    }
    if (cond & 8) {
        test_case_3(n, m);
    }
}

int main(int argc, char *argv[]) {
    /* Use command line args for variability */
    int cond = (argc > 1) ? atoi(argv[1]) : 15;
    int n = (argc > 2) ? atoi(argv[2]) : v_N;
    int m = (argc > 2) ? atoi(argv[2]) : v_M;
    int p = (argc > 2) ? atoi(argv[2]) : v_P;
    
    /* Reset checksum */
    checksum = 0;
    
    /* Execute all 8 partition cases */
    test_case_0(n, m);
    test_case_1(n, m);
    test_case_2(n, m);
    test_case_3(n, m);
    test_case_4(n, m);
    test_case_5(n, m);
    test_case_6(n, m);
    test_case_7(n, m);
    
    /* Additional tests for broader coverage */
    test_kernels_partition(n, m);
    test_omp_target(n, m);
    test_structured_data(n/2, m/2, p/2);
    
    #ifdef _OPENACC
    test_unstructured_data(n/4, m/4);
    #endif
    
    /* Conditional execution */
    run_conditional_tests(cond, n/2, m/2);
    
    /* Print result to ensure execution */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
