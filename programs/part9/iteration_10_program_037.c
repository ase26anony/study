/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning cases
 * (gang redundant, gang partitioned, ..., fully partitioned)
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

/* Global arrays to work with */
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes with attributes to prevent optimization */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m);
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m);
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m);
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m);
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m);
__attribute__((noinline, used))
int test_openmp_offload(int n, int m, int p);
__attribute__((noinline, used))
int test_acc_data_region(int n, int m);
__attribute__((noinline, used))
int test_acc_unstructured(int n, int m);

/* Case 0: gang redundant (default) */
int test_gang_redundant(int n, int m)
{
    int i, j;
    int sum = 0;
    
    /* Use runtime values to prevent static optimization */
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc parallel copy(arr1[0:rows][0:cols])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < rows; i++) {
            #pragma acc loop gang worker vector
            for (j = 0; j < cols; j++) {
                arr1[i][j] = i * cols + j;
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
int test_gang_partitioned(int n, int m)
{
    int i, j;
    int sum = 0;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc parallel copy(gang: arr1[0:rows][0:cols])
    {
        #pragma acc loop gang
        for (i = 0; i < rows; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < cols; j++) {
                arr1[i][j] = (i + 1) * (j + 1);
            }
        }
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
int test_worker_partitioned(int n, int m)
{
    int i, j;
    int sum = 0;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc parallel copy(worker: arr1[0:rows][0:cols])
    {
        #pragma acc loop gang
        for (i = 0; i < rows; i++) {
            #pragma acc loop worker
            for (j = 0; j < cols; j++) {
                arr1[i][j] = i - j;
            }
        }
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
int test_gang_worker_partitioned(int n, int m)
{
    int i, j;
    int sum = 0;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc parallel copy(gang, worker: arr1[0:rows][0:cols])
    {
        #pragma acc loop gang worker
        for (i = 0; i < rows; i++) {
            #pragma acc loop gang worker
            for (j = 0; j < cols; j++) {
                arr1[i][j] = i * i - j * j;
            }
        }
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
int test_vector_partitioned(int n, int m)
{
    int i, j;
    int sum = 0;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc parallel copy(vector: arr1[0:rows][0:cols])
    {
        #pragma acc loop gang worker
        for (i = 0; i < rows; i++) {
            #pragma acc loop vector
            for (j = 0; j < cols; j++) {
                arr1[i][j] = i + j * 2;
            }
        }
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
int test_gang_vector_partitioned(int n, int m)
{
    int i, j;
    int sum = 0;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc parallel copy(gang, vector: arr1[0:rows][0:cols])
    {
        #pragma acc loop gang vector
        for (i = 0; i < rows; i++) {
            #pragma acc loop gang vector
            for (j = 0; j < cols; j++) {
                arr1[i][j] = (i << 2) + j;
            }
        }
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
int test_worker_vector_partitioned(int n, int m)
{
    int i, j;
    int sum = 0;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc parallel copy(worker, vector: arr1[0:rows][0:cols])
    {
        #pragma acc loop worker vector
        for (i = 0; i < rows; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < cols; j++) {
                arr1[i][j] = i * 3 - j * 2;
            }
        }
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
int test_fully_partitioned(int n, int m)
{
    int i, j;
    int sum = 0;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc parallel copy(gang, worker, vector: arr1[0:rows][0:cols])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < rows; i++) {
            #pragma acc loop gang worker vector
            for (j = 0; j < cols; j++) {
                arr1[i][j] = i * j * 2;
            }
        }
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test with OpenMP offloading to engage broader infrastructure */
int test_openmp_offload(int n, int m, int p)
{
    int i, j, k;
    int sum = 0;
    
    int dim1 = n > 0 ? n : v_N;
    int dim2 = m > 0 ? m : v_M;
    int dim3 = p > 0 ? p : v_P;
    
    #pragma omp target map(tofrom: arr3[0:dim1][0:dim2][0:dim3])
    {
        #pragma omp teams distribute parallel for collapse(3)
        for (i = 0; i < dim1; i++) {
            for (j = 0; j < dim2; j++) {
                for (k = 0; k < dim3; k++) {
                    arr3[i][j][k] = i + j + k;
                }
            }
        }
    }
    
    for (i = 0; i < dim1; i++) {
        for (j = 0; j < dim2; j++) {
            for (k = 0; k < dim3; k++) {
                sum += arr3[i][j][k];
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with structured data region containing multiple compute constructs */
int test_acc_data_region(int n, int m)
{
    int i, j;
    int sum = 0;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    
    #pragma acc data copy(arr2[0:rows][0:cols])
    {
        /* First parallel region */
        #pragma acc parallel present(arr2[0:rows][0:cols])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < rows; i++) {
                #pragma acc loop gang worker vector
                for (j = 0; j < cols; j++) {
                    arr2[i][j] = 1;
                }
            }
        }
        
        /* Second parallel region with different partitioning */
        #pragma acc parallel present(arr2[0:rows][0:cols])
        {
            #pragma acc loop gang
            for (i = 0; i < rows; i++) {
                #pragma acc loop worker
                for (j = 0; j < cols; j++) {
                    arr2[i][j] += i + j;
                }
            }
        }
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr2[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test with unstructured data directives using runtime library calls */
int test_acc_unstructured(int n, int m)
{
    int i, j;
    int sum = 0;
    int *dev_ptr;
    
    int rows = n > 0 ? n : v_N;
    int cols = m > 0 ? m : v_M;
    size_t size = rows * cols * sizeof(int);
    
    /* Allocate and copy data to device */
    dev_ptr = (int*)acc_malloc(size);
    if (dev_ptr) {
        acc_map_data(arr1, dev_ptr, size);
        
        #pragma acc parallel present(arr1[0:rows][0:cols])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < rows; i++) {
                #pragma acc loop gang worker vector
                for (j = 0; j < cols; j++) {
                    arr1[i][j] = (i << 4) | (j & 0xF);
                }
            }
        }
        
        acc_unmap_data(arr1);
        acc_free(dev_ptr);
    }
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += arr1[i][j];
        }
    }
    
    return sum & 0xFF;
}

int main(int argc, char *argv[])
{
    int checksum = 0;
    volatile int flag = 0;
    
    /* Initialize arrays */
    memset(arr1, 0, sizeof(arr1));
    memset(arr2, 0, sizeof(arr2));
    memset(arr3, 0, sizeof(arr3));
    
    /* Use command line argument to create conditional execution */
    if (argc > 1) {
        flag = atoi(argv[1]);
    }
    
    /* Always execute all 8 OpenACC partitioning cases */
    checksum ^= test_gang_redundant(N, M);
    checksum ^= test_gang_partitioned(N, M);
    checksum ^= test_worker_partitioned(N, M);
    checksum ^= test_gang_worker_partitioned(N, M);
    checksum ^= test_vector_partitioned(N, M);
    checksum ^= test_gang_vector_partitioned(N, M);
    checksum ^= test_worker_vector_partitioned(N, M);
    checksum ^= test_fully_partitioned(N, M);
    
    /* Conditionally execute OpenMP offload test */
    if (flag & 1) {
        checksum ^= test_openmp_offload(N/2, M/2, P/2);
    }
    
    /* Conditionally execute data region test */
    if (flag & 2) {
        checksum ^= test_acc_data_region(N, M);
    }
    
    /* Conditionally execute unstructured test */
    if (flag & 4) {
        checksum ^= test_acc_unstructured(N, M);
    }
    
    /* Also test with kernels construct for variety */
    #pragma acc kernels copy(arr1[0:N][0:M])
    {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr1[i][j] = checksum + i + j;
            }
        }
    }
    
    /* Final checksum computation */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
        }
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
