/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning cases
 * (gang redundant, gang partitioned, worker partitioned, etc.)
 * to trigger coverage of the partition code to string mapping
 * function in omp-oacc-neuter-broadcast.cc lines 335-343.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Volatile variables to prevent constant propagation */
volatile int vn = N;
volatile int vm = M;
volatile int vp = P;

/* Helper to ensure functions aren't optimized away */
#define NOINLINE_USED __attribute__((noinline, used))

/* Case 0: gang redundant (default) */
NOINLINE_USED
int test_gang_redundant(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    /* Use runtime-determined slice to prevent static optimization */
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(arr[0:slice_n][0:slice_m])
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 100 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
NOINLINE_USED
int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(gang: arr[0:slice_n][0:slice_m])
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 200 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
NOINLINE_USED
int test_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(worker: arr[0:slice_n][0:slice_m])
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 300 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
NOINLINE_USED
int test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(gang, worker: arr[0:slice_n][0:slice_m])
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 400 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
NOINLINE_USED
int test_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(vector: arr[0:slice_n][0:slice_m])
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 500 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
NOINLINE_USED
int test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(gang, vector: arr[0:slice_n][0:slice_m])
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 600 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
NOINLINE_USED
int test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(worker, vector: arr[0:slice_n][0:slice_m])
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 700 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
NOINLINE_USED
int test_fully_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:slice_n][0:slice_m])
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 800 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Test with kernels construct for variety */
NOINLINE_USED
int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc kernels copy(gang, worker: arr[0:slice_n][0:slice_m])
    {
        for (int i = 0; i < slice_n; i++) {
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 900 + j;
                sum += arr[i][j] & 1;
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with data region containing multiple compute constructs */
NOINLINE_USED
int test_data_region(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc data copy(arr[0:slice_n][0:slice_m])
    {
        #pragma acc parallel
        for (int i = 0; i < slice_n; i++) {
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 1000 + j;
            }
        }
        
        #pragma acc parallel
        for (int i = 0; i < slice_n; i++) {
            for (int j = 0; j < slice_m; j++) {
                sum += arr[i][j] & 1;
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with unstructured data directives */
NOINLINE_USED
int test_unstructured_data(int n, int m) {
    int *arr;
    int size = n * m;
    int sum = 0;
    
    if (size > 0) {
        arr = (int*)malloc(size * sizeof(int));
        
        #pragma acc enter data create(arr[0:size])
        
        #pragma acc parallel present(arr[0:size])
        for (int i = 0; i < size; i++) {
            arr[i] = i * 1100;
        }
        
        #pragma acc parallel present(arr[0:size])
        for (int i = 0; i < size; i++) {
            sum += arr[i] & 1;
        }
        
        #pragma acc exit data delete(arr[0:size])
        
        free(arr);
    }
    
    return sum & 0xFF;
}

/* Test with OpenMP target offloading */
NOINLINE_USED
int test_omp_target(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma omp target map(tofrom: arr[0:slice_n][0:slice_m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            arr[i][j] = i * 1200 + j;
            sum += arr[i][j] & 1;
        }
    }
    
    return sum & 0xFF;
}

/* Conditional execution to force neutering analysis */
NOINLINE_USED
int test_conditional(int n, int m, int flag) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    if (flag & 1) {
        #pragma acc parallel copy(gang: arr[0:slice_n][0:slice_m])
        for (int i = 0; i < slice_n; i++) {
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 1300 + j;
                sum += arr[i][j] & 1;
            }
        }
    } else {
        #pragma acc parallel copy(worker: arr[0:slice_n][0:slice_m])
        for (int i = 0; i < slice_n; i++) {
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 1400 + j;
                sum += arr[i][j] & 1;
            }
        }
    }
    
    return sum & 0xFF;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Use command line args for runtime variability */
    int flag = argc > 1 ? atoi(argv[1]) : 1;
    
    /* Execute all test cases to trigger all partition codes */
    checksum += test_gang_redundant(vn, vm);
    checksum += test_gang_partitioned(vn, vm);
    checksum += test_worker_partitioned(vn, vm);
    checksum += test_gang_worker_partitioned(vn, vm);
    checksum += test_vector_partitioned(vn, vm);
    checksum += test_gang_vector_partitioned(vn, vm);
    checksum += test_worker_vector_partitioned(vn, vm);
    checksum += test_fully_partitioned(vn, vm);
    
    /* Additional tests for broader coverage */
    checksum += test_kernels_partition(vn, vm);
    checksum += test_data_region(vn, vm);
    checksum += test_unstructured_data(vn, vm);
    checksum += test_omp_target(vn, vm);
    checksum += test_conditional(vn, vm, flag);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
