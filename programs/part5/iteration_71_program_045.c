/* 
 * Test program to exercise all partition type cases in omp-oacc-neuter-broadcast.cc
 * Specifically targets the switch statement at lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 1e-6

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_value(int i, float factor);

#pragma acc routine gang
void gang_level_operation(float *arr, int start, int end, float factor);

/* Helper to verify results */
int verify_array(float *arr, float expected, int size) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (fabs(arr[i] - expected) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at arr[%d]: expected %f, got %f\n", 
                       i, expected, arr[i]);
            }
        }
    }
    return errors;
}

int main() {
    int num_gangs = 2;
    int num_workers = 4;
    int vector_length = 32;
    int flag = 1;
    int success_count = 0;
    int total_tests = 0;
    
    /* Initialize data arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    printf("Starting partition type coverage tests...\n\n");
    
    /* ============================================
     * TEST 1: Gang redundant (case 0)
     * Outer region with gang redundancy
     * ============================================ */
    total_tests++;
    printf("Test 1: Gang redundant\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) copy(c[0:N])
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f;
        }
    }
    
    int err1 = verify_array(c, 0.0f, N);  // Will be non-zero since we computed
    if (err1 == 0) {
        printf("  FAIL: No computation performed\n");
    } else {
        printf("  PASS: Gang redundant region executed\n");
        success_count++;
    }
    
    /* ============================================
     * TEST 2: Gang partitioned (case 1)
     * Explicit gang partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 2: Gang partitioned\n");
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    int err2 = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] + b[i])) > VERIFY_TOLERANCE) err2++;
    }
    if (err2 == 0) {
        printf("  PASS: Gang partitioned computation correct\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors in gang partitioned computation\n", err2);
    }
    
    /* ============================================
     * TEST 3: Worker partitioned (case 2)
     * Worker-level partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 3: Worker partitioned\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f;
        }
    }
    
    int err3 = verify_array(c, 0.0f, N);
    if (err3 == N) {  // All values should be modified
        printf("  PASS: Worker partitioned region executed\n");
        success_count++;
    } else {
        printf("  FAIL: Worker partition incomplete\n");
    }
    
    /* ============================================
     * TEST 4: Gang+worker partitioned (case 3)
     * Combined gang and worker partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 4: Gang+worker partitioned\n");
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    int err4 = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * b[i])) > VERIFY_TOLERANCE) err4++;
    }
    if (err4 == 0) {
        printf("  PASS: Gang+worker partitioned computation correct\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors in gang+worker computation\n", err4);
    }
    
    /* ============================================
     * TEST 5: Vector partitioned (case 4)
     * Vector-level partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 5: Vector partitioned\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop vector(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = sqrtf(a[i]);
        }
    }
    
    int err5 = verify_array(c, 0.0f, N);
    if (err5 == N) {
        printf("  PASS: Vector partitioned region executed\n");
        success_count++;
    } else {
        printf("  FAIL: Vector partition incomplete\n");
    }
    
    /* ============================================
     * TEST 6: Gang+vector partitioned (case 5)
     * Combined gang and vector partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 6: Gang+vector partitioned\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / 2.0f;
        }
    }
    
    int err6 = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] / 2.0f)) > VERIFY_TOLERANCE) err6++;
    }
    if (err6 == 0) {
        printf("  PASS: Gang+vector partitioned computation correct\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors in gang+vector computation\n", err6);
    }
    
    /* ============================================
     * TEST 7: Worker+vector partitioned (case 6)
     * Combined worker and vector partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 7: Worker+vector partitioned\n");
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    int err7 = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] - b[i])) > VERIFY_TOLERANCE) err7++;
    }
    if (err7 == 0) {
        printf("  PASS: Worker+vector partitioned computation correct\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors in worker+vector computation\n", err7);
    }
    
    /* ============================================
     * TEST 8: Fully partitioned (case 7)
     * Gang+worker+vector partitioning
     * ============================================ */
    total_tests++;
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
    }
    
    int err8 = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] + b[i] * 0.5f)) > VERIFY_TOLERANCE) err8++;
    }
    if (err8 == 0) {
        printf("  PASS: Fully partitioned computation correct\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors in fully partitioned computation\n", err8);
    }
    
    /* ============================================
     * TEST 9: Conditional partition selection
     * Runtime-determined partition configuration
     * ============================================ */
    total_tests++;
    printf("\nTest 9: Conditional partition selection\n");
    #pragma acc data copy(a[0:N], d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector(flag ? 16 : 32)
        for (int i = 0; i < N; i++) {
            d[i] = compute_value(i, 1.5f);
        }
    }
    
    int err9 = verify_array(d, 0.0f, N);
    if (err9 == N) {
        printf("  PASS: Conditional partition executed\n");
        success_count++;
    } else {
        printf("  FAIL: Conditional partition incomplete\n");
    }
    
    /* ============================================
     * TEST 10: Nested parallelism with partitioning
     * Outer gang-redundant, inner worker-partitioned
     * ============================================ */
    total_tests++;
    printf("\nTest 10: Nested parallelism\n");
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc kernels
        {
            // Outer region
            #pragma acc loop gang
            for (int g = 0; g < num_gangs; g++) {
                int start = g * (N / num_gangs);
                int end = (g + 1) * (N / num_gangs);
                
                // Inner worker-partitioned loop
                #pragma acc loop worker
                for (int i = start; i < end; i++) {
                    c[i] = a[i] * 4.0f;
                }
            }
        }
    }
    
    int err10 = verify_array(c, 0.0f, N);
    if (err10 == N) {
        printf("  PASS: Nested partitioning executed\n");
        success_count++;
    } else {
        printf("  FAIL: Nested partition incomplete\n");
    }
    
    /* ============================================
     * TEST 11: OpenMP target equivalent
     * Using OpenMP target teams distribute
     * ============================================ */
    total_tests++;
    printf("\nTest 11: OpenMP target teams\n");
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(num_gangs) num_threads(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    int err11 = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] + b[i])) > VERIFY_TOLERANCE) err11++;
    }
    if (err11 == 0) {
        printf("  PASS: OpenMP target teams executed\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors in OpenMP computation\n", err11);
    }
    
    /* ============================================
     * TEST 12: Device-specific behavior
     * Using set device_type
     * ============================================ */
    total_tests++;
    printf("\nTest 12: Device-specific partitioning\n");
    
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 0.25f;
        }
    }
    
    int err12 = 0;
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * 0.25f)) > VERIFY_TOLERANCE) err12++;
    }
    if (err12 == 0) {
        printf("  PASS: Device-specific partition executed\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors in device-specific computation\n", err12);
    }
    
    /* ============================================
     * Summary
     * ============================================ */
    printf("\n" "=" * 50 "\n");
    printf("Test Summary:\n");
    printf("  Total tests run: %d\n", total_tests);
    printf("  Successful tests: %d\n", success_count);
    printf("  Success rate: %.1f%%\n", (success_count * 100.0) / total_tests);
    
    if (success_count == total_tests) {
        printf("\nALL TESTS PASSED - All partition types should have been exercised!\n");
    } else {
        printf("\nSOME TESTS FAILED - Check individual test outputs\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (success_count == total_tests) ? 0 : 1;
}

/* Implementation of routine functions */
#pragma acc routine seq
float compute_value(int i, float factor) {
    return (float)i * factor;
}

#pragma acc routine gang
void gang_level_operation(float *arr, int start, int end, float factor) {
    #pragma acc loop worker
    for (int i = start; i < end; i++) {
        arr[i] *= factor;
    }
}
