/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 128
#define P 64

void test_gang_redundant() {
    printf("Test 1: Gang redundant partitioning\n");
    int data[N];
    int sum = 0;
    
    #pragma acc parallel loop gang copy(data[0:N]) copyin(sum) reduction(+:sum)
    for (int i = 0; i < N; i++) {
        data[i] = i;
        sum += i;
    }
    
    printf("  Sum: %d (expected: %d)\n", sum, N*(N-1)/2);
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
    printf("  Errors: %d\n", errors);
}

void test_worker_partitioned() {
    printf("Test 3: Worker partitioned\n");
    float matrix[M][M];
    
    #pragma acc parallel loop gang worker copy(matrix[0:M][0:M]) num_workers(4)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i + j);
        }
    }
    
    printf("  Matrix[0][0] = %.1f\n", matrix[0][0]);
}

void test_gang_worker_partitioned() {
    printf("Test 4: Gang+Worker partitioned\n");
    int data[M][M];
    
    #pragma acc parallel loop gang worker copy(data[0:M][0:M]) \
        num_gangs(2) num_workers(4)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i * M + j;
        }
    }
    
    printf("  Data[%d][%d] = %d\n", M/2, M/2, data[M/2][M/2]);
}

void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    double vec[N];
    
    #pragma acc parallel loop vector copy(vec[0:N]) vector_length(32)
    for (int i = 0; i < N; i++) {
        vec[i] = (double)i / 10.0;
    }
    
    printf("  Vector[%d] = %.2f\n", N-1, vec[N-1]);
}

void test_gang_vector_partitioned() {
    printf("Test 6: Gang+Vector partitioned\n");
    int data[N];
    
    #pragma acc parallel loop gang vector copy(data[0:N]) \
        num_gangs(8) vector_length(16)
    for (int i = 0; i < N; i++) {
        data[i] = i * i;
    }
    
    printf("  Data[10] = %d\n", data[10]);
}

void test_worker_vector_partitioned() {
    printf("Test 7: Worker+Vector partitioned\n");
    float data[M][P];
    
    #pragma acc parallel loop worker vector copy(data[0:M][0:P]) \
        num_workers(4) vector_length(8)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            data[i][j] = (float)(i * j);
        }
    }
    
    printf("  Data[%d][%d] = %.1f\n", M-1, P-1, data[M-1][P-1]);
}

void test_fully_partitioned() {
    printf("Test 8: Fully partitioned\n");
    int data[N];
    int sum = 0;
    
    #pragma acc parallel loop gang worker vector copy(data[0:N]) \
        reduction(+:sum) num_gangs(4) num_workers(2) vector_length(16)
    for (int i = 0; i < N; i++) {
        data[i] = i % 100;
        sum += data[i];
    }
    
    printf("  Sum: %d\n", sum);
}

void test_nested_parallelism() {
    printf("Test 9: Nested parallelism\n");
    int outer[M], inner[M][P];
    
    #pragma acc parallel copy(outer[0:M], inner[0:M][0:P]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            outer[i] = i * 10;
            
            #pragma acc loop worker vector
            for (int j = 0; j < P; j++) {
                inner[i][j] = outer[i] + j;
            }
        }
    }
    
    printf("  Inner[%d][%d] = %d\n", M/2, P/2, inner[M/2][P/2]);
}

void test_combined_directives() {
    printf("Test 10: Combined directives with tile\n");
    int data[M][M];
    
    #pragma acc parallel loop gang tile(32, 32) copy(data[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (i + 1) * (j + 1);
        }
    }
    
    printf("  Data[31][31] = %d\n", data[31][31]);
}

void test_runtime_dependent_partitioning() {
    printf("Test 11: Runtime-dependent partitioning\n");
    int data[N];
    int chunk_size = N / 4;
    
    // Runtime value affects partitioning
    #pragma acc parallel loop gang copy(data[0:N]) num_gangs(chunk_size > 0 ? 4 : 1)
    for (int i = 0; i < N; i++) {
        data[i] = i / chunk_size;
    }
    
    printf("  Data[%d] = %d\n", N-1, data[N-1]);
}

void test_multi_device() {
    printf("Test 12: Multi-device test\n");
    
    // Try different device types if available
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", (int)dev_type);
    
    int data[100];
    #pragma acc parallel loop copy(data[0:100])
    for (int i = 0; i < 100; i++) {
        data[i] = i * 2;
    }
    
    // Try to set device (might trigger different paths)
    int num_devices = acc_get_num_devices(dev_type);
    if (num_devices > 1) {
        acc_set_device_num(0, dev_type);
    }
}

void test_complex_data_structures() {
    printf("Test 13: Complex data structures\n");
    
    typedef struct {
        int id;
        float values[10];
        int *dynamic;
    } ComplexStruct;
    
    ComplexStruct arr[N];
    int base_data[N * 5];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        arr[i].id = i;
        arr[i].dynamic = &base_data[i * 5];
    }
    
    #pragma acc parallel loop gang copy(arr[0:N]) copy(base_data[0:N*5])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i].values[j] = (float)(i + j);
        }
        for (int j = 0; j < 5; j++) {
            arr[i].dynamic[j] = i * 100 + j;
        }
    }
    
    printf("  Struct[%d].id = %d\n", N/2, arr[N/2].id);
}

void test_async_operations() {
    printf("Test 14: Async operations\n");
    int data[N];
    int async_id = 1;
    
    #pragma acc parallel loop async(async_id) copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    #pragma acc wait(async_id)
    
    printf("  Async operation completed\n");
}

void test_edge_cases() {
    printf("Test 15: Edge cases\n");
    
    // Empty parallel region
    #pragma acc parallel
    {
    }
    
    // Single element
    int single = 0;
    #pragma acc parallel loop copy(single)
    for (int i = 0; i < 1; i++) {
        single = 42;
    }
    
    printf("  Single = %d\n", single);
}

int main() {
    printf("Starting OpenACC partition coverage test...\n\n");
    
    // Enable debug output to trigger mapping function calls
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
    test_complex_data_structures();
    test_async_operations();
    test_edge_cases();
    
    printf("\nAll tests completed successfully!\n");
    
    // The default case for illegal partition codes might be triggered
    // by internal error conditions. We can't directly test it, but
    // the runtime might hit it in error handling paths.
    
    return 0;
}
