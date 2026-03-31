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

/* Global arrays for data sharing between functions */
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes for each partition case */
int case_0_gang_redundant(int n, int m);
int case_1_gang_partitioned(int n, int m);
int case_2_worker_partitioned(int n, int m);
int case_3_gang_worker_partitioned(int n, int m);
int case_4_vector_partitioned(int n, int m);
int case_5_gang_vector_partitioned(int n, int m);
int case_6_worker_vector_partitioned(int n, int m);
int case_7_fully_partitioned(int n, int m);

/* OpenMP target function to engage broader offloading infrastructure */
int omp_target_function(int n, int m);

/* Functions using unstructured data directives */
int unstructured_data_region(int n, int m);
int mixed_data_clauses(int n, int m, int p);

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i + j;
            arr2[i][j] = i * j;
        }
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i + j + k;
            }
        }
    }
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int case_0_gang_redundant(int n, int m) {
    int sum = 0;
    /* Use runtime-determined slice sizes */
    #pragma acc parallel copy(arr1[0:n][0:m]) copyout(sum)
    {
        #pragma acc loop reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = arr1[i][j] * 2;
                sum += arr1[i][j];
            }
        }
    }
    return sum & 0xFF; /* Return checksum */
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int case_1_gang_partitioned(int n, int m) {
    int sum = 0;
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr1[0:n][0:m]) copyout(sum)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr1[i][j] = arr1[i][j] + 1;
                sum += arr1[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int case_2_worker_partitioned(int n, int m) {
    int sum = 0;
    /* Worker partitioning */
    #pragma acc parallel copy(worker: arr2[0:n][0:m]) copyout(sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker reduction(+:sum)
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr2[i][j] - i;
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int case_3_gang_worker_partitioned(int n, int m) {
    int sum = 0;
    /* Combined gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m], arr2[0:n][0:m]) copyout(sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker reduction(+:sum)
            for (int j = 0; j < m; j++) {
                arr1[i][j] = arr1[i][j] + arr2[i][j];
                sum += arr1[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int case_4_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Vector partitioning */
    #pragma acc parallel copy(vector: arr1[0:n][0:m]) copyout(sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < 8; k++) { /* Small inner loop for vector */
                    int idx = (j + k) % m;
                    arr1[i][idx] = arr1[i][idx] * 3;
                    sum += arr1[i][idx];
                }
            }
        }
    }
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int case_5_gang_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr2[0:n][0:m]) copyout(sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector reduction(+:sum)
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr2[i][j] / 2;
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int case_6_worker_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Worker and vector partitioning */
    #pragma acc parallel copy(worker, vector: arr1[0:n][0:m]) copyout(sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < 4; k++) {
                    arr1[i][j] = arr1[i][j] + k;
                    sum += arr1[i][j];
                }
            }
        }
    }
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int case_7_fully_partitioned(int n, int m) {
    int sum = 0;
    /* Full gang, worker, vector partitioning */
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m], arr2[0:n][0:m]) copyout(sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < 2; k++) {
                    arr1[i][j] = arr1[i][j] ^ arr2[i][j];
                    sum += arr1[i][j];
                }
            }
        }
    }
    return sum & 0xFF;
}

/* OpenMP target region to engage both offloading implementations */
__attribute__((noinline, used))
int omp_target_function(int n, int m) {
    int sum = 0;
    /* Use distribute and teams for gang-like partitioning */
    #pragma omp target map(tofrom: arr1[0:n][0:m]) map(from: sum)
    #pragma omp teams distribute parallel for reduction(+:sum) collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr1[i][j] = arr1[i][j] * 2;
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Function using structured data region with multiple compute constructs */
__attribute__((noinline, used))
int structured_data_region(int n, int m) {
    int sum1 = 0, sum2 = 0;
    
    #pragma acc data copy(arr1[0:n][0:m], arr2[0:n][0:m])
    {
        /* First compute construct - gang partitioned */
        #pragma acc parallel copy(gang: arr1[0:n][0:m]) present(arr2[0:n][0:m])
        {
            #pragma acc loop gang reduction(+:sum1)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = arr1[i][j] + 5;
                    sum1 += arr1[i][j];
                }
            }
        }
        
        /* Second compute construct - worker partitioned */
        #pragma acc parallel copy(worker: arr2[0:n][0:m]) present(arr1[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker reduction(+:sum2)
                for (int j = 0; j < m; j++) {
                    arr2[i][j] = arr2[i][j] - 3;
                    sum2 += arr2[i][j];
                }
            }
        }
    }
    return (sum1 + sum2) & 0xFF;
}

/* Function with conditional parallel regions based on volatile input */
__attribute__((noinline, used))
int conditional_parallel_regions(int n, int m, volatile int flag) {
    int sum = 0;
    
    if (flag & 0x1) {
        /* This region should be neutered when flag doesn't have bit 0 set */
        #pragma acc parallel copy(gang, worker: arr1[0:n][0:m]) copyout(sum)
        {
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = arr1[i][j] * 2;
                    sum += arr1[i][j];
                }
            }
        }
    }
    
    if (flag & 0x2) {
        /* Different partitioning for different condition */
        #pragma acc kernels copy(vector: arr2[0:n][0:m]) copy(sum)
        {
            #pragma acc loop independent reduction(+:sum)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr2[i][j] = arr2[i][j] + 10;
                    sum += arr2[i][j];
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Main function that calls all test cases */
int main(int argc, char *argv[]) {
    int checksum = 0;
    volatile int flag = 0;
    
    /* Use command line argument to vary control flow */
    if (argc > 1) {
        flag = atoi(argv[1]);
    } else {
        flag = 3; /* Default: execute both conditional paths */
    }
    
    init_arrays();
    
    /* Call all 8 partition cases */
    checksum ^= case_0_gang_redundant(v_N, v_M);
    checksum ^= case_1_gang_partitioned(v_N, v_M);
    checksum ^= case_2_worker_partitioned(v_N, v_M);
    checksum ^= case_3_gang_worker_partitioned(v_N, v_M);
    checksum ^= case_4_vector_partitioned(v_N, v_M);
    checksum ^= case_5_gang_vector_partitioned(v_N, v_M);
    checksum ^= case_6_worker_vector_partitioned(v_N, v_M);
    checksum ^= case_7_fully_partitioned(v_N, v_M);
    
    /* Engage OpenMP offloading */
    checksum ^= omp_target_function(v_N, v_M);
    
    /* Structured data region with multiple constructs */
    checksum ^= structured_data_region(v_N, v_M);
    
    /* Conditional parallel regions */
    checksum ^= conditional_parallel_regions(v_N, v_M, flag);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
