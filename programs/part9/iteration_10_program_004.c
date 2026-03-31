/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning scenarios
 * (gang redundant through fully partitioned) to trigger coverage
 * of the partition code to string mapping function in GCC's
 * omp-oacc-neuter-broadcast.cc (lines 335-343).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100
#define DIM1 50
#define DIM2 50

/* Volatile variables to prevent constant propagation */
volatile int v_N = DIM1;
volatile int v_M = DIM2;

/* Attribute to prevent inlining and ensure each function is compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Helper to generate runtime-dependent array indices */
static inline int get_idx(int i, int j, int base) {
    return (i * (base % 7) + j) % SIZE;
}

/* Case 0: gang redundant */
NOINLINE_USED
int test_gang_redundant(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    /* Initialize with non-zero pattern */
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i + j;
    
    /* Use runtime values for array sections */
    #pragma acc parallel copy(arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                /* Use volatile in computation to prevent optimization */
                int idx1 = get_idx(i, j, v_N);
                int idx2 = get_idx(j, i, v_M);
                arr[idx1 % N][idx2 % M] = i * j + (v_N % 5);
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Case 1: gang partitioned */
NOINLINE_USED
int test_gang_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i - j;
    
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                int idx1 = get_idx(i, j, v_M);
                int idx2 = get_idx(j, i, v_N);
                arr[idx1 % N][idx2 % M] = i + j * (v_M % 3);
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Case 2: worker partitioned */
NOINLINE_USED
int test_worker_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i * j;
    
    /* Explicit worker partitioning */
    #pragma acc parallel copy(worker: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                int idx1 = get_idx(i, j, v_N + v_M);
                arr[i][j] = (i * (v_N % 7) + j * (v_M % 5)) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Case 3: gang+worker partitioned */
NOINLINE_USED
int test_gang_worker_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i ^ j;
    
    /* Combined gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + i - j + (v_N % 11)) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Case 4: vector partitioned */
NOINLINE_USED
int test_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i | j;
    
    /* Explicit vector partitioning */
    #pragma acc parallel copy(vector: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] * 3 + (v_M % 13)) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Case 5: gang+vector partitioned */
NOINLINE_USED
int test_gang_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i & j;
    
    /* Combined gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + (i << 2) - j + (v_N % 17)) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Case 6: worker+vector partitioned */
NOINLINE_USED
int test_worker_vector_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = ~(i + j);
    
    /* Combined worker and vector partitioning */
    #pragma acc parallel copy(worker, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + i * 5 - j * 3 + (v_M % 19)) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Case 7: fully partitioned */
NOINLINE_USED
int test_fully_partitioned(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i % (j + 1);
    
    /* Fully partitioned across all levels */
    #pragma acc parallel copy(gang, worker, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] * 7 + i - j * 2 + (v_N + v_M) % 23) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Test with kernels construct for variety */
NOINLINE_USED
int test_kernels_partition(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = (i << 4) | j;
    
    /* Use kernels with gang partitioned data */
    #pragma acc kernels copy(gang: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + (v_N % 29)) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Test with data region containing multiple compute constructs */
NOINLINE_USED
int test_data_region(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i + j * 2;
    
    /* Structured data region */
    #pragma acc data copy(arr[0:N][0:M])
    {
        /* First compute construct */
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    arr[i][j] = (arr[i][j] * 2) & 0xFF;
                }
            }
        }
        
        /* Second compute construct with different partitioning */
        #pragma acc parallel copy(worker: arr[0:N][0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    arr[i][j] = (arr[i][j] + (v_M % 31)) & 0xFF;
                }
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Test with OpenMP target offloading for broader coverage */
NOINLINE_USED
int test_omp_target(int N, int M) {
    int arr[SIZE][SIZE];
    int sum = 0;
    
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            arr[i][j] = i * 3 - j;
    
    /* OpenMP target with distribute and teams */
    #pragma omp target map(tofrom: arr[0:N][0:M])
    #pragma omp teams distribute
    for (int i = 0; i < N; i++) {
        #pragma omp parallel for
        for (int j = 0; j < M; j++) {
            arr[i][j] = (arr[i][j] + (v_N % 37)) & 0xFF;
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFF;
    
    return sum;
}

/* Test with unstructured data directives */
NOINLINE_USED
int test_unstructured_data(int N, int M) {
    int *arr = (int*)malloc(SIZE * SIZE * sizeof(int));
    int sum = 0;
    
    if (!arr) return 0;
    
    for (int i = 0; i < SIZE * SIZE; i++)
        arr[i] = i % 256;
    
    /* Unstructured data directives */
    #pragma acc enter data copyin(arr[0:N*M])
    
    #pragma acc parallel present(arr[0:N*M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                arr[idx] = (arr[idx] * 11 + (v_N % 41)) & 0xFF;
            }
        }
    }
    
    #pragma acc exit data copyout(arr[0:N*M])
    
    for (int i = 0; i < N * M; i++)
        sum = (sum + arr[i]) & 0xFF;
    
    free(arr);
    return sum;
}

/* Main function with conditional execution to force neutering analysis */
int main(int argc, char *argv[]) {
    int checksum = 0;
    int N = DIM1;
    int M = DIM2;
    
    /* Use command line args to vary execution paths */
    if (argc > 1) {
        N = atoi(argv[1]) % SIZE;
        if (N <= 0) N = DIM1;
    }
    if (argc > 2) {
        M = atoi(argv[2]) % SIZE;
        if (M <= 0) M = DIM2;
    }
    
    /* Update volatile variables */
    v_N = N;
    v_M = M;
    
    /* Always execute base cases */
    checksum ^= test_gang_redundant(N, M);
    checksum ^= test_gang_partitioned(N, M);
    checksum ^= test_worker_partitioned(N, M);
    checksum ^= test_gang_worker_partitioned(N, M);
    checksum ^= test_vector_partitioned(N, M);
    checksum ^= test_gang_vector_partitioned(N, M);
    checksum ^= test_worker_vector_partitioned(N, M);
    checksum ^= test_fully_partitioned(N, M);
    
    /* Conditional execution to create control flow variability */
    if (v_N > 10) {
        checksum ^= test_kernels_partition(N, M);
        checksum ^= test_data_region(N, M);
    }
    
    if (v_M > 15) {
        checksum ^= test_omp_target(N, M);
    }
    
    if ((v_N + v_M) % 3 == 0) {
        checksum ^= test_unstructured_data(N, M);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
