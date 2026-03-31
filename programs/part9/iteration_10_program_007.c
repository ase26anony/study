/* This test requires GCC configured with offloading support. 
   Compile with: gcc -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage test.c -o test
   Run with: ./test
   
   The test exercises all 8 partition code cases (0-7) in GCC's omp-oacc-neuter-broadcast.cc
   by creating OpenACC compute constructs with different data clause partitioning.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 128
#define M 64
#define P 32

/* Volatile variables to prevent constant propagation */
volatile int v_N = N;
volatile int v_M = M;
volatile int v_P = P;

/* Helper to generate side effects and prevent dead code elimination */
static int checksum = 0;

/* Each function tests a specific partition case */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m) {
    int arr[N][M];
    int i, j;
    
    /* Initialize array */
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i + j;
    
    /* Case 0: gang redundant (default) */
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop
        for (i = 0; i < n; i++) {
            #pragma acc loop
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 2 + 1;
            }
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i - j;
    
    /* Case 1: gang partitioned */
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 3 - 2;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * j;
    
    /* Case 2: worker partitioned */
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] / 2 + i;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i ^ j;
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] | 0x1;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i & j;
    
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] << 1;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i | j;
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] >> 1;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i % (j + 1);
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] + i - j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = (i << 2) + j;
    
    /* Case 7: fully partitioned */
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * arr[i][j];
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

/* Test with OpenACC kernels construct for variety */
__attribute__((noinline, used))
int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i + j * 2;
    
    /* Mix of partition types in kernels region */
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] + 1;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

/* Test with OpenMP target for broader coverage */
__attribute__((noinline, used))
int test_omp_target(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = i * 3 + j;
    
    /* OpenMP target region */
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute
        for (i = 0; i < n; i++) {
            #pragma omp parallel for
            for (j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] - 5;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

/* Test with data region and multiple compute constructs */
__attribute__((noinline, used))
int test_data_region(int n, int m) {
    int arr[N][M];
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i][j] = 0;
    
    /* Structured data region */
    #pragma acc data copy(arr[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (i = 0; i < n; i++) {
                #pragma acc loop worker
                for (j = 0; j < m; j++) {
                    arr[i][j] = i * 10 + j;
                }
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < m; j++) {
                    arr[i][j] = arr[i][j] * 2;
                }
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i][j];
    
    checksum += sum;
    return sum & 0xFF;
}

/* Test with unstructured data and runtime library calls */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * m * sizeof(int));
    int i, j;
    
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            arr[i * m + j] = i + j;
    
    /* Unstructured data handling */
    void *dev_ptr = acc_create(arr, n * m * sizeof(int));
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i * m + j] = arr[i * m + j] + 100;
            }
        }
    }
    
    acc_copyout(arr, n * m * sizeof(int));
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i * m + j];
    
    free(arr);
    checksum += sum;
    return sum & 0xFF;
}

/* Main function with conditional execution to force neutering analysis */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use volatile to prevent constant folding */
    volatile int use_acc = 1;
    volatile int use_omp = 1;
    
    /* Always execute all test cases to cover all partition codes */
    result |= test_case_0_gang_redundant(v_N, v_M);
    
    if (use_acc) {
        result |= test_case_1_gang_partitioned(v_N, v_M);
        result |= test_case_2_worker_partitioned(v_N, v_M);
        result |= test_case_3_gang_worker_partitioned(v_N, v_M);
        result |= test_case_4_vector_partitioned(v_N, v_M);
        result |= test_case_5_gang_vector_partitioned(v_N, v_M);
        result |= test_case_6_worker_vector_partitioned(v_N, v_M);
        result |= test_case_7_fully_partitioned(v_N, v_M);
        result |= test_kernels_partition(v_N, v_M);
        result |= test_data_region(v_N, v_M);
        result |= test_unstructured_data(v_N / 2, v_M / 2);
    }
    
    if (use_omp) {
        result |= test_omp_target(v_N, v_M);
    }
    
    /* Additional test with conditional parallel region */
    volatile int flag = argc > 1 ? atoi(argv[1]) : 1;
    
    if (flag > 0) {
        int arr[P];
        int i;
        
        for (i = 0; i < P; i++)
            arr[i] = i;
        
        /* This region's execution depends on runtime value */
        #pragma acc parallel copy(arr[0:P])
        {
            #pragma acc loop
            for (i = 0; i < P; i++) {
                arr[i] = arr[i] * flag;
            }
        }
        
        for (i = 0; i < P; i++)
            result += arr[i];
    }
    
    /* Final checksum output to prevent optimization */
    printf("Result: %d (checksum: %d)\n", result & 0xFF, checksum & 0xFF);
    
    return 0;
}
