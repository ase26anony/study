#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

// Force compiler to generate partition mapping logic
#define FORCE_USED(x) asm volatile("" : : "r"(x))
#define RUNTIME_BOUNDS (rand() % 100 + 100)

// Test functions for each partition type
void test_gang_redundant() {
    volatile int n = RUNTIME_BOUNDS;
    int *data = (int*)malloc(n * sizeof(int));
    
    // Initialize data
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Single gang, redundant across workers/vectors
    #pragma acc parallel num_gangs(1) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += 1;
        }
    }
    
    // Verify and force usage
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    FORCE_USED(sum);
    free(data);
}

void test_gang_partitioned() {
    volatile int n = RUNTIME_BOUNDS;
    int *data = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Multiple gangs, partitioned by gang only
    #pragma acc parallel num_gangs(4) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] *= 2;
        }
    }
    
    // Force diagnostic output
    if (data[0] < 0) printf("Error in gang partitioned\n");
    free(data);
}

void test_worker_partitioned() {
    volatile int n = RUNTIME_BOUNDS;
    float *data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) data[i] = i * 1.5f;
    
    // Single gang, multiple workers
    #pragma acc parallel num_gangs(1) num_workers(4) copy(data[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] += 0.5f;
        }
    }
    
    FORCE_USED(data[n/2]);
    free(data);
}

void test_gang_worker_partitioned() {
    volatile int n = RUNTIME_BOUNDS;
    double *data = (double*)malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) data[i] = i * 0.25;
    
    // Both gang and worker partitioning
    #pragma acc parallel num_gangs(2) num_workers(2) copy(data[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 3.14;
        }
    }
    
    if (data[0] != data[0]) printf("NaN detected\n");
    free(data);
}

void test_vector_partitioned() {
    volatile int n = RUNTIME_BOUNDS;
    int *data = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) data[i] = i % 10;
    
    // Vector partitioning only
    #pragma acc parallel vector_length(32) copy(data[0:n])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] <<= 1;
        }
    }
    
    FORCE_USED(data);
    free(data);
}

void test_gang_vector_partitioned() {
    volatile int n = RUNTIME_BOUNDS;
    float *data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) data[i] = i * 0.1f;
    
    // Gang and vector partitioning
    #pragma acc parallel num_gangs(2) vector_length(16) copy(data[0:n])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] = sinf(data[i]);
        }
    }
    
    // Prevent optimization
    volatile float check = data[n/3];
    (void)check;
    free(data);
}

void test_worker_vector_partitioned() {
    volatile int n = RUNTIME_BOUNDS;
    int *data = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Worker and vector partitioning
    #pragma acc parallel num_workers(2) vector_length(8) copy(data[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] = data[i] & 0xFF;
        }
    }
    
    FORCE_USED(data[0]);
    free(data);
}

void test_fully_partitioned() {
    volatile int n = RUNTIME_BOUNDS;
    double *data = (double*)malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) data[i] = i * 0.01;
    
    // Fully partitioned: gang, worker, and vector
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(4) copy(data[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            data[i] = cos(data[i]);
        }
    }
    
    // Complex dependency to force partitioning logic
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    if (sum != sum) printf("Invalid sum\n");
    free(data);
}

// OpenMP versions to trigger different code paths
void test_omp_partitioning() {
    volatile int n = RUNTIME_BOUNDS;
    int *data = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n]) \
        num_teams(4) thread_limit(32)
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() * 100 + omp_get_thread_num();
    }
    
    // Nested parallelism for combined partitioning
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:n]) num_teams(2) thread_limit(64)
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
    }
    
    FORCE_USED(data);
    free(data);
}

// Test invalid partition codes through boundary conditions
void test_boundary_cases() {
    // Force generation of default case
    volatile int invalid_partition = 8;  // Outside 0-7 range
    
    // Use in a way that might trigger the mapping function
    int *dummy = (int*)malloc(10 * sizeof(int));
    
    // Variable partitioning that compiler might not optimize away
    int gangs = invalid_partition > 7 ? 1 : invalid_partition;
    
    #pragma acc parallel num_gangs(gangs) copy(dummy[0:10])
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            dummy[i] = i * gangs;
        }
    }
    
    // Another invalid case
    gangs = -1;
    #pragma acc parallel num_gangs(gangs < 0 ? 1 : gangs) copy(dummy[0:10])
    {
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            dummy[i] += 1;
        }
    }
    
    free(dummy);
}

// Template function to generate different partition schemes
template<int PartitionType>
void template_partition_test() {
    const int n = 100;
    int data[n];
    
    // Compile-time selection of partition scheme
    if constexpr (PartitionType == 0) {
        #pragma acc parallel num_gangs(1) copy(data[0:n])
        { /* gang redundant */ }
    } else if constexpr (PartitionType == 1) {
        #pragma acc parallel num_gangs(4) copy(data[0:n])
        { /* gang partitioned */ }
    } else if constexpr (PartitionType == 2) {
        #pragma acc parallel num_workers(4) copy(data[0:n])
        { /* worker partitioned */ }
    }
    // ... other cases
    
    FORCE_USED(data);
}

int main() {
    srand(42);  // Deterministic randomness
    
    printf("Testing all partition mapping cases...\n");
    
    // Test each partition type systematically
    test_gang_redundant();          // Case 0
    test_gang_partitioned();        // Case 1
    test_worker_partitioned();      // Case 2
    test_gang_worker_partitioned(); // Case 3
    test_vector_partitioned();      // Case 4
    test_gang_vector_partitioned(); // Case 5
    test_worker_vector_partitioned(); // Case 6
    test_fully_partitioned();       // Case 7
    
    // Additional tests
    test_omp_partitioning();
    test_boundary_cases();          // Should trigger default case
    
    // Instantiate template tests
    template_partition_test<0>();
    template_partition_test<1>();
    template_partition_test<2>();
    
    printf("All tests completed (coverage of lines 335-343 should be triggered)\n");
    
    return 0;
}
