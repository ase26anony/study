/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
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
void gang_level_operation(float *arr, int start, int end) {
    for (int i = start; i < end; i++) {
        arr[i] *= 2.0f;
    }
}

/* Helper to verify results */
int verify_results(float *a, float *b, float *c, int n, float expected_factor) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float expected = a[i] + b[i] * expected_factor + i * 0.1f;
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at %d: got %f, expected %f\n", i, c[i], expected);
            }
        }
    }
    return errors;
}

int main() {
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
    
    int total_tests = 0;
    int passed_tests = 0;
    
    printf("=== Testing OpenACC Partition Clause Variations ===\n\n");
    
    /* Test 1: gang redundant (case 0) */
    printf("Test 1: gang redundant\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    if (verify_results(a, b, c, N, 1.0f) == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 2: gang partitioned (case 1) */
    printf("\nTest 2: gang partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i] + i * 0.1f;
        }
    }
    if (verify_results(a, b, c, N, 0.0f) == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 3: worker partitioned (case 2) */
    printf("\nTest 3: worker partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = compute_element(a[i], b[i], i);
        }
    }
    if (verify_results(a, b, c, N, 1.0f) == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 4: gang+worker partitioned (case 3) */
    printf("\nTest 4: gang+worker partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], d[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) gang worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                d[i] = a[i] * 3.0f;
            }
        }
    }
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(d[i] - a[i] * 3.0f) > VERIFY_TOLERANCE) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED (%d errors)\n", errors);
    }
    
    /* Test 5: vector partitioned (case 4) */
    printf("\nTest 5: vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] - b[i])) > VERIFY_TOLERANCE) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED (%d errors)\n", errors);
    }
    
    /* Test 6: gang+vector partitioned (case 5) */
    printf("\nTest 6: gang+vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] / (b[i] + 1.0f);
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED (%d errors)\n", errors);
    }
    
    /* Test 7: worker+vector partitioned (case 6) */
    printf("\nTest 7: worker+vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f + b[i];
        }
    }
    if (verify_results(a, b, c, N, 1.0f) == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 8: fully partitioned (case 7) */
    printf("\nTest 8: fully partitioned (gang+worker+vector)\n");
    total_tests++;
    #pragma acc data copy(a[0:N], d[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers) vector_length(vector_length) gang worker vector
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                d[i] = a[i] * 4.0f;
            }
        }
    }
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(d[i] - a[i] * 4.0f) > VERIFY_TOLERANCE) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED (%d errors)\n", errors);
    }
    
    /* Test 9: Nested parallelism with partitioning */
    printf("\nTest 9: Nested parallelism (outer gang-redundant, inner worker-partitioned)\n");
    total_tests++;
    #pragma acc data copy(a[0:N], d[0:N])
    {
        #pragma acc kernels gang
        {
            gang_level_operation(a, 0, N/2);
            
            #pragma acc loop worker
            for (int i = N/2; i < N; i++) {
                d[i] = a[i] * 5.0f;
            }
        }
    }
    errors = 0;
    for (int i = 0; i < N/2; i++) {
        if (fabs(a[i] - (float)(i % 100) * 2.0f) > VERIFY_TOLERANCE) errors++;
    }
    for (int i = N/2; i < N; i++) {
        if (fabs(d[i] - (float)(i % 100) * 5.0f) > VERIFY_TOLERANCE) errors++;
    }
    if (errors == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED (%d errors)\n", errors);
    }
    
    /* Test 10: Conditional partition selection */
    printf("\nTest 10: Conditional partition selection\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector_length(vector_length) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * (flag ? 2.0f : 3.0f);
        }
    }
    if (verify_results(a, b, c, N, 2.0f) == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 11: OpenMP target equivalent */
    printf("\nTest 11: OpenMP target with gang partitioned\n");
    total_tests++;
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
    }
    if (verify_results(a, b, c, N, 0.5f) == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED\n");
    }
    
    /* Test 12: Device-specific behavior */
    printf("\nTest 12: Device-specific partition behavior\n");
    total_tests++;
    #pragma acc set device_type(nvidia)
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 0.25f + b[i];
        }
    }
    if (verify_results(a, b, c, N, 1.0f) == 0) {
        printf("  PASSED\n");
        passed_tests++;
    } else {
        printf("  FAILED\n");
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
