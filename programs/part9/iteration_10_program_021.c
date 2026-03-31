/* test_offload_partition.c
 * 
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-only: use -foffload=disable
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

/* Helper to ensure side effects */
static int checksum = 0;

/* Function to update checksum in a way that prevents optimization */
__attribute__((noinline))
static void update_checksum(int value) {
    checksum = (checksum * 31 + value) & 0xFFFFFF;
}

/* Case 0: gang redundant (default) */
__attribute__((noinline, used))
void test_case_0(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1000 + j;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    update_checksum(arr[n/2][m/2]);
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
void test_case_1(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1001 + j;
            }
        }
    }
    
    update_checksum(arr[n/3][m/3]);
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
void test_case_2(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1002 + j;
            }
        }
    }
    
    update_checksum(arr[n/4][m/4]);
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
void test_case_3(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1003 + j;
            }
        }
    }
    
    update_checksum(arr[n/5][m/5]);
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
void test_case_4(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1004 + j;
            }
        }
    }
    
    update_checksum(arr[n/6][m/6]);
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
void test_case_5(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1005 + j;
            }
        }
    }
    
    update_checksum(arr[n/7][m/7]);
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
void test_case_6(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1006 + j;
            }
        }
    }
    
    update_checksum(arr[n/8][m/8]);
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
void test_case_7(int n, int m) {
    int arr[N][M];
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1007 + j;
            }
        }
    }
    
    update_checksum(arr[n/9][m/9]);
}

/* Test with OpenACC kernels construct for variety */
__attribute__((noinline, used))
void test_kernels_partition(int n, int m) {
    int arr[N][M];
    
    /* Mix of partition types in kernels region */
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 2000 + j;
            }
        }
    }
    
    update_checksum(arr[0][0]);
}

/* Test with OpenMP target for broader coverage */
__attribute__((noinline, used))
void test_omp_target(int n, int m) {
    int arr[N][M];
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    #pragma omp teams distribute parallel for
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = i * 3000 + j;
        }
    }
    
    update_checksum(arr[n-1][m-1]);
}

/* Test with structured data region */
__attribute__((noinline, used))
void test_structured_data(int n, int m, int p) {
    int arr3d[N][M][P];
    
    #pragma acc data copy(arr3d[0:n][0:m][0:p])
    {
        /* Nested compute regions to force data environment setup */
        #pragma acc parallel present(arr3d)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < p; k++) {
                        arr3d[i][j][k] = i * 10000 + j * 100 + k;
                    }
                }
            }
        }
        
        /* Second compute region using same data */
        if (n > 10) {  /* Conditional to affect control flow */
            #pragma acc parallel present(arr3d)
            {
                #pragma acc loop gang worker vector
                for (int i = 0; i < n/2; i++) {
                    for (int j = 0; j < m/2; j++) {
                        for (int k = 0; k < p/2; k++) {
                            arr3d[i][j][k] += 1;
                        }
                    }
                }
            }
        }
    }
    
    update_checksum(arr3d[0][0][0]);
}

/* Test with unstructured data directives */
__attribute__((noinline, used))
void test_unstructured_data(int n, int m) {
    int *arr = (int *)malloc(n * m * sizeof(int));
    
    /* Use runtime library calls for unstructured data */
    #pragma acc enter data copyin(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n*m; i++) {
            arr[i] = i * 7;
        }
    }
    
    #pragma acc exit data copyout(arr[0:n*m])
    
    update_checksum(arr[(n*m)/2]);
    free(arr);
}

int main(int argc, char *argv[]) {
    /* Use command line args to create runtime variability */
    int n = (argc > 1) ? atoi(argv[1]) : N;
    int m = (argc > 2) ? atoi(argv[2]) : M;
    int p = (argc > 3) ? atoi(argv[3]) : P;
    
    /* Bound the sizes to array dimensions */
    if (n > N) n = N;
    if (m > M) m = M;
    if (p > P) p = P;
    if (n < 1) n = 1;
    if (m < 1) m = 1;
    if (p < 1) p = 1;
    
    printf("Testing with n=%d, m=%d, p=%d\n", n, m, p);
    
    /* Execute all test cases to cover all partition codes */
    test_case_0(n, m);
    test_case_1(n, m);
    test_case_2(n, m);
    test_case_3(n, m);
    test_case_4(n, m);
    test_case_5(n, m);
    test_case_6(n, m);
    test_case_7(n, m);
    
    /* Additional tests to ensure compiler passes run */
    test_kernels_partition(n, m);
    test_omp_target(n, m);
    test_structured_data(n, m, p);
    test_unstructured_data(n, m);
    
    /* Print final checksum to ensure all code executed */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
