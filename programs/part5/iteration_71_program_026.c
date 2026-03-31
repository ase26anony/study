/**
 * Test program to exercise OpenACC/OpenMP partition clause logic
 * Specifically targets the switch statement in omp-oacc-neuter-broadcast.cc
 * lines 335-343 covering all partition type cases (0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_element(float a, float b, int idx);

#pragma acc routine gang
void gang_partitioned_function(float *arr, int n);

#pragma acc routine worker
void worker_partitioned_function(float *arr, int n);

#pragma acc routine vector
void vector_partitioned_function(float *arr, int n);

/* Helper function to verify results */
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
    int flag = 1; /* For conditional partition selection */
    
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
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* ============================================== */
    /* TEST 1: Gang redundant (case 0) */
    /* ============================================== */
    printf("Test 1: Gang redundant\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
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
    
    /* ============================================== */
    /* TEST 2: Gang partitioned (case 1) */
    /* ============================================== */
    printf("\nTest 2: Gang partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
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
    
    /* ============================================== */
    /* TEST 3: Worker partitioned (case 2) */
    /* ============================================== */
    printf("\nTest 3: Worker partitioned\n");
    total_tests++;
    
    /* Nested parallelism: outer gang-redundant, inner worker-partitioned */
    #pragma acc data copyin(a[0:N], b[0:N]) copy(d[0:N])
    {
        #pragma acc kernels
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker(num_workers) worker
                for (int j = 0; j < 4; j++) {
                    int idx = i * 4 + j;
                    if (idx < N) {
                        d[idx] = a[idx] - b[idx];
                    }
                }
            }
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] - b[i];
        float diff = d[i] - expected;
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
    
    /* ============================================== */
    /* TEST 4: Gang+worker partitioned (case 3) */
    /* ============================================== */
    printf("\nTest 4: Gang+worker partitioned\n");
    total_tests++;
    
    /* Conditional partition selection */
    int dynamic_workers = flag ? 2 : 4;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(dynamic_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] / (b[0] + 1.0f));
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================== */
    /* TEST 5: Vector partitioned (case 4) */
    /* ============================================== */
    printf("\nTest 5: Vector partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f + b[i];
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] * 2.0f + b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================== */
    /* TEST 6: Gang+vector partitioned (case 5) */
    /* ============================================== */
    printf("\nTest 6: Gang+vector partitioned\n");
    total_tests++;
    
    /* Using OpenMP target for variety */
    #ifdef _OPENMP
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) thread_limit(vector_length) \
                gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * a[i] + b[i] * b[i];
        }
    }
    #else
    /* OpenACC fallback */
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * a[i] + b[i] * b[i];
        }
    }
    #endif
    
    errors = verify_results(a, b, c, N, a[0] * a[0] + b[0] * b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================== */
    /* TEST 7: Worker+vector partitioned (case 6) */
    /* ============================================== */
    printf("\nTest 7: Worker+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        }
    }
    
    errors = verify_results(a, b, c, N, sqrtf(a[0] * a[0] + b[0] * b[0]));
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================== */
    /* TEST 8: Fully partitioned (case 7) */
    /* ============================================== */
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    total_tests++;
    
    /* Complex nested structure to force full partitioning analysis */
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N], d[0:N])
    {
        /* Outer region */
        #pragma acc kernels
        {
            /* Middle region */
            #pragma acc loop gang(num_gangs)
            for (int block = 0; block < num_gangs; block++) {
                int start = block * (N / num_gangs);
                int end = (block + 1) * (N / num_gangs);
                
                /* Inner region with full partitioning */
                #pragma acc loop worker(num_workers) vector_length(vector_length) gang worker vector
                for (int i = start; i < end; i++) {
                    if (i < N) {
                        c[i] = compute_element(a[i], b[i], i);
                        d[i] = c[i] * 0.5f;
                    }
                }
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
    
    /* ============================================== */
    /* TEST 9: Device-specific partition behavior */
    /* ============================================== */
    printf("\nTest 9: Device-specific partition behavior\n");
    total_tests++;
    
    /* Simulate device-specific behavior */
    #pragma acc set device_type(nvidia)
    {
        #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i] * 3.0f;
            }
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] + b[0] * 3.0f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ============================================== */
    /* Summary */
    /* ============================================== */
    printf("\n=========================================\n");
    printf("Test Summary:\n");
    printf("  Total tests: %d\n", total_tests);
    printf("  Passed: %d\n", passed_tests);
    printf("  Failed: %d\n", total_tests - passed_tests);
    printf("=========================================\n");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (passed_tests == total_tests) ? 0 : 1;
}

/* Implementation of routine functions */
#pragma acc routine seq
float compute_element(float a, float b, int idx) {
    return a * 0.3f + b * 0.7f + (float)(idx % 10) * 0.01f;
}

#pragma acc routine gang
void gang_partitioned_function(float *arr, int n) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f;
    }
}

#pragma acc routine worker
void worker_partitioned_function(float *arr, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] / 2.0f;
    }
}

#pragma acc routine vector
void vector_partitioned_function(float *arr, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] + 1.0f;
    }
}
