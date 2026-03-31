/* This test requires GCC configured with offloading support.
   Compile with: gcc -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage test.c -o test
   Run with: ./test
   For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
   For host-fallback: use -foffload=disable */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Volatile variables to prevent constant propagation */
volatile int vN = N;
volatile int vM = M;
volatile int vP = P;

/* Helper to ensure side effects */
static int checksum = 0;

/* Case 0: gang redundant */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 200 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 300 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 400 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 500 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 600 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 700 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 800 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Test with kernels construct */
__attribute__((noinline, used))
int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma acc kernels create(gang, worker: arr[0:n][0:m]) copyout(arr[0:n][0:m])
    {
        for (i = 0; i < n; i++) {
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 900 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Test with data region containing multiple constructs */
__attribute__((noinline, used))
int test_data_region(int n, int m, int p) {
    int arr[N][M];
    int i, j;
    
    #pragma acc data copy(arr[0:n][0:m])
    {
        #pragma acc parallel present(arr)
        {
            #pragma acc loop gang
            for (i = 0; i < n; i++) {
                #pragma acc loop worker
                for (j = 0; j < m; j++) {
                    arr[i][j] = i * 1000 + j;
                }
            }
        }
        
        if (p > 0) {
            #pragma acc parallel present(arr)
            {
                #pragma acc loop gang
                for (i = 0; i < n; i++) {
                    #pragma acc loop worker
                    for (j = 0; j < m; j++) {
                        arr[i][j] += p;
                    }
                }
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Test with OpenMP target offloading */
__attribute__((noinline, used))
int test_omp_target(int n, int m) {
    int arr[N][M];
    int i, j;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute
        for (i = 0; i < n; i++) {
            #pragma omp parallel for
            for (j = 0; j < m; j++) {
                arr[i][j] = i * 1100 + j;
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < n && i < N; i++)
        for (j = 0; j < m && j < M; j++)
            sum += arr[i][j];
    return sum & 0xFF;
}

/* Test with unstructured data and runtime API */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr = (int*)malloc(n * m * sizeof(int));
    int i, j;
    
    #pragma acc enter data create(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr[i*m + j] = i * 1200 + j;
            }
        }
    }
    
    #pragma acc exit data copyout(arr[0:n*m])
    
    int sum = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < m; j++)
            sum += arr[i*m + j];
    
    free(arr);
    return sum & 0xFF;
}

/* Main function with conditional execution */
int main(int argc, char *argv[]) {
    int result = 0;
    int n = vN;
    int m = vM;
    int p = vP;
    
    /* Always execute all 8 partition cases */
    result ^= test_gang_redundant(n, m);
    result ^= test_gang_partitioned(n, m);
    result ^= test_worker_partitioned(n, m);
    result ^= test_gang_worker_partitioned(n, m);
    result ^= test_vector_partitioned(n, m);
    result ^= test_gang_vector_partitioned(n, m);
    result ^= test_worker_vector_partitioned(n, m);
    result ^= test_fully_partitioned(n, m);
    
    /* Conditional execution based on volatile variable */
    if (p > 10) {
        result ^= test_kernels_partition(n, m);
    }
    
    if (p > 20) {
        result ^= test_data_region(n, m, p);
    }
    
    /* Always execute OpenMP and unstructured tests */
    result ^= test_omp_target(n, m);
    result ^= test_unstructured_data(n/2, m/2);
    
    printf("Result: %d\n", result & 0xFF);
    return 0;
}
