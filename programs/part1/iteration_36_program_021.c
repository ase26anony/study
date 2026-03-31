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
volatile int partition_debug = 0;

// Function that could trigger partition string mapping in compiler diagnostics
void debug_partition_info(int code) {
    // This might trigger the compiler to generate the mapping logic
    if (partition_debug) {
        // Simulate using partition description (compiler might generate strings)
        switch(code) {
            case 0: case 1: case 2: case 3:
            case 4: case 5: case 6: case 7:
                printf("Partition code %d processed\n", code);
                break;
            default:
                // This should trigger the default case "<illegal>"
                printf("Invalid partition code %d\n", code);
        }
    }
}

// Test case 0: gang redundant (single gang)
void test_gang_redundant() {
    int data[ARRAY_SIZE];
    int i;
    
    // Initialize with runtime values
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * force_runtime;
    }
    
    // Single gang, redundant across gangs
    #pragma acc parallel num_gangs(1) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] += 1;
        }
    }
    
    // Verify
    int errors = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        if (data[i] != i * force_runtime + 1) errors++;
    }
    if (errors) printf("Case 0: %d errors\n", errors);
    
    debug_partition_info(0);
}

// Test case 1: gang partitioned (multiple gangs)
void test_gang_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }
    
    // Multiple gangs, partitioned across gangs
    #pragma acc parallel num_gangs(NUM_GANGS) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < ARRAY_SIZE; i++) {
            // Each gang processes different chunks
            data[i] *= 2;
        }
    }
    
    debug_partition_info(1);
}

// Test case 2: worker partitioned (single gang, multiple workers)
void test_worker_partitioned() {
    float data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i;
    }
    
    // Single gang with workers
    #pragma acc parallel num_gangs(1) num_workers(NUM_WORKERS) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] += 0.5f;
        }
    }
    
    debug_partition_info(2);
}

// Test case 3: gang+worker partitioned
void test_gang_worker_partitioned() {
    double data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i;
    }
    
    // Both gang and worker partitioning
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = data[i] * 1.5;
        }
    }
    
    debug_partition_info(3);
}

// Test case 4: vector partitioned
void test_vector_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
    }
    
    // Vector partitioning only
    #pragma acc parallel vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] += 3;
        }
    }
    
    debug_partition_info(4);
}

// Test case 5: gang+vector partitioned
void test_gang_vector_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }
    
    // Gang and vector partitioning
    #pragma acc parallel num_gangs(NUM_GANGS) vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = data[i] << 1;
        }
    }
    
    debug_partition_info(5);
}

// Test case 6: worker+vector partitioned
void test_worker_vector_partitioned() {
    float data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i;
    }
    
    // Worker and vector partitioning
    #pragma acc parallel num_workers(NUM_WORKERS) vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = data[i] / 2.0f;
        }
    }
    
    debug_partition_info(6);
}

// Test case 7: fully partitioned (gang+worker+vector)
void test_fully_partitioned() {
    double data[ARRAY_SIZE];
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i;
    }
    
    // All three levels of partitioning
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = sqrt(data[i] + 1.0);
        }
    }
    
    debug_partition_info(7);
}

// Test OpenMP equivalent partitioning
void test_omp_partitioning() {
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
        data[i] += omp_get_team_num() + omp_get_thread_num();
    }
    
    // Combined SIMD for vector partitioning
    #pragma omp target teams distribute parallel for simd \
        num_teams(NUM_GANGS) thread_limit(NUM_WORKERS) \
        map(tofrom: data[0:ARRAY_SIZE])
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] *= 2;
    }
}

// Test invalid partition codes (default case)
void test_invalid_partitions() {
    // Force generation of default case through boundary conditions
    int invalid_codes[] = {-1, 8, 100, -999};
    
    for (int j = 0; j < 4; j++) {
        debug_partition_info(invalid_codes[j]);
    }
    
    // Runtime-determined invalid code
    int dynamic_code = ARRAY_SIZE + 1;  // Definitely invalid
    debug_partition_info(dynamic_code);
}

// Complex nested partitioning to stress the compiler logic
void test_nested_partitioning() {
    int data[ARRAY_SIZE][ARRAY_SIZE/16];
    int i, j;
    
    // Initialize 2D array
    for (i = 0; i < ARRAY_SIZE; i++) {
        for (j = 0; j < ARRAY_SIZE/16; j++) {
            data[i][j] = i * j * force_runtime;
        }
    }
    
    // Complex nested loops with different partition clauses
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) \
                vector_length(VECTOR_LENGTH) copy(data[0:ARRAY_SIZE][0:ARRAY_SIZE/16])
    {
        // Outer gang-partitioned loop
        #pragma acc loop gang
        for (i = 0; i < ARRAY_SIZE; i++) {
            // Middle worker-partitioned loop
            #pragma acc loop worker
            for (j = 0; j < ARRAY_SIZE/16; j++) {
                // Inner vector-partitioned operation
                data[i][j] = data[i][j] % 17;
            }
        }
    }
    
    // Another variant with collapsed loops
    #pragma acc parallel num_gangs(NUM_GANGS) vector_length(VECTOR_LENGTH) \
                copy(data[0:ARRAY_SIZE][0:ARRAY_SIZE/16])
    {
        #pragma acc loop gang collapse(2)
        for (i = 0; i < ARRAY_SIZE; i++) {
            for (j = 0; j < ARRAY_SIZE/16; j++) {
                data[i][j] += i + j;
            }
        }
    }
}

int main() {
    printf("Testing all partition mapping cases...\n");
    
    // Enable debug output occasionally
    partition_debug = (force_runtime > 0);
    
    // Test all valid partition codes (0-7)
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
    
    // Test complex nested cases
    test_nested_partitioning();
    
    // Test invalid codes (should trigger default case)
    test_invalid_partitions();
    
    printf("All partition tests completed.\n");
    
    // Use asm to prevent dead code elimination of partition logic
    int dummy = force_runtime;
    asm volatile("" : : "r"(dummy));
    
    return 0;
}
