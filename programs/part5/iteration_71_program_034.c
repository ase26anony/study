#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_TOL 1e-6

// Function prototypes with different partition specifications
#pragma acc routine gang
void acc_gang_function(int *arr, int n);

#pragma acc routine worker
void acc_worker_function(int *arr, int n);

#pragma acc routine vector
void acc_vector_function(int *arr, int n);

// Test function declarations
void test_gang_redundant(int *a, int *b, int *c);
void test_gang_partitioned(int *a, int *b, int *c);
void test_worker_partitioned(int *a, int *b, int *c);
void test_gang_worker_partitioned(int *a, int *b, int *c);
void test_vector_partitioned(int *a, int *b, int *c);
void test_gang_vector_partitioned(int *a, int *b, int *c);
void test_worker_vector_partitioned(int *a, int *b, int *c);
void test_fully_partitioned(int *a, int *b, int *c);
void test_nested_partitioning(int *a, int *b, int *c);
void test_conditional_partitioning(int *a, int *b, int *c);
void test_device_specific_partitioning(int *a, int *b, int *c);

int main() {
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("=== Testing OpenACC/OpenMP Partition Clause Variations ===\n\n");
    
    // Test 1: Gang redundant (case 0)
    printf("Test 1: Gang redundant\n");
    test_gang_redundant(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 2: Gang partitioned (case 1)
    printf("Test 2: Gang partitioned\n");
    test_gang_partitioned(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 3: Worker partitioned (case 2)
    printf("Test 3: Worker partitioned\n");
    test_worker_partitioned(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 4: Gang+worker partitioned (case 3)
    printf("Test 4: Gang+worker partitioned\n");
    test_gang_worker_partitioned(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 5: Vector partitioned (case 4)
    printf("Test 5: Vector partitioned\n");
    test_vector_partitioned(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 6: Gang+vector partitioned (case 5)
    printf("Test 6: Gang+vector partitioned\n");
    test_gang_vector_partitioned(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 7: Worker+vector partitioned (case 6)
    printf("Test 7: Worker+vector partitioned\n");
    test_worker_vector_partitioned(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 8: Fully partitioned (case 7)
    printf("Test 8: Fully partitioned\n");
    test_fully_partitioned(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 9: Nested partitioning
    printf("Test 9: Nested partitioning\n");
    test_nested_partitioning(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 10: Conditional partitioning
    printf("Test 10: Conditional partitioning\n");
    test_conditional_partitioning(a, b, c);
    tests_passed++;
    total_tests++;
    
    // Test 11: Device-specific partitioning
    printf("Test 11: Device-specific partitioning\n");
    test_device_specific_partitioning(a, b, c);
    tests_passed++;
    total_tests++;
    
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}

// Test 1: Gang redundant (case 0)
void test_gang_redundant(int *a, int *b, int *c) {
    int num_gangs = 4;
    int num_workers = 1;
    int vector_len = 1;
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        // Gang redundant - no explicit partitioning
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == a[i] + b[i]);
    }
}

// Test 2: Gang partitioned (case 1)
void test_gang_partitioned(int *a, int *b, int *c) {
    int num_gangs = 8;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Explicit gang partitioning
        #pragma acc parallel loop gang(num_gangs)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == a[i] * b[i]);
    }
}

// Test 3: Worker partitioned (case 2)
void test_worker_partitioned(int *a, int *b, int *c) {
    int num_workers = 4;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Worker partitioned loop
        #pragma acc parallel loop worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == a[i] - b[i]);
    }
}

// Test 4: Gang+worker partitioned (case 3)
void test_gang_worker_partitioned(int *a, int *b, int *c) {
    int num_gangs = 4;
    int num_workers = 2;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Gang and worker partitioned
        #pragma acc parallel loop gang(num_gangs) worker(num_workers)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + 2 * b[i];
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == a[i] + 2 * b[i]);
    }
}

// Test 5: Vector partitioned (case 4)
void test_vector_partitioned(int *a, int *b, int *c) {
    int vector_len = 32;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Vector partitioned
        #pragma acc parallel loop vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = b[i] - a[i];
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == b[i] - a[i]);
    }
}

// Test 6: Gang+vector partitioned (case 5)
void test_gang_vector_partitioned(int *a, int *b, int *c) {
    int num_gangs = 4;
    int vector_len = 16;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Gang and vector partitioned
        #pragma acc parallel loop gang(num_gangs) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 3 + b[i];
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == a[i] * 3 + b[i]);
    }
}

