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
void test_openmp_offload(int n, int m, int *result);

__attribute__((noinline, used))
void test_unstructured_data(int n, int m);

__attribute__((noinline, used))
void test_data_region(int n, int m, int *result);

/* Test case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m)
{
    int sum = 0;
    
    /* Simple parallel region with default gang redundancy */
    #pragma acc parallel copy(arr1[0:n][0:m]) copyout(arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 2;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m)
{
    int sum = 0;
    
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
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m)
{
    int sum = 0;
    
    /* Worker partitioning */
    #pragma acc kernels copy(worker: arr1[0:n][0:m]) copyout(worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 3;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m)
{
    int sum = 0;
    
    /* Gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m]) \
                         copyout(gang, worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] - i + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m)
{
    int sum = 0;
    
    /* Vector partitioning */
    #pragma acc kernels copy(vector: arr1[0:n][0:m]) copyout(vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 4;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m)
{
    int sum = 0;
    
    /* Gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m]) \
                         copyout(gang, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + i * j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m)
{
    int sum = 0;
    
    /* Worker and vector partitioning */
    #pragma acc kernels copy(worker, vector: arr1[0:n][0:m]) \
                        copyout(worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 5;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m)
{
    int sum = 0;
    
    /* Fully partitioned (gang, worker, vector) */
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m]) \
                         copyout(gang, worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 6;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
void test_openmp_offload(int n, int m, int *result)
{
    int local_sum = 0;
    
    /* Use conditional to create control flow variability */
    volatile int use_offload = 1;
    
    if (use_offload) {
        #pragma omp target teams distribute parallel for \
                    map(tofrom: arr1[0:n][0:m]) map(from: arr2[0:n][0:m]) \
                    num_teams(2) num_threads(4)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 7;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr2[i][j];
        }
    }
    
    *result = local_sum & 0xFF;
}

/* Test unstructured data regions with runtime library calls */
__attribute__((noinline, used))
void test_unstructured_data(int n, int m)
{
    int *d_arr1, *d_arr2;
    size_t size = n * m * sizeof(int);
    
    /* Use runtime library calls for unstructured data */
    d_arr1 = (int *)acc_create(arr1, size);
    d_arr2 = (int *)acc_create(arr2, size);
    
    #pragma acc parallel present(d_arr1[0:n*m], d_arr2[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n * m; i++) {
            d_arr2[i] = d_arr1[i] * 8;
        }
    }
    
    acc_copyout(arr2, size);
    acc_delete(arr1, size);
    acc_delete(arr2, size);
}

/* Test structured data region with multiple compute constructs */
__attribute__((noinline, used))
void test_data_region(int n, int m, int *result)
{
    int local_sum = 0;
    
    #pragma acc data copy(arr1[0:n][0:m]) create(arr2[0:n][0:m])
    {
        /* First compute construct inside data region */
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr2[i][j] = arr1[i][j] * 9;
                }
            }
        }
        
        /* Second compute construct with different partitioning */
        #pragma acc kernels copy(arr2[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = arr2[i][j] / 2;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_sum += arr1[i][j];
        }
    }
    
    *result = local_sum & 0xFF;
}

int main(int argc, char *argv[])
{
    int checksum = 0;
    int n = v_N;  /* Use volatile to prevent constant propagation */
    int m = v_M;
    int p = v_P;
    
    /* Initialize arrays with some data */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i + j;
            arr2[i][j] = 0;
        }
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i + j + k;
            }
        }
    }
    
    printf("Testing OpenACC/OpenMP partition code coverage...\n");
    
    /* Execute all 8 partition cases */
    checksum ^= test_case_0_gang_redundant(n, m);
    checksum ^= test_case_1_gang_partitioned(n, m);
    checksum ^= test_case_2_worker_partitioned(n, m);
    checksum ^= test_case_3_gang_worker_partitioned(n, m);
    checksum ^= test_case_4_vector_partitioned(n, m);
    checksum ^= test_case_5_gang_vector_partitioned(n, m);
    checksum ^= test_case_6_worker_vector_partitioned(n, m);
    checksum ^= test_case_7_fully_partitioned(n, m);
    
    /* Test OpenMP offloading */
    int omp_result;
    test_openmp_offload(n, m, &omp_result);
    checksum ^= omp_result;
    
    /* Test unstructured data */
    test_unstructured_data(n, m);
    
    /* Test structured data region */
    int data_region_result;
    test_data_region(n, m, &data_region_result);
    checksum ^= data_region_result;
    
    /* Additional test with 3D array and non-constant sections */
    int sum_3d = 0;
    #pragma acc parallel copy(arr3[0:n][0:m][0:p]) reduction(+:sum_3d)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    sum_3d += arr3[i][j][k];
                }
            }
        }
    }
    checksum ^= (sum_3d & 0xFF);
    
    /* Conditional execution based on command line to create variability */
    if (argc > 1) {
        /* Use present clause with conditional */
        #pragma acc parallel present(arr1[0:n][0:m]) if(argc > 2)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr1[i][j] += 1;
                }
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum & 0xFF);
    
    return 0;
}
