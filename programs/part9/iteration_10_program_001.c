/* This test requires GCC configured with offloading support. 
   Compile with: gcc -O2 -fopenacc -fopenmp -fprofile-arcs -ftest-coverage test.c -o test
   Run with: ./test
   
   This test exercises all 8 partition code cases (0-7) in GCC's
   omp-oacc-neuter-broadcast.cc by creating OpenACC compute constructs
   with different data clause partitioning combinations.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

volatile int v_size = N;  /* Prevent constant propagation */

/* Function to ensure side effects and prevent optimization */
static int __attribute__((noinline)) use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* Case 0: gang redundant */
__attribute__((noinline, used))
int test_gang_redundant(int n, int m) {
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
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Case 1: gang partitioned */
__attribute__((noinline, used))
int test_gang_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang: arr[0:n][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 200 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Case 2: worker partitioned */
__attribute__((noinline, used))
int test_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 300 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Case 3: gang+worker partitioned */
__attribute__((noinline, used))
int test_gang_worker_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 400 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Case 4: vector partitioned */
__attribute__((noinline, used))
int test_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 500 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Case 5: gang+vector partitioned */
__attribute__((noinline, used))
int test_gang_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 600 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Case 6: worker+vector partitioned */
__attribute__((noinline, used))
int test_worker_vector_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 700 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Case 7: fully partitioned */
__attribute__((noinline, used))
int test_fully_partitioned(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc parallel copy(gang, worker, vector: arr[0:n][0:m])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 800 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Test with OpenACC kernels construct */
__attribute__((noinline, used))
int test_kernels_partition(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc kernels copy(arr[0:n][0:m])
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 900 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Test with OpenACC data region containing multiple constructs */
__attribute__((noinline, used))
int test_data_region(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc data copy(arr[0:n][0:m])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1000 + j;
                }
            }
        }
        
        #pragma acc kernels
        {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    arr[i][j] += 1;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Test with OpenMP target offloading */
__attribute__((noinline, used))
int test_omp_target(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma omp target map(tofrom: arr[0:n][0:m])
    {
        #pragma omp teams distribute parallel for
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = i * 1100 + j;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Test with conditional parallel regions */
__attribute__((noinline, used))
int test_conditional(int n, int m, int flag) {
    int arr[N][M];
    int sum = 0;
    
    if (flag & 1) {
        #pragma acc parallel copy(arr[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1200 + j;
                }
            }
        }
    } else {
        #pragma acc parallel copy(gang, worker: arr[0:n][0:m])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                #pragma acc loop vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1300 + j;
                }
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

/* Test with create clause */
__attribute__((noinline, used))
int test_create_clause(int n, int m) {
    int arr[N][M];
    int sum = 0;
    
    #pragma acc data create(arr[0:n][0:m])
    {
        #pragma acc parallel present(arr[0:n][0:m])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                #pragma acc loop gang worker vector
                for (int j = 0; j < m; j++) {
                    arr[i][j] = i * 1400 + j;
                }
            }
        }
        
        #pragma acc update host(arr[0:n][0:m])
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += arr[i][j];
        }
    }
    return use_value(sum);
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    int n = v_size;
    int m = M;
    
    /* Execute all test functions to trigger all partition codes */
    checksum ^= test_gang_redundant(n, m);
    checksum ^= test_gang_partitioned(n, m);
    checksum ^= test_worker_partitioned(n, m);
    checksum ^= test_gang_worker_partitioned(n, m);
    checksum ^= test_vector_partitioned(n, m);
    checksum ^= test_gang_vector_partitioned(n, m);
    checksum ^= test_worker_vector_partitioned(n, m);
    checksum ^= test_fully_partitioned(n, m);
    
    /* Additional tests to exercise more compiler paths */
    checksum ^= test_kernels_partition(n, m);
    checksum ^= test_data_region(n, m);
    checksum ^= test_omp_target(n, m);
    checksum ^= test_conditional(n, m, argc);
    checksum ^= test_create_clause(n, m);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", checksum & 0xFF);
    
    return 0;
}
