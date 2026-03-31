/* This test requires GCC configured with offloading support. 
   Compile with: gcc -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage test.c -o test
   Run with: ./test
   For offloading: add -foffload=nvptx-none or -foffload=amdgcn-amdhsa
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 128
#define M 64
#define P 32

/* Volatile variables to prevent constant propagation */
volatile int vN = N;
volatile int vM = M;
volatile int vP = P;

/* Function prototypes with attributes to prevent optimization */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m, int p);
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m, int p);
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m, int p);
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m, int p);
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m, int p);
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m, int p);
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m, int p);
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m, int p);
__attribute__((noinline, used))
int test_omp_offload(int n, int m, int p);
__attribute__((noinline, used))
int test_unstructured_data(int n, int m, int p);
__attribute__((noinline, used))
int test_mixed_constructs(int n, int m, int p);

/* Test case 0: gang redundant (default) */
int test_gang_redundant(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    /* Use runtime-determined slice sizes */
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < slice_n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 1: gang partitioned */
int test_gang_partitioned(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(gang: arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < slice_n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 200 + j;
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 2: worker partitioned */
int test_worker_partitioned(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(worker: arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop gang
        for (int i = 0; i < slice_n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 300 + j;
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 3: gang+worker partitioned */
int test_gang_worker_partitioned(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(gang, worker: arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < slice_n; i++) {
            #pragma acc loop gang worker
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 400 + j;
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 4: vector partitioned */
int test_vector_partitioned(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(vector: arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < slice_n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 500 + j;
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 5: gang+vector partitioned */
int test_gang_vector_partitioned(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(gang, vector: arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < slice_n; i++) {
            #pragma acc loop gang vector
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 600 + j;
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 6: worker+vector partitioned */
int test_worker_vector_partitioned(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(worker, vector: arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < slice_n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 700 + j;
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test case 7: fully partitioned */
int test_fully_partitioned(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:slice_n][0:slice_m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < slice_n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 800 + j;
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test OpenMP offloading to engage broader infrastructure */
int test_omp_offload(int n, int m, int p) {
    int arr[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    #pragma omp target map(tofrom: arr[0:slice_n][0:slice_m])
    {
        #pragma omp teams distribute parallel for collapse(2)
        for (int i = 0; i < slice_n; i++) {
            for (int j = 0; j < slice_m; j++) {
                arr[i][j] = i * 900 + j;
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test unstructured data regions with runtime library calls */
int test_unstructured_data(int n, int m, int p) {
    int *arr = (int*)malloc(N * M * sizeof(int));
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    size_t size = slice_n * slice_m * sizeof(int);
    
    /* Unstructured data directives */
    int *dev_ptr = (int*)acc_create(arr, size);
    
    #pragma acc parallel present(arr[0:slice_n*slice_m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < slice_n * slice_m; i++) {
            arr[i] = i * 1000;
        }
    }
    
    acc_copyout(arr, size);
    
    for (int i = 0; i < slice_n * slice_m; i++) {
        sum += arr[i];
    }
    
    free(arr);
    return sum & 0xFF;
}

/* Test mixed constructs and conditional execution */
int test_mixed_constructs(int n, int m, int p) {
    int arr1[N][M], arr2[N][M];
    int sum = 0;
    
    int slice_n = n > N ? N : n;
    int slice_m = m > M ? M : m;
    
    /* Data region with multiple compute constructs */
    #pragma acc data copy(arr1[0:slice_n][0:slice_m]) create(arr2[0:slice_n][0:slice_m])
    {
        /* First parallel region */
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < slice_n; i++) {
                for (int j = 0; j < slice_m; j++) {
                    arr1[i][j] = i * 1100 + j;
                }
            }
        }
        
        /* Conditional parallel region */
        if (n > 0) {  /* Always true, but compiler doesn't know */
            #pragma acc parallel
            {
                #pragma acc loop gang worker vector
                for (int i = 0; i < slice_n; i++) {
                    for (int j = 0; j < slice_m; j++) {
                        arr2[i][j] = arr1[i][j] * 2;
                    }
                }
            }
        }
        
        /* Kernels construct with different partitioning */
        #pragma acc kernels copyout(arr1[0:slice_n][0:slice_m])
        {
            for (int i = 0; i < slice_n; i++) {
                for (int j = 0; j < slice_m; j++) {
                    arr1[i][j] = arr2[i][j] + 1;
                }
            }
        }
    }
    
    for (int i = 0; i < slice_n; i++) {
        for (int j = 0; j < slice_m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Use volatile to prevent compile-time computation */
    volatile int use_acc = 1;
    
    /* Execute all test cases to trigger all partition codes */
    if (use_acc) {
        checksum ^= test_gang_redundant(vN, vM, vP);
        checksum ^= test_gang_partitioned(vN, vM, vP);
        checksum ^= test_worker_partitioned(vN, vM, vP);
        checksum ^= test_gang_worker_partitioned(vN, vM, vP);
        checksum ^= test_vector_partitioned(vN, vM, vP);
        checksum ^= test_gang_vector_partitioned(vN, vM, vP);
        checksum ^= test_worker_vector_partitioned(vN, vM, vP);
        checksum ^= test_fully_partitioned(vN, vM, vP);
    }
    
    /* Mix with OpenMP offloading */
    if (argc > 1) {  /* Conditional based on runtime */
        checksum ^= test_omp_offload(vN, vM, vP);
    }
    
    /* Test unstructured data */
    checksum ^= test_unstructured_data(vN, vM, vP);
    
    /* Test mixed constructs */
    checksum ^= test_mixed_constructs(vN, vM, vP);
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
