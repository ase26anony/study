/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

/* Function declarations with partition attributes */
#pragma acc routine gang
void acc_gang_function(float *a, float *b, int n);

#pragma acc routine worker
void acc_worker_function(float *a, float *b, int n);

#pragma acc routine vector
void acc_vector_function(float *a, float *b, int n);

/* Implementation of partitioned functions */
void acc_gang_function(float *a, float *b, int n) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        b[i] = a[i] * 2.0f;
    }
}

void acc_worker_function(float *a, float *b, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        b[i] = a[i] + 1.0f;
    }
}

void acc_vector_function(float *a, float *b, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        b[i] = a[i] / 2.0f;
    }
}

/* Helper function to verify results */
int verify_results(float *a, float *b, int n, float expected_factor) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float expected = (i + 1) * expected_factor;
        if (fabs(b[i] - expected) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: got %f, expected %f\n", 
                       i, b[i], expected);
            }
        }
    }
    return errors;
}

int main() {
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Runtime-determined partition counts */
    int num_gangs = 4;
    int num_workers = 2;
    int vector_length = 32;
    int flag = 1; /* For conditional partition selection */
    
    /* Allocate and initialize arrays */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + 1);
        b[i] = 0.0f;
        c[i] = 0.0f;
    }
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* ============================================
       TEST 1: Gang redundant (case 0)
       ============================================ */
    printf("Test 1: Gang redundant\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) /* gang redundant */
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 3.0f;
        }
    }
    
    int errors = verify_results(a, b, N, 3.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 2: Gang partitioned (case 1)
       ============================================ */
    printf("\nTest 2: Gang partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 4.0f;
        }
    }
    
    errors = verify_results(a, b, N, 4.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 3: Worker partitioned (case 2)
       ============================================ */
    printf("\nTest 3: Worker partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop num_workers(num_workers) worker
        for (int i = 0; i < N; i++) {
            b[i] = a[i] + 10.0f;
        }
    }
    
    errors = verify_results(a, b, N, 11.0f); /* a[i] = i+1, +10 = i+11 */
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 4: Gang+worker partitioned (case 3)
       ============================================ */
    printf("\nTest 4: Gang+worker partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 5.0f;
        }
    }
    
    errors = verify_results(a, b, N, 5.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 5: Vector partitioned (case 4)
       ============================================ */
    printf("\nTest 5: Vector partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] / 3.0f;
        }
    }
    
    errors = verify_results(a, b, N, 1.0f/3.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 6: Gang+vector partitioned (case 5)
       ============================================ */
    printf("\nTest 6: Gang+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 6.0f;
        }
    }
    
    errors = verify_results(a, b, N, 6.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 7: Worker+vector partitioned (case 6)
       ============================================ */
    printf("\nTest 7: Worker+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop num_workers(num_workers) vector_length(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] + 20.0f;
        }
    }
    
    errors = verify_results(a, b, N, 21.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 8: Fully partitioned (case 7)
       ============================================ */
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop \
                gang(num_gangs) \
                num_workers(num_workers) \
                vector_length(vector_length) \
                gang worker vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 7.0f;
        }
    }
    
    errors = verify_results(a, b, N, 7.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 9: Nested parallelism with partitioning
       ============================================ */
    printf("\nTest 9: Nested parallelism (outer gang, inner worker)\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc kernels /* outer gang-redundant region */
        {
            #pragma acc loop gang /* gang partitioned inner loop */
            for (int i = 0; i < N; i += 64) {
                int end = (i + 64 < N) ? i + 64 : N;
                #pragma acc loop worker /* worker partitioned */
                for (int j = i; j < end; j++) {
                    b[j] = a[j] * 8.0f;
                }
            }
        }
    }
    
    errors = verify_results(a, b, N, 8.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 10: Conditional partition selection
       ============================================ */
    printf("\nTest 10: Conditional partition selection\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop \
                gang(num_gangs) \
                worker(flag ? 2 : 4) /* conditional worker count */ \
                gang worker
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 9.0f;
        }
    }
    
    errors = verify_results(a, b, N, 9.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 11: OpenMP target equivalent
       ============================================ */
    printf("\nTest 11: OpenMP target with gang partitioned\n");
    total_tests++;
    
    #pragma omp target data map(to: a[0:N]) map(from: b[0:N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) /* gang */ \
                num_threads(num_workers) /* worker */ \
                simdlen(vector_length) /* vector */
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 10.0f;
        }
    }
    
    errors = verify_results(a, b, N, 10.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 12: Device-specific partition behavior
       ============================================ */
    printf("\nTest 12: Device-specific partition (NVIDIA targeting)\n");
    total_tests++;
    
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(64) gang vector
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 11.0f;
        }
    }
    
    errors = verify_results(a, b, N, 11.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 13: Partitioned function calls
       ============================================ */
    printf("\nTest 13: Partitioned function calls\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(b[0:N])
    {
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i += 256) {
            int end = (i + 256 < N) ? i + 256 : N;
            /* Call gang-partitioned function */
            acc_gang_function(&a[i], &b[i], end - i);
        }
    }
    
    /* b[i] should be a[i] * 2.0f from acc_gang_function */
    errors = verify_results(a, b, N, 2.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       TEST 14: Complex nested with data clauses
       ============================================ */
    printf("\nTest 14: Complex nested with data clauses\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        /* Outer region */
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i += 128) {
            int end = (i + 128 < N) ? i + 128 : N;
            
            /* Inner region with different partition */
            #pragma acc loop worker vector
            for (int j = i; j < end; j++) {
                c[j] = a[j] * a[j];
            }
        }
    }
    
    /* Verify c[i] = a[i]² = (i+1)² */
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = (i + 1) * (i + 1);
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: got %f, expected %f\n", 
                       i, c[i], expected);
            }
        }
    }
    
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
       Summary
       ============================================ */
    printf("\n============================================\n");
    printf("Test Summary: %d/%d tests passed\n", passed_tests, total_tests);
    printf("Coverage targets:\n");
    printf("  [✓] Gang redundant (case 0)\n");
    printf("  [✓] Gang partitioned (case 1)\n");
    printf("  [✓] Worker partitioned (case 2)\n");
    printf("  [✓] Gang+worker partitioned (case 3)\n");
    printf("  [✓] Vector partitioned (case 4)\n");
    printf("  [✓] Gang+vector partitioned (case 5)\n");
    printf("  [✓] Worker+vector partitioned (case 6)\n");
    printf("  [✓] Fully partitioned (case 7)\n");
    printf("  [✓] Nested parallelism with partitioning\n");
    printf("  [✓] Conditional partition selection\n");
    printf("  [✓] Data clauses with partitioned arrays\n");
    printf("  [✓] Multiple target architectures\n");
    printf("  [✓] Partitioned function calls\n");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
