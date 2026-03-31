#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

// Function prototypes with OpenACC routine directives
#pragma acc routine seq
float compute_value(int i, float factor);

#pragma acc routine gang
void gang_partitioned_function(float *arr, int n, float factor);

// Helper to verify results
int verify_results(float *a, float *b, int n, float expected_factor) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float expected = (float)i * expected_factor;
        if (fabs(a[i] - expected) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("Error at index %d: got %f, expected %f\n", 
                       i, a[i], expected);
            }
        }
    }
    return errors;
}

int main() {
    float *a, *b, *c, *d;
    int i;
    int success_count = 0;
    int total_tests = 0;
    
    // Allocate and initialize arrays
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    d = (float*)malloc(N * sizeof(float));
    
    for (i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
        d[i] = 0.0f;
    }
    
    printf("Starting partition type coverage tests...\n\n");
    
    // Runtime partition configuration variables
    int num_gangs = 4;
    int num_workers = 2;
    int vector_length = 32;
    int flag = 1;
    
    // ============================================
    // TEST 1: Gang Redundant (case 0)
    // ============================================
    printf("Test 1: Gang redundant\n");
    total_tests++;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        // Outer region with gang redundancy
        #pragma acc parallel loop gang(num_gangs) copy(c[0:N])
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f;
        }
        
        // Verify
        #pragma acc update host(c[0:N])
    }
    
    int errors = verify_results(c, a, N, 2.0f);
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 2: Gang Partitioned (case 1)
    // ============================================
    printf("\nTest 2: Gang partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Explicit gang partitioning
        #pragma acc parallel loop gang(num_gangs) gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
        
        #pragma acc update host(c[0:N])
    }
    
    errors = verify_results(c, a, N, 3.0f); // a[i] + 2*a[i] = 3*a[i]
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 3: Worker Partitioned (case 2)
    // ============================================
    printf("\nTest 3: Worker partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        // Worker partitioning within gang-redundant region
        #pragma acc parallel num_gangs(1) copy(c[0:N])
        {
            #pragma acc loop worker(num_workers) worker
            for (i = 0; i < N; i++) {
                c[i] = a[i] * 3.0f;
            }
        }
        
        #pragma acc update host(c[0:N])
    }
    
    errors = verify_results(c, a, N, 3.0f);
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 4: Gang+Worker Partitioned (case 3)
    // ============================================
    printf("\nTest 4: Gang+Worker partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Combined gang and worker partitioning
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
        
        #pragma acc update host(c[0:N])
    }
    
    errors = verify_results(c, a, N, 2.0f * (float)N); // a[i] * 2*a[i] = 2*a[i]^2
    // Note: This verification is approximate due to floating point
    int approx_errors = 0;
    for (i = 0; i < N; i++) {
        float expected = a[i] * b[i];
        if (fabs(c[i] - expected) > VERIFY_TOLERANCE * 10) {
            approx_errors++;
        }
    }
    
    if (approx_errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d approximate errors\n", approx_errors);
    }
    
    // ============================================
    // TEST 5: Vector Partitioned (case 4)
    // ============================================
    printf("\nTest 5: Vector partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        // Vector partitioning
        #pragma acc parallel loop vector_length(vector_length) vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 4.0f;
        }
        
        #pragma acc update host(c[0:N])
    }
    
    errors = verify_results(c, a, N, 4.0f);
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 6: Gang+Vector Partitioned (case 5)
    // ============================================
    printf("\nTest 6: Gang+Vector partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        // Combined gang and vector partitioning
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_length) gang vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 5.0f;
        }
        
        #pragma acc update host(c[0:N])
    }
    
    errors = verify_results(c, a, N, 5.0f);
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 7: Worker+Vector Partitioned (case 6)
    // ============================================
    printf("\nTest 7: Worker+Vector partitioned\n");
    total_tests++;
    #pragma acc data copy(a[0:N], c[0:N])
    {
        // Combined worker and vector partitioning
        #pragma acc parallel num_gangs(1) copy(c[0:N])
        {
            #pragma acc loop worker(num_workers) vector_length(vector_length) worker vector
            for (i = 0; i < N; i++) {
                c[i] = a[i] * 6.0f;
            }
        }
        
        #pragma acc update host(c[0:N])
    }
    
    errors = verify_results(c, a, N, 6.0f);
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 8: Fully Partitioned (case 7)
    // ============================================
    printf("\nTest 8: Fully partitioned (gang+worker+vector)\n");
    total_tests++;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Full three-level partitioning
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_length) gang worker vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
        
        #pragma acc update host(c[0:N])
    }
    
    errors = verify_results(c, a, N, 2.0f); // a[i] + a[i] = 2*a[i]
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 9: Conditional Partition Selection
    // ============================================
    printf("\nTest 9: Conditional partition selection\n");
    total_tests++;
    #pragma acc data copy(a[0:N], d[0:N])
    {
        // Runtime conditional partitioning
        int dynamic_workers = flag ? 2 : 4;
        int dynamic_vector = flag ? 16 : 64;
        
        #pragma acc parallel loop gang(num_gangs) worker(dynamic_workers) vector_length(dynamic_vector) gang worker vector
        for (i = 0; i < N; i++) {
            d[i] = compute_value(i, 7.0f);
        }
        
        #pragma acc update host(d[0:N])
    }
    
    errors = verify_results(d, a, N, 7.0f);
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 10: Nested Parallelism with Partitioning
    // ============================================
    printf("\nTest 10: Nested parallelism with different partitions\n");
    total_tests++;
    #pragma acc data copy(a[0:N], b[0:N], c[0:N], d[0:N])
    {
        // Outer gang-redundant region
        #pragma acc kernels copy(c[0:N], d[0:N])
        {
            // Inner worker-partitioned loop
            #pragma acc loop gang(num_gangs) independent
            for (int j = 0; j < num_gangs; j++) {
                int start = j * (N / num_gangs);
                int end = (j + 1) * (N / num_gangs);
                
                #pragma acc loop worker(num_workers) worker
                for (i = start; i < end && i < N; i++) {
                    c[i] = a[i] * 8.0f;
                }
            }
            
            // Another inner loop with vector partitioning
            #pragma acc loop vector_length(vector_length) vector
            for (i = 0; i < N; i++) {
                d[i] = b[i] * 0.25f;
            }
        }
        
        #pragma acc update host(c[0:N], d[0:N])
    }
    
    int errors_c = verify_results(c, a, N, 8.0f);
    int errors_d = verify_results(d, b, N, 0.25f);
    
    if (errors_c == 0 && errors_d == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: c errors=%d, d errors=%d\n", errors_c, errors_d);
    }
    
    // ============================================
    // TEST 11: OpenMP Target Equivalent
    // ============================================
    printf("\nTest 11: OpenMP target with partition variations\n");
    total_tests++;
    
    // Reset arrays
    for (i = 0; i < N; i++) {
        c[i] = 0.0f;
    }
    
    // OpenMP target with gang partitioning (teams)
    #pragma omp target teams distribute parallel for \
        num_teams(num_gangs) thread_limit(num_workers*vector_length) \
        map(to: a[0:N]) map(from: c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] * 9.0f;
    }
    
    errors = verify_results(c, a, N, 9.0f);
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // TEST 12: Device-specific behavior
    // ============================================
    printf("\nTest 12: Device-specific partition behavior\n");
    total_tests++;
    
    #pragma acc set device_type(nvidia)
    
    #pragma acc data copy(a[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(64) gang vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 10.0f;
        }
        
        #pragma acc update host(c[0:N])
    }
    
    errors = verify_results(c, a, N, 10.0f);
    if (errors == 0) {
        printf("  PASS\n");
        success_count++;
    } else {
        printf("  FAIL: %d errors\n", errors);
    }
    
    // ============================================
    // Summary
    // ============================================
    printf("\n============================================\n");
    printf("Test Summary:\n");
    printf("  Total tests run: %d\n", total_tests);
    printf("  Successful tests: %d\n", success_count);
    printf("  Failed tests: %d\n", total_tests - success_count);
    printf("============================================\n");
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    return (success_count == total_tests) ? 0 : 1;
}

// Implementations of declared functions
float compute_value(int i, float factor) {
    return (float)i * factor;
}

#pragma acc routine gang
void gang_partitioned_function(float *arr, int n, float factor) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * factor;
    }
}
