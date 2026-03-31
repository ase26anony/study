#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

// Prevent optimization
volatile int force_runtime = 1;

// Function to use results and prevent dead code elimination
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

// Function that could trigger partition string mapping
void debug_partition_info(int code) {
    // This mimics the internal compiler logic
    // The compiler might generate similar code for diagnostics
    if (code < 0 || code > 7) {
        // Could trigger default case
        fprintf(stderr, "Partition error: Invalid code %d\n", code);
    }
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant() {
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    
    // Initialize
    for (int i = 0; i < N; i++) data[i] = i;
    
    // Single gang, redundant across gangs
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data[i] += 1;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i + 1) errors++;
    }
    debug_partition_info(0);  // Could trigger case 0
    use_result(errors);
    
    free(data);
}

// Test case 1: Gang partitioned
void test_gang_partitioned() {
    const int N = 10000;
    int *data = (int*)malloc(N * sizeof(int));
    int gang_size = force_runtime ? 32 : 16;
    
    for (int i = 0; i < N; i++) data[i] = i;
    
    // Multiple gangs, partitioned by gang
    #pragma acc parallel copy(data[0:N]) num_gangs(gang_size) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            data[i] *= 2;
        }
    }
    
    debug_partition_info(1);  // Could trigger case 1
    use_result(data[N/2]);
    
    free(data);
}

// Test case 2: Worker partitioned
void test_worker_partitioned() {
    const int N = 1024;
    float *data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) data[i] = i * 1.0f;
    
    // Single gang, multiple workers
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker independent
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * data[i];
        }
    }
    
    debug_partition_info(2);  // Could trigger case 2
    use_result((int)data[N/4]);
    
    free(data);
}

// Test case 3: Gang+Worker partitioned
void test_gang_worker_partitioned() {
    const int N = 4096;
    double *data = (double*)malloc(N * sizeof(double));
    int workers = force_runtime ? 2 : 4;
    
    for (int i = 0; i < N; i++) data[i] = i * 0.5;
    
    // Multiple gangs and workers
    #pragma acc parallel copy(data[0:N]) num_gangs(8) num_workers(workers) vector_length(1)
    {
        #pragma acc loop gang worker independent
        for (int i = 0; i < N; i++) {
            data[i] += (i % 10) * 0.1;
        }
    }
    
    debug_partition_info(3);  // Could trigger case 3
    use_result((int)data[100]);
    
    free(data);
}

// Test case 4: Vector partitioned
void test_vector_partitioned() {
    const int N = 512;
    int *data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) data[i] = i;
    
    // Vector partitioning only
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(1) vector_length(64)
    {
        #pragma acc loop vector independent
        for (int i = 0; i < N; i++) {
            data[i] = data[i] << 1;
        }
    }
    
    debug_partition_info(4);  // Could trigger case 4
    use_result(data[256]);
    
    free(data);
}

// Test case 5: Gang+Vector partitioned
void test_gang_vector_partitioned() {
    const int N = 2048;
    float *data = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) data[i] = i * 0.25f;
    
    // Gang and vector partitioning
    #pragma acc parallel copy(data[0:N]) num_gangs(4) num_workers(1) vector_length(32)
    {
        #pragma acc loop gang vector independent
        for (int i = 0; i < N; i++) {
            data[i] = 1.0f / (data[i] + 1.0f);
        }
    }
    
    debug_partition_info(5);  // Could trigger case 5
    use_result((int)(data[512] * 1000));
    
    free(data);
}

// Test case 6: Worker+Vector partitioned
void test_worker_vector_partitioned() {
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) data[i] = i % 100;
    
    // Worker and vector partitioning
    #pragma acc parallel copy(data[0:N]) num_gangs(1) num_workers(2) vector_length(16)
    {
        #pragma acc loop worker vector independent
        for (int i = 0; i < N; i++) {
            data[i] = (data[i] * 3) % 97;
        }
    }
    
    debug_partition_info(6);  // Could trigger case 6
    use_result(data[333]);
    
    free(data);
}

// Test case 7: Fully partitioned
void test_fully_partitioned() {
    const int N = 8192;
    double *data = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) data[i] = i * 0.1;
    
    // All levels partitioned: gang, worker, and vector
    #pragma acc parallel copy(data[0:N]) num_gangs(16) num_workers(2) vector_length(64)
    {
        #pragma acc loop gang worker vector independent
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * data[i] + 1.0;
        }
    }
    
    debug_partition_info(7);  // Could trigger case 7
    use_result((int)data[4096]);
    
    free(data);
}

// Test OpenMP equivalents that might generate similar partition codes
void test_omp_partitioning() {
    const int N = 1000;
    int *data = (int*)malloc(N * sizeof(int));
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for map(tofrom: data[0:N]) \
        num_teams(4) thread_limit(64)
    for (int i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Nested parallelism that might trigger combined partitioning
    #pragma omp target teams distribute parallel for simd map(tofrom: data[0:N]) \
        num_teams(8) thread_limit(128)
    for (int i = 0; i < N; i++) {
        data[i] += (i % 3);
    }
    
    use_result(data[N-1]);
    free(data);
}

// Function to test invalid partition codes (default case)
void test_invalid_partitions() {
    // Force generation of invalid codes through boundary conditions
    int invalid_codes[] = {-1, 8, 100, -999};
    
    for (int i = 0; i < 4; i++) {
        debug_partition_info(invalid_codes[i]);
    }
    
    // Variable that could hold any partition code
    volatile int dynamic_code = 99;
    debug_partition_info(dynamic_code);
}

// Template function to generate different partition schemes
template<int PartitionType>
void templated_partition_test() {
    const int N = 512;
    int data[N];
    
    // Different partition strategies based on template parameter
    if constexpr (PartitionType == 0) {
        #pragma acc parallel copy(data[0:N]) num_gangs(1)
        for (int i = 0; i < N; i++) data[i] = i;
    } else if constexpr (PartitionType == 1) {
        #pragma acc parallel copy(data[0:N]) num_gangs(4)
        for (int i = 0; i < N; i++) data[i] = i * 2;
    }
    // ... more cases
    
    use_result(data[0]);
}

int main() {
    printf("Testing OpenACC/OpenMP partition coverage...\n");
    
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
    
    // Test invalid codes for default case
    test_invalid_partitions();
    
    // Instantiate template tests
    templated_partition_test<0>();
    templated_partition_test<1>();
    
    printf("All partition tests completed.\n");
    
    // Force runtime execution paths
    if (force_runtime) {
        // Additional runtime-determined partitioning
        int mode = rand() % 8;
        switch (mode) {
            case 0: test_gang_redundant(); break;
            case 1: test_gang_partitioned(); break;
            case 2: test_worker_partitioned(); break;
            case 3: test_gang_worker_partitioned(); break;
            case 4: test_vector_partitioned(); break;
            case 5: test_gang_vector_partitioned(); break;
            case 6: test_worker_vector_partitioned(); break;
            case 7: test_fully_partitioned(); break;
        }
    }
    
    return 0;
}