// Test 7: Worker+vector partitioned (case 6)
void test_worker_vector_partitioned(int *a, int *b, int *c) {
    int num_workers = 2;
    int vector_len = 8;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Worker and vector partitioned
        #pragma acc parallel loop worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2;
        }
    }
    
    // Verify results
    for (int i = 0; i < N; N; i++) {
        assert(c[i] == a[i] + b[i] * 2);
    }
}

// Test 8: Fully partitioned (case 7)
void test_fully_partitioned(int *a, int *b, int *c) {
    int num_gangs = 8;
    int num_workers = 2;
    int vector_len = 4;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Fully partitioned: gang, worker, and vector
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = (a[i] + b[i]) * (a[i] - b[i]);
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == (a[i] + b[i]) * (a[i] - b[i]));
    }
}

// Test 9: Nested partitioning with different levels
void test_nested_partitioning(int *a, int *b, int *c) {
    int num_gangs = 4;
    int num_workers = 2;
    int vector_len = 8;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Outer gang-redundant region
        #pragma acc kernels gang(num_gangs)
        {
            // Inner worker-partitioned loop
            #pragma acc loop worker(num_workers)
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
            
            // Another inner vector-partitioned loop
            #pragma acc loop vector_length(vector_len)
            for (int i = 0; i < N; i++) {
                c[i] += i;
            }
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == a[i] + b[i] + i);
    }
}

// Test 10: Conditional partitioning
void test_conditional_partitioning(int *a, int *b, int *c) {
    int flag = 1;
    int num_gangs = flag ? 4 : 8;
    int num_workers = flag ? 2 : 4;
    int vector_len = flag ? 16 : 32;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Conditional partitioning based on runtime flag
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = flag ? (a[i] * b[i]) : (a[i] + b[i]);
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == a[i] * b[i]); // flag is 1
    }
}

// Test 11: Device-specific partitioning
void test_device_specific_partitioning(int *a, int *b, int *c) {
    #ifdef _OPENACC
    // OpenACC device-specific partitioning
    #pragma acc set device_type(nvidia)
    #endif
    
    #ifdef _OPENMP
    // OpenMP unified shared memory
    #pragma omp requires unified_shared_memory
    #endif
    
    int num_gangs = 4;
    int num_workers = 2;
    int vector_len = 32;
    
    #pragma acc data copy(a[0:N], b[0:N], c[0:N])
    {
        // Mixed partitioning with device-specific behavior
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector_length(vector_len)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * 2 + b[i] * 3;
        }
    }
    
    // Verify results
    for (int i = 0; i < N; i++) {
        assert(c[i] == a[i] * 2 + b[i] * 3);
    }
}

// OpenMP target versions for comparison
#ifdef _OPENMP
void test_omp_gang_partitioned(int *a, int *b, int *c) {
    int num_teams = 4;
    int thread_limit = 32;
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        // OpenMP equivalent of gang partitioned
        #pragma omp target teams distribute parallel for num_teams(num_teams) thread_limit(thread_limit)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
}

void test_omp_fully_partitioned(int *a, int *b, int *c) {
    int num_teams = 8;
    int num_threads = 64;
    
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        // OpenMP with teams and distribute
        #pragma omp target teams distribute parallel for \
                    num_teams(num_teams) num_threads(num_threads)
        for (int i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
}
#endif

// Partitioned function implementations
#pragma acc routine gang
void acc_gang_function(int *arr, int n) {
    #pragma acc loop gang
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
}

#pragma acc routine worker
void acc_worker_function(int *arr, int n) {
    #pragma acc loop worker
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

#pragma acc routine vector
void acc_vector_function(int *arr, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        arr[i] -= 1;
    }
}
