/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in GCC's
 * omp-oacc-neuter-broadcast.cc by creating OpenACC compute constructs
 * with different gang/worker/vector data partitioning combinations.
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

/* Global arrays to work with */
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes for each partition case */
int case_0_gang_redundant(int n, int m);
int case_1_gang_partitioned(int n, int m);
int case_2_worker_partitioned(int n, int m);
int case_3_gang_worker_partitioned(int n, int m);
int case_4_vector_partitioned(int n, int m);
int case_5_gang_vector_partitioned(int n, int m);
int case_6_worker_vector_partitioned(int n, int m);
int case_7_fully_partitioned(int n, int m);
int openmp_target_test(int n, int m);
int unstructured_data_test(int n, int m);
int conditional_constructs(int n, int m, int flag);

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int case_0_gang_redundant(int n, int m)
{
    int sum = 0;
    /* Default gang redundancy */
    #pragma acc parallel copy(arr1[0:n][0:m]) copyout(arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 2;
                if (i == j) sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int case_1_gang_partitioned(int n, int m)
{
    int sum = 0;
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr1[0:n][0:m]) copyout(gang: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + i;
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int case_2_worker_partitioned(int n, int m)
{
    int sum = 0;
    /* Worker partitioning */
    #pragma acc kernels copy(worker: arr1[0:n][0:m]) copyout(worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] - j;
                if ((i + j) % 3 == 0) sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int case_3_gang_worker_partitioned(int n, int m)
{
    int sum = 0;
    /* Gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m]) \
                         copyout(gang, worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * 3;
                sum ^= arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int case_4_vector_partitioned(int n, int m)
{
    int sum = 0;
    /* Vector partitioning */
    #pragma acc kernels copy(vector: arr1[0:n][0:m]) copyout(vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] / 2;
                sum += arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int case_5_gang_vector_partitioned(int n, int m)
{
    int sum = 0;
    /* Gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m]) \
                         copyout(gang, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] + i - j;
                if (arr2[i][j] > 0) sum++;
            }
        }
    }
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int case_6_worker_vector_partitioned(int n, int m)
{
    int sum = 0;
    /* Worker and vector partitioning */
    #pragma acc kernels copy(worker, vector: arr1[0:n][0:m]) \
                        copyout(worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] * arr1[i][j];
                sum |= arr2[i][j];
            }
        }
    }
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int case_7_fully_partitioned(int n, int m)
{
    int sum = 0;
    /* Fully partitioned (gang, worker, vector) */
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m]) \
                         copyout(gang, worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = arr1[i][j] % 17;
                sum += arr2[i][j] * (i + j);
            }
        }
    }
    return sum & 0xFF;
}

/* OpenMP target test to engage broader offloading infrastructure */
__attribute__((noinline, used))
int openmp_target_test(int n, int m)
{
    int sum = 0;
    #pragma omp target map(tofrom: arr1[0:n][0:m]) map(from: arr2[0:n][0:m])
    #pragma omp teams distribute parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr2[i][j] = arr1[i][j] * 5;
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with unstructured data directives */
__attribute__((noinline, used))
int unstructured_data_test(int n, int m)
{
    int sum = 0;
    int *dev_arr1, *dev_arr2;
    size_t size = n * m * sizeof(int);
    
    /* Create device data */
    dev_arr1 = (int*)acc_create(arr1, size);
    dev_arr2 = (int*)acc_create(arr2, size);
    
    #pragma acc parallel present(dev_arr1[0:n*m], dev_arr2[0:n*m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n * m; i++) {
            dev_arr2[i] = dev_arr1[i] + 42;
            sum += dev_arr2[i];
        }
    }
    
    acc_copyout(arr2, size);
    acc_delete(arr1, size);
    
    return sum & 0xFF;
}

/* Conditional constructs to create control flow variability */
__attribute__((noinline, used))
int conditional_constructs(int n, int m, int flag)
{
    int sum = 0;
    
    /* Data region containing multiple compute constructs */
    #pragma acc data copy(arr1[0:n][0:m]) copyout(arr2[0:n][0:m])
    {
        if (flag & 1) {
            #pragma acc parallel
            {
                #pragma acc loop gang worker
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        arr2[i][j] = arr1[i][j] * 2;
                    }
                }
            }
        }
        
        if (flag & 2) {
            #pragma acc kernels
            {
                #pragma acc loop gang
                for (int i = 0; i < n; i++) {
                    #pragma acc loop worker vector
                    for (int j = 0; j < m; j++) {
                        arr2[i][j] += arr1[i][j];
                        sum += arr2[i][j];
                    }
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Initialize arrays with non-zero values */
void init_arrays(void)
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (i * 17 + j * 13) % 100;
            arr2[i][j] = 0;
        }
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = (i + j + k) % 50;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int checksum = 0;
    
    /* Use command line arguments for variability */
    int flag = (argc > 1) ? atoi(argv[1]) : 3;
    
    /* Initialize with volatile bounds to prevent optimization */
    int n = v_N;
    int m = v_M;
    int p = v_P;
    
    init_arrays();
    
    /* Execute all partition cases */
    checksum ^= case_0_gang_redundant(n, m);
    checksum ^= case_1_gang_partitioned(n, m);
    checksum ^= case_2_worker_partitioned(n, m);
    checksum ^= case_3_gang_worker_partitioned(n, m);
    checksum ^= case_4_vector_partitioned(n, m);
    checksum ^= case_5_gang_vector_partitioned(n, m);
    checksum ^= case_6_worker_vector_partitioned(n, m);
    checksum ^= case_7_fully_partitioned(n, m);
    
    /* Additional tests to engage compiler passes */
    checksum ^= openmp_target_test(n, m);
    checksum ^= unstructured_data_test(n, m);
    checksum ^= conditional_constructs(n, m, flag);
    
    /* Multi-dimensional array with runtime sections */
    int sum3d = 0;
    #pragma acc parallel copy(arr3[0:n][0:m][0:p])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                #pragma acc loop vector
                for (int k = 0; k < p; k++) {
                    arr3[i][j][k] += i + j + k;
                    sum3d += arr3[i][j][k];
                }
            }
        }
    }
    checksum ^= (sum3d & 0xFF);
    
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
