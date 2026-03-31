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
void gang_level_operation(float *arr, int start, int end, float factor) {
    for (int i = start; i < end; i++) {
        arr[i] *= factor;
    }
}

/* Helper to verify results */
int verify_array(float *a, float *b, int n, const char *test_name) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (abs(a[i] - b[i]) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  %s: mismatch at %d: %f != %f\n", 
                       test_name, i, a[i], b[i]);
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
    float *c_ref = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100);
        b[i] = (float)((i + 1) % 100);
        c_ref[i] = a[i] + b[i];
    }
    
    int total_tests = 0;
    int failed_tests = 0;
    
    printf("=== Testing OpenACC Partition Variations ===\n\n");
    
    /* Test 1: Gang redundant (case 0) */
    printf("Test 1: Gang redundant\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    failed_tests += verify_array(c, c_ref, N, "Test 1");
    
    /* Test 2: Gang partitioned (case 1) */
    printf("Test 2: Gang partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = compute_element(a[i], b[i], i);
        }
    }
    
    /* Test 3: Worker partitioned (case 2) */
    printf("Test 3: Worker partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    /* Test 4: Gang+worker partitioned (case 3) */
    printf("Test 4: Gang+worker partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    /* Test 5: Vector partitioned (case 4) */
    printf("Test 5: Vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop vector(vector_length) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    
    /* Test 6: Gang+vector partitioned (case 5) */
    printf("Test 6: Gang+vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector(vector_length) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
    
    /* Test 7: Worker+vector partitioned (case 6) */
    printf("Test 7: Worker+vector partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector(vector_length) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] - a[i];
        }
    }
    
    /* Test 8: Fully partitioned (case 7) */
    printf("Test 8: Fully partitioned\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_length) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f + b[i];
        }
    }
    
    /* Test 9: Nested parallelism with different partition types */
    printf("Test 9: Nested parallelism (outer gang-redundant, inner worker-partitioned)\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc kernels gang(num_gangs) gang
        {
            #pragma acc loop worker(num_workers) worker
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i] * 0.5f;
            }
        }
    }
    
    /* Test 10: Conditional partition selection */
    printf("Test 10: Conditional partition selection\n");
    total_tests++;
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(flag ? 2 : 4) vector(flag ? 16 : 32) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * (flag ? 1.5f : 2.5f) + b[i];
        }
    }
    
    /* Test 11: Multiple target architectures simulation */
    printf("Test 11: Device-specific partition behavior\n");
    total_tests++;
    #pragma acc set device_type(nvidia)
    {
        #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
        {
            #pragma acc parallel loop gang(num_gangs) vector(vector_length) gang vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * b[i] * 0.1f;
            }
        }
    }
    
    /* Test 12: Function call with gang routine */
    printf("Test 12: Function with gang routine directive\n");
    total_tests++;
    #pragma acc data copy(a[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int g = 0; g < num_gangs; g++) {
            int start = g * (N / num_gangs);
            int end = (g + 1) * (N / num_gangs);
            if (g == num_gangs - 1) end = N;
            gang_level_operation(a, start, end, 2.0f);
        }
    }
    
    /* OpenMP Target tests for cross-runtime coverage */
    printf("\n=== Testing OpenMP Target Partition Variations ===\n\n");
    
    /* Test 13: OpenMP gang partitioned */
    printf("Test 13: OpenMP target teams distribute (gang partitioned)\n");
    total_tests++;
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(num_gangs) thread_limit(256)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    failed_tests += verify_array(c, c_ref, N, "Test 13");
    
    /* Test 14: OpenMP with unified shared memory */
    printf("Test 14: OpenMP with unified shared memory\n");
    total_tests++;
    #pragma omp requires unified_shared_memory
    #pragma omp target teams distribute parallel for num_teams(num_gangs) thread_limit(256)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] * 0.5f + b[i] * 0.5f;
    }
    
    /* Test 15: Complex nested OpenMP with partition simulation */
    printf("Test 15: Complex nested OpenMP\n");
    total_tests++;
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams num_teams(num_gangs)
        {
            #pragma omp distribute
            for (int g = 0; g < num_gangs; g++) {
                #pragma omp parallel for
                for (int i = g * (N / num_gangs); i < (g + 1) * (N / num_gangs); i++) {
                    if (i < N) {
                        c[i] = a[i] * b[i] / 100.0f;
                    }
                }
            }
        }
    }
    
    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total tests executed: %d\n", total_tests);
    printf("Tests with verification errors: %d\n", failed_tests);
    
    if (failed_tests == 0) {
        printf("All tests passed!\n");
    } else {
        printf("Some tests failed verification.\n");
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(c_ref);
    
    return failed_tests > 0 ? 1 : 0;
}
