/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 64
#define P 32

void test_gang_redundant() {
    printf("Test 1: Gang redundant partitioning\n");
    int data[N];
    int result = 0;
    
    #pragma acc parallel loop gang copy(data[0:N]) copyout(result)
    for (int i = 0; i < N; i++) {
        data[i] = i;
        if (i == 0) {
            result = 1;
        }
    }
    
    printf("  Gang redundant test completed: result = %d\n", result);
}

void test_gang_partitioned() {
    printf("Test 2: Gang partitioned\n");
    int data[N];
    
    #pragma acc parallel loop gang copy(data[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) errors++;
    }
    printf("  Gang partitioned test completed, errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Test 3: Worker partitioned\n");
    int data[N];
    
    #pragma acc parallel loop worker copy(data[0:N]) num_workers(4)
    for (int i = 0; i < N; i++) {
        data[i] = i + 1;
    }
    
    printf("  Worker partitioned test completed\n");
}

void test_gang_worker_partitioned() {
    printf("Test 4: Gang+worker partitioned\n");
    int data[N][M];
    
    #pragma acc parallel loop gang worker collapse(2) copy(data[0:N][0:M]) \
        num_gangs(2) num_workers(4)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i * M + j;
        }
    }
    
    printf("  Gang+worker partitioned test completed\n");
}

void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    float data[N];
    
    #pragma acc parallel loop vector copy(data[0:N]) vector_length(32)
    for (int i = 0; i < N; i++) {
        data[i] = i * 1.5f;
    }
    
    printf("  Vector partitioned test completed\n");
}

void test_gang_vector_partitioned() {
    printf("Test 6: Gang+vector partitioned\n");
    double data[N];
    double sum = 0.0;
    
    #pragma acc parallel loop gang vector reduction(+:sum) copy(data[0:N]) \
        num_gangs(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        data[i] = i * 0.5;
        sum += data[i];
    }
    
    printf("  Gang+vector partitioned test completed, sum = %f\n", sum);
}

void test_worker_vector_partitioned() {
    printf("Test 7: Worker+vector partitioned\n");
    int data[N];
    
    #pragma acc parallel loop worker vector copy(data[0:N]) \
        num_workers(2) vector_length(8)
    for (int i = 0; i < N; i++) {
        data[i] = i % 10;
    }
    
    printf("  Worker+vector partitioned test completed\n");
}

void test_fully_partitioned() {
    printf("Test 8: Fully partitioned\n");
    int data[N][M][P];
    int total = 0;
    
    #pragma acc parallel loop gang worker vector collapse(3) \
        reduction(+:total) copy(data[0:N][0:M][0:P]) \
        num_gangs(2) num_workers(2) vector_length(4)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                data[i][j][k] = i + j + k;
                total += data[i][j][k];
            }
        }
    }
    
    printf("  Fully partitioned test completed, total = %d\n", total);
}

void test_nested_parallelism() {
    printf("Test 9: Nested parallelism\n");
    int outer[N];
    int inner[N];
    
    #pragma acc parallel copy(outer[0:N], inner[0:N]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            outer[i] = i * 2;
        }
        
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            inner[i] = outer[i] + 1;
        }
    }
    
    printf("  Nested parallelism test completed\n");
}

void test_combined_directives() {
    printf("Test 10: Combined directives with tile\n");
    int data[N][M];
    
    #pragma acc parallel loop tile(32, 16) copy(data[0:N][0:M]) \
        num_gangs(4) num_workers(2) vector_length(8)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (i * M + j) % 256;
        }
    }
    
    printf("  Combined directives test completed\n");
}

void test_runtime_dependent_partitioning() {
    printf("Test 11: Runtime-dependent partitioning\n");
    int data[N];
    int chunk_size = 128;
    
    // Runtime-dependent loop bounds
    #pragma acc parallel loop copy(data[0:N]) gang
    for (int i = 0; i < N; i += chunk_size) {
        int end = i + chunk_size;
        if (end > N) end = N;
        
        #pragma acc loop worker vector
        for (int j = i; j < end; j++) {
            data[j] = j * 3;
        }
    }
    
    printf("  Runtime-dependent partitioning test completed\n");
}

