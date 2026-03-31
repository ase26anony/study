/* test_partition_combinations.c
 * 
 * This program tests all OpenACC/OpenMP partition clause combinations
 * to exercise the partition type identification logic in the runtime.
 * Specifically designed to trigger the switch statement cases 0-7
 * in omp-oacc-neuter-broadcast.cc lines 335-343.
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
void gang_level_operation(float *arr, int start, int end);

/* Helper to verify array results */
int verify_array(float *arr, float expected, int size, const char *test_name) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (fabs(arr[i] - expected) > VERIFY_TOLERANCE) {
            if (errors == 0) {
                printf("  %s: error at arr[%d] = %f, expected %f\n", 
                       test_name, i, arr[i], expected);
            }
            errors++;
        }
    }
    return errors;
}

int main() {
    int test_count = 0;
    int success_count = 0;
    
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
        a[i] = (float)i;
        b[i] = (float)(2 * i);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    printf("Testing OpenACC/OpenMP Partition Clause Combinations\n");
    printf("====================================================\n");
    
    /* ============================================================
     * TEST 1: Gang Redundant (case 0)
     * Outer region with gang-redundant execution
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Gang Redundant\n", test_count);
        
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
        
        /* Verify results */
        int errors = 0;
        for (int i = 0; i < N; i++) {
            float expected = a[i] + b[i];
            if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
                errors++;
                if (errors == 1) printf("  Error at c[%d] = %f, expected %f\n", i, c[i], expected);
            }
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 2: Gang Partitioned (case 1)
     * Simple gang-partitioned loop
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Gang Partitioned\n", test_count);
        
        #pragma acc data copy(a[0:N], d[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) gang
            for (int i = 0; i < N; i++) {
                d[i] = a[i] * 2.0f;
            }
        }
        
        int errors = verify_array(d, 0.0f, N, "Gang Partitioned");
        for (int i = 0; i < N; i++) {
            if (fabs(d[i] - a[i] * 2.0f) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 3: Worker Partitioned (case 2)
     * Worker-partitioned loop inside gang region
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Worker Partitioned\n", test_count);
        
        #pragma acc data copy(a[0:N], b[0:N], c[0:N])
        {
            #pragma acc parallel num_gangs(num_gangs) gang
            {
                #pragma acc loop worker(num_workers) worker
                for (int i = 0; i < N; i++) {
                    c[i] = a[i] - b[i];
                }
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(c[i] - (a[i] - b[i])) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 4: Gang+Worker Partitioned (case 3)
     * Combined gang and worker partitioning
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Gang+Worker Partitioned\n", test_count);
        
        #pragma acc data copy(a[0:N], d[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
            for (int i = 0; i < N; i++) {
                d[i] = a[i] * a[i];
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(d[i] - a[i] * a[i]) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 5: Vector Partitioned (case 4)
     * Vector-partitioned loop
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Vector Partitioned\n", test_count);
        
        #pragma acc data copy(a[0:N], b[0:N], c[0:N])
        {
            #pragma acc parallel loop vector_length(vector_length) vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] / (b[i] + 1.0f);
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            float expected = a[i] / (b[i] + 1.0f);
            if (fabs(c[i] - expected) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 6: Gang+Vector Partitioned (case 5)
     * Combined gang and vector partitioning
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Gang+Vector Partitioned\n", test_count);
        
        #pragma acc data copy(a[0:N], d[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
            for (int i = 0; i < N; i++) {
                d[i] = sqrtf(a[i] + 1.0f);
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            float expected = sqrtf(a[i] + 1.0f);
            if (fabs(d[i] - expected) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 7: Worker+Vector Partitioned (case 6)
     * Combined worker and vector partitioning
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Worker+Vector Partitioned\n", test_count);
        
        #pragma acc data copy(a[0:N], b[0:N], c[0:N])
        {
            #pragma acc parallel num_gangs(num_gangs) gang
            {
                #pragma acc loop worker(num_workers) vector_length(vector_length) worker vector
                for (int i = 0; i < N; i++) {
                    c[i] = a[i] * b[i] + i;
                }
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            float expected = a[i] * b[i] + i;
            if (fabs(c[i] - expected) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 8: Fully Partitioned (case 7)
     * Gang, worker, and vector partitioning
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Fully Partitioned\n", test_count);
        
        #pragma acc data copy(a[0:N], d[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length) gang worker vector
            for (int i = 0; i < N; i++) {
                d[i] = a[i] * 3.0f + i * 0.5f;
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            float expected = a[i] * 3.0f + i * 0.5f;
            if (fabs(d[i] - expected) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 9: Conditional Partition Selection
     * Tests dynamic partition type determination
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Conditional Partition Selection\n", test_count);
        
        /* Conditional worker count */
        int dynamic_workers = flag ? 2 : 4;
        
        #pragma acc data copy(a[0:N], c[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) worker(dynamic_workers) gang worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + i * 0.1f;
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            float expected = a[i] + i * 0.1f;
            if (fabs(c[i] - expected) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 10: Nested Parallelism with Different Partitions
     * Outer kernels with inner parallel loops
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Nested Parallelism\n", test_count);
        
        #pragma acc data copy(a[0:N], b[0:N], c[0:N], d[0:N])
        {
            /* Outer gang-redundant region */
            #pragma acc kernels gang
            {
                /* Inner worker-partitioned loop */
                #pragma acc loop worker
                for (int i = 0; i < N; i++) {
                    c[i] = a[i] * 2.0f;
                }
                
                /* Another inner loop with different partition */
                #pragma acc loop vector
                for (int i = 0; i < N; i++) {
                    d[i] = b[i] * 0.5f;
                }
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(c[i] - a[i] * 2.0f) > VERIFY_TOLERANCE) errors++;
            if (fabs(d[i] - b[i] * 0.5f) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 11: OpenMP Target Version
     * Similar partitioning with OpenMP target directives
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: OpenMP Target Partitioning\n", test_count);
        
        #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
        {
            #pragma omp target teams distribute parallel for \
                num_teams(num_gangs) num_threads(num_workers) \
                simdlen(vector_length)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i] * 0.3f;
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            float expected = a[i] + b[i] * 0.3f;
            if (fabs(c[i] - expected) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* ============================================================
     * TEST 12: Device-Specific Behavior
     * Using device_type clause
     * ============================================================ */
    {
        test_count++;
        printf("\nTest %d: Device-Specific Partitioning\n", test_count);
        
        #pragma acc data copy(a[0:N], d[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) \
                device_type(nvidia) vector_length(64) \
                device_type(radeon) vector_length(128) \
                gang vector
            for (int i = 0; i < N; i++) {
                d[i] = a[i] * 1.5f;
            }
        }
        
        int errors = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(d[i] - a[i] * 1.5f) > VERIFY_TOLERANCE) errors++;
        }
        
        if (errors == 0) {
            printf("  PASS\n");
            success_count++;
        } else {
            printf("  FAIL: %d errors\n", errors);
        }
    }
    
    /* Cleanup and summary */
    free(a);
    free(b);
    free(c);
    free(d);
    
    printf("\n====================================================\n");
    printf("Test Summary: %d of %d tests passed\n", success_count, test_count);
    printf("Coverage target: Switch cases 0-7 for partition types\n");
    
    if (success_count == test_count) {
        printf("SUCCESS: All tests passed\n");
        return 0;
    } else {
        printf("FAILURE: %d tests failed\n", test_count - success_count);
        return 1;
    }
}

/* Implementation of routine functions */
float compute_element(float a, float b, int idx) {
    return a * b + idx * 0.01f;
}

void gang_level_operation(float *arr, int start, int end) {
    #pragma acc loop worker
    for (int i = start; i < end; i++) {
        arr[i] = arr[i] * 2.0f;
    }
}
