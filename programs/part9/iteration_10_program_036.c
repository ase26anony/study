/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
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

/* Global arrays for data sharing between functions */
int arr1[N][M];
int arr2[N][M];
int arr3[N][M][P];

/* Function prototypes */
int test_case_0(int n, int m);
int test_case_1(int n, int m);
int test_case_2(int n, int m);
int test_case_3(int n, int m);
int test_case_4(int n, int m);
int test_case_5(int n, int m);
int test_case_6(int n, int m);
int test_case_7(int n, int m);
int test_openmp_target(int n, int m);
int test_unstructured_data(int n, int m);
int test_nested_conditional(int n, int m, int flag);

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0(int n, int m)
{
    int i, j;
    int checksum = 0;
    
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
    
    /* Compute checksum on host */
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1(int n, int m)
{
    int i, j;
    int checksum = 0;
    
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
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    #pragma acc parallel copy(worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr2[i][j] = i * 300 + j * 3;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    #pragma acc parallel copy(gang, worker: arr1[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 400 + j * 4;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    #pragma acc parallel copy(vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < m; j++) {
                arr2[i][j] = i * 500 + j * 5;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    #pragma acc parallel copy(gang, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < m; j++) {
                arr1[i][j] = i * 600 + j * 6;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    #pragma acc parallel copy(worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < m; j++) {
                arr2[i][j] = i * 700 + j * 7;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7(int n, int m)
{
    int i, j;
    int checksum = 0;
    
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
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Test with OpenMP target to engage broader offloading infrastructure */
__attribute__((noinline, used))
int test_openmp_target(int n, int m)
{
    int i, j;
    int checksum = 0;
    
    #pragma omp target map(tofrom: arr3[0:n][0:m][0:8])
    {
        #pragma omp teams distribute parallel for collapse(2)
        for (i = 0; i < n; i++) {
            for (j = 0; j < m; j++) {
                arr3[i][j][0] = i * 900 + j * 9;
                arr3[i][j][1] = i * 901 + j * 10;
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr3[i][j][0] + arr3[i][j][1];
        }
    }
    
    return checksum & 0xFF;
}

/* Test unstructured data regions with runtime library calls */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m)
{
    int i, j;
    int checksum = 0;
    int *d_arr;
    
    /* Structured data region */
    #pragma acc data copy(arr1[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (j = 0; j < m; j++) {
                    arr1[i][j] = i * 1000 + j * 11;
                }
            }
        }
        
        #pragma acc kernels copy(arr1[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (j = 0; j < m; j++) {
                    arr1[i][j] += 1;
                }
            }
        }
    }
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Test nested conditional constructs to force neutering analysis */
__attribute__((noinline, used))
int test_nested_conditional(int n, int m, int flag)
{
    int i, j;
    int checksum = 0;
    
    if (flag & 1) {
        #pragma acc parallel copy(arr1[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (j = 0; j < m; j++) {
                    arr1[i][j] = i * 1100 + j * 12;
                }
            }
        }
    }
    
    if (flag & 2) {
        #pragma acc kernels copy(arr2[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (j = 0; j < m; j++) {
                    arr2[i][j] = i * 1200 + j * 13;
                }
            }
        }
    }
    
    /* Use volatile index to prevent optimization */
    volatile int idx = n / 2;
    for (i = 0; i < idx; i++) {
        for (j = 0; j < m; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

int main(int argc, char *argv[])
{
    int checksum = 0;
    int flag = 0;
    
    /* Use command line argument to create variability */
    if (argc > 1) {
        flag = atoi(argv[1]) & 7;
    }
    
    /* Initialize arrays */
    memset(arr1, 0, sizeof(arr1));
    memset(arr2, 0, sizeof(arr2));
    memset(arr3, 0, sizeof(arr3));
    
    /* Execute all 8 OpenACC partitioning cases */
    checksum += test_case_0(vN, vM);
    checksum += test_case_1(vN, vM);
    checksum += test_case_2(vN, vM);
    checksum += test_case_3(vN, vM);
    checksum += test_case_4(vN, vM);
    checksum += test_case_5(vN, vM);
    checksum += test_case_6(vN, vM);
    checksum += test_case_7(vN, vM);
    
    /* Test OpenMP offloading */
    checksum += test_openmp_target(vN, vM);
    
    /* Test unstructured data regions */
    checksum += test_unstructured_data(vN, vM);
    
    /* Test nested conditional with volatile control */
    checksum += test_nested_conditional(vN, vM, flag);
    
    /* Final checksum output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
