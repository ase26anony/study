/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * The test exercises all 8 OpenACC data partitioning scenarios (codes 0-7)
 * to trigger coverage in omp-oacc-neuter-broadcast.cc lines 335-343.
 */

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

/* Global arrays to work with */
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes with attributes to prevent optimization */
int test_gang_redundant(int n, int m) __attribute__((noinline,used));
int test_gang_partitioned(int n, int m) __attribute__((noinline,used));
int test_worker_partitioned(int n, int m) __attribute__((noinline,used));
int test_gang_worker_partitioned(int n, int m) __attribute__((noinline,used));
int test_vector_partitioned(int n, int m) __attribute__((noinline,used));
int test_gang_vector_partitioned(int n, int m) __attribute__((noinline,used));
int test_worker_vector_partitioned(int n, int m) __attribute__((noinline,used));
int test_fully_partitioned(int n, int m) __attribute__((noinline,used));
int test_openmp_offload(int n, int m) __attribute__((noinline,used));
int test_unstructured_data(int n, int m) __attribute__((noinline,used));
int test_mixed_data_regions(int n, int m, int p) __attribute__((noinline,used));

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i * M + j;
            arr2[i][j] = 0;
        }
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
}

/* Case 0: gang redundant (default) */
int test_gang_redundant(int n, int m) {
    int sum = 0;
    /* Use runtime-determined slice sizes */
    #pragma acc parallel copy(arr1[0:n][0:m]) copyout(arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 2;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
int test_gang_partitioned(int n, int m) {
    int sum = 0;
    #pragma acc parallel copy(gang: arr1[0:n][0:m]) copyout(gang: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + i;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
int test_worker_partitioned(int n, int m) {
    int sum = 0;
    #pragma acc parallel copy(worker: arr1[0:n][0:m]) copyout(worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
int test_gang_worker_partitioned(int n, int m) {
    int sum = 0;
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m]) copyout(gang, worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 3;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
int test_vector_partitioned(int n, int m) {
    int sum = 0;
    #pragma acc parallel copy(vector: arr1[0:n][0:m]) copyout(vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] - j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
int test_gang_vector_partitioned(int n, int m) {
    int sum = 0;
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m]) copyout(gang, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] | 0x1;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
int test_worker_vector_partitioned(int n, int m) {
    int sum = 0;
    #pragma acc parallel copy(worker, vector: arr1[0:n][0:m]) copyout(worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] & 0xFF;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
int test_fully_partitioned(int n, int m) {
    int sum = 0;
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m]) copyout(gang, worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] ^ 0x55;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with OpenMP offloading to engage broader infrastructure */
int test_openmp_offload(int n, int m) {
    int sum = 0;
    #pragma omp target map(tofrom: arr1[0:n][0:m]) map(from: arr2[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr2[i][j] = arr1[i][j] * 4;
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test unstructured data regions with runtime library calls */
int test_unstructured_data(int n, int m) {
    int sum = 0;
    int *dev_arr1, *dev_arr2;
    size_t size = n * m * sizeof(int);
    
    /* Use OpenACC runtime calls for unstructured data */
    dev_arr1 = (int *)acc_create(arr1, size);
    dev_arr2 = (int *)acc_create(arr2, size);
    
    #pragma acc parallel present(dev_arr1[0:n*m], dev_arr2[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n * m; i++) {
            dev_arr2[i] = dev_arr1[i] + 100;
        }
    }
    
    acc_copyout(arr2, size);
    acc_delete(arr1, size);
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test mixed structured data regions with conditional execution */
int test_mixed_data_regions(int n, int m, int p) {
    int sum = 0;
    volatile int flag = 1; /* Prevent dead code elimination */
    
    /* Structured data region */
    #pragma acc data copy(arr3[0:n][0:m][0:p])
    {
        /* First compute construct - always execute */
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector collapse(3)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < p; k++) {
                        arr3[i][j][k] += 1;
                    }
                }
            }
        }
        
        /* Second compute construct - conditional execution */
        if (flag) {
            #pragma acc kernels copy(arr3[0:n][0:m][0:p])
            {
                #pragma acc loop independent gang
                for (int i = 0; i < n; i++) {
                    #pragma acc loop independent worker
                    for (int j = 0; j < m; j++) {
                        #pragma acc loop independent vector
                        for (int k = 0; k < p; k++) {
                            arr3[i][j][k] *= 2;
                        }
                    }
                }
            }
        }
    }
    
    /* Compute checksum from 3D array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                sum += arr3[i][j][k];
            }
        }
    }
    return sum & 0xFF;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Initialize arrays with known values */
    init_arrays();
    
    /* Use volatile variables as bounds to prevent constant propagation */
    int n = vN > 100 ? 100 : vN;
    int m = vM > 50 ? 50 : vM;
    int p = vP > 20 ? 20 : vP;
    
    /* Execute all test functions to trigger all partition codes */
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
    checksum ^= test_unstructured_data(n, m);
    checksum ^= test_mixed_data_regions(n, m, p);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}
