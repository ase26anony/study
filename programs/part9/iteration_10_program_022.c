/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback: use -foffload=disable
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
int test_data_region(int n, int m);
int test_unstructured_data(int n, int m);
void init_array(int *arr, int size);

/* Case 0: gang redundant (default) */
__attribute__((noinline,used))
int test_case_0(int n, int m)
{
    int checksum = 0;
    /* Default gang redundancy */
    #pragma acc parallel copy(arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * 1000 + j;
                checksum += arr1[i][j];
            }
        }
    }
    return checksum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline,used))
int test_case_1(int n, int m)
{
    int checksum = 0;
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = i * 2000 + j * 2;
                checksum += arr2[i][j];
            }
        }
    }
    return checksum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline,used))
int test_case_2(int n, int m)
{
    int checksum = 0;
    /* Worker partitioning */
    #pragma acc parallel copy(worker: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * 3000 + j * 3;
                checksum += arr1[i][j];
            }
        }
    }
    return checksum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline,used))
int test_case_3(int n, int m)
{
    int checksum = 0;
    /* Gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr2[i][j] = i * 4000 + j * 4;
                checksum += arr2[i][j];
            }
        }
    }
    return checksum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline,used))
int test_case_4(int n, int m)
{
    int checksum = 0;
    /* Vector partitioning */
    #pragma acc parallel copy(vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * 5000 + j * 5;
                checksum += arr1[i][j];
            }
        }
    }
    return checksum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline,used))
int test_case_5(int n, int m)
{
    int checksum = 0;
    /* Gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = i * 6000 + j * 6;
                checksum += arr2[i][j];
            }
        }
    }
    return checksum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline,used))
int test_case_6(int n, int m)
{
    int checksum = 0;
    /* Worker and vector partitioning */
    #pragma acc parallel copy(worker, vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr1[i][j] = i * 7000 + j * 7;
                checksum += arr1[i][j];
            }
        }
    }
    return checksum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline,used))
int test_case_7(int n, int m)
{
    int checksum = 0;
    /* Fully partitioned (gang, worker, vector) */
    #pragma acc parallel copy(gang, worker, vector: arr2[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr2[i][j] = i * 8000 + j * 8;
                checksum += arr2[i][j];
            }
        }
    }
    return checksum & 0xFF;
}

/* Test with OpenMP target to engage broader offloading infrastructure */
__attribute__((noinline,used))
int test_openmp_target(int n, int m)
{
    int checksum = 0;
    /* Use OpenMP target with distribute, teams, and parallel clauses */
    #pragma omp target map(tofrom: arr3[0:n][0:m][0:10])
    {
        #pragma omp teams distribute
        for (int i = 0; i < n; i++) {
            #pragma omp parallel for
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < 10; k++) {
                    arr3[i][j][k] = i * 100 + j * 10 + k;
                    checksum += arr3[i][j][k];
                }
            }
        }
    }
    return checksum & 0xFF;
}

/* Test with structured data region containing multiple compute constructs */
__attribute__((noinline,used))
int test_data_region(int n, int m)
{
    int checksum = 0;
    /* Structured data region */
    #pragma acc data copy(arr1[0:n][0:m]) create(arr2[0:n][0:m])
    {
        /* First compute construct with gang partitioning */
        #pragma acc parallel copy(gang: arr1[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = i * 1111 + j;
                }
            }
        }
        
        /* Second compute construct with worker partitioning */
        #pragma acc parallel copy(worker: arr2[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    arr2[i][j] = arr1[i][j] * 2;
                    checksum += arr2[i][j];
                }
            }
        }
    }
    return checksum & 0xFF;
}

/* Test with unstructured data directives using runtime library calls */
__attribute__((noinline,used))
int test_unstructured_data(int n, int m)
{
    int checksum = 0;
    int *d_arr = NULL;
    size_t size = n * m * sizeof(int);
    
    /* Allocate device memory */
    d_arr = (int *)acc_malloc(size);
    if (!d_arr) return 0;
    
    /* Copy data to device */
    #pragma acc enter data copyin(arr1[0:n][0:m])
    
    /* Compute on device with vector partitioning */
    #pragma acc parallel present(arr1[0:n][0:m]) copy(vector: arr1[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n * m; i++) {
            int idx = i / m;
            int jdx = i % m;
            arr1[idx][jdx] = arr1[idx][jdx] * 3;
            checksum += arr1[idx][jdx];
        }
    }
    
    /* Copy data back */
    #pragma acc exit data copyout(arr1[0:n][0:m])
    
    acc_free(d_arr);
    return checksum & 0xFF;
}

/* Initialize array with non-zero values */
void init_array(int *arr, int size)
{
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
}

int main(int argc, char *argv[])
{
    int checksum = 0;
    int use_conditional = 0;
    
    /* Use command line argument to create conditional execution paths */
    if (argc > 1) {
        use_conditional = atoi(argv[1]) & 1;
    }
    
    /* Initialize arrays */
    init_array((int *)arr1, N * M);
    init_array((int *)arr2, N * M);
    init_array((int *)arr3, N * M * P);
    
    /* Always execute all 8 partition cases */
    checksum += test_case_0(vN, vM);
    checksum += test_case_1(vN, vM);
    checksum += test_case_2(vN, vM);
    checksum += test_case_3(vN, vM);
    checksum += test_case_4(vN, vM);
    checksum += test_case_5(vN, vM);
    checksum += test_case_6(vN, vM);
    checksum += test_case_7(vN, vM);
    
    /* Conditional execution to create control flow variability */
    if (use_conditional) {
        checksum += test_openmp_target(vN / 2, vM / 2);
    } else {
        checksum += test_data_region(vN / 2, vM / 2);
    }
    
    /* Always test unstructured data */
    checksum += test_unstructured_data(vN / 4, vM / 4);
    
    /* Also test with kernels construct for variety */
    #pragma acc kernels copy(arr1[0:vN/2][0:vM/2])
    {
        for (int i = 0; i < vN/2; i++) {
            for (int j = 0; j < vM/2; j++) {
                arr1[i][j] = arr1[i][j] * 2;
                checksum += arr1[i][j];
            }
        }
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    return 0;
}
