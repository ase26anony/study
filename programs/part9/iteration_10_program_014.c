/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning cases
 * to cover the string mapping function in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 128
#define DIM 64

/* Volatile variables to prevent constant propagation */
volatile int v_size = SIZE;
volatile int v_dim = DIM;

/* Helper to ensure side effects */
static int __attribute__((noinline)) use_value(int x) {
    volatile int dummy = x;
    return dummy;
}

/* Case 0: gang redundant */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    /* Use runtime values for array sections */
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
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
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
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 300 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 400 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 500 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 600 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 700 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
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
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with kernels construct for variety */
__attribute__((noinline, used))
int test_kernels_partition(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    /* Mix of partition clauses in kernels */
    #pragma acc kernels copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 900 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with data region containing multiple constructs */
__attribute__((noinline, used))
int test_data_region(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    #pragma acc data copy(arr[0:n][0:m])
    {
        /* Nested conditional to force neutering analysis */
        if (n > 10) {
            #pragma acc parallel copy(arr[0:n][0:m])
            {
                #pragma acc loop
                for (int i = 0; i < n; i++) {
                    #pragma acc loop
                    for (int j = 0; j < m; j++) {
                        arr[i][j] = i * 1000 + j;
                    }
                }
            }
        }
        
        #pragma acc kernels copy(arr[0:n][0:m])
        {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr[i][j] += 1;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* OpenMP offloading test to engage broader infrastructure */
__attribute__((noinline, used))
int test_omp_offload(int n, int m) {
    int arr[SIZE][DIM];
    int sum = 0;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 1100 + j;
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with unstructured data directives */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr = (int*)malloc(n * m * sizeof(int));
    int sum = 0;
    
    /* Create device data */
    #pragma acc enter data create(arr[0:n*m])
    
    /* Initialize on device */
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop
        for (int i = 0; i < n*m; i++) {
            arr[i] = i * 2;
        }
    }
    
    /* Copy back and compute checksum */
    #pragma acc update host(arr[0:n*m])
    
    for (int i = 0; i < n*m; i++) {
        sum += arr[i];
    }
    
    #pragma acc exit data delete(arr)
    free(arr);
    
    return sum & 0xFF;
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int checksum = 0;
    int n = v_size;
    int m = v_dim;
    
    /* Use command line or volatile to control flow */
    int use_all = 1;
    if (argc > 1) {
        use_all = atoi(argv[1]);
    }
    
    /* Always execute base cases */
    checksum ^= test_gang_redundant(n, m);
    checksum ^= test_gang_partitioned(n, m);
    checksum ^= test_worker_partitioned(n, m);
    checksum ^= test_gang_worker_partitioned(n, m);
    checksum ^= test_vector_partitioned(n, m);
    checksum ^= test_gang_vector_partitioned(n, m);
    checksum ^= test_worker_vector_partitioned(n, m);
    checksum ^= test_fully_partitioned(n, m);
    
    /* Conditional execution to force compiler analysis */
    if (use_all > 0) {
        checksum ^= test_kernels_partition(n, m);
        checksum ^= test_data_region(n, m);
        
        /* Nested condition with volatile */
        volatile int flag = 1;
        if (flag) {
            checksum ^= test_omp_offload(n/2, m/2);
        }
        
        checksum ^= test_unstructured_data(n/4, m/4);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
