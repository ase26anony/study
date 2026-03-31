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
volatile int vN = N;
volatile int vM = M;
volatile int vP = P;

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
    
    /* Verify on host */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 100 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
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
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 200 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
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
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 300 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
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
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 400 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
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
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 500 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
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
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 600 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
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
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 700 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
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
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 800 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
    return result & 0xFF;
}

/* Test with kernels construct for variety */
__attribute__((noinline, used))
int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    /* Mix of partition types in kernels */
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 900 + j;
                result += arr[i][j];
            }
        }
    }
    
    checksum += result;
    return result & 0xFF;
}

/* Test with data region containing multiple constructs */
__attribute__((noinline, used))
int test_data_region(int n, int m, int p) {
    int arr[N][M][P];
    int result = 0;
    
    #pragma acc data copy(arr[0:n][0:m][0:p])
    {
        /* First construct - gang partitioned */
        #pragma acc parallel copy(gang: arr[0:n][0:m][0:p])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < p; k++) {
                        arr[i][j][k] = i * 1000 + j * 100 + k;
                    }
                }
            }
        }
        
        /* Second construct - conditional execution */
        if (n > 10) {
            #pragma acc parallel copy(worker: arr[0:n][0:m][0:p])
            {
                #pragma acc loop gang
                for (int i = 0; i < n; i++) {
                    #pragma acc loop worker
                    for (int j = 0; j < m; j++) {
                        for (int k = 0; k < p; k++) {
                            arr[i][j][k] += 1;
                            result += arr[i][j][k];
                        }
                    }
                }
            }
        }
    }
    
    checksum += result;
    return result & 0xFF;
}

/* Test with unstructured data using runtime library */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * m * sizeof(int));
    int result = 0;
    
    #pragma acc enter data create(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i * m + j] = i * 1100 + j;
            }
        }
    }
    
    #pragma acc update host(arr[0:n*m])
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result += arr[i * m + j];
        }
    }
    
    #pragma acc exit data delete(arr)
    free(arr);
    
    checksum += result;
    return result & 0xFF;
}

/* OpenMP target offloading test */
__attribute__((noinline, used))
int test_omp_offload(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute
        for (int i = 0; i < n; i++) {
            #pragma omp parallel for
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1200 + j;
                result += arr[i][j];
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (arr[i][j] != i * 1200 + j) {
                result = -1;
            }
        }
    }
    
    checksum += result;
    return result & 0xFF;
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent optimization */
    volatile int use_case_0 = 1;
    volatile int use_case_1 = 1;
    volatile int use_case_2 = 1;
    volatile int use_case_3 = 1;
    volatile int use_case_4 = 1;
    volatile int use_case_5 = 1;
    volatile int use_case_6 = 1;
    volatile int use_case_7 = 1;
    
    /* Execute all 8 partition cases */
    if (use_case_0) total += test_case_0(vN, vM);
    if (use_case_1) total += test_case_1(vN, vM);
    if (use_case_2) total += test_case_2(vN, vM);
    if (use_case_3) total += test_case_3(vN, vM);
    if (use_case_4) total += test_case_4(vN, vM);
    if (use_case_5) total += test_case_5(vN, vM);
    if (use_case_6) total += test_case_6(vN, vM);
    if (use_case_7) total += test_case_7(vN, vM);
    
    /* Additional tests for variety */
    total += test_kernels_partition(vN, vM);
    total += test_data_region(vN/2, vM/2, vP);
    total += test_unstructured_data(vN/4, vM/4);
    
    /* OpenMP test - conditionally executed */
    if (argc > 1) {
        total += test_omp_offload(vN, vM);
    }
    
    printf("Result: %d (checksum: %d)\n", total & 0xFF, checksum & 0xFF);
    
    return 0;
}
