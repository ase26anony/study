/* test_oacc_partition.c
 * 
 * This program is designed to exercise GCC's OpenACC partitioning logic
 * to cover the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * It creates multiple test functions with different OpenACC partitioning
 * patterns, each targeting specific partitioning classifications.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1)
 */
void test_gang_redundant(float *arr, int size) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:size]) copy(sum) gang(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < size; i++) {
            arr[i] = i * 1.5f;
        }
        sum = arr[0] + arr[size-1];
    }
    
    if (sum < 0) printf("unlikely"); // Prevent optimization
}

/* Test 2: Gang partitioned (case 1)
 * Single loop with explicit gang partitioning
 */
void test_gang_partitioned(float *arr, int size) {
    float reduction_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:size]) copy(reduction_sum)
    {
        #pragma acc loop gang reduction(+:reduction_sum)
        for (int i = 0; i < size; i++) {
            arr[i] = arr[i] * 2.0f + i;
            reduction_sum += arr[i];
        }
    }
    
    if (reduction_sum < 0) printf("unlikely");
}

/* Test 3: Worker partitioned (case 2)
 * Nested loops with worker partitioning on inner loop
 */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (i + j) * 0.5f;
            }
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3)
 * Explicit gang and worker clauses on nested loops
 */
void test_gang_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                // Simple stencil
                arr[i][j] = (arr[i-1][j] + arr[i][j-1]) * 0.5f;
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4)
 * Loop with vector partitioning for element-wise operations
 */
void test_vector_partitioned(float *arr, int size) {
    #pragma acc parallel copy(arr[0:size])
    {
        #pragma acc loop vector
        for (int i = 0; i < size; i++) {
            // Vector-friendly operation
            arr[i] = arr[i] * arr[i] - 2.0f * arr[i] + 1.0f;
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5)
 * Combined gang and vector partitioning
 */
void test_gang_vector_partitioned(float *arr, int size) {
    #pragma acc parallel copy(arr[0:size])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < size; i++) {
            arr[i] = 1.0f / (arr[i] + 1.0f);
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6)
 * Combined worker and vector partitioning
 */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] > 0.5f) ? 1.0f : 0.0f;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loop with explicit gang, worker, and vector clauses
 */
void test_fully_partitioned(float arr[M][M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < M-1; k++) {
                    // 3D stencil computation
                    arr[i][j][k] = (arr[i-1][j][k] + arr[i][j-1][k] + 
                                   arr[i][j][k-1]) / 3.0f;
                }
            }
        }
    }
}

/* Main driver that conditionally executes different tests
 * based on command-line arguments to ensure all code paths
 * are compiled and considered by the partitioning logic.
 */
int main(int argc, char *argv[]) {
    // Initialize test data
    float arr1[N];
    float arr2[M][M];
    float arr3[M][M][M];
    
    for (int i = 0; i < N; i++) arr1[i] = (float)i;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i * M + j);
        }
    }
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                arr3[i][j][k] = (float)(i * M * M + j * M + k);
            }
        }
    }
    
    // Use argc to control which tests run
    // This ensures all OpenACC constructs are analyzed regardless of execution path
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(arr1, N);
            break;
        case 1:
            test_gang_partitioned(arr1, N);
            break;
        case 2:
            test_worker_partitioned(arr2);
            break;
        case 3:
            test_gang_worker_partitioned(arr2);
            break;
        case 4:
            test_vector_partitioned(arr1, N);
            break;
        case 5:
            test_gang_vector_partitioned(arr1, N);
            break;
        case 6:
            test_worker_vector_partitioned(arr2);
            break;
        case 7:
            test_fully_partitioned(arr3);
            break;
        default:
            // Run all tests in sequence to exercise all partitioning patterns
            test_gang_redundant(arr1, N);
            test_gang_partitioned(arr1, N);
            test_worker_partitioned(arr2);
            test_gang_worker_partitioned(arr2);
            test_vector_partitioned(arr1, N);
            test_gang_vector_partitioned(arr1, N);
            test_worker_vector_partitioned(arr2);
            test_fully_partitioned(arr3);
            break;
    }
    
    // Print results to prevent dead code elimination
    printf("Results: arr1[0]=%.2f, arr1[%d]=%.2f\n", arr1[0], N-1, arr1[N-1]);
    printf("arr2[0][0]=%.2f, arr2[%d][%d]=%.2f\n", arr2[0][0], M-1, M-1, arr2[M-1][M-1]);
    printf("arr3[0][0][0]=%.2f\n", arr3[0][0][0]);
    
    return 0;
}
