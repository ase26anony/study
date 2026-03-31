/* 
 * Test program to exercise all partition type cases in omp-oacc-neuter-broadcast.cc
 * Specifically targets the switch statement at lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_value(int i, float factor);

#pragma acc routine gang
void gang_level_operation(float *arr, int start, int end);

/* Helper to verify results */
int verify_results(float *a, float *b, int size, float expected_sum) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += a[i];
        if (fabs(a[i] - b[i]) > VERIFY_TOL) {
            return 0;
        }
    }
    return (fabs(sum - expected_sum) < VERIFY_TOL);
}

int main() {
    int success_count = 0;
    int total_tests = 0;
    
    /* Runtime-determined partition counts */
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1; /* For conditional partition selection */
    
    /* Test arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *ref = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        ref[i] = a[i] + b[i];
    }
    
    printf("Testing OpenACC/OpenMP partition type coverage...\n\n");
    
    /* ============================================
     * TEST 1: Gang Redundant (case 0)
     * Outer gang-redundant region with inner worker-partitioned loop
     * ============================================ */
    total_tests++;
    printf("Test 1: Gang Redundant\n");
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        /* Outer gang-redundant region */
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            /* Inner worker-partitioned loop */
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    if (verify_results(c, ref, N, N * (N - 1) / 2.0f + N * (N + 1) / 2.0f)) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 2: Gang Partitioned (case 1)
     * Simple gang-partitioned loop
     * ============================================ */
    total_tests++;
    printf("\nTest 2: Gang Partitioned\n");
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f;
        }
    }
    
    /* Verify */
    int valid = 1;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - a[i] * 2.0f) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 3: Worker Partitioned (case 2)
     * Worker-partitioned loop with conditional worker count
     * ============================================ */
    total_tests++;
    printf("\nTest 3: Worker Partitioned\n");
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = compute_value(i, 1.5f);
        }
    }
    
    /* Verify with tolerance */
    valid = 1;
    for (int i = 0; i < N; i++) {
        float expected = compute_value(i, 1.5f);
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 4: Gang+Worker Partitioned (case 3)
     * Nested gang and worker partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 4: Gang+Worker Partitioned\n");
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) gang worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i] * 0.5f;
            }
        }
    }
    
    /* Verify */
    valid = 1;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i] * 0.5f;
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 5: Vector Partitioned (case 4)
     * Vector-partitioned loop with runtime length
     * ============================================ */
    total_tests++;
    printf("\nTest 5: Vector Partitioned\n");
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * a[i];
        }
    }
    
    /* Verify */
    valid = 1;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - a[i] * a[i]) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 6: Gang+Vector Partitioned (case 5)
     * Combined gang and vector partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 6: Gang+Vector Partitioned\n");
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Verify */
    valid = 1;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - a[i] * b[i]) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 7: Worker+Vector Partitioned (case 6)
     * Combined worker and vector partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 7: Worker+Vector Partitioned\n");
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = sqrtf(a[i] + 1.0f);
        }
    }
    
    /* Verify with tolerance */
    valid = 1;
    for (int i = 0; i < N; i++) {
        float expected = sqrtf(a[i] + 1.0f);
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 8: Fully Partitioned (case 7)
     * Gang+Worker+Vector partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 8: Fully Partitioned\n");
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) \
                vector_length(vector_length) gang worker vector
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i] * 2.0f;
            }
        }
    }
    
    /* Verify */
    valid = 1;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i] * 2.0f;
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 9: Conditional Partition Selection
     * Tests dynamic partition type determination
     * ============================================ */
    total_tests++;
    printf("\nTest 9: Conditional Partition Selection\n");
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) \
                worker(flag ? 2 : 4) vector_length(vector_length) \
                gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f;
        }
    }
    
    /* Verify */
    valid = 1;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - a[i] * 3.0f) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * TEST 10: OpenMP Target Version
     * Similar partitioning with OpenMP syntax
     * ============================================ */
    total_tests++;
    printf("\nTest 10: OpenMP Target Partitioning\n");
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers) \
                simdlen(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    /* Verify */
    valid = 1;
    for (int i = 0; i < N; i++) {
        float expected = a[i] - b[i];
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            valid = 0;
            break;
        }
    }
    if (valid) {
        success_count++;
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
    
    /* ============================================
     * Summary
     * ============================================ */
    printf("\n========================================\n");
    printf("Test Summary: %d/%d tests passed\n", success_count, total_tests);
    printf("Coverage target: All partition type cases (0-7)\n");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(ref);
    
    return (success_count == total_tests) ? 0 : 1;
}

/* Implementation of declared functions */
#pragma acc routine seq
float compute_value(int i, float factor) {
    return (float)i * factor;
}

#pragma acc routine gang
void gang_level_operation(float *arr, int start, int end) {
    /* Simple gang-level operation */
    for (int i = start; i < end; i++) {
        arr[i] = arr[i] * 2.0f;
    }
}
