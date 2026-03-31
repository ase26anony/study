/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting the uncovered switch statement in omp-oacc-neuter-broadcast.cc
 * lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_element(float a, float b, int idx) {
    return a * b + idx * 0.1f;
}

#pragma acc routine gang
void gang_level_operation(float *arr, int start, int end, float factor) {
    for (int i = start; i < end; i++) {
        arr[i] *= factor;
    }
}

/* Device-specific partition behaviors */
#ifdef _OPENACC
#pragma acc set device_type(nvidia)
#endif

int main() {
    int i;
    int success_count = 0;
    int total_tests = 0;
    
    /* Runtime-determined partition counts */
    int num_gangs = 4;
    int num_workers = 2;
    int vector_length = 32;
    int flag = 1;
    
    /* Test arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    float *result = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = (float)(i % 100);
        b[i] = (float)((i + 1) % 100);
        c[i] = 1.0f;
        d[i] = 2.0f;
        result[i] = 0.0f;
    }
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* ============================================
     * TEST 1: Gang Redundant (case 0)
     * Outer region with gang-redundant partitioning
     * ============================================ */
    {
        total_tests++;
        printf("Test 1: Gang Redundant\n");
        
        #pragma acc data copy(a[0:N], b[0:N], result[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) copy(result[0:N])
            for (i = 0; i < N; i++) {
                result[i] = a[i] + b[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != a[i] + b[i]) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 2: Gang Partitioned (case 1)
     * Explicit gang partitioning
     * ============================================ */
    {
        total_tests++;
        printf("Test 2: Gang Partitioned\n");
        
        #pragma acc data copy(c[0:N], d[0:N], result[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) gang
            for (i = 0; i < N; i++) {
                result[i] = c[i] * d[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != c[i] * d[i]) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 3: Worker Partitioned (case 2)
     * Worker-level partitioning
     * ============================================ */
    {
        total_tests++;
        printf("Test 3: Worker Partitioned\n");
        
        #pragma acc data copy(a[0:N], result[0:N])
        {
            #pragma acc parallel loop worker(num_workers) worker
            for (i = 0; i < N; i++) {
                result[i] = a[i] * 2.0f;
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != a[i] * 2.0f) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 4: Gang+Worker Partitioned (case 3)
     * Combined gang and worker partitioning
     * ============================================ */
    {
        total_tests++;
        printf("Test 4: Gang+Worker Partitioned\n");
        
        #pragma acc data copy(b[0:N], result[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
            for (i = 0; i < N; i++) {
                result[i] = b[i] / 2.0f;
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != b[i] / 2.0f) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 5: Vector Partitioned (case 4)
     * Vector-level partitioning
     * ============================================ */
    {
        total_tests++;
        printf("Test 5: Vector Partitioned\n");
        
        #pragma acc data copy(c[0:N], result[0:N])
        {
            #pragma acc parallel loop vector(vector_length) vector
            for (i = 0; i < N; i++) {
                result[i] = c[i] + 10.0f;
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != c[i] + 10.0f) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 6: Gang+Vector Partitioned (case 5)
     * Combined gang and vector partitioning
     * ============================================ */
    {
        total_tests++;
        printf("Test 6: Gang+Vector Partitioned\n");
        
        #pragma acc data copy(d[0:N], result[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) vector(vector_length) gang vector
            for (i = 0; i < N; i++) {
                result[i] = d[i] - 5.0f;
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != d[i] - 5.0f) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 7: Worker+Vector Partitioned (case 6)
     * Combined worker and vector partitioning
     * ============================================ */
    {
        total_tests++;
        printf("Test 7: Worker+Vector Partitioned\n");
        
        #pragma acc data copy(a[0:N], b[0:N], result[0:N])
        {
            #pragma acc parallel loop worker(num_workers) vector(vector_length) worker vector
            for (i = 0; i < N; i++) {
                result[i] = compute_element(a[i], b[i], i);
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            float expected = compute_element(a[i], b[i], i);
            if (result[i] != expected) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 8: Fully Partitioned (case 7)
     * Gang+Worker+Vector partitioning
     * ============================================ */
    {
        total_tests++;
        printf("Test 8: Fully Partitioned\n");
        
        #pragma acc data copy(c[0:N], d[0:N], result[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length) gang worker vector
            for (i = 0; i < N; i++) {
                result[i] = c[i] * d[i] + i * 0.01f;
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            float expected = c[i] * d[i] + i * 0.01f;
            if (result[i] != expected) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 9: Nested Parallelism with Partitioning
     * Outer gang-redundant, inner worker-partitioned
     * ============================================ */
    {
        total_tests++;
        printf("Test 9: Nested Parallelism\n");
        
        #pragma acc data copy(a[0:N], result[0:N])
        {
            #pragma acc kernels
            {
                /* Outer gang-redundant region */
                #pragma acc loop gang
                for (int g = 0; g < num_gangs; g++) {
                    int start = g * (N / num_gangs);
                    int end = (g + 1) * (N / num_gangs);
                    if (g == num_gangs - 1) end = N;
                    
                    /* Inner worker-partitioned loop */
                    #pragma acc loop worker
                    for (i = start; i < end; i++) {
                        result[i] = a[i] * 3.0f;
                    }
                }
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != a[i] * 3.0f) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 10: Conditional Partition Selection
     * Runtime-determined partition configuration
     * ============================================ */
    {
        total_tests++;
        printf("Test 10: Conditional Partition Selection\n");
        
        int dynamic_workers = flag ? 2 : 4;
        int dynamic_vector = flag ? 16 : 64;
        
        #pragma acc data copy(b[0:N], result[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) worker(dynamic_workers) vector(dynamic_vector) gang worker
            for (i = 0; i < N; i++) {
                result[i] = b[i] + i * 0.5f;
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            float expected = b[i] + i * 0.5f;
            if (result[i] != expected) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 11: OpenMP Target Equivalent
     * Using OpenMP target teams distribute
     * ============================================ */
    {
        total_tests++;
        printf("Test 11: OpenMP Target Partitioning\n");
        
        #pragma omp target data map(to: a[0:N], b[0:N]) map(from: result[0:N])
        {
            #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers * vector_length)
            for (i = 0; i < N; i++) {
                result[i] = a[i] - b[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != a[i] - b[i]) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* ============================================
     * TEST 12: Function with Routine Directive
     * Gang routine called from partitioned region
     * ============================================ */
    {
        total_tests++;
        printf("Test 12: Function with Routine Directive\n");
        
        float factor = 2.5f;
        
        #pragma acc data copy(result[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) gang
            for (int g = 0; g < num_gangs; g++) {
                int start = g * (N / num_gangs);
                int end = (g + 1) * (N / num_gangs);
                if (g == num_gangs - 1) end = N;
                
                gang_level_operation(result, start, end, factor);
            }
        }
        
        /* Verify results - all should be 0 * factor = 0 */
        int correct = 1;
        for (i = 0; i < N; i++) {
            if (result[i] != 0.0f) {
                correct = 0;
                break;
            }
        }
        
        if (correct) {
            success_count++;
            printf("  PASS\n");
        } else {
            printf("  FAIL\n");
        }
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(result);
    
    /* Summary */
    printf("\n========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", success_count);
    printf("  Failed: %d\n", total_tests - success_count);
    printf("  Coverage: %.1f%%\n", (success_count * 100.0) / total_tests);
    printf("========================================\n");
    
    return (success_count == total_tests) ? 0 : 1;
}
