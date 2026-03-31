/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in the
 * omp-oacc-neuter-broadcast.cc file by creating OpenACC compute constructs
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
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m);
__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m);
__attribute__((noinline, used))
void test_openmp_offload(int n, int m, int p);
__attribute__((noinline, used))
void test_unstructured_data(int n, int m);
__attribute__((noinline, used))
void test_data_region(int n, int m);

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m)
{
    int checksum = 0;
    
    /* Simple copy clause without explicit partitioning */
    #pragma acc parallel copy(arr1[0:n][0:m]) copyout(arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 2;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m)
{
    int checksum = 0;
    
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr1[0:n][0:m]) copyout(gang: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + i + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m)
{
    int checksum = 0;
    
    /* Explicit worker partitioning */
    #pragma acc parallel copy(worker: arr1[0:n][0:m]) copyout(worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 3 - j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m)
{
    int checksum = 0;
    
    /* Combined gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m]) copyout(gang, worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + (i * j);
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m)
{
    int checksum = 0;
    
    /* Explicit vector partitioning */
    #pragma acc parallel copy(vector: arr1[0:n][0:m]) copyout(vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] - i + j * 2;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m)
{
    int checksum = 0;
    
    /* Combined gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m]) copyout(gang, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * i + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m)
{
    int checksum = 0;
    
    /* Combined worker and vector partitioning */
    #pragma acc parallel copy(worker, vector: arr1[0:n][0:m]) copyout(worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + i * 2 - j * 3;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m)
{
    int checksum = 0;
    
    /* Fully partitioned: gang, worker, and vector */
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m]) copyout(gang, worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 5 + i - j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Test with OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
void test_openmp_offload(int n, int m, int p)
{
    int sum = 0;
    
    /* OpenMP target with distribute and teams clauses */
    #pragma omp target map(tofrom: arr3[0:n][0:m][0:p])
    {
        #pragma omp teams distribute parallel for collapse(3)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    arr3[i][j][k] = i + j + k;
                }
            }
        }
    }
    
    /* Verify computation */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                sum += arr3[i][j][k];
            }
        }
    }
    
    /* Use sum to prevent dead code elimination */
    arr1[0][0] = sum & 0xFF;
}

/* Test unstructured data regions with runtime library calls */
__attribute__((noinline, used))
void test_unstructured_data(int n, int m)
{
    int *d_arr = NULL;
    int *h_arr = (int *)malloc(n * m * sizeof(int));
    
    if (!h_arr) return;
    
    /* Initialize host array */
    for (int i = 0; i < n * m; i++) {
        h_arr[i] = i;
    }
    
    /* Create device array */
    #pragma acc enter data copyin(h_arr[0:n*m])
    d_arr = h_arr;
    
    /* Use kernels construct with present clause */
    #pragma acc kernels present(d_arr[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n * m; i++) {
            d_arr[i] = d_arr[i] * 2;
        }
    }
    
    /* Copy back and free */
    #pragma acc exit data copyout(h_arr[0:n*m])
    
    /* Use result to prevent elimination */
    arr1[0][0] = h_arr[0] & 0xFF;
    
    free(h_arr);
}

/* Test structured data region containing multiple compute constructs */
__attribute__((noinline, used))
void test_data_region(int n, int m)
{
    int local_sum = 0;
    
    /* Data region with copy clause */
    #pragma acc data copy(arr1[0:n][0:m]) copyout(arr2[0:n][0:m])
    {
        /* First parallel region inside data region */
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = i * 10 + j;
                }
            }
        }
        
        /* Second parallel region with different partitioning */
        #pragma acc parallel copy(gang, worker: arr1[0:n][0:m])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                #pragma acc loop vector
                for (int j = 0; j < m; j++) {
                    arr2[i][j] = arr1[i][j] * 3;
                }
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr2[i][j];
        }
    }
    
    arr1[0][0] = local_sum & 0xFF;
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[])
{
    int checksum = 0;
    int use_openmp = 0;
    int use_unstructured = 0;
    
    /* Initialize arrays */
    memset(arr1, 0, sizeof(arr1));
    memset(arr2, 0, sizeof(arr2));
    memset(arr3, 0, sizeof(arr3));
    
    /* Fill with initial values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i * M + j;
            arr2[i][j] = 0;
        }
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = 0;
            }
        }
    }
    
    /* Determine which additional tests to run based on volatile input */
    if (argc > 1) {
        use_openmp = (argv[1][0] == '1');
        use_unstructured = (argc > 2 && argv[2][0] == '1');
    }
    
    /* Execute all 8 partition cases */
    checksum += test_case_0_gang_redundant(v_N, v_M);
    checksum += test_case_1_gang_partitioned(v_N, v_M);
    checksum += test_case_2_worker_partitioned(v_N, v_M);
    checksum += test_case_3_gang_worker_partitioned(v_N, v_M);
    checksum += test_case_4_vector_partitioned(v_N, v_M);
    checksum += test_case_5_gang_vector_partitioned(v_N, v_M);
    checksum += test_case_6_worker_vector_partitioned(v_N, v_M);
    checksum += test_case_7_fully_partitioned(v_N, v_M);
    
    /* Conditional OpenMP test */
    if (use_openmp) {
        test_openmp_offload(v_N, v_M, v_P);
    }
    
    /* Conditional unstructured data test */
    if (use_unstructured) {
        test_unstructured_data(v_N, v_M);
    }
    
    /* Always test data region */
    test_data_region(v_N, v_M);
    
    /* Final checksum output */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
