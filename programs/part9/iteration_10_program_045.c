/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in the
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
volatile int v_N = N;
volatile int v_M = M;
volatile int v_P = P;

/* Helper to generate side effects and prevent dead code elimination */
static int __attribute__((noinline)) update_checksum(int *arr, int size, int init) {
    int sum = init;
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2 + init;
        sum += arr[i];
    }
    return sum & 0xFF; /* Return small checksum */
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma acc parallel copy(arr[0:n])
    {
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            arr[i] = i * 3;
        }
    }
    
    checksum = update_checksum(arr, n, 0);
    return checksum;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma acc parallel copy(gang: arr[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            arr[i] = i * 5;
        }
    }
    
    checksum = update_checksum(arr, n, 1);
    return checksum;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma acc parallel copy(worker: arr[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            arr[i] = i * 7;
        }
    }
    
    checksum = update_checksum(arr, n, 2);
    return checksum;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma acc parallel copy(gang, worker: arr[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            arr[i] = i * 11;
        }
    }
    
    checksum = update_checksum(arr, n, 3);
    return checksum;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma acc parallel copy(vector: arr[0:n])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            arr[i] = i * 13;
        }
    }
    
    checksum = update_checksum(arr, n, 4);
    return checksum;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma acc parallel copy(gang, vector: arr[0:n])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            arr[i] = i * 17;
        }
    }
    
    checksum = update_checksum(arr, n, 5);
    return checksum;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma acc parallel copy(worker, vector: arr[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            arr[i] = i * 19;
        }
    }
    
    checksum = update_checksum(arr, n, 6);
    return checksum;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            arr[i] = i * 23;
        }
    }
    
    checksum = update_checksum(arr, n, 7);
    return checksum;
}

/* Test with 2D arrays and non-constant sections to force data clause processing */
__attribute__((noinline, used))
int test_2d_partitions(int n, int m, int use_kernels) {
    int arr[N][M];
    int checksum = 0;
    
    /* Use both parallel and kernels constructs */
    if (use_kernels) {
        #pragma acc kernels copy(arr[0:n][0:m])
        {
            #pragma acc loop independent gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop independent vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * m + j;
                }
            }
        }
    } else {
        #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * m + j + 1;
                }
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr[i][j];
        }
    }
    return checksum & 0xFF;
}

/* Test with data regions and multiple compute constructs */
__attribute__((noinline, used))
int test_data_region(int n, int m) {
    int arr1[N], arr2[N];
    int checksum = 0;
    
    #pragma acc data copy(arr1[0:n]) create(arr2[0:n])
    {
        #pragma acc parallel present(arr1[0:n])
        {
            #pragma acc loop
            for (int i = 0; i < n; i++) {
                arr1[i] = i * 2;
            }
        }
        
        #pragma acc parallel present(arr1[0:n], arr2[0:n])
        {
            #pragma acc loop
            for (int i = 0; i < n; i++) {
                arr2[i] = arr1[i] * 3;
            }
        }
        
        #pragma acc update host(arr2[0:n])
    }
    
    checksum = update_checksum(arr2, n, 100);
    return checksum;
}

/* Test with OpenMP target offloading for broader infrastructure coverage */
__attribute__((noinline, used))
int test_omp_offload(int n, int m) {
    int arr[N];
    int checksum = 0;
    
    #pragma omp target map(tofrom: arr[0:n])
    {
        #pragma omp teams distribute parallel for
        for (int i = 0; i < n; i++) {
            arr[i] = i * 31;
        }
    }
    
    checksum = update_checksum(arr, n, 200);
    return checksum;
}

/* Test with unstructured data directives */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * sizeof(int));
    int checksum = 0;
    
    #pragma acc enter data copyin(arr[0:n])
    
    #pragma acc parallel present(arr[0:n])
    {
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            arr[i] = i * 37;
        }
    }
    
    #pragma acc update host(arr[0:n])
    #pragma acc exit data delete(arr[0:n])
    
    checksum = update_checksum(arr, n, 300);
    free(arr);
    return checksum;
}

/* Main function with conditional execution to force control flow variability */
int main(int argc, char *argv[]) {
    int checksum = 0;
    int n = v_N;  /* Use volatile to prevent constant propagation */
    int m = v_M;
    int p = v_P;
    
    /* Use command-line argument to create conditional paths */
    int use_all = 1;
    if (argc > 1) {
        use_all = atoi(argv[1]) & 1;
    }
    
    /* Execute all 8 partition cases */
    checksum += test_case_0(n, m);
    checksum += test_case_1(n, m);
    checksum += test_case_2(n, m);
    checksum += test_case_3(n, m);
    checksum += test_case_4(n, m);
    checksum += test_case_5(n, m);
    checksum += test_case_6(n, m);
    checksum += test_case_7(n, m);
    
    /* Conditional execution to force compiler to handle control flow */
    if (use_all) {
        checksum += test_2d_partitions(n, m, 0);  /* Use parallel */
        checksum += test_2d_partitions(n, m, 1);  /* Use kernels */
    } else {
        checksum += test_2d_partitions(m, p, 0);  /* Different dimensions */
    }
    
    /* Always execute these to ensure coverage */
    checksum += test_data_region(n, m);
    checksum += test_omp_offload(n, m);
    checksum += test_unstructured_data(n, m);
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
