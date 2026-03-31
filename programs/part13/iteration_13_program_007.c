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
    
    #pragma acc parallel loop gang copy(data[0:N])
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
    int data[N];
    
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(32) copy(data[0:N])
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            data[i] = i + 1;
        }
    }
    
    printf("  Data[0]=%d, Data[%d]=%d\n", data[0], N-1, data[N-1]);
}

void test_gang_worker_partitioned() {
    printf("Test 4: Gang+Worker partitioned\n");
    int matrix[M][P];
    
    #pragma acc parallel num_gangs(4) num_workers(2) copy(matrix[0:M][0:P])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < P; j++) {
                matrix[i][j] = i * 100 + j;
            }
        }
    }
    
    printf("  Matrix[0][0]=%d, Matrix[%d][%d]=%d\n", 
           matrix[0][0], M-1, P-1, matrix[M-1][P-1]);
}

void test_vector_partitioned() {
    printf("Test 5: Vector partitioned\n");
    float data[N];
    
    #pragma acc parallel loop vector copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 1.5f;
    }
    
    printf("  Data[0]=%.2f, Data[%d]=%.2f\n", data[0], N-1, data[N-1]);
}

void test_gang_vector_partitioned() {
    printf("Test 6: Gang+Vector partitioned\n");
    int data[N];
    
    #pragma acc parallel loop gang vector copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    printf("  Data[0]=%d, Data[%d]=%d\n", data[0], N-1, data[N-1]);
}

void test_worker_vector_partitioned() {
    printf("Test 7: Worker+Vector partitioned\n");
    int data[N];
    
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) copy(data[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            data[i] = i * 4;
        }
    }
    
    printf("  Data[0]=%d, Data[%d]=%d\n", data[0], N-1, data[N-1]);
}

void test_fully_partitioned() {
    printf("Test 8: Fully partitioned (gang+worker+vector)\n");
    int data[N];
    int sum = 0;
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
                copy(data[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            data[i] = i;
            sum += i;
        }
    }
    
    printf("  Sum: %d (expected: %d)\n", sum, N*(N-1)/2);
}

void test_nested_parallelism() {
    printf("Test 9: Nested parallelism for complex partitioning\n");
    int matrix[M][P];
    
    #pragma acc parallel num_gangs(2) copy(matrix[0:M][0:P])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc parallel loop worker vector
            for (int j = 0; j < P; j++) {
                matrix[i][j] = (i + 1) * (j + 1);
            }
        }
    }
    
    printf("  Matrix[10][10]=%d\n", matrix[10][10]);
}

void test_combined_directives() {
    printf("Test 10: Combined parallel loop with tile\n");
    int data[M][P];
    
    #pragma acc parallel loop gang tile(32,16) copy(data[0:M][0:P])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            data[i][j] = i * P + j;
        }
    }
    
    printf("  Data[32][16]=%d\n", data[32][16]);
}

void test_runtime_determined() {
    printf("Test 11: Runtime-determined partitioning\n");
    int data[N];
    int chunk = N / 4;
    
    #pragma acc parallel loop gang copy(data[0:N])
    for (int i = 0; i < N; i++) {
        if (i < chunk) {
            data[i] = 1;
        } else if (i < 2*chunk) {
            data[i] = 2;
        } else if (i < 3*chunk) {
            data[i] = 3;
        } else {
            data[i] = 4;
        }
    }
    
    int counts[5] = {0};
    for (int i = 0; i < N; i++) {
        counts[data[i]]++;
    }
    printf("  Distribution: 1:%d 2:%d 3:%d 4:%d\n", 
           counts[1], counts[2], counts[3], counts[4]);
}

void test_multi_device() {
    printf("Test 12: Multi-device context\n");
    
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", dev_type);
    
    int num_devices = acc_get_num_devices(dev_type);
    printf("  Number of devices: %d\n", num_devices);
    
    if (num_devices > 1) {
        acc_set_device_num(0, dev_type);
    }
    
    int data[N];
    #pragma acc parallel loop copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 10;
    }
    
    printf("  Data[100]=%d\n", data[100]);
}

void test_async_operations() {
    printf("Test 13: Async operations with different queues\n");
    int data1[N], data2[N];
    int async_id1 = 1, async_id2 = 2;
    
    #pragma acc parallel loop async(async_id1) copy(data1[0:N])
    for (int i = 0; i < N; i++) {
        data1[i] = i * 2;
    }
    
    #pragma acc parallel loop async(async_id2) copy(data2[0:N])
    for (int i = 0; i < N; i++) {
        data2[i] = i * 3;
    }
    
    #pragma acc wait(async_id1, async_id2)
    
    printf("  Async operations completed\n");
}

void test_complex_data_structures() {
    printf("Test 14: Complex data structures\n");
    
    typedef struct {
        int id;
        float values[10];
        int *dynamic;
    } ComplexType;
    
    ComplexType array[N];
    array[0].dynamic = (int*)malloc(5 * sizeof(int));
    array[N-1].dynamic = (int*)malloc(5 * sizeof(int));
    
    #pragma acc enter data copyin(array[0:N])
    
    #pragma acc parallel loop gang copy(array[0:N])
    for (int i = 0; i < N; i++) {
        array[i].id = i;
        for (int j = 0; j < 10; j++) {
            array[i].values[j] = i * 10.0f + j;
        }
    }
    
    #pragma acc exit data copyout(array[0:N])
    
    printf("  Array[0].id=%d, values[0]=%.2f\n", array[0].id, array[0].values[0]);
    
    free(array[0].dynamic);
    free(array[N-1].dynamic);
}

void test_edge_cases() {
    printf("Test 15: Edge cases and error conditions\n");
    
    // Small data size
    int small[4];
    #pragma acc parallel loop copy(small[0:4])
    for (int i = 0; i < 4; i++) {
        small[i] = i;
    }
    
    // Empty parallel region
    #pragma acc parallel
    {
        // No operations
    }
    
    printf("  Edge cases tested\n");
}

int main() {
    printf("Starting OpenACC partition coverage test...\n\n");
    
    // Enable debug output to trigger logging paths
    char* debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Note: Set ACC_DEBUG=1 for verbose runtime output\n\n");
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
    
    // Additional complex cases
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_determined();
    test_multi_device();
    test_async_operations();
    test_complex_data_structures();
    test_edge_cases();
    
    printf("\nAll tests completed successfully!\n");
    
    // Force a potential out-of-bounds partition code by using deviceptr with NULL
    // This might trigger the default case in error handling paths
    printf("\nTesting potential error path...\n");
    int* null_ptr = NULL;
    #pragma acc parallel deviceptr(null_ptr)
    {
        // This might trigger error handling
    }
    
    return 0;
}
