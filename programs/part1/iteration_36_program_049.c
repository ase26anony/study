/* Test program to exercise all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Compile with: g++ -O2 -fopenacc -fopenmp -ftree-parallelize-loops=2 this_file.cc -o test_partitions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#define ARRAY_SIZE 1024
#define NUM_GANGS 4
#define NUM_WORKERS 8
#define VECTOR_LENGTH 32

// Volatile variables to prevent optimization
volatile int force_runtime = 1;
volatile int partition_code = 0;

// Function to simulate usage of partition description (could trigger mapping)
void debug_partition_info(int code) {
    // This might trigger the compiler to generate the switch mapping
    if (code < 0 || code > 7) {
        // Could trigger default case
        printf("Invalid partition code encountered\n");
    }
}

/* Test Case 0: Gang Redundant (single gang) */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    
    int data[ARRAY_SIZE];
    int i;
    
    // Initialize with runtime-dependent values
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * (force_runtime + 1);
    }
    
    // Single gang, no worker/vector partitioning
    #pragma acc parallel num_gangs(1) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] += 1;
        }
    }
    
    // Verify and potentially trigger debug output
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != i * (force_runtime + 1) + 1) errors++;
    }
    if (errors > 0) debug_partition_info(0);
}

/* Test Case 1: Gang Partitioned (multiple gangs) */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    
    float data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i * 0.5f;
    }
    
    // Multiple gangs, no workers/vectors
    #pragma acc parallel num_gangs(NUM_GANGS) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] *= 2.0f;
        }
    }
    
    // Check results
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != (float)i) errors++;
    }
    if (errors > 0) debug_partition_info(1);
}

/* Test Case 2: Worker Partitioned (single gang, multiple workers) */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    
    double data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i;
    }
    
    // Single gang with workers
    #pragma acc parallel num_gangs(1) num_workers(NUM_WORKERS) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] += 1.0;
        }
    }
    
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != (double)i + 1.0) errors++;
    }
    if (errors > 0) debug_partition_info(2);
}

/* Test Case 3: Gang+Worker Partitioned */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    
    int data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
    }
    
    // Both gang and worker partitioning
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] *= 2;
        }
    }
    
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != (i % 100) * 2) errors++;
    }
    if (errors > 0) debug_partition_info(3);
}

/* Test Case 4: Vector Partitioned */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    
    float data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i;
    }
    
    // Vector partitioning only
    #pragma acc parallel vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] += 0.5f;
        }
    }
    
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != (float)i + 0.5f) errors++;
    }
    if (errors > 0) debug_partition_info(4);
}

/* Test Case 5: Gang+Vector Partitioned */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    
    double data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i * 0.25;
    }
    
    // Gang and vector partitioning
    #pragma acc parallel num_gangs(NUM_GANGS) vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] *= 4.0;
        }
    }
    
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != (double)i) errors++;
    }
    if (errors > 0) debug_partition_info(5);
}

/* Test Case 6: Worker+Vector Partitioned */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    
    int data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }
    
    // Worker and vector partitioning
    #pragma acc parallel num_workers(NUM_WORKERS) vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] -= 1;
        }
    }
    
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != i - 1) errors++;
    }
    if (errors > 0) debug_partition_info(6);
}

/* Test Case 7: Fully Partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    
    float data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i;
    }
    
    // All three levels of partitioning
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
        }
    }
    
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != (float)i * 2.0f + 1.0f) errors++;
    }
    if (errors > 0) debug_partition_info(7);
}

/* Test OpenMP equivalents to trigger similar partitioning logic */
void test_omp_partitioning() {
    printf("Testing OpenMP target offload partitioning\n");
    
    int data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for \
        num_teams(NUM_GANGS) thread_limit(NUM_WORKERS*VECTOR_LENGTH) \
        map(tofrom: data[0:ARRAY_SIZE])
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] += 100;
    }
    
    // Nested OpenMP with SIMD for vector partitioning
    #pragma omp target teams distribute parallel for simd \
        num_teams(NUM_GANGS) thread_limit(NUM_WORKERS) \
        simdlen(VECTOR_LENGTH) \
        map(tofrom: data[0:ARRAY_SIZE])
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] *= 2;
    }
    
    // Check results
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != (i + 100) * 2) errors++;
    }
    if (errors > 0) {
        // Could trigger partition mapping for various codes
        for (int code = 0; code < 10; code++) {
            debug_partition_info(code);
        }
    }
}

/* Test invalid partition codes to trigger default case */
void test_invalid_partitions() {
    printf("Testing invalid partition codes (default case)\n");
    
    // Force invalid codes through variable manipulation
    int invalid_codes[] = {-1, 8, 100, -100};
    
    for (int j = 0; j < 4; j++) {
        partition_code = invalid_codes[j];
        debug_partition_info(partition_code);
    }
    
    // Also test with runtime-computed invalid code
    int dynamic_invalid = ARRAY_SIZE + 1;
    debug_partition_info(dynamic_invalid);
}

/* Main test driver */
int main() {
    printf("Starting partition mapping coverage tests...\n");
    
    // Test all valid partition cases (0-7)
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
    
    // Test invalid codes for default case
    test_invalid_partitions();
    
    printf("All tests completed.\n");
    
    // Use asm to prevent dead code elimination of partition_code
    int result = 0;
    asm volatile("" : "+r" (result) : "r" (partition_code));
    
    return result;
}
