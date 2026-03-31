/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in GCC's
 * omp-oacc-neuter-broadcast.cc by creating OpenACC compute constructs
 * with different gang/worker/vector data clause combinations.
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
int arr4[N][M];

/* Function prototypes */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m);
__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m);
__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m);
__attribute__((noinline, used))
void test_openmp_offload(int n, int m, int p);
__attribute__((noinline, used))
void test_unstructured_data(int n, int m);
__attribute__((noinline, used))
void test_structured_data_region(int n, int m);

/* Helper to get runtime dimensions */
static int get_dim(int base, int offset) {
    return base + offset % 4;  /* Non-constant size */
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m) {
    int sum = 0;
    int local_n = get_dim(n, 0);
    int local_m = get_dim(m, 1);
    
    #pragma acc parallel copy(arr1[0:local_n][0:local_m]) copyout(arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = arr1[i][j] * 2;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m) {
    int sum = 0;
    int local_n = get_dim(n, 1);
    int local_m = get_dim(m, 2);
    
    #pragma acc parallel copy(gang: arr1[0:local_n][0:local_m]) \
                         copyout(gang: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = arr1[i][j] + i + j;
            }
        }
    }
    
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m) {
    int sum = 0;
    int local_n = get_dim(n, 2);
    int local_m = get_dim(m, 3);
    
    #pragma acc kernels copy(worker: arr1[0:local_n][0:local_m]) \
                        copyout(worker: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = arr1[i][j] * 3;
            }
        }
    }
    
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m) {
    int sum = 0;
    int local_n = get_dim(n, 3);
    int local_m = get_dim(m, 0);
    
    #pragma acc parallel copy(gang, worker: arr1[0:local_n][0:local_m]) \
                         copyout(gang, worker: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = arr1[i][j] - i + j;
            }
        }
    }
    
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m) {
    int sum = 0;
    int local_n = get_dim(n, 0);
    int local_m = get_dim(m, 1);
    
    #pragma acc kernels copy(vector: arr1[0:local_n][0:local_m]) \
                        copyout(vector: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = arr1[i][j] * 4;
            }
        }
    }
    
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m) {
    int sum = 0;
    int local_n = get_dim(n, 1);
    int local_m = get_dim(m, 2);
    
    #pragma acc parallel copy(gang, vector: arr1[0:local_n][0:local_m]) \
                         copyout(gang, vector: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = arr1[i][j] + i * j;
            }
        }
    }
    
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m) {
    int sum = 0;
    int local_n = get_dim(n, 2);
    int local_m = get_dim(m, 3);
    
    #pragma acc kernels copy(worker, vector: arr1[0:local_n][0:local_m]) \
                        copyout(worker, vector: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = arr1[i][j] * 5;
            }
        }
    }
    
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m) {
    int sum = 0;
    int local_n = get_dim(n, 3);
    int local_m = get_dim(m, 0);
    
    #pragma acc parallel copy(gang, worker, vector: arr1[0:local_n][0:local_m]) \
                         copyout(gang, worker, vector: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = arr1[i][j] + i - j * 2;
            }
        }
    }
    
    for (int i = 0; i < local_n; i++) {
        for (int j = 0; j < local_m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
void test_openmp_offload(int n, int m, int p) {
    int local_n = get_dim(n, 0);
    int local_m = get_dim(m, 1);
    int local_p = get_dim(p, 2);
    
    /* Conditional to create control flow variability */
    if (vN > 100) {
        #pragma omp target map(tofrom: arr3[0:local_n][0:local_m][0:local_p])
        {
            #pragma omp teams distribute parallel for collapse(3)
            for (int i = 0; i < local_n; i++) {
                for (int j = 0; j < local_m; j++) {
                    for (int k = 0; k < local_p; k++) {
                        arr3[i][j][k] = i * 100 + j * 10 + k;
                    }
                }
            }
        }
    }
}

/* Test unstructured data regions with runtime library calls */
__attribute__((noinline, used))
void test_unstructured_data(int n, int m) {
    int local_n = get_dim(n, 1);
    int local_m = get_dim(m, 2);
    int total_size = local_n * local_m;
    
    /* Use OpenACC runtime calls for unstructured data */
    int *dev_ptr = (int *)acc_create(arr4, total_size * sizeof(int));
    
    #pragma acc parallel present(arr4[0:local_n][0:local_m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < local_m; j++) {
                arr4[i][j] = i * j;
            }
        }
    }
    
    acc_copyout(arr4, total_size * sizeof(int));
}

/* Test structured data region with multiple compute constructs */
__attribute__((noinline, used))
void test_structured_data_region(int n, int m) {
    int local_n = get_dim(n, 2);
    int local_m = get_dim(m, 3);
    
    #pragma acc data copy(arr1[0:local_n][0:local_m]) \
                      copyout(arr2[0:local_n][0:local_m])
    {
        /* First compute construct */
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < local_n; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < local_m; j++) {
                    arr2[i][j] = 0;
                }
            }
        }
        
        /* Second compute construct with different partitioning */
        #pragma acc kernels copy(worker: arr1[0:local_n][0:local_m])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < local_n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < local_m; j++) {
                    arr2[i][j] += arr1[i][j];
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = i + j;
            arr2[i][j] = 0;
            arr4[i][j] = 0;
        }
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = 0;
            }
        }
    }
    
    /* Execute all test cases to cover partition codes 0-7 */
    checksum ^= test_case_0_gang_redundant(vN, vM);
    checksum ^= test_case_1_gang_partitioned(vN, vM);
    checksum ^= test_case_2_worker_partitioned(vN, vM);
    checksum ^= test_case_3_gang_worker_partitioned(vN, vM);
    checksum ^= test_case_4_vector_partitioned(vN, vM);
    checksum ^= test_case_5_gang_vector_partitioned(vN, vM);
    checksum ^= test_case_6_worker_vector_partitioned(vN, vM);
    checksum ^= test_case_7_fully_partitioned(vN, vM);
    
    /* Additional tests to force neutering/broadcast paths */
    test_openmp_offload(vN, vM, vP);
    test_unstructured_data(vN, vM);
    test_structured_data_region(vN, vM);
    
    /* Conditional execution based on command line to create variability */
    if (argc > 1) {
        /* Another set of tests with different dimensions */
        checksum ^= test_case_0_gang_redundant(vN/2, vM/2);
        checksum ^= test_case_1_gang_partitioned(vN/2, vM/2);
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    return 0;
}
