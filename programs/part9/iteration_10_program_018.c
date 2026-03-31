/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
 *
 * The test exercises all 8 partition code cases (0-7) in the
 * omp-oacc-neuter-broadcast.cc file by creating OpenACC compute
 * constructs with different data clause partitioning combinations.
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

/* Case 0: gang redundant */
__attribute__((noinline, used))
void test_case_0(int n, int m) {
    int arr[N][M];
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
    /* Use result to prevent elimination */
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][0];
    }
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
void test_case_1(int n, int m) {
    int arr[N][M];
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
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][1];
    }
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
void test_case_2(int n, int m) {
    int arr[N][M];
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
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][2];
    }
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
void test_case_3(int n, int m) {
    int arr[N][M];
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
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][3];
    }
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
void test_case_4(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 500 + j;
            }
        }
    }
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][4];
    }
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
void test_case_5(int n, int m) {
    int arr[N][M];
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 600 + j;
            }
        }
    }
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][5];
    }
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
void test_case_6(int n, int m) {
    int arr[N][M];
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
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][6];
    }
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
void test_case_7(int n, int m) {
    int arr[N][M];
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
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][7];
    }
}

/* Test with kernels construct for variety */
__attribute__((noinline, used))
void test_kernels_partition(int n, int m) {
    int arr[N][M];
    #pragma acc kernels copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 900 + j;
            }
        }
    }
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][0];
    }
}

/* Test with data region containing multiple constructs */
__attribute__((noinline, used))
void test_data_region(int n, int m) {
    int arr[N][M];
    #pragma acc data copy(arr[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop
            for (int i = 0; i < n; i++) {
                #pragma acc loop
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1000 + j;
                }
            }
        }
        
        #pragma acc parallel copy(gang: arr[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    arr[i][j] += 1;
                }
            }
        }
    }
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][1];
    }
}

/* Test with unstructured data using runtime library calls */
__attribute__((noinline, used))
void test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * m * sizeof(int));
    #pragma acc enter data create(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i*m + j] = i * 1100 + j;
            }
        }
    }
    
    #pragma acc exit data copyout(arr[0:n*m])
    
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i*m];
    }
    free(arr);
}

/* OpenMP offloading test to engage broader infrastructure */
__attribute__((noinline, used))
void test_omp_offload(int n, int m) {
    int arr[N][M];
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 1200 + j;
        }
    }
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][2];
    }
}

/* Conditional execution to create control flow variability */
__attribute__((noinline, used))
void test_conditional(int flag, int n, int m) {
    int arr[N][M];
    if (flag) {
        #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1300 + j;
                }
            }
        }
    } else {
        #pragma acc parallel copy(worker: arr[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1400 + j;
                }
            }
        }
    }
    for (int i = 0; i < n && i < 5; i++) {
        checksum += arr[i][3];
    }
}

/* Main function that exercises all cases */
int main(int argc, char **argv) {
    int n = vN > 100 ? 100 : vN;
    int m = vM > 50 ? 50 : vM;
    int p = vP > 25 ? 25 : vP;
    
    /* Use command-line argument for conditional variability */
    int flag = (argc > 1) ? atoi(argv[1]) : 1;
    
    /* Execute all test cases to trigger partition code generation */
    test_case_0(n, m);
    test_case_1(n, m);
    test_case_2(n, m);
    test_case_3(n, m);
    test_case_4(n, m);
    test_case_5(n, m);
    test_case_6(n, m);
    test_case_7(n, m);
    
    test_kernels_partition(n, m);
    test_data_region(n, m);
    test_unstructured_data(n, m);
    test_omp_offload(n, m);
    test_conditional(flag, n, m);
    
    /* Print checksum to ensure all code executed */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
