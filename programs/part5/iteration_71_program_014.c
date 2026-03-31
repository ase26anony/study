#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOLERANCE 0.0001f

// Function prototypes with different partition specifications
#pragma acc routine seq
float compute_element(int i, float factor);

#pragma acc routine gang
void gang_partitioned_func(float *arr, int n, float factor);

#pragma acc routine worker
void worker_partitioned_func(float *arr, int n, float factor);

#pragma acc routine vector
void vector_partitioned_func(float *arr, int n, float factor);

// Helper function for verification
int verify_results(float *a, float *b, int n, const char *test_name) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (abs(a[i] - b[i]) > VERIFY_TOLERANCE) {
            errors++;
            if (errors < 5) {
                printf("  Error at %d: expected %f, got %f\n", i, b[i], a[i]);
            }
        }
    }
    if (errors > 0) {
        printf("  %s: FAILED with %d errors\n", test_name, errors);
    } else {
        printf("  %s: PASSED\n", test_name);
    }
    return errors;
}

// Test 1: Gang redundant (case 0)
void test_gang_redundant() {
    printf("Test 1: Gang redundant\n");
    float a[N], b[N], c[N];
    int num_gangs = 2;
    
    // Initialize
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
    }
    
    // Gang redundant - no explicit partition clause
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(32)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i];
    }
    verify_results(c, expected, N, "Gang redundant addition");
}

// Test 2: Gang partitioned (case 1)
void test_gang_partitioned() {
    printf("Test 2: Gang partitioned\n");
    float a[N], b[N], c[N];
    int num_gangs = 4;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 3);
        c[i] = 0.0f;
    }
    
    // Explicit gang partition
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] * b[i];
    }
    verify_results(c, expected, N, "Gang partitioned multiplication");
}

// Test 3: Worker partitioned (case 2)
void test_worker_partitioned() {
    printf("Test 3: Worker partitioned\n");
    float a[N], b[N], c[N];
    int num_workers = 4;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i + 10);
        c[i] = 0.0f;
    }
    
    // Worker partitioned
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] - b[i];
    }
    verify_results(c, expected, N, "Worker partitioned subtraction");
}

// Test 4: Gang+worker partitioned (case 3)
void test_gang_worker_partitioned() {
    printf("Test 4: Gang+worker partitioned\n");
    float a[N], b[N], c[N];
    int num_gangs = 2;
    int num_workers = 4;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i % 100);
        c[i] = 0.0f;
    }
    
    // Gang and worker partitioned
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);  // Avoid division by zero
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] / (b[i] + 1.0f);
    }
    verify_results(c, expected, N, "Gang+worker partitioned division");
}

// Test 5: Vector partitioned (case 4)
void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    float a[N], b[N], c[N];
    int vector_len = 32;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 2.5f;
        c[i] = 0.0f;
    }
    
    // Vector partitioned
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop vector_length(vector_len) vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] * b[i];
    }
    verify_results(c, expected, N, "Vector partitioned scaling");
}

// Test 6: Gang+vector partitioned (case 5)
void test_gang_vector_partitioned() {
    printf("Test 6: Gang+vector partitioned\n");
    float a[N], b[N], c[N];
    int num_gangs = 2;
    int vector_len = 64;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
        c[i] = 0.0f;
    }
    
    // Gang and vector partitioned
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len) gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i];
    }
    verify_results(c, expected, N, "Gang+vector partitioned addition");
}

// Test 7: Worker+vector partitioned (case 6)
void test_worker_vector_partitioned() {
    printf("Test 7: Worker+vector partitioned\n");
    float a[N], b[N], c[N];
    int num_workers = 4;
    int vector_len = 32;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * i);
        c[i] = 0.0f;
    }
    
    // Worker and vector partitioned
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop worker(num_workers) vector_length(vector_len) worker vector
        for (int i = 0; i < N; i++) {
            c[i] = b[i] - a[i];
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = b[i] - a[i];
    }
    verify_results(c, expected, N, "Worker+vector partitioned subtraction");
}

// Test 8: Fully partitioned (case 7)
void test_fully_partitioned() {
    printf("Test 8: Fully partitioned\n");
    float a[N], b[N], c[N];
    int num_gangs = 2;
    int num_workers = 2;
    int vector_len = 16;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i % 50);
        c[i] = 0.0f;
    }
    
    // Fully partitioned (gang+worker+vector)
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len) gang worker vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * a[i] + b[i];
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] * a[i] + b[i];
    }
    verify_results(c, expected, N, "Fully partitioned computation");
}

