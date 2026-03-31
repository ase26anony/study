/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning cases
 * to trigger coverage of the partition code to string mapping
 * function in omp-oacc-neuter-broadcast.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100
#define DIM 50

/* Volatile variables to prevent constant propagation */
volatile int v_N = DIM;
volatile int v_M = DIM;

/* Helper to ensure side effects */
static int checksum = 0;

/* Function to prevent dead code elimination */
__attribute__((noinline, used))
static void update_checksum(int value) {
    checksum ^= value;
}

/* Case 0: gang redundant */
__attribute__((noinline, used))
void test_gang_redundant(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc parallel copy(arr[0:N][0:M])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
void test_gang_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc parallel copy(gang: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 1;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
void test_worker_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc parallel copy(worker: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 2;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
void test_gang_worker_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc parallel copy(gang, worker: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 3;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
void test_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc parallel copy(vector: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 4;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
void test_gang_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc parallel copy(gang, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 5;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
void test_worker_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc parallel copy(worker, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 6;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
void test_fully_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 7;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with kernels construct for variety */
__attribute__((noinline, used))
void test_kernels_partition(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc kernels copy(gang, worker: arr[0:N][0:M])
    {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 8;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with data region containing multiple compute constructs */
__attribute__((noinline, used))
void test_data_region(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma acc data copy(arr[0:N][0:M])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < M; j++) {
                    arr[i][j] = i * 100 + j + 9;
                }
            }
        }
        
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < M; j++) {
                    arr[i][j] += 1;
                }
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Test with unstructured data using runtime API */
__attribute__((noinline, used))
void test_unstructured_data(int N, int M) {
    int *arr = (int*)malloc(N * M * sizeof(int));
    
    #pragma acc enter data create(arr[0:N*M])
    
    #pragma acc parallel present(arr[0:N*M])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N*M; i++) {
            arr[i] = i + 10;
        }
    }
    
    #pragma acc exit data copyout(arr[0:N*M])
    
    int sum = 0;
    for (int i = 0; i < N*M; i++) {
        sum += arr[i];
    }
    update_checksum(sum);
    
    free(arr);
}

/* Test with OpenMP offloading for broader coverage */
__attribute__((noinline, used))
void test_omp_offload(int N, int M) {
    int arr[SIZE][SIZE];
    
    #pragma omp target map(tofrom: arr[0:N][0:M])
    {
        #pragma omp teams distribute parallel for collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100 + j + 11;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    update_checksum(sum);
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int N = v_N;
    int M = v_M;
    
    /* Always execute all 8 partition cases */
    test_gang_redundant(N, M);
    test_gang_partitioned(N, M);
    test_worker_partitioned(N, M);
    test_gang_worker_partitioned(N, M);
    test_vector_partitioned(N, M);
    test_gang_vector_partitioned(N, M);
    test_worker_vector_partitioned(N, M);
    test_fully_partitioned(N, M);
    
    /* Conditional execution based on volatile to create control flow */
    volatile int flag = 1;
    
    if (flag) {
        test_kernels_partition(N, M);
        test_data_region(N, M);
    }
    
    if (argc > 1) {
        test_unstructured_data(N, M);
    }
    
    /* Always execute OpenMP test */
    test_omp_offload(N, M);
    
    /* Print checksum to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
