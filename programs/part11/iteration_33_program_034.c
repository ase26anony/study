/* test_oacc_partition.c
 * 
 * This program exercises GCC's OpenACC partitioning logic to cover
 * the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * Each test function targets a specific partitioning pattern.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1) */
void test_gang_redundant(int *result) {
    int local_result = 0;
    
    #pragma acc parallel copy(local_result)
    {
        local_result = 42;  /* Simple assignment in gang-redundant region */
    }
    
    *result = local_result;
}

/* Test 2: Gang partitioned (case 1)
 * Single loop with explicit gang partitioning */
void test_gang_partitioned(float *arr) {
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = i * 2.0f;
    }
}

/* Test 3: Worker partitioned (case 2)
 * Nested loops with worker partitioning on inner loop */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (float)(i + j);
            }
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3)
 * Explicit gang and worker clauses on nested loops */
void test_gang_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] * 2.0f;
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4)
 * Loop with explicit vector partitioning */
void test_vector_partitioned(float *arr) {
    #pragma acc parallel loop vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] + 1.0f;
    }
}

/* Test 6: Gang+vector partitioned (case 5)
 * Nested loops with gang and vector partitioning */
void test_gang_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] * 3.0f;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6)
 * Nested loops with worker and vector partitioning */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    arr[i][j] += 0.25f;
                }
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loop with explicit gang, worker, and vector clauses
 * Performing a stencil-like computation */
void test_fully_partitioned(float arr[M][M]) {
    float temp[M][M];
    
    /* Initialize temp array */
    #pragma acc parallel loop gang copy(temp[0:M][0:M])
    for (int i = 0; i < M; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            temp[i][j] = (float)(i * M + j);
        }
    }
    
    /* Stencil computation with full partitioning */
    #pragma acc parallel copy(arr[0:M][0:M]) copyin(temp[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 2; k++) {  /* Small vector loop */
                    /* Simple 4-point stencil */
                    arr[i][j] = (temp[i-1][j] + temp[i+1][j] + 
                                 temp[i][j-1] + temp[i][j+1]) * 0.25f;
                }
            }
        }
    }
}

/* Test 9: Mixed partitioning with reduction
 * Combines partitioning with reduction clause */
void test_mixed_with_reduction(float *arr, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:local_sum) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
        local_sum += arr[i];
    }
    
    *sum = local_sum;
}

/* Main driver that conditionally executes tests based on command-line args */
int main(int argc, char *argv[]) {
    /* Initialize data arrays */
    float arr1[N];
    float arr2[M][M];
    float sum = 0.0f;
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = 0.0f;
        }
    }
    
    /* Use argc to control which tests run, ensuring all code paths are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(&result);
            printf("Gang redundant result: %d\n", result);
            break;
        case 1:
            test_gang_partitioned(arr1);
            printf("Gang partitioned: arr1[0]=%.2f, arr1[%d]=%.2f\n", 
                   arr1[0], N-1, arr1[N-1]);
            break;
        case 2:
            test_worker_partitioned(arr2);
            printf("Worker partitioned: arr2[0][0]=%.2f\n", arr2[0][0]);
            break;
        case 3:
            test_gang_worker_partitioned(arr2);
            printf("Gang+worker partitioned: arr2[%d][%d]=%.2f\n", 
                   M-1, M-1, arr2[M-1][M-1]);
            break;
        case 4:
            test_vector_partitioned(arr1);
            printf("Vector partitioned: arr1[10]=%.2f\n", arr1[10]);
            break;
        case 5:
            test_gang_vector_partitioned(arr2);
            printf("Gang+vector partitioned: arr2[5][5]=%.2f\n", arr2[5][5]);
            break;
        case 6:
            test_worker_vector_partitioned(arr2);
            printf("Worker+vector partitioned: arr2[10][10]=%.2f\n", arr2[10][10]);
            break;
        case 7:
            test_fully_partitioned(arr2);
            printf("Fully partitioned: arr2[15][15]=%.2f\n", arr2[15][15]);
            break;
        case 8:
            test_mixed_with_reduction(arr1, &sum);
            printf("Mixed with reduction: sum=%.2f\n", sum);
            break;
        default:
            /* Run all tests to ensure all OpenACC constructs are processed */
            test_gang_redundant(&result);
            test_gang_partitioned(arr1);
            test_worker_partitioned(arr2);
            test_gang_worker_partitioned(arr2);
            test_vector_partitioned(arr1);
            test_gang_vector_partitioned(arr2);
            test_worker_vector_partitioned(arr2);
            test_fully_partitioned(arr2);
            test_mixed_with_reduction(arr1, &sum);
            
            printf("All tests compiled and executed (if conditions met)\n");
            printf("Sample results: result=%d, sum=%.2f\n", result, sum);
            break;
    }
    
    /* Print some array values to prevent dead code elimination */
    printf("Final check - arr1[0]=%.2f, arr2[0][0]=%.2f\n", arr1[0], arr2[0][0]);
    
    return 0;
}
