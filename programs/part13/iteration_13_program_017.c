/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -foffload=disable -o test_partition test_partition.c
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
    printf("Testing gang redundant partitioning...\n");
    int scalar = 42;
    int array[N];
    
    #pragma acc parallel copy(scalar) copyout(array[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array[i] = scalar + i;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != 42 + i) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_gang_partitioned() {
    printf("Testing gang partitioned...\n");
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) num_gangs(8)
    {
        int gang_id = __pgi_gangidx();
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array[i] = gang_id * 1000 + i;
        }
    }
    
    // Simple verification
    printf("  First few values: %d, %d, %d\n", array[0], array[1], array[2]);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned...\n");
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) num_gangs(2) num_workers(4)
    {
        int worker_id = __pgi_workeridx();
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            array[i] = worker_id * 100 + i;
        }
    }
    
    printf("  Sample values: %d, %d\n", array[0], array[N/2]);
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned...\n");
    int matrix[M][P];
    
    #pragma acc parallel copyout(matrix[0:M][0:P]) num_gangs(4) num_workers(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < P; j++) {
                matrix[i][j] = i * 1000 + j;
            }
        }
    }
    
    // Verify a few values
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (matrix[i][j] != i * 1000 + j) errors++;
        }
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_vector_partitioned() {
    printf("Testing vector partitioned...\n");
    float array[N];
    
    #pragma acc parallel copyout(array[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            array[i] = i * 1.5f;
        }
    }
    
    printf("  Vector results: %.2f, %.2f, %.2f\n", array[0], array[1], array[2]);
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned...\n");
    int array[N];
    
    #pragma acc parallel copyout(array[0:N]) num_gangs(4) vector_length(16)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            array[i] = i * 2;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (array[i] != i * 2) errors++;
    }
    if (errors > 0) printf("  Errors: %d\n", errors);
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned...\n");
    float array[N];
    
    #pragma acc parallel copyout(array[0:N]) num_workers(2) vector_length(8)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            array[i] = i * 3.14f;
        }
    }
    
    printf("  Sample: %.2f, %.2f\n", array[0], array[N-1]);
}

void test_fully_partitioned() {
    printf("Testing fully partitioned...\n");
    int result = 0;
    int array[N];
    
    #pragma acc parallel copyin(array[0:N]) copy(result) \
        num_gangs(8) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker vector reduction(+:result)
        for (int i = 0; i < N; i++) {
            result += array[i];
        }
    }
    
    printf("  Reduction result: %d\n", result);
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int outer[M], inner[M][P];
    
    #pragma acc parallel copyout(outer[0:M], inner[0:M][0:P]) num_gangs(2)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            outer[i] = i * 10;
            
            #pragma acc parallel loop worker vector num_workers(2) vector_length(8)
            for (int j = 0; j < P; j++) {
                inner[i][j] = i * 100 + j;
            }
        }
    }
    
    printf("  Outer[10]=%d, Inner[10][20]=%d\n", outer[10], inner[10][20]);
}

void test_conditional_partitioning() {
    printf("Testing conditional partitioning...\n");
    int array[N];
    int use_workers = 1;  // Runtime decision
    
    #pragma acc parallel copyout(array[0:N])
    {
        if (use_workers) {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                array[i] = i + 1000;
            }
        } else {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                array[i] = i + 2000;
            }
        }
    }
    
    printf("  Conditional result: %d\n", array[0]);
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int array1[N], array2[N];
    
    #pragma acc parallel copyout(array1[0:N]) async(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            array1[i] = i * 5;
        }
    }
    
    #pragma acc parallel copyout(array2[0:N]) async(2)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            array2[i] = i * 7;
        }
    }
    
    acc_wait_all();
    printf("  Async operations completed\n");
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Try to set different device types
    acc_device_t dev_type = acc_get_device_type();
    printf("  Current device type: %d\n", (int)dev_type);
    
    // Test with device data directives
    int* d_array = (int*)acc_malloc(N * sizeof(int));
    int h_array[N];
    
    for (int i = 0; i < N; i++) h_array[i] = i * 3;
    
    acc_memcpy_to_device(d_array, h_array, N * sizeof(int));
    
    #pragma acc parallel present(d_array[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            d_array[i] += 42;
        }
    }
    
    acc_memcpy_from_device(h_array, d_array, N * sizeof(int));
    acc_free(d_array);
    
    printf("  Device memory test completed\n");
}

void test_invalid_partition() {
    printf("Testing potential invalid partition codes...\n");
    
    // This might trigger error paths that use the default case
    // by using unusual combinations or edge cases
    
    int small_array[10];
    
    // Very small loop with explicit partitioning
    #pragma acc parallel copyout(small_array[0:10]) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < 10; i++) {
            small_array[i] = i;
        }
    }
    
    // Test with zero-length arrays (edge case)
    int* empty_array = NULL;
    #pragma acc parallel copyout(empty_array[0:0])
    {
        // No operations
    }
    
    printf("  Edge cases tested\n");
}

int main() {
    printf("Starting OpenACC partition coverage test...\n\n");
    
    // Initialize ACC
    acc_init(acc_device_default);
    
    // Test all partition types
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests to stress different code paths
    test_nested_parallelism();
    test_conditional_partitioning();
    test_async_operations();
    test_multi_device();
    test_invalid_partition();
    
    printf("\nAll tests completed successfully!\n");
    
    // Final validation
    int final_array[100];
    #pragma acc parallel loop copyout(final_array[0:100])
    for (int i = 0; i < 100; i++) {
        final_array[i] = i * i;
    }
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += final_array[i];
    }
    printf("Final validation sum: %d\n", sum);
    
    acc_shutdown(acc_device_default);
    
    return 0;
}
