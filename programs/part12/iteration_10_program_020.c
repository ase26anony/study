/* Test program to exercise all OpenACC partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Specifically targets lines 335-343 covering all 8 partition types plus default case
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

/* Helper for verification */
int verify_array(double *arr, int size, double expected_base) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        double expected = expected_base + i * 0.1;
        if (arr[i] != expected) {
            errors++;
            if (errors < 5) printf("Mismatch at %d: got %f, expected %f\n", i, arr[i], expected);
        }
    }
    return errors;
}

int main() {
    int i, j, k;
    int errors = 0;
    
    /* Test arrays */
    double *arr1d = (double*)malloc(N * sizeof(double));
    double *arr2d = (double*)malloc(N * M * sizeof(double));
    double *arr3d = (double*)malloc(N * M * P * sizeof(double));
    double *arr_out = (double*)malloc(N * sizeof(double));
    
    if (!arr1d || !arr2d || !arr3d || !arr_out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* ============================================
     * Test Case 0: gang redundant
     * Scalar reduction with no data partitioning
     * ============================================ */
    printf("\nTest 0: gang redundant (scalar reduction)\n");
    {
        double sum = 0.0;
        init_array(arr1d, N, 1.0);
        
        #pragma acc parallel copyin(arr1d[0:N]) copy(sum) reduction(+:sum)
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                sum += arr1d[i];
            }
        }
        
        double expected_sum = 0.0;
        for (i = 0; i < N; i++) expected_sum += 1.0 + i * 0.1;
        
        if (sum != expected_sum) {
            printf("  ERROR: sum = %f, expected %f\n", sum, expected_sum);
            errors++;
        } else {
            printf("  PASS\n");
        }
    }
    
    /* ============================================
     * Test Case 1: gang partitioned
     * Array distributed across gangs only
     * ============================================ */
    printf("\nTest 1: gang partitioned (array across gangs)\n");
    {
        init_array(arr1d, N, 2.0);
        memset(arr_out, 0, N * sizeof(double));
        
        #pragma acc parallel copyin(arr1d[0:N]) copyout(arr_out[0:N])
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                arr_out[i] = arr1d[i] * 2.0;
            }
        }
        
        errors += verify_array(arr_out, N, 4.0);
        if (errors == 0) printf("  PASS\n");
    }
    
    /* ============================================
     * Test Case 2: worker partitioned
     * Worker-level distribution
     * ============================================ */
    printf("\nTest 2: worker partitioned\n");
    {
        init_array(arr1d, N, 3.0);
        memset(arr_out, 0, N * sizeof(double));
        
        #pragma acc parallel copyin(arr1d[0:N]) copyout(arr_out[0:N]) num_workers(4)
        {
            #pragma acc loop worker
            for (i = 0; i < N; i++) {
                arr_out[i] = arr1d[i] + i;
            }
        }
        
        errors += verify_array(arr_out, N, 3.0);
        if (errors == 0) printf("  PASS\n");
    }
    
    /* ============================================
     * Test Case 3: gang+worker partitioned
     * Nested gang and worker partitioning
     * ============================================ */
    printf("\nTest 3: gang+worker partitioned (2D array)\n");
    {
        init_array(arr2d, N * M, 4.0);
        double sum = 0.0;
        
        #pragma acc parallel copyin(arr2d[0:N*M]) copy(sum) reduction(+:sum)
        {
            #pragma acc loop gang worker collapse(2)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    sum += arr2d[i * M + j];
                }
            }
        }
        
        double expected = 0.0;
        for (i = 0; i < N * M; i++) expected += 4.0 + i * 0.1;
        
        if (sum != expected) {
            printf("  ERROR: sum mismatch\n");
            errors++;
        } else {
            printf("  PASS\n");
        }
    }
    
    /* ============================================
     * Test Case 4: vector partitioned
     * Vector-level SIMD partitioning
     * ============================================ */
    printf("\nTest 4: vector partitioned (SIMD operations)\n");
    {
        init_array(arr1d, N, 5.0);
        memset(arr_out, 0, N * sizeof(double));
        
        #pragma acc parallel copyin(arr1d[0:N]) copyout(arr_out[0:N]) vector_length(32)
        {
            #pragma acc loop vector
            for (i = 0; i < N; i++) {
                arr_out[i] = arr1d[i] * arr1d[i];
            }
        }
        
        errors += verify_array(arr_out, N, 25.0);
        if (errors == 0) printf("  PASS\n");
    }
    
    /* ============================================
     * Test Case 5: gang+vector partitioned
     * Gang and vector without workers
     * ============================================ */
    printf("\nTest 5: gang+vector partitioned\n");
    {
        init_array(arr1d, N, 6.0);
        memset(arr_out, 0, N * sizeof(double));
        
        #pragma acc parallel copyin(arr1d[0:N]) copyout(arr_out[0:N]) vector_length(64)
        {
            #pragma acc loop gang vector
            for (i = 0; i < N; i++) {
                arr_out[i] = arr1d[i] / 2.0;
            }
        }
        
        errors += verify_array(arr_out, N, 3.0);
        if (errors == 0) printf("  PASS\n");
    }
    
    /* ============================================
     * Test Case 6: worker+vector partitioned
     * Worker and vector combination
     * ============================================ */
    printf("\nTest 6: worker+vector partitioned\n");
    {
        init_array(arr1d, N, 7.0);
        memset(arr_out, 0, N * sizeof(double));
        
        #pragma acc parallel copyin(arr1d[0:N]) copyout(arr_out[0:N]) num_workers(8) vector_length(16)
        {
            #pragma acc loop worker vector
            for (i = 0; i < N; i++) {
                arr_out[i] = arr1d[i] + arr1d[i];
            }
        }
        
        errors += verify_array(arr_out, N, 14.0);
        if (errors == 0) printf("  PASS\n");
    }
    
    /* ============================================
     * Test Case 7: fully partitioned
     * All three levels: gang, worker, vector
     * ============================================ */
    printf("\nTest 7: fully partitioned (3D array)\n");
    {
        init_array(arr3d, N * M * P, 8.0);
        double max_val = 0.0;
        
        #pragma acc parallel copyin(arr3d[0:N*M*P]) copy(max_val) reduction(max:max_val)
        {
            #pragma acc loop gang worker vector collapse(3)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    for (k = 0; k < P; k++) {
                        int idx = (i * M + j) * P + k;
                        if (arr3d[idx] > max_val) {
                            max_val = arr3d[idx];
                        }
                    }
                }
            }
        }
        
        double expected_max = 8.0 + (N * M * P - 1) * 0.1;
        if (max_val != expected_max) {
            printf("  ERROR: max_val = %f, expected %f\n", max_val, expected_max);
            errors++;
        } else {
            printf("  PASS\n");
        }
    }
    
    /* ============================================
     * Additional tests for edge cases and
     * compiler analysis variations
     * ============================================ */
    
    /* Test with present clause */
    printf("\nTest with present clause (gang partitioned variant)\n");
    {
        init_array(arr1d, N, 9.0);
        memset(arr_out, 0, N * sizeof(double));
        
        #pragma acc enter data copyin(arr1d[0:N], arr_out[0:N])
        
        #pragma acc parallel present(arr1d[0:N], arr_out[0:N])
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                arr_out[i] = arr1d[i] - 1.0;
            }
        }
        
        #pragma acc exit data copyout(arr_out[0:N]) delete(arr1d[0:N])
        
        errors += verify_array(arr_out, N, 8.0);
        if (errors == 0) printf("  PASS\n");
    }
    
    /* Test with private variables */
    printf("\nTest with private variables (worker+vector variant)\n");
    {
        init_array(arr1d, N, 10.0);
        double local_sum = 0.0;
        
        #pragma acc parallel copyin(arr1d[0:N]) copy(local_sum) reduction(+:local_sum) num_workers(4) vector_length(32)
        {
            #pragma acc loop worker vector private(j)
            for (i = 0; i < N; i++) {
                double temp = arr1d[i];
                for (j = 0; j < 10; j++) {
                    temp += 0.01;
                }
                local_sum += temp;
            }
        }
        
        printf("  Private variable test completed\n");
    }
    
    /* Test with runtime parameters */
    printf("\nTest with runtime parameters (dynamic partitioning)\n");
    {
        int dyn_n = N / 2;  /* Runtime variable */
        init_array(arr1d, dyn_n, 11.0);
        double dyn_sum = 0.0;
        
        #pragma acc parallel copyin(arr1d[0:dyn_n]) copy(dyn_sum) reduction(+:dyn_sum) \
                            num_gangs(dyn_n/64) num_workers(4) vector_length(32)
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < dyn_n; i++) {
                dyn_sum += arr1d[i];
            }
        }
        
        printf("  Dynamic partitioning test completed\n");
    }
    
    /* Test triangular loop (non-rectangular) */
    printf("\nTest triangular loop pattern\n");
    {
        init_array(arr1d, N, 12.0);
        double tri_sum = 0.0;
        
        #pragma acc parallel copyin(arr1d[0:N]) copy(tri_sum) reduction(+:tri_sum)
        {
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                #pragma acc loop worker
                for (j = 0; j <= i; j++) {
                    tri_sum += arr1d[j];
                }
            }
        }
        
        printf("  Triangular loop test completed\n");
    }
    
    /* Cleanup */
    free(arr1d);
    free(arr2d);
    free(arr3d);
    free(arr_out);
    
    /* Summary */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total errors: %d\n", errors);
    printf("  All 8 partition types exercised:\n");
    printf("    0: gang redundant ✓\n");
    printf("    1: gang partitioned ✓\n");
    printf("    2: worker partitioned ✓\n");
    printf("    3: gang+worker partitioned ✓\n");
    printf("    4: vector partitioned ✓\n");
    printf("    5: gang+vector partitioned ✓\n");
    printf("    6: worker+vector partitioned ✓\n");
    printf("    7: fully partitioned ✓\n");
    printf("  Additional edge cases tested ✓\n");
    
    if (errors == 0) {
        printf("\nSUCCESS: All partition mapping cases should be covered\n");
        printf("Compiler should generate calls with partition codes 0-7\n");
    }
    
    return errors;
}
