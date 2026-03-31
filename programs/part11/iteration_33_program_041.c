/* test_oacc_partition.c
 * 
 * This program exercises GCC's OpenACC partitioning logic to trigger
 * the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * Each test function targets a specific partitioning pattern.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Test 1: gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1)
 */
void test_gang_redundant(float *arr, int n) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyin(n) reduction(+:local_sum) num_gangs(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            arr[i] = i * 2.0f;
            local_sum += arr[i];
        }
    }
    
    printf("Gang redundant: sum = %f\n", local_sum);
}

/* Test 2: gang partitioned (case 1)
 * Single loop with explicit gang partitioning
 */
void test_gang_partitioned(float *arr, int n) {
    #pragma acc parallel loop gang copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 3.0f + i;
    }
    
    printf("Gang partitioned: arr[0]=%f, arr[%d]=%f\n", arr[0], n-1, arr[n-1]);
}

/* Test 3: worker partitioned (case 2)
 * Inner loop marked worker within a parallel region
 */
void test_worker_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m]) copyin(m)
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = (float)(i + j);
            }
        }
    }
    
    printf("Worker partitioned: arr[%d][%d] = %f\n", m-1, m-1, arr[m-1][m-1]);
}

/* Test 4: gang+worker partitioned (case 3)
 * Nested loops with gang and worker clauses
 */
void test_gang_worker_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m]) copyin(m)
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = arr[i][j] * 2.0f - (float)(i * j);
            }
        }
    }
    
    printf("Gang+worker partitioned: arr[0][0] = %f\n", arr[0][0]);
}

/* Test 5: vector partitioned (case 4)
 * Loop with vector clause for element-wise operations
 */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] / 2.0f + 1.0f;
    }
    
    printf("Vector partitioned: arr[%d] = %f\n", n/2, arr[n/2]);
}

/* Test 6: gang+vector partitioned (case 5)
 * Loop with both gang and vector clauses
 */
void test_gang_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop gang vector copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * arr[i] - (float)i;
    }
    
    printf("Gang+vector partitioned: arr[10] = %f\n", arr[10]);
}

/* Test 7: worker+vector partitioned (case 6)
 * Nested loops with worker and vector clauses
 */
void test_worker_vector_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m]) copyin(m)
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < m; j++) {
                if (i > 0 && j > 0) {
                    arr[i][j] = (arr[i-1][j] + arr[i][j-1]) * 0.5f;
                }
            }
        }
    }
    
    printf("Worker+vector partitioned: arr[%d][%d] = %f\n", 
           m-1, m-1, arr[m-1][m-1]);
}

/* Test 8: fully partitioned (case 7)
 * Triple-nested loop with gang, worker, and vector clauses
 * Performing a stencil-like computation
 */
void test_fully_partitioned(float arr[M][M][M], int m) {
    // Initialize the 3D array
    #pragma acc parallel loop collapse(3) copy(arr[0:m][0:m][0:m]) copyin(m)
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < m; k++) {
                arr[i][j][k] = (float)(i + j + k);
            }
        }
    }
    
    // Fully partitioned computation
    #pragma acc parallel copy(arr[0:m][0:m][0:m]) copyin(m)
    {
        #pragma acc loop gang
        for (int i = 1; i < m-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < m-1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < m-1; k++) {
                    // 3D stencil computation
                    arr[i][j][k] = (arr[i-1][j][k] + arr[i+1][j][k] +
                                   arr[i][j-1][k] + arr[i][j+1][k] +
                                   arr[i][j][k-1] + arr[i][j][k+1]) / 6.0f;
                }
            }
        }
    }
    
    printf("Fully partitioned: arr[%d][%d][%d] = %f\n", 
           m/2, m/2, m/2, arr[m/2][m/2][m/2]);
}

int main(int argc, char *argv[]) {
    // Initialize data arrays
    float arr1[N];
    float arr2[M][M];
    float arr3[M][M][M];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i * j);
        }
    }
    
    // Use command-line argument to control execution
    // This ensures all OpenACC constructs are processed by the compiler
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 9; // 0-8 for the 8 cases + default
    }
    
    // Conditional execution to force compiler analysis of all paths
    if (argc > 0) { // Always true, but compiler doesn't know
        switch (test_case) {
            case 0:
                test_gang_redundant(arr1, N);
                break;
            case 1:
                test_gang_partitioned(arr1, N);
                break;
            case 2:
                test_worker_partitioned(arr2, M);
                break;
            case 3:
                test_gang_worker_partitioned(arr2, M);
                break;
            case 4:
                test_vector_partitioned(arr1, N);
                break;
            case 5:
                test_gang_vector_partitioned(arr1, N);
                break;
            case 6:
                test_worker_vector_partitioned(arr2, M);
                break;
            case 7:
                test_fully_partitioned(arr3, M);
                break;
            default:
                // Run all tests to ensure all switch cases are compiled
                test_gang_redundant(arr1, N);
                test_gang_partitioned(arr1, N);
                test_worker_partitioned(arr2, M);
                test_gang_worker_partitioned(arr2, M);
                test_vector_partitioned(arr1, N);
                test_gang_vector_partitioned(arr1, N);
                test_worker_vector_partitioned(arr2, M);
                test_fully_partitioned(arr3, M);
                break;
        }
    }
    
    // Print some results to prevent dead code elimination
    printf("Final check: arr1[0] = %f, arr2[0][0] = %f\n", arr1[0], arr2[0][0]);
    
    return 0;
}
