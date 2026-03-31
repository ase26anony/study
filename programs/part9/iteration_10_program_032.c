/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning cases
 * to trigger coverage of the partition code to string mapping
 * function in omp-oacc-neuter-broadcast.cc.
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

/* Global arrays for data sharing between functions */
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes */
int test_gang_redundant(int n, int m);
int test_gang_partitioned(int n, int m);
int test_worker_partitioned(int n, int m);
int test_gang_worker_partitioned(int n, int m);
int test_vector_partitioned(int n, int m);
int test_gang_vector_partitioned(int n, int m);
int test_worker_vector_partitioned(int n, int m);
int test_fully_partitioned(int n, int m);
int test_openmp_offload(int n, int m, int p);
int test_unstructured_data(int n, int m);
int test_mixed_constructs(int n, int m, int p);

/* Case 0: gang redundant */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    /* Default gang redundancy - no explicit partition clause */
    #pragma acc parallel copy(arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 100 + j;
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 200 + j * 2;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    /* Worker partitioned */
    #pragma acc parallel copy(worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr2[i][j] = i * 300 + j * 3;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    /* Gang and worker partitioned */
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 400 + j * 4;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    /* Vector partitioned */
    #pragma acc parallel copy(vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr2[i][j] = i * 500 + j * 5;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    /* Gang and vector partitioned */
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 600 + j * 6;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    /* Worker and vector partitioned */
    #pragma acc parallel copy(worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr2[i][j] = i * 700 + j * 7;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    /* Fully partitioned (gang, worker, vector) */
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 800 + j * 8;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Test OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
int test_openmp_offload(int n, int m, int p)
{
    int i, j, k;
    int checksum = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int use_offload = 1;
    
    if (use_offload) {
        /* OpenMP target with distribute, teams, and parallel clauses */
        #pragma omp target map(tofrom: arr3[0:n][0:m][0:p])
        {
            #pragma omp teams distribute
            for (i = 0; i < n; i++) {
                #pragma omp parallel for
                for (j = 0; j < m; j++) {
                    for (k = 0; k < p; k++) {
                        arr3[i][j][k] = i * 1000 + j * 100 + k;
                    }
                }
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            for (k = 0; k < p; k++) {
                checksum += arr3[i][j][k];
            }
        }
    }
    
    return checksum & 0xFF;
}

/* Test unstructured data regions with runtime library calls */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m)
{
    int i, j;
    int checksum = 0;
    int *dynamic_arr = (int*)malloc(n * m * sizeof(int));
    
    if (!dynamic_arr) return 0;
    
    /* Create data on device */
    #pragma acc enter data create(dynamic_arr[0:n*m])
    
    /* Use kernels construct with conditional execution */
    volatile int use_kernels = 1;
    if (use_kernels) {
        #pragma acc kernels present(dynamic_arr[0:n*m])
        {
            #pragma acc loop independent
            for (i = 0; i < n; i++) {
                for (j = 0; j < m; j++) {
                    dynamic_arr[i * m + j] = i * m + j;
                }
            }
        }
    }
    
    /* Copy data back */
    #pragma acc update host(dynamic_arr[0:n*m])
    
    /* Compute checksum */
    for (i = 0; i < n * m; i++) {
        checksum += dynamic_arr[i];
    }
    
    #pragma acc exit data delete(dynamic_arr)
    free(dynamic_arr);
    
    return checksum & 0xFF;
}

/* Test mixed constructs in data regions */
__attribute__((noinline, used))
int test_mixed_constructs(int n, int m, int p)
{
    int i, j, k;
    int checksum = 0;
    
    /* Structured data region containing multiple compute constructs */
    #pragma acc data copy(arr3[0:n][0:m][0:p])
    {
        /* First parallel region */
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < m; j++) {
                    for (k = 0; k < p; k++) {
                        arr3[i][j][k] = 1;
                    }
                }
            }
        }
        
        /* Second kernels region with different partitioning */
        #pragma acc kernels
        {
            #pragma acc loop independent gang
            for (i = 0; i < n; i++) {
                #pragma acc loop independent worker
                for (j = 0; j < m; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < p; k++) {
                        arr3[i][j][k] += i + j + k;
                    }
                }
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            for (k = 0; k < p; k++) {
                checksum += arr3[i][j][k];
            }
        }
    }
    
    return checksum & 0xFF;
}

int main(int argc, char *argv[])
{
    int total_checksum = 0;
    
    /* Initialize arrays */
    memset(arr1, 0, sizeof(arr1));
    memset(arr2, 0, sizeof(arr2));
    memset(arr3, 0, sizeof(arr3));
    
    /* Use command line argument to create control flow variability */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 3;
    }
    
    /* Execute all partitioning test cases */
    total_checksum += test_gang_redundant(v_N, v_M);
    total_checksum += test_gang_partitioned(v_N, v_M);
    total_checksum += test_worker_partitioned(v_N, v_M);
    total_checksum += test_gang_worker_partitioned(v_N, v_M);
    total_checksum += test_vector_partitioned(v_N, v_M);
    total_checksum += test_gang_vector_partitioned(v_N, v_M);
    total_checksum += test_worker_vector_partitioned(v_N, v_M);
    total_checksum += test_fully_partitioned(v_N, v_M);
    
    /* Conditional execution based on command line */
    if (test_case == 0) {
        total_checksum += test_openmp_offload(v_N/2, v_M/2, v_P/2);
    } else if (test_case == 1) {
        total_checksum += test_unstructured_data(v_N/2, v_M/2);
    } else {
        total_checksum += test_mixed_constructs(v_N/4, v_M/4, v_P/4);
    }
    
    printf("Result: %d\n", total_checksum & 0xFF);
    
    return 0;
}
