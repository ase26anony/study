/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 512
#define P 256

void test_gang_redundant() {
    printf("Test 1: Gang redundant partitioning\n");
    int data[N];
    int result = 0;
    
    #pragma acc parallel loop gang copyin(data[0:N]) copyout(result) num_gangs(4)
    for (int i = 0; i < N; i++) {
        data[i] = i;
        #pragma acc atomic
        result += data[i];
    }
    
    printf("  Result: %d (expected: %d)\n", result, N*(N-1)/2);
}

void test_gang_partitioned() {
    printf("Test 2: Gang partitioned\n");
    int A[N], B[N];
    
    #pragma acc parallel loop gang copy(A[0:N], B[0:N]) num_gangs(8)
    for (int i = 0; i < N; i++) {
        A[i] = i * 2;
        B[i] = A[i] + 1;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (B[i] != i*2 + 1) errors++;
    }
    printf("  Errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Test 3: Worker partitioned\n");
    float matrix[M][M];
    
    #pragma acc parallel loop gang worker copy(matrix[0:M][0:M]) \
        num_gangs(2) num_workers(4)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i + j);
        }
    }
    
    printf("  Matrix[0][0] = %.1f\n", matrix[0][0]);
}

void test_gang_worker_partitioned() {
    printf("Test 4: Gang+worker partitioned\n");
    int data3d[P][P][P];
    
    #pragma acc parallel loop gang worker collapse(3) \
        copy(data3d[0:P][0:P][0:P]) \
        num_gangs(4) num_workers(2)
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < P; k++) {
                data3d[i][j][k] = i * j * k;
            }
        }
    }
    
    printf("  Data[1][1][1] = %d\n", data3d[1][1][1]);
}

void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    double vec[N];
    
    #pragma acc parallel loop vector copy(vec[0:N]) vector_length(32)
    for (int i = 0; i < N; i++) {
        vec[i] = (double)i * 3.14159;
    }
    
    printf("  vec[100] = %.5f\n", vec[100]);
}

void test_gang_vector_partitioned() {
    printf("Test 6: Gang+vector partitioned\n");
    int A[N], B[N], C[N];
    
    #pragma acc parallel loop gang vector copyin(A[0:N], B[0:N]) \
        copyout(C[0:N]) num_gangs(16) vector_length(64)
    for (int i = 0; i < N; i++) {
        C[i] = A[i] + B[i];
    }
    
    printf("  Vector operation completed\n");
}

void test_worker_vector_partitioned() {
    printf("Test 7: Worker+vector partitioned\n");
    float complex_data[M][M];
    
    #pragma acc parallel loop worker vector collapse(2) \
        copy(complex_data[0:M][0:M]) \
        num_workers(8) vector_length(128)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            complex_data[i][j] = (float)(i * i + j * j);
        }
    }
    
    printf("  Complex data computed\n");
}

void test_fully_partitioned() {
    printf("Test 8: Fully partitioned (gang+worker+vector)\n");
    int reduction_result = 0;
    int data[N];
    
    for (int i = 0; i < N; i++) {
        data[i] = i % 10;
    }
    
    #pragma acc parallel loop gang worker vector reduction(+:reduction_result) \
        copyin(data[0:N]) num_gangs(8) num_workers(4) vector_length(32)
    for (int i = 0; i < N; i++) {
        reduction_result += data[i];
    }
    
    int expected = 0;
    for (int i = 0; i < N; i++) {
        expected += i % 10;
    }
    printf("  Reduction: %d (expected: %d)\n", reduction_result, expected);
}

void test_nested_parallelism() {
    printf("Test 9: Nested parallelism\n");
    int outer_data[M];
    int inner_data[M][M];
    
    #pragma acc parallel copy(outer_data[0:M]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            outer_data[i] = i * 10;
            
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                inner_data[i][j] = outer_data[i] + j;
            }
        }
    }
    
    printf("  Nested computation completed\n");
}

void test_combined_directives() {
    printf("Test 10: Combined directives with tile\n");
    int tiled_data[M][M];
    
    #pragma acc parallel loop tile(32, 32) copy(tiled_data[0:M][0:M]) \
        num_gangs(8) num_workers(2) vector_length(16)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            tiled_data[i][j] = (i << 8) | j;
        }
    }
    
    printf("  Tiled data[32][32] = %d\n", tiled_data[32][32]);
}

void test_runtime_dependent_partitioning() {
    printf("Test 11: Runtime-dependent partitioning\n");
    int dynamic_data[N];
    int partition_type = 3;  // Could be runtime determined
    
    #pragma acc parallel loop copy(dynamic_data[0:N]) \
        num_gangs(partition_type * 2) \
        num_workers(partition_type) \
        vector_length(16 << (partition_type % 3))
    for (int i = 0; i < N; i++) {
        dynamic_data[i] = i * partition_type;
    }
    
    printf("  Dynamic partitioning completed\n");
}

void test_multi_device() {
    printf("Test 12: Multi-device test\n");
    
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", dev_type);
    
    int num_devices = acc_get_num_devices(dev_type);
    printf("  Number of devices: %d\n", num_devices);
    
    if (num_devices > 1) {
        acc_set_device_num(0, dev_type);
    }
    
    int device_data[N];
    #pragma acc parallel loop copy(device_data[0:N]) 
    for (int i = 0; i < N; i++) {
        device_data[i] = i * 100;
    }
    
    printf("  Multi-device test completed\n");
}

void test_async_operations() {
    printf("Test 13: Async operations\n");
    int async_data[N];
    int async_handle;
    
    #pragma acc parallel loop copy(async_data[0:N]) async(async_handle) \
        num_gangs(4) num_workers(2) vector_length(32)
    for (int i = 0; i < N; i++) {
        async_data[i] = i * i;
    }
    
    acc_wait(async_handle);
    printf("  Async operation completed\n");
}

void test_private_firstprivate() {
    printf("Test 14: Private/firstprivate clauses\n");
    int private_var = 42;
    int firstprivate_var = 100;
    int shared_result[N];
    
    #pragma acc parallel loop private(private_var) \
        firstprivate(firstprivate_var) copyout(shared_result[0:N]) \
        num_gangs(8) vector_length(16)
    for (int i = 0; i < N; i++) {
        private_var = i;
        shared_result[i] = private_var + firstprivate_var;
    }
    
    printf("  Private/firstprivate test completed\n");
}

void test_error_conditions() {
    printf("Test 15: Error condition simulation\n");
    // This may trigger internal error paths
    int *null_ptr = NULL;
    
    // Try to use deviceptr with NULL (may trigger error handling)
    #pragma acc parallel loop deviceptr(null_ptr) 
    for (int i = 0; i < 10; i++) {
        // This will likely fail or trigger error handling
    }
    
    printf("  Error condition test attempted\n");
}

int main() {
    printf("Starting OpenACC partition coverage test...\n\n");
    
    // Enable debug output to increase likelihood of partition mapping calls
    char *debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Note: Set ACC_DEBUG=1 for detailed runtime output\n");
    }
    
    // Test all partition types
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests for complex scenarios
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_dependent_partitioning();
    test_multi_device();
    test_async_operations();
    test_private_firstprivate();
    
    // Test error conditions last
    test_error_conditions();
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
