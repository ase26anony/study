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
volatile int v_N = N;
volatile int v_M = M;
volatile int v_P = P;

/* Function prototypes */
int test_case_0_gang_redundant(int n, int m);
int test_case_1_gang_partitioned(int n, int m);
int test_case_2_worker_partitioned(int n, int m);
int test_case_3_gang_worker_partitioned(int n, int m);
int test_case_4_vector_partitioned(int n, int m);
int test_case_5_gang_vector_partitioned(int n, int m);
int test_case_6_worker_vector_partitioned(int n, int m);
int test_case_7_fully_partitioned(int n, int m);
int test_openmp_offload(int n, int m);
int test_unstructured_data(int n, int m);
int test_nested_conditional(int n, int m, int flag);
int test_multi_dimensional(int n, int m, int p);

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    /* Use runtime-determined slice */
    int slice_n = n > N ? N : n;
    
    #pragma acc parallel copy(arr[0:slice_n])
    {
        #pragma acc loop
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 2 + 1;
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    #pragma acc parallel copy(gang: arr[0:slice_n])
    {
        #pragma acc loop gang
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 3 + 2;
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    #pragma acc parallel copy(worker: arr[0:slice_n])
    {
        #pragma acc loop worker
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 4 + 3;
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    #pragma acc parallel copy(gang, worker: arr[0:slice_n])
    {
        #pragma acc loop gang worker
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 5 + 4;
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    #pragma acc parallel copy(vector: arr[0:slice_n])
    {
        #pragma acc loop vector
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 6 + 5;
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    #pragma acc parallel copy(gang, vector: arr[0:slice_n])
    {
        #pragma acc loop gang vector
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 7 + 6;
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    #pragma acc parallel copy(worker, vector: arr[0:slice_n])
    {
        #pragma acc loop worker vector
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 8 + 7;
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:slice_n])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 9 + 8;
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Test with OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
int test_openmp_offload(int n, int m) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    #pragma omp target map(tofrom: arr[0:slice_n])
    #pragma omp teams distribute parallel for
    for (i = 0; i < slice_n; i++) {
        arr[i] = i * 10 + 9;
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Test with unstructured data regions using runtime library calls */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m) {
    int *arr;
    int i, sum = 0;
    int slice_n = n > N ? N : n;
    
    arr = (int *)malloc(slice_n * sizeof(int));
    
    /* Create device data */
    #pragma acc enter data create(arr[0:slice_n])
    
    #pragma acc parallel present(arr[0:slice_n])
    {
        #pragma acc loop
        for (i = 0; i < slice_n; i++) {
            arr[i] = i * 11 + 10;
        }
    }
    
    /* Copy data back */
    #pragma acc update host(arr[0:slice_n])
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    
    #pragma acc exit data delete(arr[0:slice_n])
    free(arr);
    
    return sum & 0xFF;
}

/* Test nested conditional constructs */
__attribute__((noinline, used))
int test_nested_conditional(int n, int m, int flag) {
    int arr[N];
    int i, sum = 0;
    
    int slice_n = n > N ? N : n;
    
    if (flag & 1) {
        #pragma acc data copy(arr[0:slice_n])
        {
            #pragma acc parallel
            {
                #pragma acc loop
                for (i = 0; i < slice_n; i++) {
                    arr[i] = i * 12 + 11;
                }
            }
            
            if (flag & 2) {
                #pragma acc parallel
                {
                    #pragma acc loop
                    for (i = 0; i < slice_n; i++) {
                        arr[i] += i;
                    }
                }
            }
        }
    } else {
        #pragma acc kernels copy(arr[0:slice_n])
        {
            #pragma acc loop
            for (i = 0; i < slice_n; i++) {
                arr[i] = i * 13 + 12;
            }
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        sum += arr[i];
    }
    return sum & 0xFF;
}

/* Test with multi-dimensional arrays */
__attribute__((noinline, used))
int test_multi_dimensional(int n, int m, int p) {
    int arr[N][M];
    int i, j, sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    /* Use gang partitioned on first dimension, worker on second */
    #pragma acc parallel copy(gang, worker: arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop gang
        for (i = 0; i < slice_n; i++) {
            #pragma acc loop worker
            for (j = 0; j < slice_m; j++) {
                arr[i][j] = i * slice_m + j + 13;
            }
        }
    }
    
    for (i = 0; i < slice_n; i++) {
        for (j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    int flag = 0;
    
    /* Use command line argument to vary control flow */
    if (argc > 1) {
        flag = atoi(argv[1]) & 7;
    }
    
    /* Execute all test cases to cover all partition codes */
    checksum ^= test_case_0_gang_redundant(v_N, v_M);
    checksum ^= test_case_1_gang_partitioned(v_N, v_M);
    checksum ^= test_case_2_worker_partitioned(v_N, v_M);
    checksum ^= test_case_3_gang_worker_partitioned(v_N, v_M);
    checksum ^= test_case_4_vector_partitioned(v_N, v_M);
    checksum ^= test_case_5_gang_vector_partitioned(v_N, v_M);
    checksum ^= test_case_6_worker_vector_partitioned(v_N, v_M);
    checksum ^= test_case_7_fully_partitioned(v_N, v_M);
    
    /* Additional tests to force neutering/broadcast paths */
    checksum ^= test_openmp_offload(v_N, v_M);
    checksum ^= test_unstructured_data(v_N, v_M);
    checksum ^= test_nested_conditional(v_N, v_M, flag);
    checksum ^= test_multi_dimensional(v_N, v_M, v_P);
    
    /* Use volatile to prevent dead code elimination */
    volatile int result = checksum & 0xFF;
    printf("Result: %d\n", result);
    
    return 0;
}
