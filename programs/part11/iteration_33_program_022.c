/* test_oacc_partition.c
 * 
 * This program exercises GCC's OpenACC partitioning logic to cover
 * the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * Each test function targets a specific partitioning pattern.
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define DIM 32

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1) */
void test_gang_redundant(float *arr, int n) {
    float scalar = 3.14f;
    
    #pragma acc parallel copy(arr[0:n]) copyin(scalar)
    {
        /* No loop - all gangs execute the same code redundantly */
        arr[0] = scalar;
        arr[n-1] = scalar * 2.0f;
    }
    
    /* Also test with gang(1) explicit */
    #pragma acc parallel loop gang(1) copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] + 1.0f;
    }
}

/* Test 2: Gang partitioned (case 1)
 * Outer loop partitioned across gangs with reduction */
void test_gang_partitioned(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(arr[0:n]) copyout(sum)
    for (int i = 0; i < n; i++) {
        arr[i] = (float)i * 0.5f;
        sum += arr[i];
    }
    
    printf("Gang partitioned sum: %f\n", sum);
}

/* Test 3: Worker partitioned (case 2)
 * Inner loop partitioned across workers within a nested loop */
void test_worker_partitioned(float arr[DIM][DIM]) {
    #pragma acc parallel copy(arr[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker
            for (int j = 0; j < DIM; j++) {
                arr[i][j] = (float)(i + j);
            }
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3)
 * Nested loops with gang and worker clauses */
void test_gang_worker_partitioned(float arr[DIM][DIM]) {
    #pragma acc parallel copy(arr[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker
            for (int j = 0; j < DIM; j++) {
                arr[i][j] = arr[i][j] * 2.0f;
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4)
 * Loop with vector clause for element-wise operations */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * arr[i];  /* Square each element */
    }
}

/* Test 6: Gang+vector partitioned (case 5)
 * Loop with both gang and vector clauses */
void test_gang_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop gang vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = sqrtf(arr[i] + 1.0f);
    }
}

/* Test 7: Worker+vector partitioned (case 6)
 * Nested loops with worker and vector clauses */
void test_worker_vector_partitioned(float arr[DIM][DIM]) {
    #pragma acc parallel copy(arr[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < DIM; j++) {
                arr[i][j] = sinf(arr[i][j]);
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loop with gang, worker, and vector clauses
 * Performing a stencil-like computation */
void test_fully_partitioned(float arr3d[DIM][DIM][DIM]) {
    /* Initialize the 3D array */
    #pragma acc parallel loop collapse(3) copy(arr3d[0:DIM][0:DIM][0:DIM])
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            for (int k = 0; k < DIM; k++) {
                arr3d[i][j][k] = (float)(i + j + k);
            }
        }
    }
    
    /* Stencil computation with explicit partitioning */
    #pragma acc parallel copy(arr3d[0:DIM][0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 1; i < DIM-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < DIM-1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < DIM-1; k++) {
                    /* Simple 7-point stencil */
                    arr3d[i][j][k] = (arr3d[i-1][j][k] + arr3d[i+1][j][k] +
                                     arr3d[i][j-1][k] + arr3d[i][j+1][k] +
                                     arr3d[i][j][k-1] + arr3d[i][j][k+1]) / 6.0f;
                }
            }
        }
    }
}

/* Helper to initialize arrays */
void init_array(float *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = (float)i;
    }
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize test arrays */
    float arr1[SIZE];
    float arr2[DIM][DIM];
    float arr3d[DIM][DIM][DIM];
    
    init_array(arr1, SIZE);
    
    /* Use command-line argument to control which tests run
     * This ensures all OpenACC constructs are processed by the compiler
     * even if not all are executed at runtime */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Conditional execution to force compiler analysis of all paths */
    if (test_case == 0 || argc == 1) {
        /* Run all tests when no argument or test_case == 0 */
        test_gang_redundant(arr1, SIZE);
        test_gang_partitioned(arr1, SIZE);
        test_worker_partitioned(arr2);
        test_gang_worker_partitioned(arr2);
        test_vector_partitioned(arr1, SIZE);
        test_gang_vector_partitioned(arr1, SIZE);
        test_worker_vector_partitioned(arr2);
        test_fully_partitioned(arr3d);
    } else {
        /* Run specific test based on argument */
        switch (test_case) {
            case 1: test_gang_redundant(arr1, SIZE); break;
            case 2: test_gang_partitioned(arr1, SIZE); break;
            case 3: test_worker_partitioned(arr2); break;
            case 4: test_gang_worker_partitioned(arr2); break;
            case 5: test_vector_partitioned(arr1, SIZE); break;
            case 6: test_gang_vector_partitioned(arr1, SIZE); break;
            case 7: test_worker_vector_partitioned(arr2); break;
            case 8: test_fully_partitioned(arr3d); break;
            default: break;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: arr1[0]=%f, arr1[%d]=%f\n", 
           arr1[0], SIZE-1, arr1[SIZE-1]);
    printf("arr2[0][0]=%f, arr2[%d][%d]=%f\n",
           arr2[0][0], DIM-1, DIM-1, arr2[DIM-1][DIM-1]);
    printf("arr3d[0][0][0]=%f\n", arr3d[0][0][0]);
    
    return 0;
}
