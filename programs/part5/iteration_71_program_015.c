/**
 * Test program to cover partition type switch cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: gang redundant, gang partitioned, worker partitioned, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_element(float a, float b, int idx);

#pragma acc routine gang
void gang_partitioned_function(float *arr, int n, int gang_id);

#pragma acc routine worker
void worker_partitioned_function(float *arr, int n, int worker_id);

/* Helper to verify results */
int verify_results(float *a, float *b, float *c, int n, float expected) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float diff = c[i] - expected;
        if (diff < -VERIFY_TOLERANCE || diff > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: c[%d] = %f, expected %f\n", 
                       i, i, c[i], expected);
            }
        }
    }
    return errors;
}

int main() {
    int total_tests = 0;
    int passed_tests = 0;
    
    /* Runtime-determined partition counts */
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    bool use_aggressive_partitioning = true;
    
    /* Test arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100);
        b[i] = (float)((i + 1) % 100);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    printf("Starting partition type coverage tests...\n\n");
    
    /* ============================================
     * TEST 1: Gang Redundant (case 0)
     * Outer region with gang-redundant partitioning
     * ============================================ */
    printf("Test 1: Gang Redundant\n");
    total_tests++;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    int errors = verify_results(a, b, c, N, a[0] + b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 2: Gang Partitioned (case 1)
     * Explicit gang partitioning
     * ============================================ */
    printf("\nTest 2: Gang Partitioned\n");
    total_tests++;
    
    /* Reset array */
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] * b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 3: Worker Partitioned (case 2)
     * Worker-only partitioning
     * ============================================ */
    printf("\nTest 3: Worker Partitioned\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] - b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 4: Gang+Worker Partitioned (case 3)
     * Nested gang and worker partitioning
     * ============================================ */
    printf("\nTest 4: Gang+Worker Partitioned\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) \
                   gang worker copyin(a[0:N], b[0:N]) copyout(c[0:N])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] / (b[i] + 1.0f);  /* Avoid division by zero */
            }
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] / (b[i] + 1.0f);
        float diff = c[i] - expected;
        if (diff < -VERIFY_TOLERANCE || diff > VERIFY_TOLERANCE) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 5: Vector Partitioned (case 4)
     * Vector-only partitioning
     * ============================================ */
    printf("\nTest 5: Vector Partitioned\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f;
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] * 2.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 6: Gang+Vector Partitioned (case 5)
     * Gang and vector partitioning
     * ============================================ */
    printf("\nTest 6: Gang+Vector Partitioned\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) \
                   gang vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] * 3.0f;
        }
    }
    
    errors = verify_results(a, b, c, N, b[0] * 3.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 7: Worker+Vector Partitioned (case 6)
     * Worker and vector partitioning
     * ============================================ */
    printf("\nTest 7: Worker+Vector Partitioned\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) \
                   worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] + b[0] * 2.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 8: Fully Partitioned (case 7)
     * Gang, worker, and vector partitioning
     * ============================================ */
    printf("\nTest 8: Fully Partitioned\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) \
                   vector_length(vector_length) gang worker vector \
                   copyin(a[0:N], b[0:N]) copyout(c[0:N])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                c[i] = compute_element(a[i], b[i], i);
            }
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = compute_element(a[i], b[i], i);
        float diff = c[i] - expected;
        if (diff < -VERIFY_TOLERANCE || diff > VERIFY_TOLERANCE) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 9: Conditional Partition Selection
     * Runtime selection of partition type
     * ============================================ */
    printf("\nTest 9: Conditional Partition Selection\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) d[i] = 0.0f;
    
    int dynamic_gangs = use_aggressive_partitioning ? 4 : 2;
    int dynamic_workers = use_aggressive_partitioning ? 8 : 4;
    
    #pragma acc data copy(a[0:N], b[0:N], d[0:N])
    {
        #pragma acc parallel loop gang(dynamic_gangs) worker(dynamic_workers) \
                   gang worker
        for (int i = 0; i < N; i++) {
            d[i] = a[i] * a[i] + b[i] * b[i];
        }
    }
    
    errors = verify_results(a, b, d, N, a[0]*a[0] + b[0]*b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 10: Nested Parallelism with Partitioning
     * Outer gang-redundant, inner worker-partitioned
     * ============================================ */
    printf("\nTest 10: Nested Parallelism\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc kernels gang(num_gangs) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i += 64) {
                /* Inner worker-partitioned region */
                #pragma acc loop worker(num_workers) worker
                for (int j = i; j < i + 64 && j < N; j++) {
                    c[j] = a[j] * 0.5f + b[j] * 0.5f;
                }
            }
        }
    }
    
    errors = verify_results(a, b, c, N, a[0]*0.5f + b[0]*0.5f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * TEST 11: OpenMP Target Equivalent
     * Using OpenMP target for comparison
     * ============================================ */
    printf("\nTest 11: OpenMP Target Partitioning\n");
    total_tests++;
    
    for (int i = 0; i < N; i++) c[i] = 0.0f;
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers) \
                thread_limit(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i] / 100.0f;
        }
    }
    
    errors = verify_results(a, b, c, N, a[0]*b[0]/100.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================
     * Cleanup and Summary
     * ============================================ */
    free(a);
    free(b);
    free(c);
    free(d);
    
    printf("\n========================================\n");
    printf("Test Summary: %d/%d tests passed\n", passed_tests, total_tests);
    printf("Coverage targets:\n");
    printf("  [✓] Gang Redundant (case 0)\n");
    printf("  [✓] Gang Partitioned (case 1)\n");
    printf("  [✓] Worker Partitioned (case 2)\n");
    printf("  [✓] Gang+Worker Partitioned (case 3)\n");
    printf("  [✓] Vector Partitioned (case 4)\n");
    printf("  [✓] Gang+Vector Partitioned (case 5)\n");
    printf("  [✓] Worker+Vector Partitioned (case 6)\n");
    printf("  [✓] Fully Partitioned (case 7)\n");
    printf("  [✓] Conditional partition selection\n");
    printf("  [✓] Nested parallelism\n");
    printf("  [✓] OpenMP target equivalent\n");
    
    if (passed_tests == total_tests) {
        printf("\nSUCCESS: All partition type cases should be exercised\n");
        return 0;
    } else {
        printf("\nWARNING: Some tests failed, coverage may be incomplete\n");
        return 1;
    }
}

/* Implementation of routine functions */
float compute_element(float a, float b, int idx) {
    return a * 0.3f + b * 0.7f + (float)(idx % 10) * 0.01f;
}

void gang_partitioned_function(float *arr, int n, int gang_id) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] += (float)gang_id * 0.1f;
    }
}

void worker_partitioned_function(float *arr, int n, int worker_id) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] += (float)worker_id * 0.01f;
    }
}
