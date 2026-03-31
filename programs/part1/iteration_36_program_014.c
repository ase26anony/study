#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

// Prevent optimization
volatile int force_runtime = 1;

// Function that could trigger partition string mapping
void debug_partition_info(int partition_code) {
    // This might trigger the compiler's internal partition mapping
    // when compiled with debugging/diagnostic flags
    #ifdef _DEBUG
    printf("Partition code: %d\n", partition_code);
    #endif
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant() {
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    
    // Initialize
    for (int i = 0; i < N; i++) data[i] = i;
    
    // Single gang, redundant across gangs
    #pragma acc parallel num_gangs(1) copy(data[0:N])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        #endif
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] += gang_id;  // All elements get same gang_id
        }
    }
    
    // Force runtime evaluation
    if (force_runtime) {
        int sum = 0;
        for (int i = 0; i < N; i++) sum += data[i];
        debug_partition_info(0);  // Could trigger case 0
    }
    
    free(data);
}

// Test case 1: Gang partitioned
void test_gang_partitioned() {
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    int gangs = 4;
    
    for (int i = 0; i < N; i++) data[i] = i;
    
    // Multiple gangs, partitioned by gangs
    #pragma acc parallel num_gangs(gangs) copy(data[0:N])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        #endif
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            // Each gang processes different chunks
            if (i % gangs == gang_id) {
                data[i] += gang_id * 1000;
            }
        }
    }
    
    if (force_runtime) {
        debug_partition_info(1);  // Could trigger case 1
    }
    
    free(data);
}

// Test case 2: Worker partitioned
void test_worker_partitioned() {
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) data[i] = i;
    
    // Single gang, multiple workers
    #pragma acc parallel num_gangs(1) num_workers(4) copy(data[0:N])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = __pgi_workeridx();
        #endif
        
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            if (i % 4 == worker_id) {
                data[i] += worker_id * 100;
            }
        }
    }
    
    if (force_runtime) {
        debug_partition_info(2);  // Could trigger case 2
    }
    
    free(data);
}

// Test case 3: Gang+worker partitioned
void test_gang_worker_partitioned() {
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) data[i] = i;
    
    // Multiple gangs and workers
    #pragma acc parallel num_gangs(4) num_workers(4) copy(data[0:N])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        worker_id = __pgi_workeridx();
        #endif
        
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            int idx = (gang_id * 4 + worker_id) % N;
            if (i == idx) {
                data[i] += gang_id * 1000 + worker_id;
            }
        }
    }
    
    if (force_runtime) {
        debug_partition_info(3);  // Could trigger case 3
    }
    
    free(data);
}

// Test case 4: Vector partitioned
void test_vector_partitioned() {
    const int N = 1000;
    float *data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) data[i] = i * 1.0f;
    
    // Vector partitioning
    #pragma acc parallel vector_length(32) copy(data[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] *= 2.0f;
        }
    }
    
    if (force_runtime) {
        debug_partition_info(4);  // Could trigger case 4
    }
    
    free(data);
}

// Test case 5: Gang+vector partitioned
void test_gang_vector_partitioned() {
    const int N = 1000;
    float *data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) data[i] = i * 1.0f;
    
    // Gang and vector partitioning
    #pragma acc parallel num_gangs(4) vector_length(32) copy(data[0:N])
    {
        int gang_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        #endif
        
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            if (i % 4 == gang_id) {
                data[i] += gang_id * 100.0f;
            }
        }
    }
    
    if (force_runtime) {
        debug_partition_info(5);  // Could trigger case 5
    }
    
    free(data);
}

// Test case 6: Worker+vector partitioned
void test_worker_vector_partitioned() {
    const int N = 1000;
    float *data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) data[i] = i * 1.0f;
    
    // Worker and vector partitioning
    #pragma acc parallel num_workers(4) vector_length(32) copy(data[0:N])
    {
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = __pgi_workeridx();
        #endif
        
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            if (i % 4 == worker_id) {
                data[i] += worker_id * 50.0f;
            }
        }
    }
    
    if (force_runtime) {
        debug_partition_info(6);  // Could trigger case 6
    }
    
    free(data);
}

// Test case 7: Fully partitioned (gang+worker+vector)
void test_fully_partitioned() {
    const int N = 1024;
    float *data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) data[i] = i * 1.0f;
    
    // Full partitioning: gang, worker, and vector
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(32) copy(data[0:N])
    {
        int gang_id = 0, worker_id = 0;
        #ifdef _OPENACC
        gang_id = __pgi_gangidx();
        worker_id = __pgi_workeridx();
        #endif
        
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            int idx = (gang_id * 16 + worker_id * 4 + (i % 4)) % N;
            if (i == idx) {
                data[i] += gang_id * 1000.0f + worker_id * 100.0f + (i % 4);
            }
        }
    }
    
    if (force_runtime) {
        debug_partition_info(7);  // Could trigger case 7
    }
    
    free(data);
}

// Test OpenMP variants that might trigger similar partitioning logic
void test_omp_partitioning() {
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) data[i] = i;
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for \
            num_teams(4) thread_limit(64) map(tofrom: data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] *= 2;
    }
    
    // Nested parallelism with SIMD
    #pragma omp target teams distribute parallel for simd \
            num_teams(2) thread_limit(32) simdlen(8) \
            map(tofrom: data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] += i % 8;
    }
    
    free(data);
}

// Function that could generate invalid partition codes
void test_invalid_partition() {
    // Use volatile to prevent compile-time optimization
    volatile int invalid_code = 8;  // Outside 0-7 range
    
    // This might trigger the default case
    debug_partition_info(invalid_code);
    
    // Array access with variable index that could be out of bounds
    int codes[] = {0, 1, 2, 3, 4, 5, 6, 7};
    volatile int idx = 8;  // Invalid index
    
    // Prevent dead code elimination
    asm volatile("" : : "r"(idx));
    
    // This could generate invalid partition code access
    if (force_runtime && idx >= 0 && idx < 8) {
        debug_partition_info(codes[idx]);
    }
}

int main() {
    printf("Testing OpenACC/OpenMP partition coverage...\n");
    
    // Test all valid partition cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test OpenMP variants
    test_omp_partitioning();
    
    // Test invalid partition code (default case)
    test_invalid_partition();
    
    printf("All tests completed.\n");
    
    // Force runtime execution of all code paths
    if (force_runtime) {
        // Use results to prevent dead code elimination
        volatile int dummy = 0;
        asm volatile("" : : "r"(dummy));
    }
    
    return 0;
}