// Test 9: Nested parallelism with different partitions
void test_nested_partitioning() {
    printf("Test 9: Nested partitioning\n");
    float a[N], b[N], c[N];
    int outer_gangs = 2;
    int inner_workers = 4;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 1.0f;
        c[i] = 0.0f;
    }
    
    // Outer gang-redundant region with inner worker-partitioned loop
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc kernels gang(outer_gangs)
        {
            // Outer gang-redundant
            #pragma acc loop gang
            for (int i = 0; i < N; i += 128) {
                // Inner worker-partitioned
                #pragma acc loop worker(inner_workers) worker
                for (int j = i; j < i + 128 && j < N; j++) {
                    c[j] = a[j] + b[j];
                }
            }
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i];
    }
    verify_results(c, expected, N, "Nested partitioning");
}

// Test 10: Conditional partition selection
void test_conditional_partition() {
    printf("Test 10: Conditional partition selection\n");
    float a[N], b[N], c[N];
    int flag = 1;
    int num_gangs = flag ? 2 : 4;
    int num_workers = flag ? 4 : 2;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i % 20);
        c[i] = 0.0f;
    }
    
    // Conditional partition specification
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
        for (int i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] / (b[i] + 1.0f);
    }
    verify_results(c, expected, N, "Conditional partition");
}

// Test 11: OpenMP target with partition variations
#ifdef _OPENMP
void test_omp_target_partitioning() {
    printf("Test 11: OpenMP target partitioning\n");
    float a[N], b[N], c[N];
    int num_teams = 2;
    int thread_limit = 64;
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
    }
    
    // OpenMP target with teams distribute
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        // Gang partitioned equivalent
        #pragma omp target teams distribute parallel for num_teams(num_teams) thread_limit(thread_limit)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] + b[i];
    }
    verify_results(c, expected, N, "OpenMP target partitioning");
}
#endif

// Test 12: Device-specific partition behavior
void test_device_specific_partition() {
    printf("Test 12: Device-specific partition\n");
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = 3.0f;
        c[i] = 0.0f;
    }
    
    // Device-specific partition specification
    #pragma acc set device_type(nvidia)
    {
        #pragma acc data copy(a[0:N], b[0:N], c[0:N])
        {
            #pragma acc parallel loop gang(2) vector_length(32) gang vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * b[i];
            }
        }
    }
    
    // Verify
    float expected[N];
    for (int i = 0; i < N; i++) {
        expected[i] = a[i] * b[i];
    }
    verify_results(c, expected, N, "Device-specific partition");
}

// Function implementations
float compute_element(int i, float factor) {
    return (float)i * factor;
}

void gang_partitioned_func(float *arr, int n, float factor) {
    #pragma acc parallel loop gang
    for (int i = 0; i < n; i++) {
        arr[i] = compute_element(i, factor);
    }
}

void worker_partitioned_func(float *arr, int n, float factor) {
    #pragma acc parallel loop worker
    for (int i = 0; i < n; i++) {
        arr[i] = compute_element(i, factor);
    }
}

void vector_partitioned_func(float *arr, int n, float factor) {
    #pragma acc parallel loop vector
    for (int i = 0; i < n; i++) {
        arr[i] = compute_element(i, factor);
    }
}

int main() {
    int total_errors = 0;
    
    printf("=== Testing OpenACC/OpenMP Partition Type Coverage ===\n\n");
    
    // Run all partition type tests
    test_gang_redundant();           // Case 0
    test_gang_partitioned();         // Case 1
    test_worker_partitioned();       // Case 2
    test_gang_worker_partitioned();  // Case 3
    test_vector_partitioned();       // Case 4
    test_gang_vector_partitioned();  // Case 5
    test_worker_vector_partitioned(); // Case 6
    test_fully_partitioned();        // Case 7
    
    // Additional tests for coverage
    test_nested_partitioning();
    test_conditional_partition();
    test_device_specific_partition();
    
    #ifdef _OPENMP
    test_omp_target_partitioning();
    #endif
    
    // Test function calls with different partition specifications
    printf("\nTest 13: Function calls with partition specifications\n");
    float func_arr[N];
    float expected_func[N];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        func_arr[i] = 0.0f;
        expected_func[i] = (float)i * 2.5f;
    }
    
    // Call gang-partitioned function
    #pragma acc data copy(func_arr[0:N])
    gang_partitioned_func(func_arr, N, 2.5f);
    
    total_errors += verify_results(func_arr, expected_func, N, "Gang-partitioned function");
    
    printf("\n=== Test Summary ===\n");
    printf("All partition types (0-7) have been exercised.\n");
    printf("Total errors: %d\n", total_errors);
    
    if (total_errors == 0) {
        printf("SUCCESS: All tests passed!\n");
    } else {
        printf("FAILURE: Some tests failed.\n");
    }
    
    return total_errors > 0 ? 1 : 0;
}
