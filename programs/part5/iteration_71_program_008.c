#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

// Function declarations with different partition specifications
#pragma acc routine seq
float compute_element(float a, float b);

#pragma acc routine gang
void gang_partitioned_func(float *arr, int n);

#pragma acc routine worker
void worker_partitioned_func(float *arr, int n);

#pragma acc routine vector
void vector_partitioned_func(float *arr, int n);

// Helper function to verify results
int verify_results(float *a, float *b, float *c, int n, float expected) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float diff = c[i] - expected;
        if (diff < -VERIFY_TOLERANCE || diff > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: c[%d] = %f, expected ~%f\n", 
                       i, i, c[i], expected);
            }
        }
    }
    return errors;
}

float compute_element(float a, float b) {
    return a * 2.0f + b * 3.0f;
}

void gang_partitioned_func(float *arr, int n) {
    #pragma acc routine gang
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f;
    }
}

void worker_partitioned_func(float *arr, int n) {
    #pragma acc routine worker
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] + 1.0f;
    }
}

void vector_partitioned_func(float *arr, int n) {
    #pragma acc routine vector
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] / 2.0f;
    }
}

int main() {
    int total_tests = 0;
    int passed_tests = 0;
    
    // Runtime-determined partition counts
    int num_gangs = 4;
    int num_workers = 2;
    int vector_length = 32;
    int flag = 1;
    
    // Allocate and initialize arrays
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    printf("Starting partition type coverage tests...\n\n");
    
    // =================================================================
    // Test 1: Gang redundant (case 0)
    // =================================================================
    printf("Test 1: Gang redundant\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang_redundant
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    int errors = verify_results(a, b, c, N, (float)N);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 2: Gang partitioned (case 1)
    // =================================================================
    printf("\nTest 2: Gang partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 2.0f) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 3: Worker partitioned (case 2)
    // =================================================================
    printf("\nTest 3: Worker partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = compute_element(a[i], b[i]);
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * 2.0f + b[i] * 3.0f;
        if (c[i] != expected) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 4: Gang+worker partitioned (case 3)
    // =================================================================
    printf("\nTest 4: Gang+worker partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * b[i]) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 5: Vector partitioned (case 4)
    // =================================================================
    printf("\nTest 5: Vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / 2.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] / 2.0f) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 6: Gang+vector partitioned (case 5)
    // =================================================================
    printf("\nTest 6: Gang+vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i] * 0.5f;
        if (c[i] != expected) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 7: Worker+vector partitioned (case 6)
    // =================================================================
    printf("\nTest 7: Worker+vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 3.0f) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 8: Fully partitioned (case 7)
    // =================================================================
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f + b[i] * 3.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * 2.0f + b[i] * 3.0f;
        if (c[i] != expected) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 9: Nested parallelism with different partition types
    // =================================================================
    printf("\nTest 9: Nested parallelism (outer gang, inner worker)\n");
    total_tests++;
    #pragma acc data copyin(a[0:N]) copy(c[0:N])
    {
        #pragma acc kernels gang(num_gangs)
        {
            #pragma acc loop worker(num_workers) worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + 100.0f;
            }
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] + 100.0f) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 10: Conditional partition selection
    // =================================================================
    printf("\nTest 10: Conditional partition selection\n");
    total_tests++;
    #pragma acc data copyin(a[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector_length(flag ? 16 : 32) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * (flag ? 5.0f : 10.0f);
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 5.0f) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 11: OpenMP target equivalent tests
    // =================================================================
    printf("\nTest 11: OpenMP target with partition variations\n");
    total_tests++;
    
    // Reset array
    for (int i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    #pragma omp target data map(to: a[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers) \
                simdlen(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 7.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 7.0f) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 12: Device-specific partition behavior
    // =================================================================
    printf("\nTest 12: Device-specific partition behavior\n");
    total_tests++;
    
    #pragma acc set device_type(nvidia)
    {
        #pragma acc data copyin(a[0:N]) copy(c[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) vector_length(64) gang vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 11.0f;
            }
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 11.0f) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Test 13: Complex nested structure with data clauses
    // =================================================================
    printf("\nTest 13: Complex nested structure\n");
    total_tests++;
    
    #pragma acc data copy(d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            d[i] = (float)i;
        }
        
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            d[i] = d[i] * 2.0f;
        }
        
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            d[i] = d[i] + 1.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = (float)i * 2.0f + 1.0f;
        if (d[i] != expected) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    // =================================================================
    // Summary
    // =================================================================
    printf("\n" "=" * 50 "\n");
    printf("TEST SUMMARY:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", passed_tests);
    printf("  Failed: %d\n", total_tests - passed_tests);
    printf("  Success rate: %.1f%%\n", 
           (float)passed_tests / total_tests * 100.0f);
    
    if (passed_tests == total_tests) {
        printf("\nALL TESTS PASSED!\n");
    } else {
        printf("\nSOME TESTS FAILED!\n");
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
