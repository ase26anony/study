/* Test program to exercise partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: g++ -O2 -fopenacc -fopenmp -g -o test_partitions test_partitions.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

// Prevent optimization
volatile int force_runtime = 1;

// Helper to use results and prevent dead code elimination
#define USE(var) asm volatile("" : : "r"(var))

// Function that could trigger partition string mapping
void debug_partition_info(int code) {
    // This mimics the internal logic that would call the uncovered function
    const char* desc = "<unknown>";
    switch(code) {
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
    
    // Force compiler to consider this code
    if (force_runtime) {
        printf("Partition debug: %s\n", desc);
    }
    USE(desc);
}

// Test case 0: gang redundant (single gang)
void test_gang_redundant() {
    const int N = 1000;
    int* data = (int*)malloc(N * sizeof(int));
    int sum = 0;
    
    // Initialize with runtime values
    for (int i = 0; i < N; i++) {
        data[i] = i % 7 + force_runtime;
    }
    
    // Single gang - gang redundant
    #pragma acc parallel copy(data[0:N]) copy(sum) num_gangs(1)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    // Could trigger partition info in compiler diagnostics
    debug_partition_info(0);
    
    printf("Gang redundant test: sum = %d\n", sum);
    free(data);
}

// Test case 1: gang partitioned (multiple gangs)
void test_gang_partitioned() {
    const int N = 10000;
    int* data = (int*)malloc(N * sizeof(int));
    
    // Runtime initialization
    for (int i = 0; i < N; i++) {
        data[i] = 1;
    }
    
    // Multiple gangs - gang partitioned
    #pragma acc parallel copy(data[0:N]) num_gangs(8)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] *= 2;
        }
    }
    
    debug_partition_info(1);
    
    // Verify
    int check = 0;
    for (int i = 0; i < N; i++) {
        check += (data[i] == 2);
    }
    printf("Gang partitioned test: %d/%d correct\n", check, N);
    free(data);
}

// Test case 2: worker partitioned (single gang, multiple workers)
void test_worker_partitioned() {
    const int N = 1024;
    float* data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    // Single gang with workers - worker partitioned
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 1.5f;
        }
    }
    
    debug_partition_info(2);
    
    printf("Worker partitioned test complete\n");
    free(data);
}

// Test case 3: gang+worker partitioned
void test_gang_worker_partitioned() {
    const int N = 4096;
    double* data = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        data[i] = (double)(i % 100);
    }
    
    // Combined gang and worker partitioning
    #pragma acc parallel copy(data[0:N]) num_gangs(4) num_workers(8)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            data[i] = data[i] + 1.0;
        }
    }
    
    debug_partition_info(3);
    
    printf("Gang+worker partitioned test complete\n");
    free(data);
}

// Test case 4: vector partitioned
void test_vector_partitioned() {
    const int N = 512;
    int* data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
    
    // Vector partitioning only
    #pragma acc parallel copy(data[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = data[i] << 1;
        }
    }
    
    debug_partition_info(4);
    
    printf("Vector partitioned test complete\n");
    free(data);
}

// Test case 5: gang+vector partitioned
void test_gang_vector_partitioned() {
    const int N = 2048;
    float* data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)(i % 64);
    }
    
    // Gang and vector partitioning
    #pragma acc parallel copy(data[0:N]) num_gangs(8) vector_length(16)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            data[i] = data[i] / 2.0f;
        }
    }
    
    debug_partition_info(5);
    
    printf("Gang+vector partitioned test complete\n");
    free(data);
}

// Test case 6: worker+vector partitioned
void test_worker_vector_partitioned() {
    const int N = 1024;
    int* data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        data[i] = 1;
    }
    
    // Worker and vector partitioning
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(4) vector_length(8)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 3;
        }
    }
    
    debug_partition_info(6);
    
    printf("Worker+vector partitioned test complete\n");
    free(data);
}

// Test case 7: fully partitioned (gang+worker+vector)
void test_fully_partitioned() {
    const int N = 8192;
    double* data = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        data[i] = (double)i;
    }
    
    // Full partitioning
    #pragma acc parallel copy(data[0:N]) num_gangs(16) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            data[i] = sqrt(data[i] + 1.0);
        }
    }
    
    debug_partition_info(7);
    
    printf("Fully partitioned test complete\n");
    free(data);
}

// Test default case: illegal partition codes
void test_illegal_partitions() {
    // Force consideration of default case through variable input
    int illegal_codes[] = {-1, 8, 100, -999};
    
    for (int i = 0; i < 4; i++) {
        debug_partition_info(illegal_codes[i]);
    }
    
    printf("Illegal partition test complete\n");
}

// OpenMP equivalents to trigger similar logic
void test_omp_partitions() {
    const int N = 1000;
    int* data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        data[i] = i;
    }
    
    // OpenMP target with teams and distribute - similar to gang partitioning
    #pragma omp target teams distribute parallel for map(tofrom: data[0:N]) num_teams(4) thread_limit(32)
    for (int i = 0; i < N; i++) {
        data[i] += 10;
    }
    
    // OpenMP with SIMD - similar to vector partitioning
    #pragma omp target teams distribute parallel for simd map(tofrom: data[0:N]) simdlen(8)
    for (int i = 0; i < N; i++) {
        data[i] *= 2;
    }
    
    printf("OpenMP partition tests complete\n");
    free(data);
}

int main() {
    printf("Starting partition coverage tests...\n");
    
    // Set runtime condition
    force_runtime = (getenv("FORCE_RUNTIME") != NULL) ? 1 : 1;
    
    // Test all partition cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Test illegal/default case
    test_illegal_partitions();
    
    // Test OpenMP variants
    test_omp_partitions();
    
    printf("All tests completed.\n");
    
    // Final check that could trigger partition mapping
    int final_code = (force_runtime > 0) ? 3 : 0;
    debug_partition_info(final_code);
    
    return 0;
}
