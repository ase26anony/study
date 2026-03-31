/* Test program to cover partition mapping logic in omp-oacc-neuter-broadcast.cc
 * Compile with: g++ -O2 -fopenacc -fopenmp -o partition_test partition_test.cc
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

// Prevent optimization
volatile int force_runtime = 0;

// Helper to use results
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

// Test case 0: gang redundant (single gang)
void test_gang_redundant() {
    int data[ARRAY_SIZE];
    int i;
    
    // Initialize with runtime value
    int size = ARRAY_SIZE;
    if (force_runtime) size = 512;
    
    #pragma acc parallel loop gang(num:1) copy(data[0:size])
    for (i = 0; i < size; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    int errors = 0;
    for (i = 0; i < size; i++) {
        if (data[i] != i * 2) errors++;
    }
    
    // Could trigger partition string mapping in diagnostics
    if (errors > 0) {
        fprintf(stderr, "Partition type: gang redundant - Errors: %d\n", errors);
    }
    use_result(errors);
}

// Test case 1: gang partitioned (multiple gangs)
void test_gang_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    int size = ARRAY_SIZE;
    if (force_runtime) size = 768;
    
    #pragma acc parallel loop gang(num:NUM_GANGS) copy(data[0:size])
    for (i = 0; i < size; i++) {
        data[i] = i + 1;
    }
    
    int sum = 0;
    #pragma acc parallel loop gang(num:NUM_GANGS) reduction(+:sum) copy(data[0:size])
    for (i = 0; i < size; i++) {
        sum += data[i];
    }
    
    use_result(sum);
}

// Test case 2: worker partitioned (single gang, multiple workers)
void test_worker_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    int size = ARRAY_SIZE;
    if (force_runtime) size = 256;
    
    #pragma acc parallel loop gang(num:1) worker(num:NUM_WORKERS) copy(data[0:size])
    for (i = 0; i < size; i++) {
        data[i] = i * 3;
    }
    
    // Nested worker parallelism
    #pragma acc parallel gang(num:1) worker(num:NUM_WORKERS) copy(data[0:size])
    {
        #pragma acc loop worker
        for (i = 0; i < size; i++) {
            data[i] += 1;
        }
    }
    
    use_result(data[size-1]);
}

// Test case 3: gang+worker partitioned
void test_gang_worker_partitioned() {
    int data[ARRAY_SIZE * 2];
    int i;
    
    int size = ARRAY_SIZE;
    if (force_runtime) size = 384;
    
    #pragma acc parallel loop gang(num:NUM_GANGS) worker(num:NUM_WORKERS) copy(data[0:size*2])
    for (i = 0; i < size * 2; i++) {
        data[i] = i % 17;
    }
    
    // Complex dependency requiring both gang and worker partitioning
    #pragma acc parallel gang(num:NUM_GANGS) worker(num:NUM_WORKERS) copy(data[0:size*2])
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        #pragma acc loop gang worker
        for (i = 0; i < size; i++) {
            int idx = i + gang_id * worker_id;
            if (idx < size * 2) {
                data[idx] = gang_id * 1000 + worker_id * 10 + i;
            }
        }
    }
    
    use_result(data[0]);
}

// Test case 4: vector partitioned
void test_vector_partitioned() {
    float data[ARRAY_SIZE];
    int i;
    
    int size = ARRAY_SIZE;
    if (force_runtime) size = 128;
    
    #pragma acc parallel loop vector(length:VECTOR_LENGTH) copy(data[0:size])
    for (i = 0; i < size; i++) {
        data[i] = i * 1.5f;
    }
    
    // Vector operations
    #pragma acc parallel loop vector(length:VECTOR_LENGTH) copy(data[0:size])
    for (i = 0; i < size; i++) {
        data[i] = data[i] * 2.0f;
    }
    
    use_result((int)data[size-1]);
}

// Test case 5: gang+vector partitioned
void test_gang_vector_partitioned() {
    double data[ARRAY_SIZE];
    int i;
    
    int size = ARRAY_SIZE;
    if (force_runtime) size = 512;
    
    #pragma acc parallel loop gang(num:NUM_GANGS) vector(length:VECTOR_LENGTH) copy(data[0:size])
    for (i = 0; i < size; i++) {
        data[i] = (double)i / (i + 1.0);
    }
    
    // Combined gang and vector operations
    #pragma acc parallel gang(num:NUM_GANGS) vector(length:VECTOR_LENGTH) copy(data[0:size])
    {
        #pragma acc loop gang vector
        for (i = 0; i < size; i++) {
            data[i] = data[i] * __pgi_gangidx() + __pgi_vectoridx();
        }
    }
    
    use_result((int)data[0]);
}

// Test case 6: worker+vector partitioned
void test_worker_vector_partitioned() {
    int data[ARRAY_SIZE];
    int i;
    
    int size = ARRAY_SIZE;
    if (force_runtime) size = 256;
    
    #pragma acc parallel loop worker(num:NUM_WORKERS) vector(length:VECTOR_LENGTH) copy(data[0:size])
    for (i = 0; i < size; i++) {
        data[i] = i * i;
    }
    
    // Worker-vector nested computation
    #pragma acc parallel worker(num:NUM_WORKERS) vector(length:VECTOR_LENGTH) copy(data[0:size])
    {
        #pragma acc loop worker vector
        for (i = 0; i < size; i++) {
            data[i] = data[i] % 97;
        }
    }
    
    use_result(data[size/2]);
}

// Test case 7: fully partitioned (gang+worker+vector)
void test_fully_partitioned() {
    int data[ARRAY_SIZE * 4];
    int i, j;
    
    int size = ARRAY_SIZE;
    if (force_runtime) size = 192;
    
    // Fully nested parallelism
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) vector_length(VECTOR_LENGTH) copy(data[0:size*4])
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        #pragma acc loop gang worker vector
        for (i = 0; i < size; i++) {
            int idx = i + gang_id * 100 + worker_id * 10 + vector_id;
            if (idx < size * 4) {
                data[idx] = gang_id * 10000 + worker_id * 100 + vector_id * 1 + i;
            }
        }
    }
    
    // Reduction with full partitioning
    int total = 0;
    #pragma acc parallel loop gang(num:NUM_GANGS) worker(num:NUM_WORKERS) vector(length:VECTOR_LENGTH) reduction(+:total) copy(data[0:size*4])
    for (i = 0; i < size * 4; i++) {
        total += data[i] % 13;
    }
    
    use_result(total);
}

// Test default case (invalid partition codes)
void test_invalid_partition() {
    // This might trigger the default case through internal compiler paths
    // We'll use OpenMP with unusual configurations
    
    int data[100];
    int i;
    
    // OpenMP version that might map to unusual partition codes
    #pragma omp target teams distribute parallel for map(tofrom: data[0:100])
    for (i = 0; i < 100; i++) {
        data[i] = omp_get_team_num() * 1000 + omp_get_thread_num();
    }
    
    // Try to create ambiguous partitioning
    int chunk = 10;
    #pragma omp target teams distribute parallel for num_teams(0) thread_limit(0) map(tofrom: data[0:100])
    for (i = 0; i < 100; i++) {
        data[i] += i;
    }
    
    use_result(data[99]);
}

// OpenMP equivalents to cover additional paths
void test_omp_partitioning() {
    int data[ARRAY_SIZE];
    int i;
    
    // OpenMP target with teams - similar to gang partitioning
    #pragma omp target teams distribute parallel for num_teams(NUM_GANGS) map(tofrom: data[0:ARRAY_SIZE])
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 2;
    }
    
    // Nested parallelism with simd (vector)
    #pragma omp target teams distribute simd num_teams(NUM_GANGS) map(tofrom: data[0:ARRAY_SIZE])
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] += 1;
    }
    
    use_result(data[0]);
}

int main(int argc, char **argv) {
    // Force runtime decisions
    if (argc > 1) {
        force_runtime = atoi(argv[1]);
    }
    
    printf("Testing partition mapping coverage...\n");
    
    // Execute all test cases
    test_gang_redundant();
    printf("  Case 0 tested\n");
    
    test_gang_partitioned();
    printf("  Case 1 tested\n");
    
    test_worker_partitioned();
    printf("  Case 2 tested\n");
    
    test_gang_worker_partitioned();
    printf("  Case 3 tested\n");
    
    test_vector_partitioned();
    printf("  Case 4 tested\n");
    
    test_gang_vector_partitioned();
    printf("  Case 5 tested\n");
    
    test_worker_vector_partitioned();
    printf("  Case 6 tested\n");
    
    test_fully_partitioned();
    printf("  Case 7 tested\n");
    
    test_omp_partitioning();
    printf("  OpenMP partitioning tested\n");
    
    test_invalid_partition();
    printf("  Invalid partition case tested\n");
    
    printf("All partition tests completed.\n");
    
    return 0;
}
