/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
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
volatile int vN = N;
volatile int vM = M;
volatile int vP = P;

/* Global arrays to work with */
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes with attributes to prevent optimization */
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
int test_openmp_target(int n, int m, int p);
__attribute__((noinline, used))
int test_acc_data_region(int n, int m);
__attribute__((noinline, used))
int test_unstructured_data(int n, int m);

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
int test_case_2_worker_partitioned(int n, int m) {
    int sum = 0;
    /* Worker partitioning */
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
int test_case_3_gang_worker_partitioned(int n, int m) {
    int sum = 0;
    /* Combined gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m]) copyout(gang, worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * arr1[i][j];
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
int test_case_4_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Vector partitioning */
    #pragma acc parallel copy(vector: arr1[0:n][0:m]) copyout(vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] - i - j;
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
int test_case_5_gang_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m]) copyout(gang, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
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
int test_case_6_worker_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Worker and vector partitioning */
    #pragma acc parallel copy(worker, vector: arr1[0:n][0:m]) copyout(worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
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
int test_case_7_fully_partitioned(int n, int m) {
    int sum = 0;
    /* Full gang, worker, vector partitioning */
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m]) copyout(gang, worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] ^ 0xAA;
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

/* Test with OpenMP target to engage broader offloading infrastructure */
int test_openmp_target(int n, int m, int p) {
    int sum = 0;
    /* Use OpenMP target with distribute and teams clauses */
    #pragma omp target map(tofrom: arr3[0:n][0:m][0:p])
    {
        #pragma omp teams distribute parallel for collapse(3)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    arr3[i][j][k] += 1;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < p; k++) {
                sum += arr3[i][j][k];
            }
        }
    }
    return sum & 0xFF;
}

/* Test structured data region with multiple compute constructs */
int test_acc_data_region(int n, int m) {
    int sum = 0;
    #pragma acc data copy(arr1[0:n][0:m]) create(arr2[0:n][0:m])
    {
        /* First compute construct */
        #pragma acc parallel present(arr1, arr2)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr2[i][j] = arr1[i][j] * 3;
                }
            }
        }
        
        /* Second compute construct with conditional */
        if (n > 10) {
            #pragma acc parallel present(arr1, arr2)
            {
                #pragma acc loop gang worker vector
                for (int i = 0; i < n; i++) {
                    #pragma acc loop gang worker vector
                    for (int j = 0; j < m; j++) {
                        arr2[i][j] += arr1[i][j];
                    }
                }
            }
        }
        
        /* Compute checksum inside data region */
        #pragma acc parallel present(arr2) reduction(+:sum)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    sum += arr2[i][j];
                }
            }
        }
    }
    return sum & 0xFF;
}

/* Main function that orchestrates all tests */
int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Initialize data */
    init_arrays();
    
    /* Use command-line arguments to create control flow variability */
    int use_all = 1;
    if (argc > 1) {
        use_all = atoi(argv[1]) & 1;
    }
    
    /* Execute all test cases to cover all partition codes */
    checksum ^= test_case_0_gang_redundant(vN, vM);
    checksum ^= test_case_1_gang_partitioned(vN, vM);
    checksum ^= test_case_2_worker_partitioned(vN, vM);
    checksum ^= test_case_3_gang_worker_partitioned(vN, vM);
    checksum ^= test_case_4_vector_partitioned(vN, vM);
    checksum ^= test_case_5_gang_vector_partitioned(vN, vM);
    checksum ^= test_case_6_worker_vector_partitioned(vN, vM);
    checksum ^= test_case_7_fully_partitioned(vN, vM);
    
    /* Additional tests to force neutering/broadcast paths */
    if (use_all) {
        checksum ^= test_openmp_target(vN/2, vM/2, vP/2);
        checksum ^= test_acc_data_region(vN, vM);
    } else {
        /* Alternative path with conditional offloading */
        #pragma acc parallel copy(arr1[0:vN][0:vM]) if(use_all)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < vN; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < vM; j++) {
                    arr1[i][j] = i - j;
                }
            }
        }
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
