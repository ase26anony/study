/* This test requires GCC configured with offloading support. 
   Compile with: gcc -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage test_offload_partition.c -o test_offload_partition_executable
   Run with: ./test_offload_partition_executable
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 25

/* Volatile variables to prevent constant propagation */
volatile int v_N = N;
volatile int v_M = M;
volatile int v_P = P;

/* Attribute to prevent inlining and ensure functions are compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Gang redundant - case 0 */
NOINLINE_USED int test_gang_redundant(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Gang partitioned - case 1 */
NOINLINE_USED int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 1;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Worker partitioned - case 2 */
NOINLINE_USED int test_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 2;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Gang+worker partitioned - case 3 */
NOINLINE_USED int test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 3;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Vector partitioned - case 4 */
NOINLINE_USED int test_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 4;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Gang+vector partitioned - case 5 */
NOINLINE_USED int test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 5;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Worker+vector partitioned - case 6 */
NOINLINE_USED int test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 6;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Fully partitioned - case 7 */
NOINLINE_USED int test_fully_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 7;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with kernels construct */
NOINLINE_USED int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc kernels copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 8;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with data region containing multiple compute constructs */
NOINLINE_USED int test_data_region(int n, int m, int p) {
    int arr1[N][M];
    int arr2[M][P];
    int sum = 0;
    
    #pragma acc data copy(arr1[0:n][0:m]) create(arr2[0:m][0:p])
    {
        #pragma acc parallel copy(gang: arr1[0:n][0:m])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr1[i][j] = i * m + j;
                }
            }
        }
        
        #pragma acc parallel copy(worker: arr2[0:m][0:p])
        {
            #pragma acc loop worker
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < p; j++) {
                    arr2[i][j] = arr1[i % n][j % m] + 1;
                }
            }
        }
        
        /* Conditional compute region */
        if (n > 10) {
            #pragma acc parallel copy(vector: arr1[0:n][0:m])
            {
                #pragma acc loop vector
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < m; j++) {
                        arr1[i][j] += arr2[j % m][i % p];
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr1[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with OpenMP target offloading */
NOINLINE_USED int test_omp_target(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute
        for (int i = 0; i < n; i++) {
            #pragma omp parallel for
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 100 + j + 9;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return sum & 0xFF;
}

/* Test with unstructured data directives */
NOINLINE_USED int test_unstructured_data(int n, int m) {
    int *arr = (int*)malloc(n * m * sizeof(int));
    int sum = 0;
    
    #pragma acc enter data copyin(arr[0:n*m])
    
    #pragma acc parallel present(arr[0:n*m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n*m; i++) {
            arr[i] = i + 10;
        }
    }
    
    #pragma acc exit data copyout(arr[0:n*m])
    
    for (int i = 0; i < n*m; i++) {
        sum += arr[i];
    }
    
    free(arr);
    return sum & 0xFF;
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int checksum = 0;
    int n = v_N;
    int m = v_M;
    int p = v_P;
    
    /* Execute all test functions to cover all partition codes */
    checksum ^= test_gang_redundant(n, m);
    checksum ^= test_gang_partitioned(n, m);
    checksum ^= test_worker_partitioned(n, m);
    checksum ^= test_gang_worker_partitioned(n, m);
    checksum ^= test_vector_partitioned(n, m);
    checksum ^= test_gang_vector_partitioned(n, m);
    checksum ^= test_worker_vector_partitioned(n, m);
    checksum ^= test_fully_partitioned(n, m);
    
    /* Additional tests to trigger various compiler paths */
    checksum ^= test_kernels_partition(n, m);
    checksum ^= test_data_region(n, m, p);
    checksum ^= test_omp_target(n, m);
    checksum ^= test_unstructured_data(n, m);
    
    /* Conditional execution based on volatile variable */
    if (argc > 1) {
        checksum ^= test_gang_redundant(m, p);
        checksum ^= test_worker_partitioned(p, n);
    }
    
    printf("Result: %d\n", checksum & 0xFF);
    return 0;
}
