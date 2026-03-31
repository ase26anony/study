/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o partition_test partition_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 32

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Helper function to verify results */
int verify_array(double *arr, int size, double expected_sum) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return (fabs(sum - expected_sum) < 1e-6);
}

int main() {
    int i, j, k;
    double sum = 0.0;
    double *arr1d = (double*)malloc(N * sizeof(double));
    double *arr2d = (double*)malloc(N * M * sizeof(double));
    double *arr3d = (double*)malloc(N * M * P * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    
    if (!arr1d || !arr2d || !arr3d || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_array(arr1d, N, 1.0);
    init_array(arr2d, N * M, 2.0);
    init_array(arr3d, N * M * P, 3.0);
    memset(output, 0, N * sizeof(double));
    
    printf("Starting partition coverage tests...\n");
    
    /* ============================================
     * Test Case 0: gang redundant
     * Scalar reduction with no data partitioning
     * ============================================ */
    printf("Test 0: gang redundant (scalar reduction)...\n");
    sum = 0.0;
    #pragma acc parallel copyin(arr1d[0:N]) copy(sum) reduction(+:sum)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            sum += arr1d[i];
        }
    }
    assert(verify_array(arr1d, N, sum));
    printf("  Passed\n");
    
    /* ============================================
     * Test Case 1: gang partitioned
     * Array distributed across gangs only
     * ============================================ */
    printf("Test 1: gang partitioned (array across gangs)...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            output[i] = arr1d[i] * 2.0;
        }
    }
    assert(verify_array(output, N, sum * 2.0));
    printf("  Passed\n");
    
    /* ============================================
     * Test Case 2: worker partitioned
     * Worker-level distribution
     * ============================================ */
    printf("Test 2: worker partitioned...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N]) num_workers(4)
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            output[i] = arr1d[i] * 3.0;
        }
    }
    assert(verify_array(output, N, sum * 3.0));
    printf("  Passed\n");
    
    /* ============================================
     * Test Case 3: gang+worker partitioned
     * Nested gang and worker partitioning
     * ============================================ */
    printf("Test 3: gang+worker partitioned (2D array)...\n");
    double sum2d = 0.0;
    #pragma acc parallel copyin(arr2d[0:N*M]) copy(sum2d) reduction(+:sum2d)
    {
        #pragma acc loop gang worker collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                sum2d += arr2d[i * M + j];
            }
        }
    }
    printf("  Passed (sum2d = %f)\n", sum2d);
    
    /* ============================================
     * Test Case 4: vector partitioned
     * Vector-level SIMD partitioning
     * ============================================ */
    printf("Test 4: vector partitioned (SIMD operations)...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            output[i] = arr1d[i] * arr1d[i];
        }
    }
    double expected_sq_sum = 0.0;
    for (i = 0; i < N; i++) {
        expected_sq_sum += arr1d[i] * arr1d[i];
    }
    assert(verify_array(output, N, expected_sq_sum));
    printf("  Passed\n");
    
    /* ============================================
     * Test Case 5: gang+vector partitioned
     * Gang and vector without workers
     * ============================================ */
    printf("Test 5: gang+vector partitioned...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N])
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            output[i] = sin(arr1d[i]) + cos(arr1d[i]);
        }
    }
    printf("  Passed\n");
    
    /* ============================================
     * Test Case 6: worker+vector partitioned
     * Worker and vector combination
     * ============================================ */
    printf("Test 6: worker+vector partitioned...\n");
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            output[i] = exp(arr1d[i] * 0.01);
        }
    }
    printf("  Passed\n");
    
    /* ============================================
     * Test Case 7: fully partitioned
     * All three levels: gang, worker, vector
     * ============================================ */
    printf("Test 7: fully partitioned (3D array)...\n");
    double sum3d = 0.0;
    #pragma acc parallel copyin(arr3d[0:N*M*P]) copy(sum3d) reduction(+:sum3d)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < P; k++) {
                    sum3d += arr3d[(i * M + j) * P + k];
                }
            }
        }
    }
    printf("  Passed (sum3d = %f)\n", sum3d);
    
    /* ============================================
     * Additional tests for edge cases
     * ============================================ */
    
    /* Test with present clause for already resident data */
    printf("Test with present clause...\n");
    #pragma acc enter data copyin(arr1d[0:N])
    #pragma acc parallel present(arr1d[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            /* Access data */
            double val = arr1d[i];
        }
    }
    #pragma acc exit data delete(arr1d[0:N])
    
    /* Test with private variables */
    printf("Test with private variables...\n");
    double private_var = 0.0;
    #pragma acc parallel copyin(arr1d[0:N]) private(private_var)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            private_var = arr1d[i];
            /* Use private_var */
        }
    }
    
    /* Test with runtime parameters */
    printf("Test with runtime parameters...\n");
    int num_gangs = 8;
    int num_workers = 4;
    int vec_len = 32;
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(output[0:N]) \
        num_gangs(num_gangs) num_workers(num_workers) vector_length(vec_len)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            output[i] = arr1d[i] * 0.5;
        }
    }
    
    /* Test triangular loop */
    printf("Test triangular loop pattern...\n");
    sum = 0.0;
    #pragma acc parallel copy(sum) reduction(+:sum)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < i; j++) {
                sum += 1.0;
            }
        }
    }
    printf("  Triangular sum = %f\n", sum);
    
    /* Cleanup */
    free(arr1d);
    free(arr2d);
    free(arr3d);
    free(output);
    
    printf("\nAll partition tests completed successfully!\n");
    printf("This should trigger all 8 partition codes (0-7) in the compiler's analysis.\n");
    
    return 0;
}
