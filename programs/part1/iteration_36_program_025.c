/* Test program to exercise all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Compile with: g++ -O2 -fopenacc -fopenmp -g -o test_partitions test_partitions.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#define ARRAY_SIZE 1024
#define NUM_GANGS 4
#define NUM_WORKERS 2
#define VECTOR_LENGTH 8

// Volatile variables to prevent optimization
volatile int force_runtime = 1;
volatile int check_result = 0;

// Function that could trigger partition string mapping
void debug_partition_info(int partition_code) {
    // This mimics the internal compiler logic
    // The compiler might generate similar code when debugging is enabled
    const char* desc = "<unknown>";
    switch (partition_code) {
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
    
    // Force compiler to keep the switch logic
    if (force_runtime) {
        printf("Partition debug: %s\n", desc);
    }
}

// Test case 0: gang redundant (single gang)
void test_gang_redundant() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    // Initialize with runtime values
    for (int i = 0; i < n; i++) {
        data[i] = (float)(i % 100);
    }
    
    // Single gang, no worker/vector partitioning
    #pragma acc parallel num_gangs(1) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += 1.0f;
        }
    }
    
    // Verify and potentially trigger debug output
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] != (float)(i % 100) + 1.0f) {
            errors++;
        }
    }
    
    if (errors > 0 || force_runtime) {
        debug_partition_info(0);  // Could trigger case 0
    }
    
    free(data);
}

// Test case 1: gang partitioned
void test_gang_partitioned() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        data[i] = (float)i;
    }
    
    // Multiple gangs, no workers/vectors
    #pragma acc parallel num_gangs(NUM_GANGS) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] *= 2.0f;
        }
    }
    
    // Check with runtime condition
    check_result = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] != (float)i * 2.0f) {
            check_result = 1;
        }
    }
    
    if (check_result || force_runtime) {
        debug_partition_info(1);  // Could trigger case 1
    }
    
    free(data);
}

// Test case 2: worker partitioned
void test_worker_partitioned() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        data[i] = (float)(i * 2);
    }
    
    // Single gang, multiple workers
    #pragma acc parallel num_gangs(1) num_workers(NUM_WORKERS) copy(data[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] += (float)__pgi_workeridx();
        }
    }
    
    if (force_runtime) {
        debug_partition_info(2);  // Could trigger case 2
    }
    
    free(data);
}

// Test case 3: gang+worker partitioned
void test_gang_worker_partitioned() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        data[i] = (float)i;
    }
    
    // Multiple gangs and workers
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) copy(data[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] = data[i] + (float)__pgi_gangidx() + (float)__pgi_workeridx();
        }
    }
    
    // Complex runtime check
    volatile int should_debug = 1;
    if (should_debug) {
        debug_partition_info(3);  // Could trigger case 3
    }
    
    free(data);
}

// Test case 4: vector partitioned
void test_vector_partitioned() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        data[i] = (float)i;
    }
    
    // Vector operations only
    #pragma acc parallel vector_length(VECTOR_LENGTH) copy(data[0:n])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] = sqrtf(data[i]);
        }
    }
    
    if (force_runtime) {
        debug_partition_info(4);  // Could trigger case 4
    }
    
    free(data);
}

// Test case 5: gang+vector partitioned
void test_gang_vector_partitioned() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        data[i] = (float)i;
    }
    
    // Gangs and vectors
    #pragma acc parallel num_gangs(NUM_GANGS) vector_length(VECTOR_LENGTH) copy(data[0:n])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] = sinf(data[i]) + cosf(data[i]);
        }
    }
    
    // Use asm to prevent optimization
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    asm volatile("" : : "r"(sum));
    
    debug_partition_info(5);  // Could trigger case 5
    
    free(data);
}

// Test case 6: worker+vector partitioned
void test_worker_vector_partitioned() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        data[i] = (float)i;
    }
    
    // Workers and vectors
    #pragma acc parallel num_workers(NUM_WORKERS) vector_length(VECTOR_LENGTH) copy(data[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 0.5f + (float)__pgi_workeridx();
        }
    }
    
    if (force_runtime) {
        debug_partition_info(6);  // Could trigger case 6
    }
    
    free(data);
}

// Test case 7: fully partitioned
void test_fully_partitioned() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        data[i] = (float)i;
    }
    
    // All three levels: gangs, workers, and vectors
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) vector_length(VECTOR_LENGTH) copy(data[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            data[i] = data[i] + (float)__pgi_gangidx() * 100.0f 
                     + (float)__pgi_workeridx() * 10.0f 
                     + (float)__pgi_vectoridx();
        }
    }
    
    // Force complex control flow
    for (int i = 0; i < n; i++) {
        if (data[i] > 10000.0f && force_runtime) {
            debug_partition_info(7);  // Could trigger case 7
            break;
        }
    }
    
    free(data);
}

// Test default case (illegal partition codes)
void test_illegal_partitions() {
    // Generate various illegal codes through different code paths
    int illegal_codes[] = {-1, 8, 100, -100};
    
    for (int i = 0; i < 4; i++) {
        int code = illegal_codes[i];
        
        // Use runtime variable to prevent compile-time optimization
        if (force_runtime) {
            debug_partition_info(code);  // Should trigger default case
        }
    }
    
    // Also test with OpenMP to potentially generate different partition codes
    #pragma omp target teams distribute parallel for if(force_runtime)
    for (int i = 0; i < 10; i++) {
        // Some computation
        volatile int x = i * 2;
        asm volatile("" : : "r"(x));
    }
}

// OpenMP version with teams/distribute
void test_omp_partitioning() {
    int n = ARRAY_SIZE;
    float* data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        data[i] = (float)i;
    }
    
    // OpenMP target with teams - may generate different partition mappings
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += 1.0f;
    }
    
    // Nested OpenMP for complex partitioning
    #pragma omp target teams distribute parallel for simd \
        num_teams(NUM_GANGS) thread_limit(NUM_WORKERS*VECTOR_LENGTH) \
        map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] *= 2.0f;
    }
    
    free(data);
}

int main() {
    printf("Testing partition mapping cases...\n");
    
    // Test all valid partition cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test illegal/default cases
    test_illegal_partitions();
    
    // Test OpenMP variants
    test_omp_partitioning();
    
    printf("All partition tests completed.\n");
    
    return 0;
}
