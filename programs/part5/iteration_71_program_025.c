#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

// Function declarations with OpenACC routine directives
#pragma acc routine seq
float compute_value(float a, float b);

#pragma acc routine gang
void gang_partitioned_func(float *arr, int n);

// Helper to verify results
int verify_results(float *a, float *b, float *c, int n, float expected_factor) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        float expected = a[i] + b[i] * expected_factor;
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            errors++;
            if (errors < 5) {
                printf("Error at index %d: got %f, expected %f\n", 
                       i, c[i], expected);
            }
        }
    }
    return errors;
}

// Test 1: Gang redundant (case 0)
void test_gang_redundant() {
    printf("Test 1: Gang redundant\n");
    float a[N], b[N], c[N];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
    
    // Runtime-determined partition counts
    int num_gangs = 4;
    int num_workers = 2;
    int vector_len = 32;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        // Outer gang-redundant region
        #pragma acc parallel num_gangs(num_gangs) gang
        {
            // Inner loop with gang partitioning
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    int errors = verify_results(a, b, c, N, 1.0f);
    printf("  Errors: %d\n", errors);
}

// Test 2: Gang partitioned (case 1)
void test_gang_partitioned() {
    printf("Test 2: Gang partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + 1);
        b[i] = (float)(i * 3);
    }
    
    int num_gangs = 8;
    int use_workers = 1;  // Runtime flag
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Explicit gang partitioning
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2.0f + b[i];
        }
    }
    
    int errors = verify_results(a, b, c, N, 0.0f);
    printf("  Errors: %d\n", errors);
}

// Test 3: Worker partitioned (case 2)
void test_worker_partitioned() {
    printf("Test 3: Worker partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i * 0.5f);
        b[i] = (float)(i * 1.5f);
    }
    
    int num_workers = 4;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Worker partitioned loop
        #pragma acc parallel loop worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2.0f;
        }
    }
    
    int errors = verify_results(a, b, c, N, 2.0f);
    printf("  Errors: %d\n", errors);
}

// Test 4: Gang+worker partitioned (case 3)
void test_gang_worker_partitioned() {
    printf("Test 4: Gang+worker partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
    
    int num_gangs = 2;
    int num_workers = 4;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Combined gang and worker partitioning
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = (a[i] + b[i]) / 2.0f;
        }
    }
    
    // Custom verification for this test
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = (a[i] + b[i]) / 2.0f;
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            errors++;
        }
    }
    printf("  Errors: %d\n", errors);
}

// Test 5: Vector partitioned (case 4)
void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 10);
        b[i] = (float)(i % 20);
    }
    
    int vector_length = 64;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Vector partitioned loop
        #pragma acc parallel loop vector(vector_length)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3.0f - b[i];
        }
    }
    
    int errors = verify_results(a, b, c, N, 0.0f);
    printf("  Errors: %d\n", errors);
}

// Test 6: Gang+vector partitioned (case 5)
void test_gang_vector_partitioned() {
    printf("Test 6: Gang+vector partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i * i);
        b[i] = (float)i;
    }
    
    int num_gangs = 4;
    int vector_len = 32;
    int flag = 1;  // Runtime condition
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Conditional partitioning
        #pragma acc parallel loop gang(num_gangs) vector(flag ? vector_len : 16)
        for (int i = 0; i < N; i++) {
            c[i] = sqrtf(a[i]) + b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = sqrtf(a[i]) + b[i];
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            errors++;
        }
    }
    printf("  Errors: %d\n", errors);
}

// Test 7: Worker+vector partitioned (case 6)
void test_worker_vector_partitioned() {
    printf("Test 7: Worker+vector partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 50);
        b[i] = (float)(i % 25);
    }
    
    int num_workers = 2;
    int vector_len = 128;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Worker and vector partitioning
        #pragma acc parallel loop worker(num_workers) vector(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * a[i] - b[i] * b[i];
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = a[i] * a[i] - b[i] * b[i];
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            errors++;
        }
    }
    printf("  Errors: %d\n", errors);
}

// Test 8: Fully partitioned (case 7)
void test_fully_partitioned() {
    printf("Test 8: Fully partitioned\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i);
        b[i] = (float)(1000 - i);
    }
    
    int num_gangs = 8;
    int num_workers = 4;
    int vector_len = 16;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Fully partitioned: gang, worker, and vector
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = (a[i] + b[i]) * 0.5f;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float expected = (a[i] + b[i]) * 0.5f;
        if (fabs(c[i] - expected) > VERIFY_TOL) {
            errors++;
        }
    }
    printf("  Errors: %d\n", errors);
}

// Test 9: Nested parallelism with different partition types
void test_nested_partitioning() {
    printf("Test 9: Nested partitioning\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 100);
        b[i] = (float)(i % 200);
    }
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Outer gang-redundant region
        #pragma acc parallel num_gangs(4) gang
        {
            // Inner worker-partitioned loop
            #pragma acc loop worker(2)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * 2.0f + b[i] * 3.0f;
            }
        }
    }
    
    int errors = verify_results(a, b, c, N, 3.0f);
    printf("  Errors: %d\n", errors);
}

// Test 10: OpenMP target version with partition variations
void test_omp_target_partitioning() {
    printf("Test 10: OpenMP target partitioning\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
    
    int num_teams = 4;
    int thread_limit = 32;
    
    // Test different OpenMP partition configurations
    #pragma omp target map(to: a[0:N], b[0:N]) map(from: c[0:N])
    #pragma omp teams distribute parallel for num_teams(num_teams) thread_limit(thread_limit)
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    int errors = verify_results(a, b, c, N, 1.0f);
    printf("  Errors: %d\n", errors);
}

// Test 11: Device-specific partitioning
void test_device_specific_partitioning() {
    printf("Test 11: Device-specific partitioning\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i % 64);
        b[i] = (float)(i % 32);
    }
    
    // Simulate device-specific behavior
    #ifdef _OPENACC
    #pragma acc set device_type(nvidia)
    #endif
    
    int num_gangs = 16;
    int use_vector = 1;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Conditional vector length
        #pragma acc parallel loop gang(num_gangs) vector(use_vector ? 64 : 32)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 4.0f - b[i] * 2.0f;
        }
    }
    
    int errors = verify_results(a, b, c, N, -2.0f);
    printf("  Errors: %d\n", errors);
}

// Test 12: Function calls with partition attributes
void test_function_partitioning() {
    printf("Test 12: Function with partition attributes\n");
    float arr[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = (float)i;
    }
    
    #pragma acc data copy(arr[0:N])
    {
        #pragma acc parallel loop gang(4)
        for (int i = 0; i < N; i++) {
            // This should trigger gang-partitioned function analysis
            arr[i] = compute_value(arr[i], (float)(i * 2));
        }
    }
    
    // Simple verification
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (arr[i] != (float)(i + i * 2)) {  // compute_value just adds
            errors++;
        }
    }
    printf("  Errors: %d\n", errors);
}

// Implementation of partitioned function
#pragma acc routine seq
float compute_value(float a, float b) {
    return a + b;
}

int main() {
    printf("Starting partition type coverage tests...\n\n");
    
    int total_errors = 0;
    int tests_passed = 0;
    int total_tests = 12;
    
    // Run all tests
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_nested_partitioning();
    test_omp_target_partitioning();
    test_device_specific_partitioning();
    test_function_partitioning();
    
    printf("\nAll tests completed.\n");
    printf("Total tests run: %d\n", total_tests);
    
    return 0;
}
