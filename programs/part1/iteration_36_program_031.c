#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

// Volatile variables to prevent optimization
volatile int force_runtime = 1;
volatile int debug_output = 0;

// Function to use results and prevent dead code elimination
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

// Function that could trigger partition string mapping
void log_partition_info(int partition_code) {
    // This mimics the internal compiler logic that maps codes to strings
    if (debug_output) {
        // In real compiler code, this would call the function with switch statement
        printf("Partition code: %d\n", partition_code);
    }
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Single gang - gang redundant
    #pragma acc parallel num_gangs(1) copy(data[0:n])
    {
        int gang_id = acc_gang_id();
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += gang_id;  // All gangs see same data
        }
    }
    
    // Force runtime evaluation
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// Test case 1: Gang partitioned
void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int gangs = force_runtime ? 4 : 2;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Multiple gangs - gang partitioned
    #pragma acc parallel num_gangs(gangs) copy(data[0:n])
    {
        int gang_id = acc_gang_id();
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            if (i % gangs == gang_id) {
                data[i] += gang_id * 1000;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// Test case 2: Worker partitioned
void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int workers = force_runtime ? 8 : 4;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Single gang with multiple workers
    #pragma acc parallel num_gangs(1) num_workers(workers) copy(data[0:n])
    {
        int worker_id = acc_worker_id();
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            if (i % workers == worker_id) {
                data[i] += worker_id * 100;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// Test case 3: Gang+worker partitioned
void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int gangs = force_runtime ? 2 : 1;
    int workers = force_runtime ? 4 : 2;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Combined gang and worker partitioning
    #pragma acc parallel num_gangs(gangs) num_workers(workers) copy(data[0:n])
    {
        int gang_id = acc_gang_id();
        int worker_id = acc_worker_id();
        int combined_id = gang_id * workers + worker_id;
        
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            if (i % (gangs * workers) == combined_id) {
                data[i] += combined_id * 10;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// Test case 4: Vector partitioned
void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int vector_len = force_runtime ? 32 : 16;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Vector partitioning only
    #pragma acc parallel vector_length(vector_len) copy(data[0:n])
    {
        int vector_id = acc_vector_id();
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            if (i % vector_len == vector_id) {
                data[i] += vector_id;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// Test case 5: Gang+vector partitioned
void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int gangs = force_runtime ? 2 : 1;
    int vector_len = force_runtime ? 16 : 8;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Gang and vector partitioning
    #pragma acc parallel num_gangs(gangs) vector_length(vector_len) copy(data[0:n])
    {
        int gang_id = acc_gang_id();
        int vector_id = acc_vector_id();
        int combined_id = gang_id * vector_len + vector_id;
        
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            if (i % (gangs * vector_len) == combined_id) {
                data[i] += combined_id * 5;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// Test case 6: Worker+vector partitioned
void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int workers = force_runtime ? 4 : 2;
    int vector_len = force_runtime ? 8 : 4;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Worker and vector partitioning
    #pragma acc parallel num_workers(workers) vector_length(vector_len) copy(data[0:n])
    {
        int worker_id = acc_worker_id();
        int vector_id = acc_vector_id();
        int combined_id = worker_id * vector_len + vector_id;
        
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            if (i % (workers * vector_len) == combined_id) {
                data[i] += combined_id * 3;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// Test case 7: Fully partitioned (gang+worker+vector)
void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    int gangs = force_runtime ? 2 : 1;
    int workers = force_runtime ? 2 : 1;
    int vector_len = force_runtime ? 4 : 2;
    
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Full three-level partitioning
    #pragma acc parallel num_gangs(gangs) num_workers(workers) vector_length(vector_len) copy(data[0:n])
    {
        int gang_id = acc_gang_id();
        int worker_id = acc_worker_id();
        int vector_id = acc_vector_id();
        int combined_id = (gang_id * workers + worker_id) * vector_len + vector_id;
        
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            if (i % (gangs * workers * vector_len) == combined_id) {
                data[i] += combined_id;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// OpenMP equivalents to trigger similar partitioning logic
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    
    // OpenMP target offload with teams and distribute
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    // Nested parallelism with SIMD
    #pragma omp target teams distribute parallel for simd map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += i;
    }
    
    int sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    use_result(sum);
    
    free(data);
}

// Function to test invalid partition codes (default case)
void test_invalid_partition(int n) {
    // This might trigger the default case through boundary conditions
    int *data = (int*)malloc(n * sizeof(int));
    
    // Use runtime values that could lead to invalid partition codes
    int dynamic_gangs = force_runtime ? -1 : 1;
    int dynamic_workers = force_runtime ? 0 : 2;
    
    // Conditional compilation to avoid actual errors
    #ifdef TEST_BOUNDARY_CASES
    #pragma acc parallel num_gangs(dynamic_gangs) num_workers(dynamic_workers) copy(data[0:n])
    {
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            data[i] = i;
        }
    }
    #endif
    
    free(data);
}

int main() {
    int n = 1024;
    
    printf("Testing all partition types...\n");
    
    // Test all valid partition cases (0-7)
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
    test_invalid_partition(n);
    
    // Simulate calling the partition mapping function
    for (int code = 0; code <= 8; code++) {
        log_partition_info(code);
    }
    
    printf("All tests completed.\n");
    
    return 0;
}
