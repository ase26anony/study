#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

// Volatile variables to prevent optimization
volatile int force_runtime = 1;
volatile int partition_code = 0;

// Function that uses partition-dependent results
void use_result(int val, const char* expected) {
    // Prevent dead code elimination
    asm volatile("" : : "r"(val));
    
    // This could trigger the string mapping in debug builds
    if (val == -1) {
        // This branch might use the partition description
        fprintf(stderr, "Partition error: %s\n", expected);
    }
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Single gang - should map to case 0
    #pragma acc parallel num_gangs(1) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += 1;
        }
    }
    
    // Verify and use result
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum, "gang redundant");
    
    free(data);
}

// Test case 1: Gang partitioned (multiple gangs)
void test_gang_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Multiple gangs - should map to case 1
    int gangs = (n > 100) ? 4 : 2;
    #pragma acc parallel num_gangs(gangs) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] *= 2;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum, "gang partitioned");
    
    free(data);
}

// Test case 2: Worker partitioned (single gang, multiple workers)
void test_worker_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Single gang with workers - should map to case 2
    #pragma acc parallel num_gangs(1) num_workers(4) copy(data[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] += i % 10;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum, "worker partitioned");
    
    free(data);
}

// Test case 3: Gang+worker partitioned
void test_gang_worker_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Both gangs and workers - should map to case 3
    #pragma acc parallel num_gangs(2) num_workers(2) copy(data[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 3 + 1;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum, "gang+worker partitioned");
    
    free(data);
}

// Test case 4: Vector partitioned
void test_vector_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Vector partitioning - should map to case 4
    #pragma acc parallel vector_length(128) copy(data[0:n])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] += (i & 1);
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum, "vector partitioned");
    
    free(data);
}

// Test case 5: Gang+vector partitioned
void test_gang_vector_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Gang and vector - should map to case 5
    #pragma acc parallel num_gangs(4) vector_length(64) copy(data[0:n])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2 - 1;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum, "gang+vector partitioned");
    
    free(data);
}

// Test case 6: Worker+vector partitioned
void test_worker_vector_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Worker and vector - should map to case 6
    #pragma acc parallel num_workers(2) vector_length(32) copy(data[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] += (i % 3);
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum, "worker+vector partitioned");
    
    free(data);
}

// Test case 7: Fully partitioned (gang+worker+vector)
void test_fully_partitioned(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // All three levels - should map to case 7
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) copy(data[0:n])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            data[i] = (data[i] << 1) | 1;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum, "fully partitioned");
    
    free(data);
}

// OpenMP versions to trigger similar partitioning logic
void test_omp_partitioning(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // OpenMP target with teams and distribute - can trigger various partitions
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n]) \
        num_teams(4) thread_limit(32)
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() + omp_get_thread_num();
    }
    
    // SIMD vector partitioning
    #pragma omp target teams distribute parallel for simd map(tofrom: data[0:n]) \
        num_teams(2)
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
    }
    
    free(data);
}

// Function that could generate invalid partition codes
void test_invalid_partition(int code) {
    // This might trigger the default case if code > 7
    partition_code = code;
    
    // Simulate runtime partition selection
    int actual_partition = code % 8;
    
    // Force compiler to consider all possibilities
    switch (actual_partition) {
        case 0: test_gang_redundant(100); break;
        case 1: test_gang_partitioned(100); break;
        case 2: test_worker_partitioned(100); break;
        case 3: test_gang_worker_partitioned(100); break;
        case 4: test_vector_partitioned(100); break;
        case 5: test_gang_vector_partitioned(100); break;
        case 6: test_worker_vector_partitioned(100); break;
        case 7: test_fully_partitioned(100); break;
        default: 
            // This should trigger "<illegal>" string
            use_result(-1, "<illegal>");
            break;
    }
}

// Template function to force different partition expansions
template<int PartitionType>
void template_partition_test(int n) {
    int* data = new int[n];
    
    // Compile-time selection of partition strategy
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
    // ... etc for all cases
    
    delete[] data;
}

int main(int argc, char** argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    
    printf("Testing all partition types...\n");
    
    // Test all valid partition codes (0-7)
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
    
    // Test boundary/invalid cases
    for (int i = 8; i < 12; i++) {
        test_invalid_partition(i);
    }
    
    // Force template instantiations
    template_partition_test<0>(10);
    template_partition_test<1>(10);
    template_partition_test<2>(10);
    
    printf("All tests completed.\n");
    
    return 0;
}
