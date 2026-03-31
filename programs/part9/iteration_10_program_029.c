/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
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

/* Global arrays to work with */
int arr1[N][M];
int arr2[N][M];
int arr3[P][N][M];

/* Function prototypes */
__attribute__((noinline, used)) int test_case_0(int n, int m);
__attribute__((noinline, used)) int test_case_1(int n, int m);
__attribute__((noinline, used)) int test_case_2(int n, int m);
__attribute__((noinline, used)) int test_case_3(int n, int m);
__attribute__((noinline, used)) int test_case_4(int n, int m);
__attribute__((noinline, used)) int test_case_5(int n, int m);
__attribute__((noinline, used)) int test_case_6(int n, int m);
__attribute__((noinline, used)) int test_case_7(int n, int m);
__attribute__((noinline, used)) int test_omp_offload(int n, int m, int p);
__attribute__((noinline, used)) int test_acc_data_region(int n, int m);
__attribute__((noinline, used)) int test_unstructured_data(int n, int m);

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0(int n, int m)
{
    int sum = 0;
    
    /* Use runtime-determined slice sizes */
    #pragma acc parallel copy(arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * j;
                sum += arr1[i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1(int n, int m)
{
    int sum = 0;
    
    #pragma acc parallel copy(gang: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i + j;
                sum += arr1[i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2(int n, int m)
{
    int sum = 0;
    
    #pragma acc parallel copy(worker: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i - j;
                sum += arr1[i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3(int n, int m)
{
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * 2 + j;
                sum += arr1[i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4(int n, int m)
{
    int sum = 0;
    
    #pragma acc parallel copy(vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i + j * 2;
                sum += arr1[i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5(int n, int m)
{
    int sum = 0;
    
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * 3 - j;
                sum += arr1[i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6(int n, int m)
{
    int sum = 0;
    
    #pragma acc parallel copy(worker, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i - j * 3;
                sum += arr1[i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7(int n, int m)
{
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * j * 2;
                sum += arr1[i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
int test_omp_offload(int n, int m, int p)
{
    int sum = 0;
    
    /* Use multi-dimensional array with runtime sizes */
    #pragma omp target map(tofrom: arr3[0:p][0:n][0:m]) map(tofrom: sum)
    #pragma omp teams distribute parallel for collapse(3) reduction(+:sum)
    for (int k = 0; k < p; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr3[k][i][j] = k * i * j;
                sum += arr3[k][i][j];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with structured data region containing multiple compute constructs */
__attribute__((noinline, used))
int test_acc_data_region(int n, int m)
{
    int sum = 0;
    
    #pragma acc data copy(arr2[0:n][0:m])
    {
        /* First compute construct */
        #pragma acc parallel present(arr2[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr2[i][j] = i + j;
                }
            }
        }
        
        /* Second compute construct with different partitioning */
        #pragma acc parallel present(arr2[0:n][0:m])
        {
            #pragma acc loop gang worker vector reduction(+:sum)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr2[i][j] += i * j;
                    sum += arr2[i][j];
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with unstructured data directives */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m)
{
    int sum = 0;
    int *dev_ptr;
    
    /* Allocate device memory */
    dev_ptr = (int *)acc_create(arr1, n * m * sizeof(int));
    
    if (dev_ptr) {
        /* Copy data to device */
        acc_copyin(arr1, n * m * sizeof(int));
        
        /* Compute on device */
        #pragma acc parallel present(arr1[0:n][0:m])
        {
            #pragma acc loop gang worker vector reduction(+:sum)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = (i + j) * 3;
                    sum += arr1[i][j];
                }
            }
        }
        
        /* Copy data back */
        acc_copyout(arr1, n * m * sizeof(int));
        
        /* Free device memory */
        acc_delete(arr1, n * m * sizeof(int));
    }
    
    return sum & 0xFF;
}

int main(int argc, char *argv[])
{
    int checksum = 0;
    volatile int use_offload = 1; /* Force runtime decision */
    
    /* Initialize arrays */
    memset(arr1, 0, sizeof(arr1));
    memset(arr2, 0, sizeof(arr2));
    memset(arr3, 0, sizeof(arr3));
    
    printf("Starting OpenACC/OpenMP partition test...\n");
    
    /* Execute all test cases in conditional blocks to create control flow variability */
    if (use_offload || argc > 1) {
        checksum += test_case_0(v_N, v_M);
    }
    
    checksum += test_case_1(v_N, v_M);
    
    if (use_offload) {
        checksum += test_case_2(v_N, v_M);
    }
    
    checksum += test_case_3(v_N, v_M);
    
    if (use_offload || argc > 1) {
        checksum += test_case_4(v_N, v_M);
    }
    
    checksum += test_case_5(v_N, v_M);
    
    if (use_offload) {
        checksum += test_case_6(v_N, v_M);
    }
    
    checksum += test_case_7(v_N, v_M);
    
    /* Test OpenMP offloading */
    if (use_offload || argc > 1) {
        checksum += test_omp_offload(v_P, v_N, v_M);
    }
    
    /* Test structured data region */
    checksum += test_acc_data_region(v_N, v_M);
    
    /* Test unstructured data */
    if (use_offload) {
        checksum += test_unstructured_data(v_N, v_M);
    }
    
    /* Also test with kernels construct for variety */
    #pragma acc kernels copy(arr1[0:v_N][0:v_M])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < v_N; i++) {
            for (int j = 0; j < v_M; j++) {
                arr1[i][j] = i * j + 1;
            }
        }
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
