/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in the
 * omp-oacc-neuter-broadcast.cc file by creating OpenACC compute
 * constructs with different data clause partitioning combinations.
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
int arr3[N][M];
int arr4[N][M];
int arr5[N][M];
int arr6[N][M];
int arr7[N][M];
int arr8[N][M];

/* OpenACC test functions for each partition case */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m) {
    int sum = 0;
    /* Case 0: gang redundant (default) */
    #pragma acc parallel copy(arr1[0:n][0:m]) present(arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * j;
                sum += arr1[i][j];
            }
        }
    }
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m) {
    int sum = 0;
    /* Case 1: gang partitioned */
    #pragma acc parallel copy(gang: arr2[0:n][0:m])
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr2[i][j] = i + j;
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m) {
    int sum = 0;
    /* Case 2: worker partitioned */
    #pragma acc parallel copy(worker: arr3[0:n][0:m])
    {
        #pragma acc loop worker reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr3[i][j] = i - j;
                sum += arr3[i][j];
            }
        }
    }
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m) {
    int sum = 0;
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(gang, worker: arr4[0:n][0:m])
    {
        #pragma acc loop gang worker reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr4[i][j] = i * 2 + j;
                sum += arr4[i][j];
            }
        }
    }
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(vector: arr5[0:n][0:m])
    {
        #pragma acc loop vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr5[i][j] = i + j * 3;
                sum += arr5[i][j];
            }
        }
    }
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel copy(gang, vector: arr6[0:n][0:m])
    {
        #pragma acc loop gang vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr6[i][j] = i * 3 - j;
                sum += arr6[i][j];
            }
        }
    }
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m) {
    int sum = 0;
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel copy(worker, vector: arr7[0:n][0:m])
    {
        #pragma acc loop worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr7[i][j] = i - j * 2;
                sum += arr7[i][j];
            }
        }
    }
    return sum & 0xFF;
}

__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m) {
    int sum = 0;
    /* Case 7: fully partitioned */
    #pragma acc parallel copy(gang, worker, vector: arr8[0:n][0:m])
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr8[i][j] = i * j * 2;
                sum += arr8[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* OpenMP offloading test to engage broader infrastructure */
__attribute__((noinline, used))
int test_omp_offload(int n, int m) {
    int sum = 0;
    int local_arr[N][M];
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            local_arr[i][j] = i + j;
        }
    }
    
    /* OpenMP target with distribute, teams, and parallel clauses */
    #pragma omp target map(tofrom: local_arr[0:n][0:m]) map(tofrom: sum)
    {
        #pragma omp teams distribute parallel for reduction(+:sum) collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                local_arr[i][j] *= 2;
                sum += local_arr[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Function with structured data region */
__attribute__((noinline, used))
int test_structured_data_region(int n, int m) {
    int sum = 0;
    int temp_arr[N][M];
    
    #pragma acc data copy(temp_arr[0:n][0:m])
    {
        /* First compute construct */
        #pragma acc parallel present(temp_arr[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    temp_arr[i][j] = i * j;
                }
            }
        }
        
        /* Second compute construct with different partitioning */
        #pragma acc parallel present(temp_arr[0:n][0:m])
        {
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    sum += temp_arr[i][j];
                }
            }
        }
    }
    return sum & 0xFF;
}

/* Function with unstructured data dynamics using runtime library calls */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int sum = 0;
    int *d_arr;
    size_t size = n * m * sizeof(int);
    
    /* Allocate and copy data using runtime calls */
    d_arr = (int *)malloc(size);
    
    #pragma acc enter data copyin(d_arr[0:n*m])
    
    /* Compute on device */
    #pragma acc parallel present(d_arr[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n*m; i++) {
            d_arr[i] = i % 256;
        }
    }
    
    /* Copy back and compute checksum */
    #pragma acc update host(d_arr[0:n*m])
    
    for (int i = 0; i < n*m; i++) {
        sum += d_arr[i];
    }
    
    #pragma acc exit data delete(d_arr[0:n*m])
    free(d_arr);
    
    return sum & 0xFF;
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int checksum = 0;
    int use_openacc = 1;
    int use_openmp = 1;
    
    /* Use command line arguments to create control flow variability */
    if (argc > 1) {
        use_openacc = atoi(argv[1]) & 1;
        use_openmp = (atoi(argv[1]) >> 1) & 1;
    }
    
    /* Initialize arrays */
    memset(arr1, 0, sizeof(arr1));
    memset(arr2, 0, sizeof(arr2));
    memset(arr3, 0, sizeof(arr3));
    memset(arr4, 0, sizeof(arr4));
    memset(arr5, 0, sizeof(arr5));
    memset(arr6, 0, sizeof(arr6));
    memset(arr7, 0, sizeof(arr7));
    memset(arr8, 0, sizeof(arr8));
    
    /* Execute all partition cases */
    if (use_openacc) {
        checksum ^= test_case_0_gang_redundant(v_N, v_M);
        checksum ^= test_case_1_gang_partitioned(v_N, v_M);
        checksum ^= test_case_2_worker_partitioned(v_N, v_M);
        checksum ^= test_case_3_gang_worker_partitioned(v_N, v_M);
        checksum ^= test_case_4_vector_partitioned(v_N, v_M);
        checksum ^= test_case_5_gang_vector_partitioned(v_N, v_M);
        checksum ^= test_case_6_worker_vector_partitioned(v_N, v_M);
        checksum ^= test_case_7_fully_partitioned(v_N, v_M);
        
        /* Additional tests with control flow variability */
        if (v_N > 50) {
            checksum ^= test_structured_data_region(v_N/2, v_M/2);
        }
        
        if (v_M > 30) {
            checksum ^= test_unstructured_data(v_N/4, v_M/4);
        }
    }
    
    /* OpenMP offloading test */
    if (use_openmp) {
        checksum ^= test_omp_offload(v_N, v_M);
    }
    
    /* Also test with kernels construct for different code paths */
    #pragma acc kernels copy(arr1[0:v_N][0:v_M])
    {
        for (int i = 0; i < v_N; i++) {
            for (int j = 0; j < v_M; j++) {
                arr1[i][j] += checksum;
            }
        }
    }
    
    /* Final checksum computation */
    for (int i = 0; i < v_N; i++) {
        for (int j = 0; j < v_M; j++) {
            checksum += arr1[i][j] + arr2[i][j] + arr3[i][j] + 
                       arr4[i][j] + arr5[i][j] + arr6[i][j] + 
                       arr7[i][j] + arr8[i][j];
        }
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    return 0;
}
