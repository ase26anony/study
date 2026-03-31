/* test_offload_partition.c
 *
 * This test requires GCC configured with offloading support.
 * Compile with: -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage
 * For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
 * For host-fallback testing: use -foffload=disable
 *
 * This program exercises all 8 OpenACC data partitioning scenarios
 * (gang redundant through fully partitioned) to trigger coverage
 * of the partition code to string mapping function in GCC's
 * omp-oacc-neuter-broadcast.cc.
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

/* Global arrays to ensure data environment setup */
static int arr1[N][M];
static int arr2[N][M];
static int arr3[N][M];
static int arr4[N][M];
static int arr5[N][M];
static int arr6[N][M];
static int arr7[N][M];
static int arr8[N][M];

/* OpenMP target array */
static int omp_arr[N][M];

/* Function prototypes with attributes to prevent optimization */
int test_gang_redundant(int n, int m) __attribute__((noinline,used));
int test_gang_partitioned(int n, int m) __attribute__((noinline,used));
int test_worker_partitioned(int n, int m) __attribute__((noinline,used));
int test_gang_worker_partitioned(int n, int m) __attribute__((noinline,used));
int test_vector_partitioned(int n, int m) __attribute__((noinline,used));
int test_gang_vector_partitioned(int n, int m) __attribute__((noinline,used));
int test_worker_vector_partitioned(int n, int m) __attribute__((noinline,used));
int test_fully_partitioned(int n, int m) __attribute__((noinline,used));

int test_openmp_target(int n, int m) __attribute__((noinline,used));
int test_acc_data_region(int n, int m) __attribute__((noinline,used));
int test_unstructured_data(int n, int m) __attribute__((noinline,used));

/* Helper to introduce runtime variability */
int get_size_modifier(void) {
    static volatile int counter = 0;
    return (counter++ % 8) + 1;
}

