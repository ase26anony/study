/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning scenarios
 * (gang redundant through fully partitioned) to trigger coverage
 * of the partition code to string mapping function in GCC's
 * omp-oacc-neuter-broadcast.cc.
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

/* Global arrays for data region testing */
int global_arr_2d[100][100];
int global_arr_3d[50][50][50];

/* Function prototypes */
int test_gang_redundant(int n, int m);
int test_gang_partitioned(int n, int m);
int test_worker_partitioned(int n, int m);
int test_gang_worker_partitioned(int n, int m);
int test_vector_partitioned(int n, int m);
int test_gang_vector_partitioned(int n, int m);
int test_worker_vector_partitioned(int n, int m);
int test_fully_partitioned(int n, int m);
int test_openmp_offload(int n, int m);
int test_data_region(int n, int m);
int test_unstructured_data(int n, int m);
int test_mixed_constructs(int n, int m, int p, int use_acc);

/* Test functions for each partition case */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Case 0: gang redundant (default) */
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] += 1;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] += 2;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Case 2: worker partitioned */
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] += 3;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] += 4;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] += 5;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] += 6;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] += 7;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_fully_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Case 7: fully partitioned */
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] += 8;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
int test_openmp_offload(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* OpenMP target with distribute/teams/parallel clauses */
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] += 9;
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test structured data region */
__attribute__((noinline, used))
int test_data_region(int n, int m) {
    int sum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            global_arr_2d[i][j] = i * 100 + j;
        }
    }
    
    /* Structured data region with multiple compute constructs */
    #pragma acc data copy(global_arr_2d[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    global_arr_2d[i][j] += 10;
                }
            }
        }
        
        /* Second compute construct in same data region */
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    global_arr_2d[i][j] += 1;
                }
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += global_arr_2d[i][j];
        }
    }
    
    return sum & 0xFF;
}

/* Test unstructured data directives */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr;
    int sum = 0;
    
    arr = (int*)malloc(n * m * sizeof(int));
    if (!arr) return 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i * m + j] = i * 100 + j;
        }
    }
    
    /* Unstructured data directives */
    #pragma acc enter data copyin(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i * m + j] += 11;
            }
        }
    }
    
    #pragma acc exit data copyout(arr[0:n*m])
    
    /* Compute checksum */
    for (int i = 0; i < n * m; i++) {
        sum += arr[i];
    }
    
    free(arr);
    return sum & 0xFF;
}

/* Test mixed constructs with conditional execution */
__attribute__((noinline, used))
int test_mixed_constructs(int n, int m, int p, int use_acc) {
    int sum = 0;
    
    /* Initialize 3D array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                global_arr_3d[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    /* Conditional execution based on volatile variable */
    if (use_acc) {
        /* OpenACC kernels construct with gang/worker/vector clauses */
        #pragma acc kernels copy(global_arr_3d[0:n][0:m][0:p])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < p; k++) {
                        global_arr_3d[i][j][k] += 12;
                    }
                }
            }
        }
    } else {
        /* OpenMP target teams distribute parallel for */
        #pragma omp target teams distribute parallel for collapse(3) \
                     map(tofrom: global_arr_3d[0:n][0:m][0:p])
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    global_arr_3d[i][j][k] += 13;
                }
            }
        }
    }
    
    /* Another conditional region */
    if (n > 10) {
        #pragma acc parallel copy(global_arr_3d[0:n][0:m][0:p])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < p; k++) {
                        global_arr_3d[i][j][k] += 1;
                    }
                }
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                sum += global_arr_3d[i][j][k];
            }
        }
    }
    
    return sum & 0xFF;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Use command-line arguments for variability */
    int use_acc = (argc > 1) ? atoi(argv[1]) : 1;
    int n = v_N;
    int m = v_M;
    int p = v_P;
    
    printf("Testing OpenACC/OpenMP offload partition coverage...\n");
    
    /* Execute all test functions to trigger partition code generation */
    checksum ^= test_gang_redundant(n, m);
    checksum ^= test_gang_partitioned(n, m);
    checksum ^= test_worker_partitioned(n, m);
    checksum ^= test_gang_worker_partitioned(n, m);
    checksum ^= test_vector_partitioned(n, m);
    checksum ^= test_gang_vector_partitioned(n, m);
    checksum ^= test_worker_vector_partitioned(n, m);
    checksum ^= test_fully_partitioned(n, m);
    
    /* Additional tests to engage broader compiler infrastructure */
    checksum ^= test_openmp_offload(n, m);
    checksum ^= test_data_region(n, m);
    checksum ^= test_unstructured_data(n, m);
    checksum ^= test_mixed_constructs(n/2, m/2, p/2, use_acc);
    
    /* Final checksum output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
