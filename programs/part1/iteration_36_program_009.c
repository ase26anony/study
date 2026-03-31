#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#ifdef __cplusplus
extern "C" {
#endif

// Prevent optimization
volatile int force_runtime = 1;

// Function to use results and prevent dead code elimination
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

// Error reporting function that could trigger partition string lookup
void report_partition_error(int code, const char* msg) {
    // This mimics the kind of diagnostic output that would use the partition strings
    fprintf(stderr, "Partition error %d: %s\n", code, msg);
}

// Test functions for each partition type
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Case 0: Single gang (gang redundant)
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += 1;
        }
    }
    
    // Verify and use result
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Case 1: Multiple gangs (gang partitioned)
    #pragma acc parallel copy(data[0:n]) num_gangs(4) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            if (i % 4 == acc_gang()) {
                data[i] *= 2;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Case 2: Worker partitioned (single gang, multiple workers)
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] += acc_worker();
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Case 3: Gang+worker partitioned
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(2) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            int idx = (acc_gang() * 2 + acc_worker()) % n;
            data[idx] += 1;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Case 4: Vector partitioned
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(1) vector_length(128)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] += acc_vector();
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Case 5: Gang+vector partitioned
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            if (i % 2 == acc_gang()) {
                data[i] += acc_vector();
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Case 6: Worker+vector partitioned
    #pragma acc parallel copy(data[0:n]) num_gangs(1) num_workers(2) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] += acc_worker() * 10 + acc_vector();
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Case 7: Fully partitioned (gang+worker+vector)
    #pragma acc parallel copy(data[0:n]) num_gangs(2) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            int idx = (acc_gang() * 4 + acc_worker() * 2 + acc_vector()) % n;
            data[idx] += 1;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// OpenMP equivalents to trigger similar partitioning logic
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(32) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() + omp_get_thread_num();
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    // Nested parallelism for combined partitioning
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(64) simdlen(8) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
    }
    
    free(data);
}

// Function to test boundary/invalid cases
void test_invalid_partitions(int n) {
    // This function uses runtime values that could lead to invalid partition codes
    int *data = (int*)malloc(n * sizeof(int));
    
    // Use volatile to prevent compile-time optimization
    volatile int invalid_gangs = -1;
    volatile int invalid_workers = 0;
    volatile int invalid_vector = 256; // Might be too large
    
    // Conditional compilation to potentially generate invalid partition codes
    #ifdef TEST_INVALID
    #pragma acc parallel copy(data[0:n]) \
        num_gangs(invalid_gangs) num_workers(invalid_workers) vector_length(invalid_vector)
    {
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            data[i] = i;
        }
    }
    #endif
    
    // Runtime check that could trigger error reporting with partition strings
    if (n < 0) {  // Always false, but compiler doesn't know
        report_partition_error(-1, "Invalid partition detected");
    }
    
    free(data);
}

// Template/macro approach for different partition schemes
#define GENERATE_PARTITION_TEST(NAME, GANGS, WORKERS, VECTOR) \
void test_##NAME(int n) { \
    int *data = (int*)malloc(n * sizeof(int)); \
    for (int i = 0; i < n; i++) data[i] = i; \
    \
    #pragma acc parallel copy(data[0:n]) \
        num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR) \
    { \
        #pragma acc loop gang worker vector \
        for (int i = 0; i < n; i++) { \
            data[i] += 1; \
        } \
    } \
    \
    int sum = 0; \
    for (int i = 0; i < n; i++) sum += data[i]; \
    use_result(sum); \
    free(data); \
}

// Generate specific test cases using the macro
GENERATE_PARTITION_TEST(macro_gang_heavy, 8, 1, 1)
GENERATE_PARTITION_TEST(macro_worker_heavy, 1, 8, 1)
GENERATE_PARTITION_TEST(macro_vector_heavy, 1, 1, 256)
GENERATE_PARTITION_TEST(macro_balanced, 4, 4, 16)

int main(int argc, char** argv) {
    // Use runtime-determined size to prevent compile-time optimization
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    if (n <= 0) n = 1000;
    
    printf("Testing partition mapping with n = %d\n", n);
    
    // Test all valid partition cases (0-7)
    test_gang_redundant(n);
    test_gang_partitioned(n);
    test_worker_partitioned(n);
    test_gang_worker_partitioned(n);
    test_vector_partitioned(n);
    test_gang_vector_partitioned(n);
    test_worker_vector_partitioned(n);
    test_fully_partitioned(n);
    
    // Test OpenMP partitioning
    test_omp_partitioning(n);
    
    // Test macro-generated partitions
    test_macro_gang_heavy(n);
    test_macro_worker_heavy(n);
    test_macro_vector_heavy(n);
    test_macro_balanced(n);
    
    // Test boundary/invalid cases
    test_invalid_partitions(n);
    
    printf("All partition tests completed\n");
    
    // Force runtime evaluation of partition codes
    if (force_runtime) {
        // This empty block ensures the compiler can't optimize away everything
        asm volatile("" : : : "memory");
    }
    
    return 0;
}

#ifdef __cplusplus
}
#endif
