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

/* Helper to ensure side effects */
static int checksum = 0;

/* Case 0: gang redundant */
__attribute__((noinline, used))
int test_case_0(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    /* Default gang redundancy */
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 200 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 300 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 400 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 500 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 600 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 700 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 800 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Mixed OpenACC kernels construct with different partitioning */
__attribute__((noinline, used))
int test_kernels_mixed(int n, int m, int p) {
    int arr3d[N][M][P];
    int result = 0;
    
    /* Data region with multiple compute constructs */
    #pragma acc data copy(arr3d[0:n][0:m][0:p])
    {
        /* Conditional compute region */
        if (n > 10) {
            #pragma acc kernels copy(gang: arr3d[0:n/2][0:m][0:p])
            {
                #pragma acc loop gang
                for (int i = 0; i < n/2; i++) {
                    #pragma acc loop worker
                    for (int j = 0; j < m; j++) {
                        #pragma acc loop vector
                        for (int k = 0; k < p; k++) {
                            arr3d[i][j][k] = i * 1000 + j * 100 + k;
                            result += arr3d[i][j][k];
                        }
                    }
                }
            }
        }
        
        #pragma acc kernels copy(worker: arr3d[n/2:n-n/2][0:m][0:p])
        {
            #pragma acc loop gang
            for (int i = n/2; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < p; k++) {
                        arr3d[i][j][k] = i * 2000 + j * 100 + k;
                        result += arr3d[i][j][k];
                    }
                }
            }
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* OpenMP target offloading variant */
__attribute__((noinline, used))
int test_omp_target(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 900 + j;
            result += arr[i][j];
        }
    }
    
    checksum += result & 0xFF;
    return result & 0xFF;
}

/* Unstructured data lifetime with runtime library calls */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr = (int*)malloc(n * m * sizeof(int));
    int result = 0;
    
    /* Create device data */
    #pragma acc enter data create(arr[0:n*m])
    
    /* Compute with present clause */
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n*m; i++) {
            arr[i] = i * 2;
            result += arr[i];
        }
    }
    
    /* Copy data back and exit */
    #pragma acc update host(arr[0:n*m])
    #pragma acc exit data delete(arr)
    
    /* Verify on host */
    for (int i = 0; i < n*m; i++) {
        result -= arr[i];
    }
    
    free(arr);
    checksum += result & 0xFF;
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int final_result = 0;
    
    /* Use command line args for variability */
    int use_all = 1;
    if (argc > 1) {
        use_all = atoi(argv[1]) & 1;
    }
    
    /* Execute all test cases to cover all partition codes */
    final_result += test_case_0(v_N, v_M);
    final_result += test_case_1(v_N, v_M);
    final_result += test_case_2(v_N, v_M);
    final_result += test_case_3(v_N, v_M);
    final_result += test_case_4(v_N, v_M);
    final_result += test_case_5(v_N, v_M);
    final_result += test_case_6(v_N, v_M);
    final_result += test_case_7(v_N, v_M);
    
    /* Conditional execution to force control flow analysis */
    if (use_all) {
        final_result += test_kernels_mixed(v_N, v_M, v_P);
        final_result += test_omp_target(v_N/2, v_M/2);
        final_result += test_unstructured_data(v_N/4, v_M/4);
    }
    
    /* Ensure all code paths contribute to output */
    printf("Result: %d (checksum: %d)\n", final_result & 0xFF, checksum & 0xFF);
    
    return (final_result + checksum) & 0xFF;
}
