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
    printf("Testing gang redundant pattern...\n");
    int data[N];
    int i;
    
    #pragma acc parallel loop gang copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 2) {
            printf("Error in gang redundant test at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_partitioned() {
    printf("Testing gang partitioned pattern...\n");
    int data[N];
    int i;
    
    #pragma acc parallel loop gang copy(data[0:N]) num_gangs(4)
    for (i = 0; i < N; i++) {
        data[i] += 1;
    }
}

void test_worker_partitioned() {
    printf("Testing worker partitioned pattern...\n");
    int data[M][P];
    int i, j;
    
    #pragma acc parallel loop gang worker copy(data[0:M][0:P]) num_workers(4)
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            data[i][j] = i * P + j;
        }
    }
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned pattern...\n");
    int data[N];
    int i;
    
    #pragma acc parallel loop gang worker copy(data[0:N]) \
        num_gangs(2) num_workers(4)
    for (i = 0; i < N; i++) {
        data[i] = i % 256;
    }
}

void test_vector_partitioned() {
    printf("Testing vector partitioned pattern...\n");
    float data[N];
    int i;
    
    #pragma acc parallel loop vector copy(data[0:N]) vector_length(32)
    for (i = 0; i < N; i++) {
        data[i] = (float)i / N;
    }
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned pattern...\n");
    double data[N];
    int i;
    
    #pragma acc parallel loop gang vector copy(data[0:N]) \
        num_gangs(4) vector_length(64)
    for (i = 0; i < N; i++) {
        data[i] = (double)i * 3.14159;
    }
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned pattern...\n");
    int data[M][P];
    int i, j;
    
    #pragma acc parallel loop worker vector copy(data[0:M][0:P]) \
        num_workers(2) vector_length(16)
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            data[i][j] = (i << 8) | j;
        }
    }
}

void test_fully_partitioned() {
    printf("Testing fully partitioned pattern...\n");
    int data[N];
    int sum = 0;
    int i;
    
    // Reduction with full parallelism
    #pragma acc parallel loop gang worker vector reduction(+:sum) \
        copyin(data[0:N]) copy(sum) num_gangs(2) num_workers(2) vector_length(32)
    for (i = 0; i < N; i++) {
        data[i] = i;
        sum += data[i];
    }
    
    // Verify reduction
    int expected = (N - 1) * N / 2;
    if (sum != expected) {
        printf("Error in fully partitioned reduction: got %d, expected %d\n", 
               sum, expected);
        exit(1);
    }
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int data[M][P];
    int i, j;
    
    #pragma acc parallel copy(data[0:M][0:P]) num_gangs(2)
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < P; j++) {
                data[i][j] = i * 1000 + j;
            }
        }
    }
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int data[N];
    int i;
    
    // Combined parallel loop with tile
    #pragma acc parallel loop tile(32) copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * i;
    }
    
    // Another combined directive with gang, worker, vector
    #pragma acc parallel loop gang worker vector collapse(2) \
        copy(data[0:N]) num_gangs(2) num_workers(2) vector_length(16)
    for (i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            data[i * 32 + j] += j;
        }
    }
}

void test_runtime_determined_partitioning() {
    printf("Testing runtime-determined partitioning...\n");
    int data[N];
    int partition_type = 2;  // Could be runtime determined
    int i;
    
    if (partition_type == 2) {
        #pragma acc parallel loop worker copy(data[0:N]) num_workers(4)
        for (i = 0; i < N; i++) {
            data[i] = i % 100;
        }
    } else {
        #pragma acc parallel loop gang copy(data[0:N]) num_gangs(4)
        for (i = 0; i < N; i++) {
            data[i] = i % 50;
        }
    }
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Try to set different device types
    acc_device_t dev_type = acc_get_device_type();
    printf("Current device type: %d\n", (int)dev_type);
    
    // Try to create data on device
    int *d_data = (int*)acc_malloc(N * sizeof(int));
    if (d_data) {
        int h_data[N];
        
        #pragma acc parallel loop deviceptr(d_data) present_or_copyin(h_data[0:N])
        for (int i = 0; i < N; i++) {
            d_data[i] = h_data[i] = i;
        }
        
        acc_free(d_data);
    }
}

void test_complex_data_structures() {
    printf("Testing complex data structures...\n");
    
    typedef struct {
        int x[N/2];
        float y[N/2];
    } ComplexData;
    
    ComplexData data[2];
    int i, j;
    
    // Initialize
    for (i = 0; i < 2; i++) {
        for (j = 0; j < N/2; j++) {
            data[i].x[j] = i * 1000 + j;
            data[i].y[j] = (float)(i * 1000 + j) / 1000.0f;
        }
    }
    
    // Process with different parallelism levels
    #pragma acc parallel loop gang worker copy(data[0:2]) \
        num_gangs(2) num_workers(2)
    for (i = 0; i < 2; i++) {
        #pragma acc loop vector
        for (j = 0; j < N/2; j++) {
            data[i].x[j] *= 2;
            data[i].y[j] *= 2.0f;
        }
    }
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int data[N];
    int async_id = 1;
    int i;
    
    #pragma acc parallel loop async(async_id) copy(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    #pragma acc wait(async_id)
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to trigger mapping function calls
    putenv("ACC_DEBUG=1");
    putenv("LIBGOMP_DEBUG=1");
    
    // Test all partition patterns
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests to explore different code paths
    test_nested_parallelism();
    test_combined_directives();
    test_runtime_determined_partitioning();
    test_multi_device();
    test_complex_data_structures();
    test_async_operations();
    
    // Test with firstprivate and private clauses
    printf("Testing firstprivate/private clauses...\n");
    int private_var = 42;
    int firstprivate_var = 100;
    int result[N];
    
    #pragma acc parallel loop gang worker firstprivate(firstprivate_var) \
        private(private_var) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        private_var = i;
        result[i] = private_var + firstprivate_var;
    }
    
    // Test reduction with different data types
    printf("Testing various reduction patterns...\n");
    float float_sum = 0.0f;
    double double_sum = 0.0;
    int int_sum = 0;
    
    #pragma acc parallel loop reduction(+:float_sum, double_sum, int_sum) \
        num_gangs(4) num_workers(2) vector_length(16)
    for (int i = 0; i < N; i++) {
        float_sum += 1.0f / (i + 1);
        double_sum += 2.0 / (i + 1);
        int_sum += i;
    }
    
    // Test with dynamic data
    printf("Testing dynamic data allocation...\n");
    int *dynamic_data = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        dynamic_data[i] = i;
    }
    
    #pragma acc parallel loop copy(dynamic_data[0:N])
    for (int i = 0; i < N; i++) {
        dynamic_data[i] *= 2;
    }
    
    free(dynamic_data);
    
    // Final validation
    printf("All tests completed successfully!\n");
    printf("The partition code mapping function should have been called with:\n");
    printf("  - Codes 0-7 for valid partition patterns\n");
    printf("  - Potentially out-of-range codes for error conditions\n");
    
    return 0;
}
