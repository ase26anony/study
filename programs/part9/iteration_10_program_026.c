/* test_offload_partition.c
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 128
#define M 64
#define P 32

/* Volatile variables to prevent constant propagation */
volatile int vN = N;
volatile int vM = M;
volatile int vP = P;

/* Helper to generate side effects */
static int __attribute__((noinline)) update_checksum(int *arr, int size, int base) {
    int sum = base;
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2 + base;
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Case 0: gang redundant */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j;
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

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 1;
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
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 2;
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
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 3;
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
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 4;
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
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 5;
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
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 6;
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
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 7;
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

/* Test with kernels construct (different code path) */
__attribute__((noinline, used))
int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 8;
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
int test_data_region(int n, int m, int p) {
    int arr[N][M][P];
    int sum = 0;
    
    #pragma acc data copy(arr[0:n][0:m][0:p])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < p; k++) {
                        arr[i][j][k] = i * m * p + j * p + k;
                    }
                }
            }
        }
        
        #pragma acc kernels
        {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < p; k++) {
                        arr[i][j][k] += 1;
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                sum += arr[i][j][k];
            }
        }
    }
    return sum & 0xFF;
}

/* Test with unstructured data using runtime API */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * m * sizeof(int));
    int sum = 0;
    
    #pragma acc enter data create(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i * m + j] = i * m + j + 9;
            }
        }
    }
    
    #pragma acc update host(arr[0:n*m])
    
    for (int i = 0; i < n * m; i++) {
        sum += arr[i];
    }
    
    #pragma acc exit data delete(arr)
    free(arr);
    return sum & 0xFF;
}

/* Test with OpenMP offloading (shares infrastructure) */
__attribute__((noinline, used))
int test_omp_offload(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute
        for (int i = 0; i < n; i++) {
            #pragma omp parallel for
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * m + j + 10;
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

/* Conditional execution to force neutering analysis */
__attribute__((noinline, used))
int test_conditional(int n, int m, int flag) {
    int arr[N][M];
    int sum = 0;
    
    if (flag & 1) {
        #pragma acc parallel copy(arr[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * m + j + 11;
                }
            }
        }
    } else {
        #pragma acc kernels copy(arr[0:n][0:m])
        {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * m + j + 12;
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

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Use volatile to prevent constant folding */
    volatile int flag = (argc > 1) ? atoi(argv[1]) : 1;
    
    /* Execute all test cases */
    checksum ^= test_gang_redundant(vN, vM);
    checksum ^= test_gang_partitioned(vN, vM);
    checksum ^= test_worker_partitioned(vN, vM);
    checksum ^= test_gang_worker_partitioned(vN, vM);
    checksum ^= test_vector_partitioned(vN, vM);
    checksum ^= test_gang_vector_partitioned(vN, vM);
    checksum ^= test_worker_vector_partitioned(vN, vM);
    checksum ^= test_fully_partitioned(vN, vM);
    
    checksum ^= test_kernels_partition(vN, vM);
    checksum ^= test_data_region(vN/2, vM/2, vP);
    checksum ^= test_unstructured_data(vN/4, vM/4);
    checksum ^= test_omp_offload(vN, vM);
    checksum ^= test_conditional(vN, vM, flag);
    
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
