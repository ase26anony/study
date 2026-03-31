/**
 * Test program to exercise OpenACC/OpenMP partition clause variations
 * targeting uncovered lines 335-343 in omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

/* Function declarations with OpenACC routine directives */
#pragma acc routine seq
float compute_element(float a, float b, int idx);

#pragma acc routine gang
void gang_level_function(float *arr, int n);

#pragma acc routine worker
void worker_level_function(float *arr, int n);

/* Helper to verify results */
int verify_results(float *a, float *b, float *c, int n, float expected_factor) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float expected = a[i] + b[i] * expected_factor;
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: got %f, expected %f\n", 
                       i, c[i], expected);
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
    
    /* Data arrays */
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    float *d = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    int total_errors = 0;
    int test_count = 0;
    
    printf("=== Testing OpenACC/OpenMP Partition Clause Variations ===\n\n");
    
    /* Test 1: Gang redundant (case 0) */
    printf("Test %d: Gang redundant\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) /* gang redundant */
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    total_errors += verify_results(a, b, c, N, 1.0f);
    
    /* Test 2: Gang partitioned (case 1) */
    printf("Test %d: Gang partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] += a[i] * 2.0f;
        }
    }
    /* Verification for gang partitioned */
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i] + a[i] * 2.0f;
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 3: Worker partitioned (case 2) */
    printf("Test %d: Worker partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(d[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            d[i] = b[i] - a[i];
        }
    }
    for (int i = 0; i < N; i++) {
        if (fabs(d[i] - (b[i] - a[i])) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 4: Gang+worker partitioned (case 3) */
    printf("Test %d: Gang+worker partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    for (int i = 0; i < N; i++) {
        if (fabs(c[i] - (a[i] * b[i])) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 5: Vector partitioned (case 4) */
    printf("Test %d: Vector partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(d[0:N])
    {
        #pragma acc parallel loop vector_length(vector_length) vector
        for (int i = 0; i < N; i++) {
            d[i] = a[i] / (b[i] + 1.0f);
        }
    }
    for (int i = 0; i < N; i++) {
        if (fabs(d[i] - (a[i] / (b[i] + 1.0f))) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 6: Gang+vector partitioned (case 5) */
    printf("Test %d: Gang+vector partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = sqrtf(a[i] * a[i] + b[i] * b[i]);
        }
    }
    for (int i = 0; i < N; i++) {
        float expected = sqrtf(a[i] * a[i] + b[i] * b[i]);
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 7: Worker+vector partitioned (case 6) */
    printf("Test %d: Worker+vector partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(d[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            d[i] = sinf(a[i]) + cosf(b[i]);
        }
    }
    for (int i = 0; i < N; i++) {
        float expected = sinf(a[i]) + cosf(b[i]);
        if (fabs(d[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 8: Fully partitioned (case 7) */
    printf("Test %d: Fully partitioned\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f + b[i] * 2.0f;
        }
    }
    for (int i = 0; i < N; i++) {
        float expected = a[i] * 3.0f + b[i] * 2.0f;
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 9: Nested parallelism with different partition specifications */
    printf("Test %d: Nested parallelism (outer gang-redundant, inner worker-partitioned)\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(d[0:N])
    {
        #pragma acc kernels /* outer gang-redundant */
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i += 64) {
                int end = (i + 64 < N) ? i + 64 : N;
                #pragma acc loop worker /* inner worker-partitioned */
                for (int j = i; j < end; j++) {
                    d[j] = a[j] + b[j] * 0.5f;
                }
            }
        }
    }
    for (int i = 0; i < N; i++) {
        float expected = a[i] + b[i] * 0.5f;
        if (fabs(d[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 10: Conditional partition selection */
    printf("Test %d: Conditional partition selection\n", ++test_count);
    #pragma acc data copyin(a[0:N], b[0:N]) copy(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector_length(flag ? 16 : 32)
        for (int i = 0; i < N; i++) {
            c[i] = (a[i] + b[i]) * (flag ? 1.5f : 2.0f);
        }
    }
    for (int i = 0; i < N; i++) {
        float expected = (a[i] + b[i]) * 1.5f; /* flag = 1 */
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 11: OpenMP target equivalent - Gang partitioned */
    printf("Test %d: OpenMP target - Gang partitioned\n", ++test_count);
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: d[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(num_gangs) /* gang partitioned */
        for (int i = 0; i < N; i++) {
            d[i] = a[i] * 0.25f + b[i] * 0.75f;
        }
    }
    for (int i = 0; i < N; i++) {
        float expected = a[i] * 0.25f + b[i] * 0.75f;
        if (fabs(d[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 12: OpenMP target with vector partitioning */
    printf("Test %d: OpenMP target - Vector partitioned\n", ++test_count);
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for simd num_teams(num_gangs) simdlen(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i] / 100.0f;
        }
    }
    for (int i = 0; i < N; i++) {
        float expected = a[i] * b[i] / 100.0f;
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 13: Device-specific behavior simulation */
    printf("Test %d: Device-specific partition behavior\n", ++test_count);
    #pragma acc set device_type(nvidia)
    {
        #pragma acc data copyin(a[0:N], b[0:N]) copy(d[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) vector_length(64)
            for (int i = 0; i < N; i++) {
                d[i] = compute_element(a[i], b[i], i);
            }
        }
    }
    for (int i = 0; i < N; i++) {
        float expected = compute_element(a[i], b[i], i);
        if (fabs(d[i] - expected) > VERIFY_TOLERANCE) total_errors++;
    }
    
    /* Test 14: Routine with gang partitioning */
    printf("Test %d: Routine with gang partitioning\n", ++test_count);
    #pragma acc data copy(c[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs)
        {
            gang_level_function(c, N);
        }
    }
    
    /* Test 15: Routine with worker partitioning */
    printf("Test %d: Routine with worker partitioning\n", ++test_count);
    #pragma acc data copy(d[0:N])
    {
        #pragma acc parallel num_gangs(num_gangs) num_workers(num_workers)
        {
            worker_level_function(d, N);
        }
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests executed: %d\n", test_count);
    printf("Total errors: %d\n", total_errors);
    
    if (total_errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("Some tests failed. Check implementation.\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return total_errors > 0 ? 1 : 0;
}

/* Function implementations */
float compute_element(float a, float b, int idx) {
    return a * 0.3f + b * 0.7f + (float)(idx % 10) * 0.01f;
}

void gang_level_function(float *arr, int n) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] = (float)i * 0.01f;
    }
}

void worker_level_function(float *arr, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] = (float)(i % 100) * 0.1f;
    }
}
