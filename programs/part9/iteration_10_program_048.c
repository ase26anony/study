/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in GCC's
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

/* Global arrays to work with */
int arr_1d[N];
int arr_2d[N][M];
int arr_3d[N][M][P];

/* Function prototypes */
__attribute__((noinline, used)) int test_case_0(int n, int m);
__attribute__((noinline, used)) int test_case_1(int n, int m);
__attribute__((noinline, used)) int test_case_2(int n, int m);
__attribute__((noinline, used)) int test_case_3(int n, int m);
__attribute__((noinline, used)) int test_case_4(int n, int m);
__attribute__((noinline, used)) int test_case_5(int n, int m);
__attribute__((noinline, used)) int test_case_6(int n, int m);
__attribute__((noinline, used)) int test_case_7(int n, int m);
__attribute__((noinline, used)) int test_omp_offload(int n, int m);
__attribute__((noinline, used)) int test_unstructured_data(int n, int m);
__attribute__((noinline, used)) int test_conditional_regions(int n, int m, int flag);

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0(int n, int m) {
    int sum = 0;
    
    #pragma acc parallel copy(arr_2d[0:n][0:m]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr_2d[i][j] = i * 100 + j;
                sum += arr_2d[i][j] % 256;
            }
        }
    }
    
    return sum % 256;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1(int n, int m) {
    int sum = 0;
    
    #pragma acc parallel copy(gang: arr_2d[0:n][0:m]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr_2d[i][j] = i * 200 + j;
                sum += arr_2d[i][j] % 256;
            }
        }
    }
    
    return sum % 256;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2(int n, int m) {
    int sum = 0;
    
    #pragma acc parallel copy(worker: arr_2d[0:n][0:m]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr_2d[i][j] = i * 300 + j;
                sum += arr_2d[i][j] % 256;
            }
        }
    }
    
    return sum % 256;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3(int n, int m) {
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker: arr_2d[0:n][0:m]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr_2d[i][j] = i * 400 + j;
                sum += arr_2d[i][j] % 256;
            }
        }
    }
    
    return sum % 256;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4(int n, int m) {
    int sum = 0;
    
    #pragma acc parallel copy(vector: arr_2d[0:n][0:m]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr_2d[i][j] = i * 500 + j;
                sum += arr_2d[i][j] % 256;
            }
        }
    }
    
    return sum % 256;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5(int n, int m) {
    int sum = 0;
    
    #pragma acc parallel copy(gang, vector: arr_2d[0:n][0:m]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr_2d[i][j] = i * 600 + j;
                sum += arr_2d[i][j] % 256;
            }
        }
    }
    
    return sum % 256;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6(int n, int m) {
    int sum = 0;
    
    #pragma acc parallel copy(worker, vector: arr_2d[0:n][0:m]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr_2d[i][j] = i * 700 + j;
                sum += arr_2d[i][j] % 256;
            }
        }
    }
    
    return sum % 256;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7(int n, int m) {
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr_2d[0:n][0:m]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr_2d[i][j] = i * 800 + j;
                sum += arr_2d[i][j] % 256;
            }
        }
    }
    
    return sum % 256;
}

/* Test with OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
int test_omp_offload(int n, int m) {
    int sum = 0;
    
    #pragma omp target map(tofrom: arr_2d[0:n][0:m], sum)
    #pragma omp teams distribute parallel for collapse(2) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr_2d[i][j] = i * 900 + j;
            sum += arr_2d[i][j] % 256;
        }
    }
    
    return sum % 256;
}

/* Test with unstructured data regions using runtime library calls */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int sum = 0;
    int *dev_arr = NULL;
    
    /* Create device data */
    dev_arr = (int *)acc_create(arr_1d, n * sizeof(int));
    
    #pragma acc parallel present(dev_arr[0:n]) copy(sum)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dev_arr[i] = i * 1000;
            sum += dev_arr[i] % 256;
        }
    }
    
    /* Copy data back */
    acc_copyout(arr_1d, n * sizeof(int));
    
    return sum % 256;
}

/* Test conditional regions to force control flow variability */
__attribute__((noinline, used))
int test_conditional_regions(int n, int m, int flag) {
    int sum = 0;
    
    if (flag & 1) {
        #pragma acc kernels copy(arr_2d[0:n][0:m]) copy(sum)
        {
            #pragma acc loop gang worker vector reduction(+:sum)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr_2d[i][j] = i * 1100 + j;
                    sum += arr_2d[i][j] % 256;
                }
            }
        }
    }
    
    if (flag & 2) {
        #pragma acc data copy(arr_3d[0:n][0:m][0:8]) copy(sum)
        {
            #pragma acc parallel
            {
                #pragma acc loop gang worker vector reduction(+:sum)
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        for (int k = 0; k < 8; k++) {
                            arr_3d[i][j][k] = i * 10000 + j * 100 + k;
                            sum += arr_3d[i][j][k] % 256;
                        }
                    }
                }
            }
            
            #pragma acc parallel
            {
                #pragma acc loop gang worker vector reduction(+:sum)
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        for (int k = 0; k < 8; k++) {
                            sum += arr_3d[i][j][k] % 16;
                        }
                    }
                }
            }
        }
    }
    
    return sum % 256;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    int n = v_N;  /* Use volatile to prevent constant propagation */
    int m = v_M;
    int flag = 0;
    
    /* Use command line argument to create runtime variability */
    if (argc > 1) {
        flag = atoi(argv[1]) % 4;
    }
    
    /* Initialize arrays */
    memset(arr_1d, 0, sizeof(arr_1d));
    memset(arr_2d, 0, sizeof(arr_2d));
    memset(arr_3d, 0, sizeof(arr_3d));
    
    /* Execute all test cases to cover all partition codes */
    checksum += test_case_0(n, m);
    checksum += test_case_1(n, m);
    checksum += test_case_2(n, m);
    checksum += test_case_3(n, m);
    checksum += test_case_4(n, m);
    checksum += test_case_5(n, m);
    checksum += test_case_6(n, m);
    checksum += test_case_7(n, m);
    
    /* Additional tests to force neutering/broadcast paths */
    checksum += test_omp_offload(n, m);
    checksum += test_unstructured_data(n, m);
    checksum += test_conditional_regions(n, m, flag);
    
    /* Final checksum output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
