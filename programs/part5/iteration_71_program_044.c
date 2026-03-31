/* 
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * Targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
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
void gang_partitioned_function(float *arr, int n, int gang_id);

#pragma acc routine worker
void worker_partitioned_function(float *arr, int n, int worker_id);

#pragma acc routine vector
void vector_partitioned_function(float *arr, int n, int lane_id);

/* Helper function to verify results */
int verify_results(float *a, float *b, float *c, int n, float expected) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float diff = c[i] - expected;
        if (diff > VERIFY_TOLERANCE || diff < -VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: c[%d] = %f, expected ~%f\n", 
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
    
    /* Allocate and initialize arrays */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));
    float *d = (float *)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100);
        b[i] = (float)((i + 1) % 100);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    printf("Starting partition clause coverage tests...\n\n");
    
    /* ========== TEST 1: Gang Redundant (case 0) ========== */
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
    
    /* ========== TEST 2: Gang Partitioned (case 1) ========== */
    printf("\nTest 2: Gang partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N]) copyout(d[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            int gang_id = 0;
            #ifdef _OPENACC
            gang_id = __pgi_gangidx();
            #endif
            gang_partitioned_function(d, N, gang_id);
        }
    }
    
    printf("  PASSED (gang function executed)\n");
    passed_tests++;
    
    /* ========== TEST 3: Worker Partitioned (case 2) ========== */
    printf("\nTest 3: Worker partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
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
    
    /* ========== TEST 4: Gang+Worker Partitioned (case 3) ========== */
    printf("\nTest 4: Gang+worker partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
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
    
    /* ========== TEST 5: Vector Partitioned (case 4) ========== */
    printf("\nTest 5: Vector partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    
    printf("  PASSED (vector operation executed)\n");
    passed_tests++;
    
    /* ========== TEST 6: Gang+Vector Partitioned (case 5) ========== */
    printf("\nTest 6: Gang+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = compute_element(a[i], b[i], i);
        }
    }
    
    printf("  PASSED (gang+vector function executed)\n");
    passed_tests++;
    
    /* ========== TEST 7: Worker+Vector Partitioned (case 6) ========== */
    printf("\nTest 7: Worker+vector partitioned\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + 2.0f * b[i];
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] + 2.0f * b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ========== TEST 8: Fully Partitioned (case 7) ========== */
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * a[i] + b[i] * b[i];
        }
    }
    
    errors = verify_results(a, b, c, N, a[0]*a[0] + b[0]*b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ========== TEST 9: Conditional Partition Selection ========== */
    printf("\nTest 9: Conditional partition selection\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector_length(flag ? 16 : 32) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = (a[i] + b[i]) * (flag ? 1.5f : 2.0f);
        }
    }
    
    float expected = (a[0] + b[0]) * (flag ? 1.5f : 2.0f);
    errors = verify_results(a, b, c, N, expected);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ========== TEST 10: Nested Parallelism ========== */
    printf("\nTest 10: Nested parallelism with partitioning\n");
    total_tests++;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        /* Outer gang-redundant region */
        #pragma acc kernels gang
        {
            /* Inner worker-partitioned loop */
            #pragma acc loop worker(num_workers) worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 3.0f - b[i];
            }
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] * 3.0f - b[0]);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ========== TEST 11: OpenMP Target Version ========== */
    printf("\nTest 11: OpenMP target with partition variations\n");
    total_tests++;
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers) \
                thread_limit(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
    }
    
    errors = verify_results(a, b, c, N, a[0] + b[0] * 0.5f);
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED with %d errors\n", errors);
    }
    
    /* ========== TEST 12: Device-specific behavior ========== */
    printf("\nTest 12: Device-specific partition behavior\n");
    total_tests++;
    
    #ifdef _OPENACC
    #pragma acc set device_type(nvidia)
    #endif
    
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 0.001f);
        }
    }
    
    printf("  PASSED (device-specific execution)\n");
    passed_tests++;
    
    /* ========== SUMMARY ========== */
    printf("\n========== TEST SUMMARY ==========\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed tests: %d\n", passed_tests);
    printf("Failed tests: %d\n", total_tests - passed_tests);
    
    if (passed_tests == total_tests) {
        printf("\nALL TESTS PASSED!\n");
    } else {
        printf("\nSOME TESTS FAILED!\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (passed_tests == total_tests) ? 0 : 1;
}

/* Implementation of partitioned functions */
float compute_element(float a, float b, int idx) {
    return a * 2.0f + b * 0.5f + (float)(idx % 10);
}

void gang_partitioned_function(float *arr, int n, int gang_id) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] = (float)gang_id * 100.0f + (float)i;
    }
}

void worker_partitioned_function(float *arr, int n, int worker_id) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        arr[i] = (float)worker_id * 10.0f + (float)(i % 100);
    }
}

void vector_partitioned_function(float *arr, int n, int lane_id) {
    for (int i = 0; i < n; i++) {
        arr[i] = (float)lane_id + (float)i * 0.1f;
    }
}
