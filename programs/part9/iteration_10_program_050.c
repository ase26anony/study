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

/* Helper to generate runtime-dependent indices */
static int get_size(int base, int offset) {
    return base + (offset % 3);
}

/* Case 0: gang redundant */
__attribute__((noinline, used))
int test_case_0(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 100 + j;
    
    /* Gang redundant (default) */
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] += 1;
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 200 + j;
    
    /* Gang partitioned */
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] += 2;
            }
        }
    }
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 300 + j;
    
    /* Worker partitioned */
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] += 3;
            }
        }
    }
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 400 + j;
    
    /* Gang+worker partitioned */
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] += 4;
            }
        }
    }
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 500 + j;
    
    /* Vector partitioned */
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] += 5;
            }
        }
    }
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 600 + j;
    
    /* Gang+vector partitioned */
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] += 6;
            }
        }
    }
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 700 + j;
    
    /* Worker+vector partitioned */
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] += 7;
            }
        }
    }
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 800 + j;
    
    /* Fully partitioned */
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] += 8;
            }
        }
    }
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Mixed OpenACC kernels with different partitionings */
__attribute__((noinline, used))
int test_kernels_mixed(int n, int m, int p) {
    int arr1[N][M];
    int arr2[N][M];
    int arr3[N][M];
    int i, j, sum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            arr1[i][j] = i * 1000 + j;
            arr2[i][j] = i * 2000 + j;
            arr3[i][j] = i * 3000 + j;
        }
    }
    
    /* Data region with multiple compute constructs */
    #pragma acc data copy(arr1[0:n][0:m]) create(arr2[0:n][0:m]) present(arr3[0:n][0:m])
    {
        /* Kernels with gang partitioning */
        #pragma acc kernels copy(gang: arr1[0:n][0:m])
        {
            for (i = 0; i < n; i++)
                for (j = 0; j < m; j++)
                    arr1[i][j] += 1;
        }
        
        /* Conditional parallel region */
        if (n > 10) {
            #pragma acc parallel copy(worker: arr2[0:n][0:m])
            {
                #pragma acc loop gang
                for (i = 0; i < n; i++) {
                    #pragma acc loop worker vector
                    for (j = 0; j < m; j++) {
                        arr2[i][j] += 2;
                    }
                }
            }
        }
        
        /* Another with vector partitioning */
        #pragma acc parallel copy(vector: arr3[0:n][0:m])
        {
            #pragma acc loop gang
            for (i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < m; j++) {
                    arr3[i][j] += 3;
                }
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j] + arr2[i][j] + arr3[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* OpenMP target offloading variant */
__attribute__((noinline, used))
int test_omp_target(int n, int m) {
    int arr[N][M];
    int i, j, sum = 0;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 9000 + j;
    
    /* OpenMP target with distribute and teams */
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            arr[i][j] += 9;
        }
    }
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    return sum & 0xFF;
}

/* Unstructured data lifetime with runtime library calls */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr1, *arr2;
    int i, sum = 0;
    size_t size = n * m * sizeof(int);
    
    arr1 = (int *)malloc(size);
    arr2 = (int *)malloc(size);
    
    if (!arr1 || !arr2) return 0;
    
    /* Initialize */
    for (i = 0; i < n * m; i++) {
        arr1[i] = i * 10;
        arr2[i] = i * 20;
    }
    
    /* Use OpenACC runtime calls for unstructured data */
    #pragma acc enter data copyin(arr1[0:n*m]) create(arr2[0:n*m])
    
    #pragma acc parallel present(arr1[0:n*m], arr2[0:n*m])
    {
        #pragma acc loop gang vector
        for (i = 0; i < n * m; i++) {
            arr2[i] = arr1[i] * 2;
        }
    }
    
    #pragma acc exit data copyout(arr2[0:n*m]) delete(arr1[0:n*m])
    
    /* Compute checksum */
    for (i = 0; i < n * m; i++) {
        sum += arr2[i];
    }
    
    free(arr1);
    free(arr2);
    
    return sum & 0xFF;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    int n, m, p;
    
    /* Use volatile to prevent constant folding */
    n = get_size(v_N, argc);
    m = get_size(v_M, argc);
    p = get_size(v_P, argc);
    
    printf("Testing with n=%d, m=%d, p=%d\n", n, m, p);
    
    /* Execute all test cases to cover all partition codes */
    checksum ^= test_case_0(n, m);
    checksum ^= test_case_1(n, m);
    checksum ^= test_case_2(n, m);
    checksum ^= test_case_3(n, m);
    checksum ^= test_case_4(n, m);
    checksum ^= test_case_5(n, m);
    checksum ^= test_case_6(n, m);
    checksum ^= test_case_7(n, m);
    
    /* Additional tests to ensure neutering/broadcast paths */
    checksum ^= test_kernels_mixed(n, m, p);
    checksum ^= test_omp_target(n, m);
    checksum ^= test_unstructured_data(n, m);
    
    /* Conditional execution to create control flow variability */
    if (argc > 1) {
        /* Extra test with different partitionings */
        int extra_arr[P][M];
        int i, j;
        
        for (i = 0; i < p; i++)
            for (j = 0; j < m; j++)
                extra_arr[i][j] = i * 10000 + j;
        
        /* Test gang partitioned with create clause */
        #pragma acc parallel create(gang: extra_arr[0:p][0:m])
        {
            #pragma acc loop gang
            for (i = 0; i < p; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < m; j++) {
                    extra_arr[i][j] += 100;
                }
            }
        }
        
        for (i = 0; i < p; i++)
            for (j = 0; j < m; j++)
                checksum += extra_arr[i][j];
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
