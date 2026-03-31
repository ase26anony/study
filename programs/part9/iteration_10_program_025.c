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

/* Function prototypes with attributes to prevent optimization */
__attribute__((noinline,used))
int test_case_0_gang_redundant(int n, int m);
__attribute__((noinline,used))
int test_case_1_gang_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_2_worker_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_3_gang_worker_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_4_vector_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_5_gang_vector_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_6_worker_vector_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_7_fully_partitioned(int n, int m);
__attribute__((noinline,used))
void test_openmp_target(int n, int m, int *checksum);
__attribute__((noinline,used))
void test_unstructured_data(int n, int m, int *checksum);
__attribute__((noinline,used))
void test_structured_data_region(int n, int m, int *checksum);

/* Case 0: gang redundant (default) */
__attribute__((noinline,used))
int test_case_0_gang_redundant(int n, int m)
{
    int arr[N][M];
    int i, j;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i + j;
    
    /* OpenACC parallel with default gang redundancy */
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++)
        {
            #pragma acc loop gang worker vector
            for (j = 0; j < m; j++)
            {
                /* Simple computation with volatile index */
                int idx = i * v_M + j;
                arr[i][j] = arr[i][j] * 2 + (idx % 7);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 1: gang partitioned */
__attribute__((noinline,used))
int test_case_1_gang_partitioned(int n, int m)
{
    int arr[N][M];
    int i, j;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * j;
    
    /* OpenACC parallel with gang partitioned data */
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++)
        {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++)
            {
                arr[i][j] = arr[i][j] * 3 + (i % 5);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 2: worker partitioned */
__attribute__((noinline,used))
int test_case_2_worker_partitioned(int n, int m)
{
    int arr[N][M];
    int i, j;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i - j;
    
    /* OpenACC parallel with worker partitioned data */
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++)
        {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++)
            {
                arr[i][j] = arr[i][j] * 4 + (j % 3);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline,used))
int test_case_3_gang_worker_partitioned(int n, int m)
{
    int arr[N][M];
    int i, j;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i ^ j;
    
    /* OpenACC parallel with gang+worker partitioned data */
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++)
        {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++)
            {
                arr[i][j] = arr[i][j] * 5 + ((i + j) % 11);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 4: vector partitioned */
__attribute__((noinline,used))
int test_case_4_vector_partitioned(int n, int m)
{
    int arr[N][M];
    int i, j;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i | j;
    
    /* OpenACC parallel with vector partitioned data */
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++)
        {
            #pragma acc loop vector
            for (j = 0; j < m; j++)
            {
                arr[i][j] = arr[i][j] * 6 + ((i * j) % 13);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline,used))
int test_case_5_gang_vector_partitioned(int n, int m)
{
    int arr[N][M];
    int i, j;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i & j;
    
    /* OpenACC parallel with gang+vector partitioned data */
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (i = 0; i < n; i++)
        {
            #pragma acc loop vector
            for (j = 0; j < m; j++)
            {
                arr[i][j] = arr[i][j] * 7 + ((i + j * 2) % 17);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline,used))
int test_case_6_worker_vector_partitioned(int n, int m)
{
    int arr[N][M];
    int i, j;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = ~(i + j);
    
    /* OpenACC parallel with worker+vector partitioned data */
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop worker vector
        for (i = 0; i < n; i++)
        {
            #pragma acc loop vector
            for (j = 0; j < m; j++)
            {
                arr[i][j] = arr[i][j] * 8 + ((i * 3 + j) % 19);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 7: fully partitioned */
__attribute__((noinline,used))
int test_case_7_fully_partitioned(int n, int m)
{
    int arr[N][M];
    int i, j;
    int sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 2 - j;
    
    /* OpenACC parallel with fully partitioned data */
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++)
        {
            #pragma acc loop gang worker vector
            for (j = 0; j < m; j++)
            {
                arr[i][j] = arr[i][j] * 9 + ((i * j + i + j) % 23);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Test with OpenMP target to engage broader offloading infrastructure */
__attribute__((noinline,used))
void test_openmp_target(int n, int m, int *checksum)
{
    int arr[N][M];
    int i, j;
    int local_sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * i + j * j;
    
    /* OpenMP target region with distribute and teams */
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute
        for (i = 0; i < n; i++)
        {
            #pragma omp parallel for
            for (j = 0; j < m; j++)
            {
                arr[i][j] = arr[i][j] + (i % 7) - (j % 5);
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            local_sum = (local_sum + arr[i][j]) & 0xFFF;
    
    *checksum = (*checksum + local_sum) & 0xFFF;
}

/* Test unstructured data regions with runtime library calls */
__attribute__((noinline,used))
void test_unstructured_data(int n, int m, int *checksum)
{
    int *arr = (int *)malloc(n * m * sizeof(int));
    int i, j;
    int local_sum = 0;
    
    if (!arr) return;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i * m + j] = i * 3 + j * 2;
    
    /* Create device data */
    #pragma acc enter data copyin(arr[0:n*m])
    
    /* Nested conditional to create control flow variability */
    if (v_N > 100) {
        #pragma acc parallel present(arr[0:n*m])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < n; i++)
            {
                #pragma acc loop gang worker vector
                for (j = 0; j < m; j++)
                {
                    int idx = i * v_M + j;
                    arr[i * m + j] = arr[i * m + j] * 2 + (idx % 29);
                }
            }
        }
    } else {
        #pragma acc kernels present(arr[0:n*m])
        {
            #pragma acc loop independent
            for (i = 0; i < n; i++)
            {
                #pragma acc loop independent
                for (j = 0; j < m; j++)
                {
                    arr[i * m + j] = arr[i * m + j] + (i % 11) * (j % 7);
                }
            }
        }
    }
    
    /* Copy data back */
    #pragma acc exit data copyout(arr[0:n*m])
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            local_sum = (local_sum + arr[i * m + j]) & 0xFFF;
    
    *checksum = (*checksum + local_sum) & 0xFFF;
    free(arr);
}

/* Test structured data region with multiple compute constructs */
__attribute__((noinline,used))
void test_structured_data_region(int n, int m, int *checksum)
{
    int arr1[N][M], arr2[N][M];
    int i, j;
    int local_sum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            arr1[i][j] = i * 5 + j * 3;
            arr2[i][j] = i * 7 - j * 2;
        }
    }
    
    /* Structured data region */
    #pragma acc data copy(arr1[0:n][0:m], arr2[0:n][0:m])
    {
        /* First compute construct */
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < n; i++)
            {
                #pragma acc loop gang worker vector
                for (j = 0; j < m; j++)
                {
                    arr1[i][j] = arr1[i][j] * 3 + (i % 13);
                }
            }
        }
        
        /* Second compute construct with different partitioning */
        #pragma acc parallel copy(gang, worker: arr2[0:n][0:m])
        {
            #pragma acc loop gang worker
            for (i = 0; i < n; i++)
            {
                #pragma acc loop worker vector
                for (j = 0; j < m; j++)
                {
                    arr2[i][j] = arr2[i][j] + arr1[i][j] + (j % 17);
                }
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            local_sum = (local_sum + arr1[i][j] + arr2[i][j]) & 0xFFF;
    
    *checksum = (*checksum + local_sum) & 0xFFF;
}

int main(int argc, char *argv[])
{
    int checksum = 0;
    int n = v_N > N ? N : v_N;
    int m = v_M > M ? M : v_M;
    
    printf("Testing OpenACC/OpenMP partition codes...\n");
    
    /* Execute all 8 partition cases */
    checksum = (checksum + test_case_0_gang_redundant(n, m)) & 0xFFF;
    checksum = (checksum + test_case_1_gang_partitioned(n, m)) & 0xFFF;
    checksum = (checksum + test_case_2_worker_partitioned(n, m)) & 0xFFF;
    checksum = (checksum + test_case_3_gang_worker_partitioned(n, m)) & 0xFFF;
    checksum = (checksum + test_case_4_vector_partitioned(n, m)) & 0xFFF;
    checksum = (checksum + test_case_5_gang_vector_partitioned(n, m)) & 0xFFF;
    checksum = (checksum + test_case_6_worker_vector_partitioned(n, m)) & 0xFFF;
    checksum = (checksum + test_case_7_fully_partitioned(n, m)) & 0xFFF;
    
    /* Test OpenMP target */
    test_openmp_target(n, m, &checksum);
    
    /* Test unstructured data */
    test_unstructured_data(n, m, &checksum);
    
    /* Test structured data region */
    test_structured_data_region(n, m, &checksum);
    
    /* Final checksum output */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
