/* This test requires GCC configured with offloading support. 
   Compile with: gcc -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage test_offload_partition.c -o test_offload_partition_executable
   Run with: ./test_offload_partition_executable
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 25

/* Volatile variables to prevent constant propagation */
volatile int v_N = N;
volatile int v_M = M;
volatile int v_P = P;

/* Attribute to prevent inlining and ensure functions are generated */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function prototypes for each partition case */
int case_0_gang_redundant(int n, int m);
int case_1_gang_partitioned(int n, int m);
int case_2_worker_partitioned(int n, int m);
int case_3_gang_worker_partitioned(int n, int m);
int case_4_vector_partitioned(int n, int m);
int case_5_gang_vector_partitioned(int n, int m);
int case_6_worker_vector_partitioned(int n, int m);
int case_7_fully_partitioned(int n, int m);
int openmp_target_test(int n, int m);
int unstructured_data_test(int n, int m);
int nested_conditional_test(int n, int m, int flag);

/* Main function that calls all test cases */
int main(int argc, char **argv) {
    int checksum = 0;
    int flag = (argc > 1) ? atoi(argv[1]) : 1;
    
    printf("Testing OpenACC/OpenMP partition code coverage...\n");
    
    /* Test all 8 OpenACC partition cases */
    checksum ^= case_0_gang_redundant(v_N, v_M);
    checksum ^= case_1_gang_partitioned(v_N, v_M);
    checksum ^= case_2_worker_partitioned(v_N, v_M);
    checksum ^= case_3_gang_worker_partitioned(v_N, v_M);
    checksum ^= case_4_vector_partitioned(v_N, v_M);
    checksum ^= case_5_gang_vector_partitioned(v_N, v_M);
    checksum ^= case_6_worker_vector_partitioned(v_N, v_M);
    checksum ^= case_7_fully_partitioned(v_N, v_M);
    
    /* Test OpenMP offloading */
    checksum ^= openmp_target_test(v_N, v_M);
    
    /* Test unstructured data regions */
    checksum ^= unstructured_data_test(v_N, v_M);
    
    /* Test nested conditional constructs */
    checksum ^= nested_conditional_test(v_N, v_M, flag);
    
    printf("Final checksum: %d\n", checksum & 0xFF);
    return 0;
}

/* Case 0: gang redundant (default) */
NOINLINE_USED
int case_0_gang_redundant(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Case 1: gang partitioned */
NOINLINE_USED
int case_1_gang_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 200 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Case 2: worker partitioned */
NOINLINE_USED
int case_2_worker_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 300 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Case 3: gang+worker partitioned */
NOINLINE_USED
int case_3_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 400 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Case 4: vector partitioned */
NOINLINE_USED
int case_4_vector_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 500 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Case 5: gang+vector partitioned */
NOINLINE_USED
int case_5_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 600 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Case 6: worker+vector partitioned */
NOINLINE_USED
int case_6_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 700 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Case 7: fully partitioned */
NOINLINE_USED
int case_7_fully_partitioned(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 800 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Test with OpenMP target constructs */
NOINLINE_USED
int openmp_target_test(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 900 + j;
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Test unstructured data regions with runtime library calls */
NOINLINE_USED
int unstructured_data_test(int n, int m) {
    int arr[N][M];
    int result = 0;
    
    /* Structured data region */
    #pragma acc data copy(arr[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1000 + j;
                }
            }
        }
        
        /* Another compute construct in same data region */
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    arr[i][j] += 1;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}

/* Test nested conditional constructs */
NOINLINE_USED
int nested_conditional_test(int n, int m, int flag) {
    int arr[N][M];
    int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = 0;
        }
    }
    
    /* Conditional parallel region - forces neutering analysis */
    if (flag & 1) {
        #pragma acc parallel copy(arr[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    arr[i][j] += i * 1100 + j;
                }
            }
        }
    }
    
    if (flag & 2) {
        #pragma acc kernels copy(arr[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] += i * 1200 + j;
                }
            }
        }
    }
    
    /* Nested conditional with different partition */
    if (flag > 0) {
        #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker
                for (int j = 0; j < m; j++) {
                    arr[i][j] += 1;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            result ^= arr[i][j];
        }
    }
    return result;
}
