/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning cases
 * (codes 0-7) to trigger coverage in omp-oacc-neuter-broadcast.cc
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

/* Global arrays for data sharing between functions */
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes with attributes to prevent optimization */
__attribute__((noinline,used))
int test_case_0_gang_redundant(int n, int m);
__attribute__((noinline,used))
int test_case_1_gang_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_2_worker_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_3_gang_worker_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_4_vector_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_5_gang_vector_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_6_worker_vector_partitioned(int n, int m);
__attribute__((noinline,used))
int test_case_7_fully_partitioned(int n, int m);
__attribute__((noinline,used))
void test_openmp_offload(int n, int m, int p);
__attribute__((noinline,used))
void test_acc_data_region(int n, int m);
__attribute__((noinline,used))
void test_unstructured_data(int n, int m);

/* Case 0: gang redundant (default) */
int test_case_0_gang_redundant(int n, int m) {
    int sum = 0;
    /* Use runtime-determined slice sizes */
    #pragma acc parallel copy(arr1[0:n][0:m]) copyout(arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 2;
                if (i == j) sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF; /* Return checksum */
}

/* Case 1: gang partitioned */
int test_case_1_gang_partitioned(int n, int m) {
    int sum = 0;
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr1[0:n][0:m]) copyout(gang: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + i;
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
int test_case_2_worker_partitioned(int n, int m) {
    int sum = 0;
    /* Explicit worker partitioning */
    #pragma acc parallel copy(worker: arr1[0:n][0:m]) copyout(worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + j;
                sum ^= arr2[i][j]; /* Different computation */
            }
        }
    }
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
int test_case_3_gang_worker_partitioned(int n, int m) {
    int sum = 0;
    /* Combined gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m]) create(gang, worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * arr1[i][j];
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
int test_case_4_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Vector partitioning only */
    #pragma acc parallel copy(vector: arr1[0:n][0:m]) copyout(vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] - (i + j);
                sum |= arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
int test_case_5_gang_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Gang and vector partitioning */
    #pragma acc kernels copy(gang, vector: arr1[0:n][0:m]) present(gang, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] / ((i + j) % 8 + 1);
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
int test_case_6_worker_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Worker and vector partitioning */
    #pragma acc parallel copy(worker, vector: arr1[0:n][0:m]) copyout(worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = (arr1[i][j] << 2) | (i & 3);
                sum ^= arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
int test_case_7_fully_partitioned(int n, int m) {
    int sum = 0;
    /* All three levels partitioned */
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m]) \
                         copyout(gang, worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 3 + i - j;
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Test OpenMP offloading to engage broader infrastructure */
void test_openmp_offload(int n, int m, int p) {
    int sum = 0;
    
    /* Conditional execution to create control flow variability */
    if (vN > 0) {
        #pragma omp target map(tofrom: arr3[0:n][0:m][0:p]) map(to: sum)
        {
            #pragma omp teams distribute parallel for collapse(3)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < p; k++) {
                        arr3[i][j][k] = i * 10000 + j * 100 + k;
                        sum += arr3[i][j][k];
                    }
                }
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    arr1[0][0] = sum & 0xFF;
}

/* Test structured data region with multiple compute constructs */
void test_acc_data_region(int n, int m) {
    #pragma acc data copy(arr1[0:n][0:m]) create(arr2[0:n][0:m])
    {
        /* First compute construct */
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr2[i][j] = arr1[i][j] + 1;
                }
            }
        }
        
        /* Second compute construct with different partitioning */
        #pragma acc parallel copy(gang: arr2[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = arr2[i][j] * 2;
                }
            }
        }
    }
}

/* Test unstructured data directives */
void test_unstructured_data(int n, int m) {
    int *d_arr1, *d_arr2;
    size_t size = n * m * sizeof(int);
    
    /* Allocate device memory */
    d_arr1 = (int*)acc_malloc(size);
    d_arr2 = (int*)acc_malloc(size);
    
    if (d_arr1 && d_arr2) {
        /* Copy data to device */
        acc_memcpy_to_device(d_arr1, arr1, size);
        
        /* Compute on device */
        #pragma acc parallel present(d_arr1[0:n*m], d_arr2[0:n*m])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n*m; i++) {
                d_arr2[i] = d_arr1[i] * 3;
            }
        }
        
        /* Copy result back */
        acc_memcpy_from_device(arr2, d_arr2, size);
        
        /* Free device memory */
        acc_free(d_arr1);
        acc_free(d_arr2);
    }
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (i * 17 + j * 13) % 100;
            arr2[i][j] = 0;
        }
    }
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = 0;
            }
        }
    }
    
    /* Execute all 8 test cases with volatile bounds */
    checksum ^= test_case_0_gang_redundant(vN, vM);
    checksum ^= test_case_1_gang_partitioned(vN, vM);
    checksum ^= test_case_2_worker_partitioned(vN, vM);
    checksum ^= test_case_3_gang_worker_partitioned(vN, vM);
    checksum ^= test_case_4_vector_partitioned(vN, vM);
    checksum ^= test_case_5_gang_vector_partitioned(vN, vM);
    checksum ^= test_case_6_worker_vector_partitioned(vN, vM);
    checksum ^= test_case_7_fully_partitioned(vN, vM);
    
    /* Conditional execution based on command line argument */
    if (argc > 1) {
        test_openmp_offload(vN/2, vM/2, vP);
    }
    
    /* Always execute these to ensure coverage */
    test_acc_data_region(vN/4, vM/4);
    test_unstructured_data(vN/8, vM/8);
    
    /* Final checksum to prevent optimization */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
