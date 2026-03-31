/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning cases
 * to trigger coverage of the partition code to string mapping
 * function in omp-oacc-neuter-broadcast.cc.
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

/* Global arrays for data regions */
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
int test_openmp_offload(int n, int m, int p);
__attribute__((noinline, used))
int test_unstructured_data(int n, int m);
__attribute__((noinline, used))
int test_nested_conditional(int n, int m, int flag);

/* Test case 0: gang redundant (default) */
int test_case_0_gang_redundant(int n, int m)
{
    int i, j, sum = 0;
    
    /* Use runtime-determined slice sizes */
    #pragma acc parallel copy(arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 100 + j;
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 1: gang partitioned */
int test_case_1_gang_partitioned(int n, int m)
{
    int i, j, sum = 0;
    
    #pragma acc parallel copy(gang: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 200 + j * 2;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 2: worker partitioned */
int test_case_2_worker_partitioned(int n, int m)
{
    int i, j, sum = 0;
    
    #pragma acc parallel copy(worker: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 300 + j * 3;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 3: gang+worker partitioned */
int test_case_3_gang_worker_partitioned(int n, int m)
{
    int i, j, sum = 0;
    
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 400 + j * 4;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 4: vector partitioned */
int test_case_4_vector_partitioned(int n, int m)
{
    int i, j, sum = 0;
    
    #pragma acc parallel copy(vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 500 + j * 5;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 5: gang+vector partitioned */
int test_case_5_gang_vector_partitioned(int n, int m)
{
    int i, j, sum = 0;
    
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 600 + j * 6;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 6: worker+vector partitioned */
int test_case_6_worker_vector_partitioned(int n, int m)
{
    int i, j, sum = 0;
    
    #pragma acc parallel copy(worker, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 700 + j * 7;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 7: fully partitioned */
int test_case_7_fully_partitioned(int n, int m)
{
    int i, j, sum = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 800 + j * 8;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test OpenMP offloading with distribute/teams/parallel clauses */
int test_openmp_offload(int n, int m, int p)
{
    int i, j, k, sum = 0;
    
    #pragma omp target map(tofrom: arr3[0:n][0:m][0:p])
    #pragma omp teams distribute parallel for collapse(3)
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            for (k = 0; k < p; k++) {
                arr3[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            for (k = 0; k < p; k++) {
                sum += arr3[i][j][k];
            }
        }
    }
    return sum & 0xFF;
}

/* Test unstructured data regions with acc_create/acc_copyin */
int test_unstructured_data(int n, int m)
{
    int i, j, sum = 0;
    int *dev_ptr;
    
    /* Create device data */
    dev_ptr = (int *)acc_create(arr2, n * m * sizeof(int));
    
    #pragma acc parallel present(arr2[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (j = 0; j < m; j++) {
                arr2[i][j] = i * 900 + j * 9;
            }
        }
    }
    
    /* Copy data back implicitly through present clause */
    acc_copyout(arr2, n * m * sizeof(int));
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr2[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test nested conditional compute constructs */
int test_nested_conditional(int n, int m, int flag)
{
    int i, j, sum = 0;
    
    if (flag > 0) {
        #pragma acc kernels copy(arr1[0:n][0:m])
        {
            #pragma acc loop independent gang
            for (i = 0; i < n; i++) {
                #pragma acc loop independent worker vector
                for (j = 0; j < m; j++) {
                    arr1[i][j] = i * 1000 + j * 10 + flag;
                }
            }
        }
    } else {
        #pragma acc parallel copy(arr1[0:n][0:m])
        {
            #pragma acc loop seq
            for (i = 0; i < n; i++) {
                #pragma acc loop seq
                for (j = 0; j < m; j++) {
                    arr1[i][j] = i * 2000 + j * 20 + flag;
                }
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Main function that exercises all test cases */
int main(int argc, char *argv[])
{
    int checksum = 0;
    int flag = 0;
    
    /* Use command line argument to create conditional execution */
    if (argc > 1) {
        flag = atoi(argv[1]) & 1;
    }
    
    /* Initialize arrays */
    memset(arr1, 0, sizeof(arr1));
    memset(arr2, 0, sizeof(arr2));
    memset(arr3, 0, sizeof(arr3));
    
    /* Execute all 8 OpenACC partitioning cases */
    checksum ^= test_case_0_gang_redundant(vN, vM);
    checksum ^= test_case_1_gang_partitioned(vN, vM);
    checksum ^= test_case_2_worker_partitioned(vN, vM);
    checksum ^= test_case_3_gang_worker_partitioned(vN, vM);
    checksum ^= test_case_4_vector_partitioned(vN, vM);
    checksum ^= test_case_5_gang_vector_partitioned(vN, vM);
    checksum ^= test_case_6_worker_vector_partitioned(vN, vM);
    checksum ^= test_case_7_fully_partitioned(vN, vM);
    
    /* Test OpenMP offloading */
    checksum ^= test_openmp_offload(vP, vM/2, vN/4);
    
    /* Test unstructured data */
    checksum ^= test_unstructured_data(vN, vM);
    
    /* Test conditional execution */
    checksum ^= test_nested_conditional(vN, vM, flag);
    
    /* Final checksum output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
