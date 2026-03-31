#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

// Prevent optimization
volatile int force_runtime = 1;

// Function that could trigger partition string mapping
void debug_partition_info(int partition_code) {
    // This mimics the internal compiler logic
    // The compiler might generate similar code when debugging is enabled
    const char* desc;
    switch(partition_code) {
        case 0: desc = "gang redundant"; break;
        case 1: desc = "gang partitioned"; break;
        case 2: desc = "worker partitioned"; break;
        case 3: desc = "gang+worker partitioned"; break;
        case 4: desc = "vector partitioned"; break;
        case 5: desc = "gang+vector partitioned"; break;
        case 6: desc = "worker+vector partitioned"; break;
        case 7: desc = "fully partitioned"; break;
        default: desc = "<illegal>"; break;
    }
    
    // Force compiler to keep the string mapping logic
    asm volatile("" : : "r"(desc));
    
    if (force_runtime) {
        printf("Partition debug: %s\n", desc);
    }
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant() {
    const int N = 1000;
    int* data = (int*)malloc(N * sizeof(int));
    
    #pragma acc parallel num_gangs(1) copy(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i * 2;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) errors++;
    }
    
    if (errors > 0) debug_partition_info(0);
    free(data);
}

// Test case 1: Gang partitioned
void test_gang_partitioned() {
    const int N = 10000;
    int* data = (int*)malloc(N * sizeof(int));
    int gang_size = force_runtime ? 32 : 16;
    
    #pragma acc parallel num_gangs(gang_size) copy(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] = i + 1;
        }
    }
    
    // Force runtime evaluation
    volatile int check = 0;
    for (int i = 0; i < N; i++) {
        check += data[i];
    }
    
    if (check > 0) debug_partition_info(1);
    free(data);
}

// Test case 2: Worker partitioned
void test_worker_partitioned() {
    const int N = 1000;
    float* data = (float*)malloc(N * sizeof(float));
    
    #pragma acc parallel num_gangs(1) num_workers(4) copy(data[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = (float)i / 2.0f;
        }
    }
    
    // Conditional debug output
    if (force_runtime) {
        debug_partition_info(2);
    }
    free(data);
}

// Test case 3: Gang+Worker partitioned
void test_gang_worker_partitioned() {
    const int N = 2000;
    double* data = (double*)malloc(N * sizeof(double));
    int dynamic_size = force_runtime ? 8 : 4;
    
    #pragma acc parallel num_gangs(dynamic_size) num_workers(2) copy(data[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 3.14;
        }
    }
    
    // Use result to prevent optimization
    volatile double sum = 0.0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    
    if (sum != 0.0) debug_partition_info(3);
    free(data);
}

// Test case 4: Vector partitioned
void test_vector_partitioned() {
    const int N = 1024;
    int* data = (int*)malloc(N * sizeof(int));
    
    #pragma acc parallel vector_length(128) copy(data[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = i % 256;
        }
    }
    
    // Runtime check
    int max_val = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] > max_val) max_val = data[i];
    }
    
    if (max_val == 255) debug_partition_info(4);
    free(data);
}

// Test case 5: Gang+Vector partitioned
void test_gang_vector_partitioned() {
    const int N = 4096;
    float* data = (float*)malloc(N * sizeof(float));
    
    #pragma acc parallel num_gangs(16) vector_length(64) copy(data[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            data[i] = (float)i / 100.0f;
        }
    }
    
    // Force evaluation
    volatile float total = 0.0f;
    #pragma acc parallel loop reduction(+:total) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        total += data[i];
    }
    
    if (total > 0.0f) debug_partition_info(5);
    free(data);
}

// Test case 6: Worker+Vector partitioned
void test_worker_vector_partitioned() {
    const int N = 2048;
    int* data = (int*)malloc(N * sizeof(int));
    
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) copy(data[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            data[i] = (i * 3) % 1024;
        }
    }
    
    // Complex dependency to force partitioning
    int pattern_errors = 0;
    for (int i = 1; i < N; i++) {
        if (data[i] != ((data[i-1] + 3) % 1024)) pattern_errors++;
    }
    
    if (pattern_errors > N/2) debug_partition_info(6);
    free(data);
}

// Test case 7: Fully partitioned
void test_fully_partitioned() {
    const int N = 8192;
    double* data = (double*)malloc(N * sizeof(double));
    
    // Dynamic parameters to prevent compile-time optimization
    int gangs = force_runtime ? 32 : 16;
    int workers = force_runtime ? 4 : 2;
    int vector_len = force_runtime ? 64 : 32;
    
    #pragma acc parallel num_gangs(gangs) num_workers(workers) vector_length(vector_len) copy(data[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            data[i] = (double)i * 1.5;
        }
    }
    
    // Nested parallel region with different partitioning
    #pragma acc parallel loop gang worker vector collapse(2) copy(data[0:N])
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 128; j++) {
            int idx = i * 128 + j;
            if (idx < N) {
                data[idx] += 1.0;
            }
        }
    }
    
    if (force_runtime) debug_partition_info(7);
    free(data);
}

// Test default case (illegal partition codes)
void test_illegal_partitions() {
    // Generate various invalid partition codes through macro expansion
    #define TEST_INVALID(code) \
        if (force_runtime) { \
            debug_partition_info(code); \
        }
    
    // Test boundary cases
    TEST_INVALID(-1)
    TEST_INVALID(8)
    TEST_INVALID(255)
    TEST_INVALID(1024)
    
    // Dynamic invalid code
    int invalid_code = 999;
    volatile int* ptr = &invalid_code;
    debug_partition_info(*ptr);
}

// OpenMP equivalents to trigger similar partitioning logic
void test_omp_partitioning() {
    const int N = 1000;
    int* data = (int*)malloc(N * sizeof(int));
    
    // OpenMP target with teams - similar to gang partitioning
    #pragma omp target teams distribute parallel for map(tofrom: data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    // Combined parallel constructs
    #pragma omp target teams distribute parallel for simd \
        num_teams(8) thread_limit(32) simdlen(8) \
        map(tofrom: data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] += 1;
    }
    
    free(data);
}

// Template function to generate different partition scenarios
template<int PartitionType>
void template_partition_test() {
    const int N = 512;
    float data[N];
    
    // Different directives based on template parameter
    if constexpr (PartitionType == 0) {
        #pragma acc parallel num_gangs(1) copy(data[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) data[i] = 1.0f;
        }
    } else if constexpr (PartitionType == 1) {
        #pragma acc parallel num_gangs(4) copy(data[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) data[i] = 2.0f;
        }
    }
    // ... more cases
    
    debug_partition_info(PartitionType);
}

int main() {
    printf("Testing OpenACC/OpenMP partition coverage...\n");
    
    // Execute all test cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test OpenMP partitioning
    test_omp_partitioning();
    
    // Test illegal partitions
    test_illegal_partitions();
    
    // Template instantiations
    template_partition_test<0>();
    template_partition_test<1>();
    template_partition_test<7>();
    
    printf("All partition tests completed.\n");
    
    // Final check to ensure all code paths are considered
    volatile int final_check = 0;
    #pragma acc parallel copy(final_check)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < 10; i++) {
            final_check += i;
        }
    }
    
    return final_check > 0 ? 0 : 1;
}
