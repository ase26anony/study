/* 
 * Test program to cover all partition type cases in omp-oacc-neuter-broadcast.cc
 * Specifically targeting lines 335-343 switch statement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

/* Function prototypes with different partition specifications */
#pragma acc routine seq
float compute_element(float a, float b);

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
    
    int total_errors = 0;
    int test_count = 0;
    
    printf("Starting partition type coverage tests...\n\n");
    
    /* ======================================================================
     * TEST 1: Gang redundant (case 0)
     * Outer gang-redundant region with inner worker-partitioned loop
     * ====================================================================== */
    printf("Test %d: Gang redundant\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
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
    
    total_errors += verify_results(a, b, c, N, a[0] + b[0]);
    printf("  Errors: %d\n", verify_results(a, b, c, N, a[0] + b[0]));
    
    /* ======================================================================
     * TEST 2: Gang partitioned (case 1)
     * ====================================================================== */
    printf("\nTest %d: Gang partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N]) copy(d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            d[i] = a[i] * 2.0f;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (d[i] != a[i] * 2.0f) errors++;
    }
    total_errors += errors;
    printf("  Errors: %d\n", errors);
    
    /* ======================================================================
     * TEST 3: Worker partitioned (case 2)
     * ====================================================================== */
    printf("\nTest %d: Worker partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    total_errors += verify_results(a, b, c, N, a[0] - b[0]);
    printf("  Errors: %d\n", verify_results(a, b, c, N, a[0] - b[0]));
    
    /* ======================================================================
     * TEST 4: Gang+worker partitioned (case 3)
     * ====================================================================== */
    printf("\nTest %d: Gang+worker partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N]) copy(d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            d[i] = a[i] / 2.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (d[i] != a[i] / 2.0f) errors++;
    }
    total_errors += errors;
    printf("  Errors: %d\n", errors);
    
    /* ======================================================================
     * TEST 5: Vector partitioned (case 4)
     * ====================================================================== */
    printf("\nTest %d: Vector partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    total_errors += verify_results(a, b, c, N, a[0] * b[0]);
    printf("  Errors: %d\n", verify_results(a, b, c, N, a[0] * b[0]));
    
    /* ======================================================================
     * TEST 6: Gang+vector partitioned (case 5)
     * ====================================================================== */
    printf("\nTest %d: Gang+vector partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N]) copy(d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            d[i] = a[i] + 100.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (d[i] != a[i] + 100.0f) errors++;
    }
    total_errors += errors;
    printf("  Errors: %d\n", errors);
    
    /* ======================================================================
     * TEST 7: Worker+vector partitioned (case 6)
     * ====================================================================== */
    printf("\nTest %d: Worker+vector partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] - a[i];
        }
    }
    
    total_errors += verify_results(a, b, c, N, b[0] - a[0]);
    printf("  Errors: %d\n", verify_results(a, b, c, N, b[0] - a[0]));
    
    /* ======================================================================
     * TEST 8: Fully partitioned (case 7)
     * ====================================================================== */
    printf("\nTest %d: Fully partitioned (gang+worker+vector)\n", ++test_count);
    #pragma acc data copyin(a[0:N]) copy(d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length) gang worker vector
        for (int i = 0; i < N; i++) {
            d[i] = a[i] * 3.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (d[i] != a[i] * 3.0f) errors++;
    }
    total_errors += errors;
    printf("  Errors: %d\n", errors);
    
    /* ======================================================================
     * TEST 9: Conditional partition selection
     * Tests dynamic partition type determination
     * ====================================================================== */
    printf("\nTest %d: Conditional partition selection\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector_length(flag ? 16 : 32)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, a[0] + b[0] * 2.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, a[0] + b[0] * 2.0f));
    
    /* ======================================================================
     * TEST 10: Nested parallelism with different partition types
     * Outer kernels region with inner parallel loops
     * ====================================================================== */
    printf("\nTest %d: Nested parallelism with partitioning\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N], d[0:N])
    {
        #pragma acc kernels
        {
            /* First loop: gang partitioned */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 4.0f;
            }
            
            /* Second loop: worker partitioned */
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                d[i] = b[i] / 4.0f;
            }
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != a[i] * 4.0f) errors++;
        if (d[i] != b[i] / 4.0f) errors++;
    }
    total_errors += errors;
    printf("  Errors: %d\n", errors);
    
    /* ======================================================================
     * TEST 11: OpenMP target version with partition variations
     * ====================================================================== */
    printf("\nTest %d: OpenMP target with partition clauses\n", ++test_count);
    
    /* Reset arrays for OpenMP test */
    for (int i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for \
            num_teams(num_gangs) num_threads(num_workers) \
            gang worker /* Simulating gang+worker partition */
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 3.0f;
        }
    }
    
    total_errors += verify_results(a, b, c, N, a[0] + b[0] * 3.0f);
    printf("  Errors: %d\n", verify_results(a, b, c, N, a[0] + b[0] * 3.0f));
    
    /* ======================================================================
     * TEST 12: Device-specific partition behavior
     * ====================================================================== */
    printf("\nTest %d: Device-specific partition behavior\n", ++test_count);
    
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copyin(a[0:N]) copy(d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            d[i] = a[i] * 5.0f;
        }
    }
    
    errors = 0;
    for (int i = 0; i < N; i++) {
        if (d[i] != a[i] * 5.0f) errors++;
    }
    total_errors += errors;
    printf("  Errors: %d\n", errors);
    
    /* ======================================================================
     * Summary
     * ====================================================================== */
    printf("\n" "=" * 60 "\n");
    printf("Test Summary:\n");
    printf("  Total tests executed: %d\n", test_count);
    printf("  Total errors across all tests: %d\n", total_errors);
    
    if (total_errors == 0) {
        printf("  SUCCESS: All tests passed!\n");
    } else {
        printf("  WARNING: Some tests had errors\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return total_errors > 0 ? 1 : 0;
}

/* Implementation of partitioned functions */
float compute_element(float a, float b) {
    return a + b;
}

void gang_partitioned_function(float *arr, int n) {
    #pragma acc parallel loop gang
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f;
    }
}

void worker_partitioned_function(float *arr, int n) {
    #pragma acc parallel loop worker
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] / 2.0f;
    }
}

void vector_partitioned_function(float *arr, int n) {
    #pragma acc parallel loop vector
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] + 1.0f;
    }
}
