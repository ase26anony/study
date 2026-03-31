#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function to prevent optimization
void use_int(int x) {
    asm volatile("" : : "r"(x));
}

// Function to generate partition-dependent output
void debug_partition(int code, const char* desc) {
    // This mimics the internal compiler logic
    if (code < 0 || code > 7) {
        printf("Invalid partition code %d: %s\n", code, desc);
    }
}

// Test functions for each partition type
void test_gang_redundant(int n) {
    volatile int force_runtime = n;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:1) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    // Force partition logic
    use_int(data[0]);
    free(data);
}

void test_gang_partitioned(int n) {
    int gangs = 4;
    if (n < 100) gangs = 2;
    
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:gangs) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i + (i % gangs);
    }
    
    use_int(data[n-1]);
    free(data);
}

void test_worker_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:1) worker(num:4) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 3;
    }
    
    use_int(data[n/2]);
    free(data);
}

void test_gang_worker_partitioned(int n) {
    int gangs = 2;
    int workers = 4;
    
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:gangs) worker(num:workers) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        int gang_id = (i / (n/gangs)) % gangs;
        data[i] = i + gang_id;
    }
    
    use_int(data[0] + data[n-1]);
    free(data);
}

void test_vector_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop vector_length(128) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 4;
    }
    
    use_int(data[1]);
    free(data);
}

void test_gang_vector_partitioned(int n) {
    int gangs = 2;
    
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:gangs) vector_length(64) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * (1 + (i % gangs));
    }
    
    use_int(data[n/4]);
    free(data);
}

void test_worker_vector_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop worker(num:2) vector_length(32) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 5;
    }
    
    use_int(data[n/3]);
    free(data);
}

void test_fully_partitioned(int n) {
    int gangs = 2;
    int workers = 2;
    int vector_len = 16;
    
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:gangs) worker(num:workers) vector_length(vector_len) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        int idx = i % (gangs * workers * vector_len);
        data[i] = idx;
    }
    
    use_int(data[0] + data[n-1] + data[n/2]);
    free(data);
}

// OpenMP versions for additional coverage
void test_omp_gang_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma omp target teams distribute parallel for num_teams(4) thread_limit(1) map(tofrom:data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 6;
    }
    
    use_int(data[0]);
    free(data);
}

void test_omp_worker_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma omp target teams distribute parallel for num_teams(1) thread_limit(8) map(tofrom:data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 7;
    }
    
    use_int(data[1]);
    free(data);
}

void test_omp_fully_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma omp target teams distribute parallel for simd num_teams(2) thread_limit(4) map(tofrom:data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 8;
    }
    
    use_int(data[2]);
    free(data);
}

// Function to test invalid partition codes
void test_invalid_partitions() {
    // Force generation of invalid codes through variable manipulation
    volatile int invalid_codes[] = {-1, 8, 100, -100};
    
    for (int i = 0; i < 4; i++) {
        int code = invalid_codes[i];
        // This should trigger the default case
        debug_partition(code, "Testing invalid");
    }
}

// Template/macro approach for compile-time variations
#define GENERATE_PARTITION_TEST(partition_type, n) \
    do { \
        int* arr = (int*)malloc((n) * sizeof(int)); \
        partition_type(arr, (n)); \
        free(arr); \
    } while(0)

void partition_gang(int* arr, int n) {
    #pragma acc parallel loop gang(num:2) copy(arr[0:n])
    for (int i = 0; i < n; i++) arr[i] = i;
}

void partition_worker(int* arr, int n) {
    #pragma acc parallel loop worker(num:4) copy(arr[0:n])
    for (int i = 0; i < n; i++) arr[i] = i * 2;
}

void partition_vector(int* arr, int n) {
    #pragma acc parallel loop vector_length(64) copy(arr[0:n])
    for (int i = 0; i < n; i++) arr[i] = i * 3;
}

