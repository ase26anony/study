/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
 *
 * This program exercises all 8 partition code cases (0-7) in the
 * omp-oacc-neuter-broadcast.cc file by creating OpenACC compute constructs
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
int test_acc_data_region(int n, int m);
__attribute__((noinline, used))
int test_unstructured_data(int n, int m);

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
int test_case_0_gang_redundant(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    /* Simple copy clause - default gang redundancy */
    #pragma acc parallel copy(arr1[0:local_n][0:local_m])
    {
        #pragma acc loop reduction(+:sum)
        for (int i = 0; i < local_n; i++) {
            for (int j = 0; j < local_m; j++) {
                arr1[i][j] = i * 100 + j;
                sum += arr1[i][j] & 0x1;
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_case_1_gang_partitioned(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    /* Explicit gang partitioning */
    #pragma acc parallel copy(gang: arr1[0:local_n][0:local_m])
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < local_m; j++) {
                arr1[i][j] = i * 200 + j;
                sum += arr1[i][j] & 0x2;
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_case_2_worker_partitioned(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    /* Worker partitioned data clause */
    #pragma acc parallel copy(worker: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker reduction(+:sum)
            for (int j = 0; j < local_m; j++) {
                arr2[i][j] = i * 300 + j;
                sum += arr2[i][j] & 0x4;
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_case_3_gang_worker_partitioned(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    /* Combined gang and worker partitioning */
    #pragma acc parallel copy(gang, worker: arr1[0:local_n][0:local_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker reduction(+:sum)
            for (int j = 0; j < local_m; j++) {
                arr1[i][j] = i * 400 + j;
                sum += arr1[i][j] & 0x8;
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_case_4_vector_partitioned(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    /* Vector partitioned data clause */
    #pragma acc parallel copy(vector: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < local_m; j++) {
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < 4; k++) {
                    arr2[i][j] = i * 500 + j + k;
                    sum += arr2[i][j] & 0x10;
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_case_5_gang_vector_partitioned(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    /* Combined gang and vector partitioning */
    #pragma acc parallel copy(gang, vector: arr1[0:local_n][0:local_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < local_m; j++) {
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < 4; k++) {
                    arr1[i][j] = i * 600 + j + k;
                    sum += arr1[i][j] & 0x20;
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_case_6_worker_vector_partitioned(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    /* Combined worker and vector partitioning */
    #pragma acc parallel copy(worker, vector: arr2[0:local_n][0:local_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < local_m; j++) {
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < 4; k++) {
                    arr2[i][j] = i * 700 + j + k;
                    sum += arr2[i][j] & 0x40;
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_case_7_fully_partitioned(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    /* Fully partitioned: gang, worker, and vector */
    #pragma acc parallel copy(gang, worker, vector: arr1[0:local_n][0:local_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < local_n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < local_m; j++) {
                #pragma acc loop vector reduction(+:sum)
                for (int k = 0; k < 4; k++) {
                    arr1[i][j] = i * 800 + j + k;
                    sum += arr1[i][j] & 0x80;
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with OpenMP offloading to engage broader infrastructure */
__attribute__((noinline, used))
int test_openmp_offload(int n, int m, int p)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    volatile int local_p = p;
    
    #pragma omp target map(tofrom: arr3[0:local_n][0:local_m][0:local_p])
    {
        #pragma omp teams distribute parallel for collapse(3) reduction(+:sum)
        for (int i = 0; i < local_n; i++) {
            for (int j = 0; j < local_m; j++) {
                for (int k = 0; k < local_p; k++) {
                    arr3[i][j][k] = i * 1000 + j * 100 + k;
                    sum += arr3[i][j][k] & 0x0F;
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with structured data region containing multiple compute constructs */
__attribute__((noinline, used))
int test_acc_data_region(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    
    #pragma acc data copy(arr1[0:local_n][0:local_m]) copy(arr2[0:local_n][0:local_m])
    {
        /* First compute construct - gang partitioned */
        #pragma acc parallel copy(gang: arr1[0:local_n][0:local_m])
        {
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < local_n; i++) {
                for (int j = 0; j < local_m; j++) {
                    arr1[i][j] = i * 900 + j;
                    sum += arr1[i][j] & 0x11;
                }
            }
        }
        
        /* Second compute construct - worker partitioned */
        #pragma acc parallel copy(worker: arr2[0:local_n][0:local_m])
        {
            #pragma acc loop gang
            for (int i = 0; i < local_n; i++) {
                #pragma acc loop worker reduction(+:sum)
                for (int j = 0; j < local_m; j++) {
                    arr2[i][j] = i * 950 + j;
                    sum += arr2[i][j] & 0x22;
                }
            }
        }
    }
    
    return sum & 0xFF;
}

/* Test with unstructured data directives */
__attribute__((noinline, used))
int test_unstructured_data(int n, int m)
{
    int sum = 0;
    volatile int local_n = n;
    volatile int local_m = m;
    int *dev_ptr = NULL;
    
    /* Use runtime library calls for unstructured data */
    dev_ptr = (int *)acc_create(arr1, local_n * local_m * sizeof(int));
    
    if (dev_ptr) {
        #pragma acc parallel present(arr1[0:local_n][0:local_m])
        {
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < local_n; i++) {
                for (int j = 0; j < local_m; j++) {
                    arr1[i][j] = i * 1100 + j;
                    sum += arr1[i][j] & 0x33;
                }
            }
        }
        
        acc_copyout(arr1, local_n * local_m * sizeof(int));
    }
    
    return sum & 0xFF;
}

int main(int argc, char *argv[])
{
    int checksum = 0;
    volatile int condition = 1;
    
    /* Initialize arrays */
    memset(arr1, 0, sizeof(arr1));
    memset(arr2, 0, sizeof(arr2));
    memset(arr3, 0, sizeof(arr3));
    
    printf("Starting OpenACC/OpenMP partition test...\n");
    
    /* Always execute all 8 partition cases */
    checksum ^= test_case_0_gang_redundant(vN, vM);
    checksum ^= test_case_1_gang_partitioned(vN, vM);
    checksum ^= test_case_2_worker_partitioned(vN, vM);
    checksum ^= test_case_3_gang_worker_partitioned(vN, vM);
    checksum ^= test_case_4_vector_partitioned(vN, vM);
    checksum ^= test_case_5_gang_vector_partitioned(vN, vM);
    checksum ^= test_case_6_worker_vector_partitioned(vN, vM);
    checksum ^= test_case_7_fully_partitioned(vN, vM);
    
    /* Conditional execution to create control flow variability */
    if (condition) {
        checksum ^= test_openmp_offload(vN/2, vM/2, vP);
    }
    
    if (argc > 1) {
        checksum ^= test_acc_data_region(vN, vM);
    }
    
    /* Another conditional branch */
    if (condition && argc > 1) {
        checksum ^= test_unstructured_data(vN, vM);
    }
    
    /* Also test with kernels construct for variety */
    #pragma acc kernels copy(arr1[0:vN][0:vM])
    {
        for (int i = 0; i < vN; i++) {
            for (int j = 0; j < vM; j++) {
                arr1[i][j] = i * 1200 + j;
                checksum += arr1[i][j] & 0x44;
            }
        }
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    return 0;
}