/* Case 0: gang redundant (default) */
int test_gang_redundant(int n, int m) {
    int checksum = 0;
    int i, j;
    
    /* Use runtime-determined slice sizes */
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc parallel copy(arr1[0:slice_n][0:slice_m])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < slice_n; i++) {
                #pragma acc loop gang worker vector
                for (j = 0; j < slice_m; j++) {
                    arr1[i][j] = i * 100 + j;
                }
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr1[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 1: gang partitioned */
int test_gang_partitioned(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc parallel copy(gang: arr2[0:slice_n][0:slice_m])
        {
            #pragma acc loop gang
            for (i = 0; i < slice_n; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < slice_m; j++) {
                    arr2[i][j] = i * 200 + j * 2;
                }
            }
        }
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 2: worker partitioned */
int test_worker_partitioned(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc parallel copy(worker: arr3[0:slice_n][0:slice_m])
        {
            #pragma acc loop gang
            for (i = 0; i < slice_n; i++) {
                #pragma acc loop worker
                for (j = 0; j < slice_m; j++) {
                    arr3[i][j] = i * 300 + j * 3;
                }
            }
        }
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr3[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 3: gang+worker partitioned */
int test_gang_worker_partitioned(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc parallel copy(gang, worker: arr4[0:slice_n][0:slice_m])
        {
            #pragma acc loop gang worker
            for (i = 0; i < slice_n; i++) {
                #pragma acc loop vector
                for (j = 0; j < slice_m; j++) {
                    arr4[i][j] = i * 400 + j * 4;
                }
            }
        }
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr4[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 4: vector partitioned */
int test_vector_partitioned(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc parallel copy(vector: arr5[0:slice_n][0:slice_m])
        {
            #pragma acc loop gang worker
            for (i = 0; i < slice_n; i++) {
                #pragma acc loop vector
                for (j = 0; j < slice_m; j++) {
                    arr5[i][j] = i * 500 + j * 5;
                }
            }
        }
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr5[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 5: gang+vector partitioned */
int test_gang_vector_partitioned(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc parallel copy(gang, vector: arr6[0:slice_n][0:slice_m])
        {
            #pragma acc loop gang vector
            for (i = 0; i < slice_n; i++) {
                #pragma acc loop worker
                for (j = 0; j < slice_m; j++) {
                    arr6[i][j] = i * 600 + j * 6;
                }
            }
        }
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr6[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 6: worker+vector partitioned */
int test_worker_vector_partitioned(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc parallel copy(worker, vector: arr7[0:slice_n][0:slice_m])
        {
            #pragma acc loop gang
            for (i = 0; i < slice_n; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < slice_m; j++) {
                    arr7[i][j] = i * 700 + j * 7;
                }
            }
        }
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr7[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Case 7: fully partitioned */
int test_fully_partitioned(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc parallel copy(gang, worker, vector: arr8[0:slice_n][0:slice_m])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < slice_n; i++) {
                #pragma acc loop gang worker vector
                for (j = 0; j < slice_m; j++) {
                    arr8[i][j] = i * 800 + j * 8;
                }
            }
        }
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr8[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Test with OpenMP target to engage broader offloading infrastructure */
int test_openmp_target(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma omp target map(tofrom: omp_arr[0:slice_n][0:slice_m])
        #pragma omp teams distribute parallel for collapse(2)
        for (i = 0; i < slice_n; i++) {
            for (j = 0; j < slice_m; j++) {
                omp_arr[i][j] = i * 900 + j * 9;
            }
        }
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += omp_arr[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Test with structured data region containing multiple compute constructs */
int test_acc_data_region(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        #pragma acc data copy(arr1[0:slice_n][0:slice_m]) \
                         copy(arr2[0:slice_n][0:slice_m])
        {
            /* First compute construct */
            #pragma acc parallel present(arr1[0:slice_n][0:slice_m])
            {
                #pragma acc loop gang worker vector
                for (i = 0; i < slice_n; i++) {
                    #pragma acc loop gang worker vector
                    for (j = 0; j < slice_m; j++) {
                        arr1[i][j] += i + j;
                    }
                }
            }
            
            /* Second compute construct with different partitioning */
            #pragma acc kernels present(arr2[0:slice_n][0:slice_m])
            {
                #pragma acc loop gang
                for (i = 0; i < slice_n; i++) {
                    #pragma acc loop worker vector
                    for (j = 0; j < slice_m; j++) {
                        arr2[i][j] += i * j;
                    }
                }
            }
        }
    }
    
    /* Compute combined checksum */
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    return checksum & 0xFF;
}

/* Test with unstructured data directives using runtime library calls */
int test_unstructured_data(int n, int m) {
    int checksum = 0;
    int i, j;
    
    int slice_n = n - get_size_modifier();
    int slice_m = m - get_size_modifier();
    
    if (slice_n > 0 && slice_m > 0) {
        /* Allocate device memory */
        #pragma acc enter data copyin(arr3[0:slice_n][0:slice_m])
        
        /* Nested conditional to create control flow variability */
        volatile int flag = get_size_modifier() % 2;
        
        if (flag) {
            #pragma acc parallel present(arr3[0:slice_n][0:slice_m])
            {
                #pragma acc loop gang worker vector
                for (i = 0; i < slice_n; i++) {
                    #pragma acc loop gang worker vector
                    for (j = 0; j < slice_m; j++) {
                        arr3[i][j] = i * 1000 + j * 10;
                    }
                }
            }
        } else {
            #pragma acc kernels present(arr3[0:slice_n][0:slice_m])
            {
                #pragma acc loop gang
                for (i = 0; i < slice_n; i++) {
                    #pragma acc loop worker vector
                    for (j = 0; j < slice_m; j++) {
                        arr3[i][j] = i * 1100 + j * 11;
                    }
                }
            }
        }
        
        /* Copy data back and release */
        #pragma acc exit data copyout(arr3[0:slice_n][0:slice_m])
    }
    
    for (i = 0; i < slice_n && i < N; i++) {
        for (j = 0; j < slice_m && j < M; j++) {
            checksum += arr3[i][j];
        }
    }
    
    return checksum & 0xFF;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Use command line arguments to introduce runtime variability */
    int use_all = 1;
    if (argc > 1) {
        use_all = atoi(argv[1]) != 0;
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
    memset(omp_arr, 0, sizeof(omp_arr));
    
    /* Execute all 8 partition scenarios */
    checksum += test_gang_redundant(vN, vM);
    checksum += test_gang_partitioned(vN, vM);
    checksum += test_worker_partitioned(vN, vM);
    checksum += test_gang_worker_partitioned(vN, vM);
    checksum += test_vector_partitioned(vN, vM);
    checksum += test_gang_vector_partitioned(vN, vM);
    checksum += test_worker_vector_partitioned(vN, vM);
    checksum += test_fully_partitioned(vN, vM);
    
    /* Additional tests to force neutering/broadcast paths */
    if (use_all) {
        checksum += test_openmp_target(vN, vM);
        checksum += test_acc_data_region(vN, vM);
        checksum += test_unstructured_data(vN, vM);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
