/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_element(float a, float b, int idx);

#pragma acc routine gang
void gang_redundant_operation(float *arr, int n, int gang_id);

/* Global variables to force runtime evaluation */
int dynamic_gangs = 2;
int dynamic_workers = 4;
int dynamic_vector = 32;
int use_alternate_partition = 0;

int main() {
    /* Initialize test data */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    float *result = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + 1);
        b[i] = (float)(i * 2);
        c[i] = (float)(i * 3);
        d[i] = (float)(i * 4);
        result[i] = 0.0f;
    }
    
    int success_count = 0;
    int total_tests = 0;
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* ============================================
     * TEST 1: Gang redundant (case 0)
     * ============================================ */
    {
        printf("Test 1: Gang redundant\n");
        total_tests++;
        
        #pragma acc data copyin(a[0:N], b[0:N]) copyout(result[0:N])
        {
            #pragma acc parallel loop gang(num_gangs:dynamic_gangs) gang
            for (int i = 0; i < N; i++) {
                result[i] = a[i] + b[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
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
     * TEST 2: Gang partitioned (case 1)
     * ============================================ */
    {
        printf("\nTest 2: Gang partitioned\n");
        total_tests++;
        
        #pragma acc data copyin(a[0:N], c[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop gang(dynamic_gangs) vector(dynamic_vector)
            for (int i = 0; i < N; i++) {
                result[i] = a[i] * c[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (result[i] != a[i] * c[i]) {
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
     * TEST 3: Worker partitioned (case 2)
     * ============================================ */
    {
        printf("\nTest 3: Worker partitioned\n");
        total_tests++;
        
        #pragma acc data copyin(b[0:N], c[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop worker(dynamic_workers) vector(dynamic_vector)
            for (int i = 0; i < N; i++) {
                result[i] = b[i] - c[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (result[i] != b[i] - c[i]) {
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
     * TEST 4: Gang+worker partitioned (case 3)
     * ============================================ */
    {
        printf("\nTest 4: Gang+worker partitioned\n");
        total_tests++;
        
        #pragma acc data copyin(a[0:N], b[0:N], c[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop gang(dynamic_gangs) worker(dynamic_workers)
            for (int i = 0; i < N; i++) {
                result[i] = a[i] + b[i] + c[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (result[i] != a[i] + b[i] + c[i]) {
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
     * TEST 5: Vector partitioned (case 4)
     * ============================================ */
    {
        printf("\nTest 5: Vector partitioned\n");
        total_tests++;
        
        #pragma acc data copyin(a[0:N], d[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop vector(dynamic_vector)
            for (int i = 0; i < N; i++) {
                result[i] = a[i] / (d[i] + 1.0f);
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            float expected = a[i] / (d[i] + 1.0f);
            if (result[i] - expected > VERIFY_TOL || expected - result[i] > VERIFY_TOL) {
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
     * TEST 6: Gang+vector partitioned (case 5)
     * ============================================ */
    {
        printf("\nTest 6: Gang+vector partitioned\n");
        total_tests++;
        
        #pragma acc data copyin(b[0:N], d[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop gang(dynamic_gangs) vector(dynamic_vector)
            for (int i = 0; i < N; i++) {
                result[i] = b[i] * d[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (result[i] != b[i] * d[i]) {
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
     * TEST 7: Worker+vector partitioned (case 6)
     * ============================================ */
    {
        printf("\nTest 7: Worker+vector partitioned\n");
        total_tests++;
        
        #pragma acc data copyin(c[0:N], d[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop worker(dynamic_workers) vector(dynamic_vector)
            for (int i = 0; i < N; i++) {
                result[i] = c[i] - d[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (result[i] != c[i] - d[i]) {
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
     * TEST 8: Fully partitioned (case 7)
     * ============================================ */
    {
        printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
        total_tests++;
        
        #pragma acc data copyin(a[0:N], b[0:N], c[0:N], d[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop gang(dynamic_gangs) worker(dynamic_workers) vector(dynamic_vector)
            for (int i = 0; i < N; i++) {
                result[i] = a[i] + b[i] + c[i] + d[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (result[i] != a[i] + b[i] + c[i] + d[i]) {
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
     * TEST 9: Nested parallelism with partitioning
     * ============================================ */
    {
        printf("\nTest 9: Nested parallelism (outer gang, inner worker)\n");
        total_tests++;
        
        #pragma acc data copyin(a[0:N], b[0:N]) copy(result[0:N])
        {
            #pragma acc kernels gang(dynamic_gangs)
            {
                #pragma acc loop worker(dynamic_workers)
                for (int i = 0; i < N; i++) {
                    result[i] = compute_element(a[i], b[i], i);
                }
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
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
     * TEST 10: Conditional partition selection
     * ============================================ */
    {
        printf("\nTest 10: Conditional partition selection\n");
        total_tests++;
        
        int flag = 1; /* Runtime condition */
        
        #pragma acc data copyin(a[0:N], b[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop gang(dynamic_gangs) worker(flag ? 2 : 4) vector(flag ? 16 : 32)
            for (int i = 0; i < N; i++) {
                result[i] = a[i] * 2.0f + b[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (result[i] != a[i] * 2.0f + b[i]) {
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
     * TEST 11: OpenMP target equivalent
     * ============================================ */
    {
        printf("\nTest 11: OpenMP target with partition variations\n");
        total_tests++;
        
        #pragma omp target data map(to: a[0:N], b[0:N]) map(from: result[0:N])
        {
            /* Test gang+worker+vector partitioning */
            #pragma omp target teams distribute parallel for \
                num_teams(dynamic_gangs) num_threads(dynamic_workers) \
                thread_limit(dynamic_workers)
            for (int i = 0; i < N; i++) {
                result[i] = a[i] - b[i];
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
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
     * TEST 12: Device-specific behavior
     * ============================================ */
    {
        printf("\nTest 12: Device-specific partition behavior\n");
        total_tests++;
        
        #pragma acc set device_type(nvidia)
        
        #pragma acc data copyin(a[0:N], c[0:N]) copy(result[0:N])
        {
            #pragma acc parallel loop gang(dynamic_gangs) vector(dynamic_vector)
            for (int i = 0; i < N; i++) {
                result[i] = a[i] * c[i] * 0.5f;
            }
        }
        
        /* Verify results */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            float expected = a[i] * c[i] * 0.5f;
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
     * Summary
     * ============================================ */
    printf("\n============================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", success_count);
    printf("  Failed: %d\n", total_tests - success_count);
    printf("============================================\n");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(result);
    
    return (success_count == total_tests) ? 0 : 1;
}

/* Helper function implementations */
#pragma acc routine seq
float compute_element(float a, float b, int idx) {
    return a * 0.5f + b * 0.3f + (float)idx * 0.2f;
}

#pragma acc routine gang
void gang_redundant_operation(float *arr, int n, int gang_id) {
    /* This function is marked as gang-level routine */
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] += (float)gang_id * 0.01f;
    }
}
