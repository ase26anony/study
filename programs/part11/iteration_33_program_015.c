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

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1) */
void test_gang_redundant(float *arr) {
    int i;
    #pragma acc parallel copy(arr[0:N]) gang(1)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            arr[i] = i * 2.0f;
        }
    }
}

/* Test 2: Gang partitioned (case 1)
 * Simple loop with explicit gang partitioning */
void test_gang_partitioned(float *arr, float *result) {
    float sum = 0.0f;
    int i;
    
    #pragma acc parallel copy(arr[0:N]) copyout(result[0:1]) \
        reduction(+:sum) num_gangs(8)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    result[0] = sum;
}

/* Test 3: Worker partitioned (case 2)
 * Nested loops with worker partitioning on inner loop */
void test_worker_partitioned(float arr[M][M]) {
    int i, j;
    
    #pragma acc parallel copy(arr[0:M][0:M]) num_workers(4)
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                arr[i][j] = (i + j) * 0.5f;
            }
        }
    }
}

/* Test 4: Vector partitioned (case 4)
 * Element-wise operation suitable for vectorization */
void test_vector_partitioned(float *a, float *b, float *c) {
    int i;
    
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        vector_length(128)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
}

/* Test 5: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(float arr[M][M]) {
    int i, j;
    
    #pragma acc parallel copy(arr[0:M][0:M]) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] * 1.5f;
            }
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(float *arr) {
    int i;
    
    #pragma acc parallel copy(arr[0:N]) num_gangs(8) vector_length(64)
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            arr[i] = arr[i] * 3.0f - 1.0f;
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(float arr[M][M]) {
    int i, j;
    
    #pragma acc parallel copy(arr[0:M][0:M]) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + i - j) * 0.25f;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loop with explicit gang, worker, vector clauses */
void test_fully_partitioned(float arr[M][M]) {
    int i, j, k;
    float temp[M][M];
    
    #pragma acc parallel copy(arr[0:M][0:M]) create(temp[0:M][0:M]) \
        num_gangs(4) num_workers(2) vector_length(16)
    {
        /* Initialize temp array */
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                #pragma acc loop vector
                for (k = 0; k < 4; k++) {
                    temp[i][j] = 0.0f;
                }
            }
        }
        
        /* Stencil-like computation with data dependencies */
        #pragma acc loop gang
        for (i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (k = 0; k < 2; k++) {  /* Small vector loop */
                    /* Jacobi-style smoothing */
                    temp[i][j] = (arr[i-1][j] + arr[i][j-1] + 
                                 arr[i+1][j] + arr[i][j+1]) * 0.25f;
                }
            }
        }
        
        /* Copy back */
        #pragma acc loop gang
        for (i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (k = 0; k < 2; k++) {
                    arr[i][j] = temp[i][j];
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    float arr1[N], arr2[N], arr3[N];
    float arr2d[M][M];
    float result[1];
    int test_num = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i;
        arr2[i] = (float)(i * 2);
        arr3[i] = 0.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = (float)(i * M + j);
        }
    }
    
    /* Use command-line argument to select test, but ensure all are compiled */
    if (argc > 1) {
        test_num = atoi(argv[1]) % 9;  /* 0-8 */
    }
    
    /* Conditional execution to ensure compiler analyzes all paths */
    if (test_num == 0 || argc == 1) {
        test_gang_redundant(arr1);
    }
    if (test_num == 1 || argc == 1) {
        test_gang_partitioned(arr1, result);
    }
    if (test_num == 2 || argc == 1) {
        test_worker_partitioned(arr2d);
    }
    if (test_num == 3 || argc == 1) {
        test_vector_partitioned(arr1, arr2, arr3);
    }
    if (test_num == 4 || argc == 1) {
        test_gang_worker_partitioned(arr2d);
    }
    if (test_num == 5 || argc == 1) {
        test_gang_vector_partitioned(arr1);
    }
    if (test_num == 6 || argc == 1) {
        test_worker_vector_partitioned(arr2d);
    }
    if (test_num == 7 || argc == 1) {
        test_fully_partitioned(arr2d);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("arr1[0]=%.2f, arr1[%d]=%.2f\n", arr1[0], N-1, arr1[N-1]);
    printf("arr3[0]=%.2f, arr3[%d]=%.2f\n", arr3[0], N-1, arr3[N-1]);
    printf("arr2d[0][0]=%.2f, arr2d[%d][%d]=%.2f\n", 
           arr2d[0][0], M-1, M-1, arr2d[M-1][M-1]);
    if (argc > 1) {
        printf("result[0]=%.2f\n", result[0]);
    }
    
    return 0;
}
