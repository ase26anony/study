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
void test_gang_redundant(float *arr, int size) {
    float val = 3.14f;
    
    #pragma acc parallel copy(arr[0:size]) copyin(val) gang(1)
    {
        /* Simple assignment - no loop */
        arr[0] = val;
    }
    
    /* Also test with a single-gang loop */
    #pragma acc parallel loop gang(1) copy(arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * 2.0f;
    }
}

/* Test 2: gang partitioned (case 1)
 * Outer loop marked explicitly as gang partitioned
 */
void test_gang_partitioned(float *arr, int size) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] = (float)i * 0.5f;
        sum += arr[i];
    }
    
    /* Use result to prevent dead code elimination */
    arr[0] = sum / size;
}

/* Test 3: worker partitioned (case 2)
 * Inner loop marked as worker partitioned within a nested structure
 */
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

/* Test 4: gang+worker partitioned (case 3)
 * Explicit gang and worker clauses on nested loops
 */
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

/* Test 5: vector partitioned (case 4)
 * Loop marked for vector partitioning with element-wise operations
 */
void test_vector_partitioned(float *arr, int size) {
    #pragma acc parallel loop vector copy(arr[0:size])
    for (int i = 0; i < size; i++) {
        /* Element-wise operation suitable for vectorization */
        arr[i] = arr[i] * arr[i] + 1.0f;
    }
}

/* Test 6: gang+vector partitioned (case 5)
 * Combined gang and vector partitioning
 */
void test_gang_vector_partitioned(float *arr, int size) {
    #pragma acc parallel loop gang vector copy(arr[0:size])
    for (int i = 0; i < size; i++) {
        arr[i] = sqrtf(arr[i] + 1.0f);
    }
}

/* Test 7: worker+vector partitioned (case 6)
 * Combined worker and vector partitioning
 */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = sinf(arr[i][j]) + cosf(arr[i][j]);
            }
        }
    }
}

/* Test 8: fully partitioned (case 7)
 * Triple-nested loop with explicit gang, worker, and vector clauses
 * Performing a stencil-like computation to create data dependencies
 */
void test_fully_partitioned(float arr[M][M]) {
    float temp[M][M];
    
    /* Initialize temp array */
    #pragma acc parallel loop gang collapse(2) copy(temp[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            temp[i][j] = (float)(i * M + j);
        }
    }
    
    /* Complex nested computation with all three levels */
    #pragma acc parallel copy(arr[0:M][0:M]) copyin(temp[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    /* Stencil computation with data dependencies */
                    arr[i][j] = (temp[i-1][j] + temp[i][j-1] + 
                                temp[i+1][j] + temp[i][j+1]) * 0.25f;
                }
            }
        }
    }
}

/* Helper function to initialize arrays */
void init_array(float *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = (float)i;
    }
}

void init_2d_array(float arr[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (float)(i * M + j);
        }
    }
}

int main(int argc, char *argv[]) {
    float arr1[N];
    float arr2[M][M];
    float arr3[N];
    
    /* Initialize arrays */
    init_array(arr1, N);
    init_2d_array(arr2);
    init_array(arr3, N);
    
    /* Use command-line argument to control which tests run
     * This ensures all OpenACC constructs are processed by the compiler
     * even if not all are executed at runtime
     */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Conditional execution to force compiler analysis of all paths */
    if (test_case == 0 || test_case == 1) {
        test_gang_redundant(arr1, N);
    }
    
    if (test_case == 0 || test_case == 2) {
        test_gang_partitioned(arr1, N);
    }
    
    if (test_case == 0 || test_case == 3) {
        test_worker_partitioned(arr2);
    }
    
    if (test_case == 0 || test_case == 4) {
        test_gang_worker_partitioned(arr2);
    }
    
    if (test_case == 0 || test_case == 5) {
        test_vector_partitioned(arr3, N);
    }
    
    if (test_case == 0 || test_case == 6) {
        test_gang_vector_partitioned(arr3, N);
    }
    
    if (test_case == 0 || test_case == 7) {
        test_worker_vector_partitioned(arr2);
    }
    
    if (test_case == 0 || test_case == 8) {
        test_fully_partitioned(arr2);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("arr1[0] = %f, arr1[%d] = %f\n", arr1[0], N-1, arr1[N-1]);
    printf("arr2[0][0] = %f, arr2[%d][%d] = %f\n", 
           arr2[0][0], M-1, M-1, arr2[M-1][M-1]);
    printf("arr3[0] = %f, arr3[%d] = %f\n", arr3[0], N-1, arr3[N-1]);
    
    return 0;
}
