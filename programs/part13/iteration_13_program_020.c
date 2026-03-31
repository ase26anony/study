/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

void test_gang_redundant() {
    printf("Testing gang redundant partitioning...\n");
    int data[N];
    
    #pragma acc parallel loop gang copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 2) errors++;
    }
    printf("  Gang redundant: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_gang_partitioned() {
    printf("Testing gang partitioned...\n");
    int data[N];
    int sum = 0;
    
    #pragma acc parallel loop gang reduction(+:sum) copyin(data[0:N]) copyout(sum)
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    
    printf("  Gang partitioned sum: %d\n", sum);
}

void test_worker_partitioned() {
    printf("Testing worker partitioned...\n");
    int data[M];
    
    #pragma acc parallel loop worker num_workers(4) copyout(data[0:M])
    for (int i = 0; i < M; i++) {
        data[i] = i + 1;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (data[i] != i + 1) errors++;
    }
    printf("  Worker partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_gang_worker_partitioned() {
    printf("Testing gang+worker partitioned...\n");
    int matrix[N][M];
    
    #pragma acc parallel loop gang worker collapse(2) copyout(matrix[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = i * M + j;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (matrix[i][j] != i * M + j) errors++;
        }
    }
    printf("  Gang+worker partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_vector_partitioned() {
    printf("Testing vector partitioned...\n");
    float data[P];
    
    #pragma acc parallel loop vector vector_length(32) copyout(data[0:P])
    for (int i = 0; i < P; i++) {
        data[i] = i * 1.5f;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < P; i++) {
        if (data[i] != i * 1.5f) errors++;
    }
    printf("  Vector partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_gang_vector_partitioned() {
    printf("Testing gang+vector partitioned...\n");
    int data[N];
    
    #pragma acc parallel loop gang vector num_gangs(4) vector_length(32) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (data[i] != i * 3) errors++;
    }
    printf("  Gang+vector partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_worker_vector_partitioned() {
    printf("Testing worker+vector partitioned...\n");
    float data[M];
    
    #pragma acc parallel loop worker vector num_workers(2) vector_length(16) copyout(data[0:M])
    for (int i = 0; i < M; i++) {
        data[i] = i * 2.5f;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (data[i] != i * 2.5f) errors++;
    }
    printf("  Worker+vector partitioned: %s\n", errors == 0 ? "PASS" : "FAIL");
}

void test_fully_partitioned() {
    printf("Testing fully partitioned...\n");
    int matrix[N][M];
    int total_sum = 0;
    
    #pragma acc parallel loop gang worker vector collapse(2) \
        reduction(+:total_sum) copyout(matrix[0:N][0:M]) copy(total_sum)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = i + j;
            total_sum += matrix[i][j];
        }
    }
    
    // Verify
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            expected_sum += i + j;
        }
    }
    printf("  Fully partitioned sum: %d (expected: %d) %s\n", 
           total_sum, expected_sum, total_sum == expected_sum ? "PASS" : "FAIL");
}

void test_nested_parallelism() {
    printf("Testing nested parallelism...\n");
    int outer[N];
    int inner[M];
    
    #pragma acc parallel copyout(outer[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            outer[i] = i * 10;
            
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                // Nested computation
                if (j == 0) {
                    outer[i] += j;
                }
            }
        }
    }
    
    printf("  Nested parallelism completed\n");
}

void test_runtime_determined_partitioning() {
    printf("Testing runtime-determined partitioning...\n");
    int data[N];
    int chunk_size = N / 4;
    
    // Runtime-dependent partitioning
    for (int chunk = 0; chunk < 4; chunk++) {
        int start = chunk * chunk_size;
        int end = (chunk == 3) ? N : start + chunk_size;
        
        #pragma acc parallel loop gang copyout(data[start:end-start])
        for (int i = start; i < end; i++) {
            data[i] = i * (chunk + 1);
        }
    }
    
    printf("  Runtime-determined partitioning completed\n");
}

void test_multi_device() {
    printf("Testing multi-device scenarios...\n");
    
    // Get device type
    acc_device_t dev_type = acc_get_device_type();
    printf("  Device type: %d\n", dev_type);
    
    // Try to set device (may trigger different paths)
    int num_devices = acc_get_num_devices(dev_type);
    if (num_devices > 0) {
        acc_set_device_num(0, dev_type);
        printf("  Set device 0 of type %d\n", dev_type);
    }
    
    // Simple parallel region after device selection
    int test_data[10];
    #pragma acc parallel loop copyout(test_data[0:10])
    for (int i = 0; i < 10; i++) {
        test_data[i] = i;
    }
}

void test_async_operations() {
    printf("Testing async operations...\n");
    int data[N];
    int async_id = 1;
    
    #pragma acc parallel loop async(async_id) copyout(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = i * 7;
    }
    
    #pragma acc wait(async_id)
    
    printf("  Async operations completed\n");
}

void test_combined_directives() {
    printf("Testing combined directives...\n");
    int data[N][M];
    
    // Combined parallel loop with tile
    #pragma acc parallel loop gang worker tile(32, 16) copyout(data[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = i * M + j;
        }
    }
    
    printf("  Combined directives completed\n");
}

void test_data_clauses_variations() {
    printf("Testing various data clauses...\n");
    
    // Test different data mappings
    int create_arr[N];
    int copyin_arr[N];
    int copyout_arr[N];
    int copy_arr[N];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        copyin_arr[i] = i;
        copy_arr[i] = i * 2;
    }
    
    #pragma acc parallel loop gang \
        create(create_arr[0:N]) \
        copyin(copyin_arr[0:N]) \
        copyout(copyout_arr[0:N]) \
        copy(copy_arr[0:N])
    for (int i = 0; i < N; i++) {
        create_arr[i] = i * 3;
        copyout_arr[i] = copyin_arr[i] * 2;
        copy_arr[i] += i;
    }
    
    printf("  Data clauses variations completed\n");
}

int main() {
    printf("Starting OpenACC partition coverage test...\n\n");
    
    // Enable debug output to trigger string mapping function calls
    // This is done via environment variable ACC_DEBUG=1
    
    // Test all partition types
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
    test_runtime_determined_partitioning();
    test_multi_device();
    test_async_operations();
    test_combined_directives();
    test_data_clauses_variations();
    
    printf("\nAll tests completed.\n");
    
    // Try to trigger potential error/default case
    // by using invalid device operations
    printf("\nAttempting to trigger error paths...\n");
    acc_shutdown(acc_device_nvidia);  // May trigger error paths
    
    return 0;
}
