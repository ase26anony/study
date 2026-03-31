/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in the
 * omp-oacc-neuter-broadcast.cc file by creating OpenACC parallel
 * regions with different data clause partitioning combinations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100
#define DIM 50

/* Volatile variables to prevent constant propagation */
volatile int v_N = SIZE;
volatile int v_M = DIM;

/* Helper to get runtime values */
static int get_size(void) { return v_N; }
static int get_dim(void) { return v_M; }

/* Each test function is noinline and used to prevent optimization */
#define DECLARE_TEST_FUNC(num) \
    __attribute__((noinline, used)) \
    static int test_partition_##num(int *arr, int n, int m)

/* Case 0: gang redundant (default) */
DECLARE_TEST_FUNC(0) {
    int sum = 0;
    #pragma acc parallel copy(arr[0:n*m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                arr[idx] = idx * 2;
                sum += arr[idx];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
DECLARE_TEST_FUNC(1) {
    int sum = 0;
    #pragma acc parallel copy(gang: arr[0:n*m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                arr[idx] = idx * 3;
                sum += arr[idx];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
DECLARE_TEST_FUNC(2) {
    int sum = 0;
    #pragma acc parallel copy(worker: arr[0:n*m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                arr[idx] = idx * 4;
                sum += arr[idx];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
DECLARE_TEST_FUNC(3) {
    int sum = 0;
    #pragma acc parallel copy(gang, worker: arr[0:n*m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                arr[idx] = idx * 5;
                sum += arr[idx];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
DECLARE_TEST_FUNC(4) {
    int sum = 0;
    #pragma acc parallel copy(vector: arr[0:n*m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                arr[idx] = idx * 6;
                sum += arr[idx];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
DECLARE_TEST_FUNC(5) {
    int sum = 0;
    #pragma acc parallel copy(gang, vector: arr[0:n*m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                arr[idx] = idx * 7;
                sum += arr[idx];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
DECLARE_TEST_FUNC(6) {
    int sum = 0;
    #pragma acc parallel copy(worker, vector: arr[0:n*m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                arr[idx] = idx * 8;
                sum += arr[idx];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
DECLARE_TEST_FUNC(7) {
    int sum = 0;
    #pragma acc parallel copy(gang, worker, vector: arr[0:n*m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int idx = i * m + j;
                arr[idx] = idx * 9;
                sum += arr[idx];
            }
        }
    }
    return sum & 0xFF;
}

/* OpenMP offloading variant to engage broader infrastructure */
__attribute__((noinline, used))
static int test_omp_offload(int *arr, int n, int m) {
    int sum = 0;
    #pragma omp target map(tofrom: arr[0:n*m]) map(tofrom: sum)
    #pragma omp teams distribute parallel for collapse(2) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            arr[idx] = idx * 11;
            sum += arr[idx];
        }
    }
    return sum & 0xFF;
}

/* Mixed structured/unstructured data regions */
__attribute__((noinline, used))
static int test_mixed_data_regions(int *arr, int n, int m) {
    int sum = 0;
    
    /* Structured data region */
    #pragma acc data copy(arr[0:n*m])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    int idx = i * m + j;
                    arr[idx] = idx * 13;
                }
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector reduction(+:sum)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    int idx = i * m + j;
                    sum += arr[idx];
                }
            }
        }
    }
    
    /* Unstructured data using runtime API calls */
    void *dev_ptr = acc_create(arr, n * m * sizeof(int));
    if (dev_ptr) {
        #pragma acc parallel present(arr[0:n*m])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    int idx = i * m + j;
                    arr[idx] = arr[idx] * 2;
                }
            }
        }
        acc_delete(arr, n * m * sizeof(int));
    }
    
    return sum & 0xFF;
}

/* Conditional parallel regions based on volatile input */
__attribute__((noinline, used))
static int test_conditional_regions(int *arr, int n, int m, volatile int flag) {
    int sum = 0;
    
    if (flag & 1) {
        #pragma acc parallel copy(arr[0:n*m]) if(flag > 0)
        {
            #pragma acc loop gang worker vector reduction(+:sum)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    int idx = i * m + j;
                    arr[idx] = idx * 17;
                    sum += arr[idx];
                }
            }
        }
    }
    
    if (flag & 2) {
        #pragma acc kernels copy(arr[0:n*m]) if(flag > 1)
        {
            #pragma acc loop independent gang worker vector reduction(+:sum)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    int idx = i * m + j;
                    arr[idx] = arr[idx] * 3;
                    sum += arr[idx];
                }
            }
        }
    }
    
    return sum & 0xFF;
}

int main(int argc, char **argv) {
    int checksum = 0;
    int n = get_size();
    int m = get_dim();
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(n * m * sizeof(int));
    int *arr2 = (int *)malloc(n * m * sizeof(int));
    int *arr3 = (int *)malloc(n * m * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    memset(arr1, 0, n * m * sizeof(int));
    memset(arr2, 0, n * m * sizeof(int));
    memset(arr3, 0, n * m * sizeof(int));
    
    /* Execute all partition cases */
    checksum ^= test_partition_0(arr1, n, m);
    checksum ^= test_partition_1(arr2, n, m);
    checksum ^= test_partition_2(arr1, n, m);
    checksum ^= test_partition_3(arr2, n, m);
    checksum ^= test_partition_4(arr1, n, m);
    checksum ^= test_partition_5(arr2, n, m);
    checksum ^= test_partition_6(arr1, n, m);
    checksum ^= test_partition_7(arr2, n, m);
    
    /* Execute OpenMP offloading test */
    checksum ^= test_omp_offload(arr3, n, m);
    
    /* Execute mixed data regions test */
    checksum ^= test_mixed_data_regions(arr1, n, m);
    
    /* Execute conditional regions test */
    volatile int flag = argc > 1 ? atoi(argv[1]) : 3;
    checksum ^= test_conditional_regions(arr2, n, m, flag);
    
    /* Final verification output */
    printf("Result: %d\n", checksum & 0xFF);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