void test_multi_device() {
    printf("Test 12: Multi-device test\n");
    
    // Try different device types if available
    acc_device_t dev_type = acc_get_device_type();
    printf("  Current device type: %d\n", dev_type);
    
    int num_devices = acc_get_num_devices(dev_type);
    printf("  Number of devices: %d\n", num_devices);
    
    if (num_devices > 0) {
        int data[N];
        
        #pragma acc parallel loop copy(data[0:N]) gang worker
        for (int i = 0; i < N; i++) {
            data[i] = i * 4;
        }
        
        printf("  Multi-device test completed on device 0\n");
    }
}

void test_conditional_clauses() {
    printf("Test 13: Conditional clauses\n");
    int data[N];
    int use_vector = 1;
    
    #pragma acc parallel loop copy(data[0:N]) \
        gang worker vector if(use_vector)
    for (int i = 0; i < N; i++) {
        data[i] = i * 5;
    }
    
    printf("  Conditional clauses test completed\n");
}

void test_async_operations() {
    printf("Test 14: Async operations\n");
    int data1[N], data2[N];
    int async_id = 1;
    
    #pragma acc parallel loop copy(data1[0:N]) async(async_id) gang
    for (int i = 0; i < N; i++) {
        data1[i] = i * 6;
    }
    
    #pragma acc parallel loop copy(data2[0:N]) async(async_id + 1) worker
    for (int i = 0; i < N; i++) {
        data2[i] = data1[i] + 1;
    }
    
    #pragma acc wait
    
    printf("  Async operations test completed\n");
}

void test_complex_data_structures() {
    printf("Test 15: Complex data structures\n");
    
    typedef struct {
        int values[M];
        float weights[M];
    } element_t;
    
    element_t elements[N];
    
    #pragma acc parallel loop copy(elements[0:N]) gang worker
    for (int i = 0; i < N; i++) {
        #pragma acc loop vector
        for (int j = 0; j < M; j++) {
            elements[i].values[j] = i * M + j;
            elements[i].weights[j] = (i * M + j) * 0.1f;
        }
    }
    
    printf("  Complex data structures test completed\n");
}

void test_firstprivate_reduction() {
    printf("Test 16: Firstprivate with reduction\n");
    int base = 100;
    int sum = 0;
    
    #pragma acc parallel loop gang worker vector \
        firstprivate(base) reduction(+:sum) copyout(sum)
    for (int i = 0; i < N; i++) {
        sum += base + i;
    }
    
    printf("  Firstprivate+reduction test completed, sum = %d\n", sum);
}

void test_invalid_conditions() {
    printf("Test 17: Attempt to trigger edge cases\n");
    
    // This might trigger internal error paths
    int *invalid_ptr = NULL;
    
    #pragma acc parallel present_or_copyin(invalid_ptr[0:1]) if(0)
    {
        // Empty - should not execute
    }
    
    printf("  Edge case test completed (should not crash)\n");
}

int main() {
    printf("Starting OpenACC partition coverage tests...\n\n");
    
    // Enable debug output to increase chance of calling the mapping function
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    // Test all partition types
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests for varied execution paths
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_dependent_partitioning();
    test_multi_device();
    test_conditional_clauses();
    test_async_operations();
    test_complex_data_structures();
    test_firstprivate_reduction();
    test_invalid_conditions();
    
    printf("\nAll tests completed successfully!\n");
    
    // Final validation
    int final_data[10];
    #pragma acc parallel loop copy(final_data[0:10]) gang worker vector
    for (int i = 0; i < 10; i++) {
        final_data[i] = i * 10;
    }
    
    int success = 1;
    for (int i = 0; i < 10; i++) {
        if (final_data[i] != i * 10) {
            success = 0;
            break;
        }
    }
    
    if (success) {
        printf("Final validation passed.\n");
        return 0;
    } else {
        printf("Final validation failed.\n");
        return 1;
    }
}