// Complex nested partitioning
void test_nested_partitioning(int n) {
    int* outer = (int*)malloc(n * sizeof(int));
    int* inner = (int*)malloc(n * sizeof(int));
    
    // Outer gang partitioning
    #pragma acc parallel loop gang(num:2) copy(outer[0:n])
    for (int i = 0; i < n; i++) {
        outer[i] = i;
        
        // Inner worker partitioning
        #pragma acc loop worker(num:2)
        for (int j = 0; j < 10; j++) {
            inner[(i * 10 + j) % n] = outer[i] + j;
        }
    }
    
    use_int(outer[0] + inner[0]);
    free(outer);
    free(inner);
}

// Conditional partitioning based on runtime
void test_conditional_partitioning(int n, int mode) {
    int* data = (int*)malloc(n * sizeof(int));
    
    if (mode == 0) {
        #pragma acc parallel loop gang(num:1) worker(num:1) vector_length(1) copy(data[0:n])
        for (int i = 0; i < n; i++) data[i] = i;
    } else if (mode == 1) {
        #pragma acc parallel loop gang(num:2) copy(data[0:n])
        for (int i = 0; i < n; i++) data[i] = i * 2;
    } else if (mode == 2) {
        #pragma acc parallel loop worker(num:4) copy(data[0:n])
        for (int i = 0; i < n; i++) data[i] = i * 3;
    } else {
        #pragma acc parallel loop gang(num:2) worker(num:2) vector_length(32) copy(data[0:n])
        for (int i = 0; i < n; i++) data[i] = i * 4;
    }
    
    use_int(data[n-1]);
    free(data);
}

int main() {
    int n = 1000;
    volatile int runtime_n = n;  // Prevent compile-time optimization
    
    printf("Testing all partition types...\n");
    
    // Test all switch cases systematically
    test_gang_redundant(runtime_n);          // Case 0
    test_gang_partitioned(runtime_n);        // Case 1
    test_worker_partitioned(runtime_n);      // Case 2
    test_gang_worker_partitioned(runtime_n); // Case 3
    test_vector_partitioned(runtime_n);      // Case 4
    test_gang_vector_partitioned(runtime_n); // Case 5
    test_worker_vector_partitioned(runtime_n); // Case 6
    test_fully_partitioned(runtime_n);       // Case 7
    
    // Additional OpenMP tests
    test_omp_gang_partitioned(runtime_n);
    test_omp_worker_partitioned(runtime_n);
    test_omp_fully_partitioned(runtime_n);
    
    // Test invalid codes (default case)
    test_invalid_partitions();
    
    // Template/macro tests
    GENERATE_PARTITION_TEST(partition_gang, runtime_n);
    GENERATE_PARTITION_TEST(partition_worker, runtime_n);
    GENERATE_PARTITION_TEST(partition_vector, runtime_n);
    
    // Complex scenarios
    test_nested_partitioning(runtime_n);
    
    // Conditional partitioning
    for (int mode = 0; mode < 4; mode++) {
        test_conditional_partitioning(runtime_n, mode);
    }
    
    // Data-dependent partitioning
    int* sizes = (int*)malloc(8 * sizeof(int));
    for (int i = 0; i < 8; i++) {
        sizes[i] = runtime_n * (i + 1);
    }
    
    // Different partition types based on array size
    for (int i = 0; i < 8; i++) {
        int size = sizes[i];
        int* arr = (int*)malloc(size * sizeof(int));
        
        if (size < 500) {
            #pragma acc parallel loop gang(num:1) copy(arr[0:size])
            for (int j = 0; j < size; j++) arr[j] = j;
        } else if (size < 1000) {
            #pragma acc parallel loop gang(num:2) copy(arr[0:size])
            for (int j = 0; j < size; j++) arr[j] = j * 2;
        } else {
            #pragma acc parallel loop gang(num:4) worker(num:2) copy(arr[0:size])
            for (int j = 0; j < size; j++) arr[j] = j * 3;
        }
        
        use_int(arr[0]);
        free(arr);
    }
    
    free(sizes);
    
    printf("All partition tests completed.\n");
    
    return 0;
}

#ifdef __cplusplus
}
#endif
