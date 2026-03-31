/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning cases
 * (gang redundant through fully partitioned) to trigger coverage
 * of the partition code to string mapping function in GCC's
 * omp-oacc-neuter-broadcast.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100
#define DIM 50

/* Volatile variables to prevent constant propagation */
volatile int v_N = SIZE;
volatile int v_M = DIM;

/* Attribute to prevent inlining and ensure each function is compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Helper to generate runtime-dependent array indices */
static int get_idx(int i, int j) {
    volatile int offset = 0;
    return (i * DIM + j) + offset;
}

/* Case 0: gang redundant */
NOINLINE_USED
int test_gang_redundant(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i * j;
    
    /* OpenACC parallel with default gang redundancy */
    #pragma acc parallel copy(arr[0:N][0:M])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < M; j++) {
                int idx = get_idx(i, j);
                if (idx < N * M)
                    arr[i][j] += 1;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 1: gang partitioned */
NOINLINE_USED
int test_gang_partitioned(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i + j;
    
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                volatile int k = i * M + j;
                arr[i][j] = (arr[i][j] * 2) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 2: worker partitioned */
NOINLINE_USED
int test_worker_partitioned(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i - j;
    
    /* Worker partitioning */
    #pragma acc kernels copy(worker: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + j * 3) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 3: gang+worker partitioned */
NOINLINE_USED
int test_gang_worker_partitioned(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i | j;
    
    /* Gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                volatile int modifier = 5;
                arr[i][j] = (arr[i][j] + modifier) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 4: vector partitioned */
NOINLINE_USED
int test_vector_partitioned(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i ^ j;
    
    /* Vector partitioning */
    #pragma acc kernels copy(vector: arr[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] * 7) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 5: gang+vector partitioned */
NOINLINE_USED
int test_gang_vector_partitioned(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i & j;
    
    /* Gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                volatile int shift = 2;
                arr[i][j] = (arr[i][j] << shift) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 6: worker+vector partitioned */
NOINLINE_USED
int test_worker_vector_partitioned(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i % (j + 1);
    
    /* Worker and vector partitioning */
    #pragma acc kernels copy(worker, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + i * 11) & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Case 7: fully partitioned */
NOINLINE_USED
int test_fully_partitioned(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i * 13 + j;
    
    /* Fully partitioned: gang, worker, and vector */
    #pragma acc parallel copy(gang, worker, vector: arr[0:N][0:M])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < M; j++) {
                volatile int mask = 0x7F;
                arr[i][j] = (arr[i][j] & mask) | 0x80;
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Mixed OpenACC data region with structured data lifetimes */
NOINLINE_USED
int test_data_region(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    #pragma acc data copy(arr[0:N][0:M])
    {
        /* First compute construct */
        #pragma acc parallel present(arr[0:N][0:M])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < M; j++) {
                    arr[i][j] = i * j * 3;
                }
            }
        }
        
        /* Second compute construct with different partitioning */
        #pragma acc kernels present(arr[0:N][0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    arr[i][j] += 1;
                }
            }
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* OpenMP offloading variant to engage broader compiler infrastructure */
NOINLINE_USED
int test_omp_offload(int N, int M) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            arr[i][j] = i * 17 + j * 19;
    
    /* OpenMP target with teams and distribute */
    #pragma omp target map(tofrom: arr[0:N][0:M])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            volatile int mod = 23;
            arr[i][j] = (arr[i][j] % mod) + i;
        }
    }
    
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            sum = (sum + arr[i][j]) & 0xFFF;
    
    return sum;
}

/* Function using unstructured data directives */
NOINLINE_USED
int test_unstructured_data(int N, int M) {
    int *arr = (int*)malloc(N * M * sizeof(int));
    int sum = 0;
    
    if (!arr) return 0;
    
    for (int i = 0; i < N * M; i++)
        arr[i] = i * 29;
    
    /* Unstructured data directives */
    #pragma acc enter data copyin(arr[0:N*M])
    
    #pragma acc parallel present(arr[0:N*M])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N * M; i++) {
            volatile int inc = 31;
            arr[i] += inc;
        }
    }
    
    #pragma acc exit data copyout(arr[0:N*M])
    
    for (int i = 0; i < N * M; i++)
        sum = (sum + arr[i]) & 0xFFF;
    
    free(arr);
    return sum;
}

/* Main function with conditional execution to create control flow variability */
int main(int argc, char *argv[]) {
    int N = v_N;
    int M = v_M;
    int checksum = 0;
    
    /* Use command-line argument to create conditional paths */
    int use_all = 1;
    if (argc > 1) {
        use_all = atoi(argv[1]) & 1;
    }
    
    /* Always execute base cases */
    checksum ^= test_gang_redundant(N, M);
    checksum ^= test_gang_partitioned(N, M);
    checksum ^= test_worker_partitioned(N, M);
    checksum ^= test_gang_worker_partitioned(N, M);
    
    /* Conditionally execute remaining cases */
    if (use_all) {
        checksum ^= test_vector_partitioned(N, M);
        checksum ^= test_gang_vector_partitioned(N, M);
        checksum ^= test_worker_vector_partitioned(N, M);
        checksum ^= test_fully_partitioned(N, M);
        checksum ^= test_data_region(N, M);
        
        /* Mix with OpenMP offloading */
        checksum ^= test_omp_offload(N, M);
        
        /* Unstructured data test */
        checksum ^= test_unstructured_data(N / 2, M / 2);
    } else {
        /* Alternative path with different combinations */
        volatile int alt_N = N / 2;
        volatile int alt_M = M / 2;
        
        #pragma acc parallel copy(worker, vector: arr[0:alt_N][0:alt_M])
        {
            /* Dummy region to ensure compilation */
        }
    }
    
    /* Final checksum output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
