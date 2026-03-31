/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
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

/* Global checksum to ensure all code paths execute */
static int global_checksum = 0;

/* Helper to update checksum in a way compiler can't eliminate */
static void update_checksum(int value) {
    global_checksum = (global_checksum * 31 + value) & 0x7FFFFFFF;
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 100 + j;
    
    /* Default gang redundancy - case 0 */
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 2 + 1;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 200 + j;
    
    /* Explicit gang partitioning - case 1 */
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 3 - 1;
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 300 + j;
    
    /* Worker partitioning - case 2 */
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] + i - j;
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 400 + j;
    
    /* Gang+worker partitioning - case 3 */
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 2 / (i + j + 1);
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 500 + j;
    
    /* Vector partitioning - case 4 */
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] ^ (i * j);
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 600 + j;
    
    /* Gang+vector partitioning - case 5 */
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] | (i << 8) | j;
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 700 + j;
    
    /* Worker+vector partitioning - case 6 */
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = (arr[i][j] + i) * (j + 1);
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 800 + j;
    
    /* Fully partitioned - case 7 */
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] % 997;
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Test with OpenACC kernels construct for variety */
__attribute__((noinline, used))
int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 900 + j;
    
    /* Kernels with gang partitioned data clause */
    #pragma acc kernels copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] + 1;
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Test with OpenMP target for broader coverage */
__attribute__((noinline, used))
int test_omp_target(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 1000 + j;
    
    /* OpenMP target with distribute and teams */
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute
    for (int i = 0; i < n; i++) {
        #pragma omp parallel for
        for (int j = 0; j < m; j++) {
            arr[i][j] = arr[i][j] * 2;
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Test with data region containing multiple constructs */
__attribute__((noinline, used))
int test_data_region(int n, int m, int flag) {
    int arr[N][M];
    int result = 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = i * 1100 + j;
    
    #pragma acc data copy(arr[0:n][0:m])
    {
        /* First parallel region inside data region */
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] += 10;
                }
            }
        }
        
        /* Conditional second region to create control flow */
        if (flag) {
            #pragma acc parallel copy(arr[0:n][0:m])
            {
                #pragma acc loop gang
                for (int i = 0; i < n; i++) {
                    #pragma acc loop worker vector
                    for (int j = 0; j < m; j++) {
                        arr[i][j] *= 2;
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i][j];
    
    update_checksum(result);
    return result & 0xFF;
}

/* Test with unstructured data using runtime library calls */
#ifdef _OPENACC
#include <openacc.h>
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * m * sizeof(int));
    int result = 0;
    
    if (!arr) return 0;
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            arr[i * m + j] = i * 1200 + j;
    
    /* Unstructured data management */
    void *dev_ptr = acc_create(arr, n * m * sizeof(int));
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i * m + j] = arr[i * m + j] + 100;
            }
        }
    }
    
    acc_copyout(arr, n * m * sizeof(int));
    
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            result += arr[i * m + j];
    
    free(arr);
    update_checksum(result);
    return result & 0xFF;
}
#endif

int main(int argc, char *argv[]) {
    int checksum = 0;
    int n = v_N;  /* Use volatile to prevent constant propagation */
    int m = v_M;
    int flag = (argc > 1) ? atoi(argv[1]) : 1;
    
    printf("Testing OpenACC/OpenMP partition codes...\n");
    
    /* Execute all test cases to cover all partition codes */
    checksum ^= test_case_0(n, m);
    checksum ^= test_case_1(n, m);
    checksum ^= test_case_2(n, m);
    checksum ^= test_case_3(n, m);
    checksum ^= test_case_4(n, m);
    checksum ^= test_case_5(n, m);
    checksum ^= test_case_6(n, m);
    checksum ^= test_case_7(n, m);
    
    /* Additional tests for broader coverage */
    checksum ^= test_kernels_partition(n, m);
    checksum ^= test_omp_target(n, m);
    checksum ^= test_data_region(n, m, flag);
    
    #ifdef _OPENACC
    checksum ^= test_unstructured_data(n, m);
    #endif
    
    /* Mix in some conditional execution based on volatile */
    if (v_P > 0) {
        checksum ^= test_case_0(n/2, m/2);
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global checksum: %d\n", global_checksum & 0xFF);
    
    return (checksum == 0) ? 0 : 1;
}
