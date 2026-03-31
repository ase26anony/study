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
    int i;
    int data[N];
    
    // Initialize data
    for (i = 0; i < N; i++) data[i] = i;
    
    // Simple parallel region - likely gang redundant
    #pragma acc parallel copy(data[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data[i] = data[i] * 2;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 2) {
            fprintf(stderr, "Error in test_gang_redundant at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_partitioned() {
    int i, j;
    int matrix[N][M];
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = i + j;
        }
    }
    
    // Parallel with gang partitioning
    #pragma acc parallel copy(matrix) num_gangs(8) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop worker vector independent
            for (j = 0; j < M; j++) {
                matrix[i][j] = matrix[i][j] * 3;
            }
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (matrix[i][j] != (i + j) * 3) {
                fprintf(stderr, "Error in test_gang_partitioned at (%d,%d)\n", i, j);
                exit(1);
            }
        }
    }
}

void test_worker_partitioned() {
    int i, j;
    float array[N][M];
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            array[i][j] = (float)(i * j);
        }
    }
    
    // Worker-level partitioning
    #pragma acc parallel copy(array) num_workers(4) vector_length(64)
    {
        #pragma acc loop worker independent
        for (i = 0; i < N; i++) {
            #pragma acc loop vector independent
            for (j = 0; j < M; j++) {
                array[i][j] = array[i][j] / 2.0f;
            }
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (array[i][j] != (float)(i * j) / 2.0f) {
                fprintf(stderr, "Error in test_worker_partitioned at (%d,%d)\n", i, j);
                exit(1);
            }
        }
    }
}

void test_gang_worker_partitioned() {
    int i, j, k;
    int cube[N][M][P];
    
    // Initialize 3D array
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                cube[i][j][k] = i + j + k;
            }
        }
    }
    
    // Complex nested parallelism - gang+worker partitioned
    #pragma acc parallel copy(cube) num_gangs(4) num_workers(8) vector_length(16)
    {
        #pragma acc loop gang independent collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                #pragma acc loop worker vector independent
                for (k = 0; k < P; k++) {
                    cube[i][j][k] = cube[i][j][k] * 2;
                }
            }
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                if (cube[i][j][k] != (i + j + k) * 2) {
                    fprintf(stderr, "Error in test_gang_worker_partitioned at (%d,%d,%d)\n", i, j, k);
                    exit(1);
                }
            }
        }
    }
}

void test_vector_partitioned() {
    int i;
    double vec[N];
    
    // Initialize
    for (i = 0; i < N; i++) vec[i] = (double)i;
    
    // Vector-level partitioning
    #pragma acc parallel copy(vec) vector_length(128)
    {
        #pragma acc loop vector independent
        for (i = 0; i < N; i++) {
            vec[i] = vec[i] + 100.0;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (vec[i] != (double)i + 100.0) {
            fprintf(stderr, "Error in test_vector_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_vector_partitioned() {
    int i, j;
    int matrix[N][M];
    int sum = 0;
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = 1;
        }
    }
    
    // Reduction with gang+vector partitioning
    #pragma acc parallel copy(matrix) reduction(+:sum) num_gangs(8) vector_length(32)
    {
        #pragma acc loop gang reduction(+:sum)
        for (i = 0; i < N; i++) {
            #pragma acc loop vector reduction(+:sum)
            for (j = 0; j < M; j++) {
                sum += matrix[i][j];
            }
        }
    }
    
    // Verify
    int expected = N * M;
    if (sum != expected) {
        fprintf(stderr, "Error in test_gang_vector_partitioned: sum=%d, expected=%d\n", sum, expected);
        exit(1);
    }
}

void test_worker_vector_partitioned() {
    int i, j;
    float array[N][M];
    
    // Initialize with pattern
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            array[i][j] = (float)(i * M + j);
        }
    }
    
    // Worker+vector partitioning
    #pragma acc parallel copy(array) num_workers(4) vector_length(64)
    {
        #pragma acc loop worker independent
        for (i = 0; i < N; i++) {
            #pragma acc loop vector independent
            for (j = 0; j < M; j++) {
                array[i][j] = array[i][j] * array[i][j];
            }
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            float expected = (float)(i * M + j);
            expected = expected * expected;
            if (array[i][j] != expected) {
                fprintf(stderr, "Error in test_worker_vector_partitioned at (%d,%d)\n", i, j);
                exit(1);
            }
        }
    }
}

void test_fully_partitioned() {
    int i, j, k;
    int cube[N/2][M/2][P/2];
    int total = 0;
    
    // Initialize smaller 3D array
    for (i = 0; i < N/2; i++) {
        for (j = 0; j < M/2; j++) {
            for (k = 0; k < P/2; k++) {
                cube[i][j][k] = 2;
            }
        }
    }
    
    // Fully partitioned with all levels
    #pragma acc parallel copy(cube) reduction(+:total) \
                num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang independent collapse(3) reduction(+:total)
        for (i = 0; i < N/2; i++) {
            for (j = 0; j < M/2; j++) {
                for (k = 0; k < P/2; k++) {
                    #pragma acc loop worker vector reduction(+:total)
                    for (int l = 0; l < 2; l++) {  // Additional inner loop
                        total += cube[i][j][k] + l;
                    }
                }
            }
        }
    }
    
    // Verify
    int elements = (N/2) * (M/2) * (P/2);
    int expected = elements * 2 * 2;  // 2 from cube value, *2 from inner loop
    if (total != expected) {
        fprintf(stderr, "Error in test_fully_partitioned: total=%d, expected=%d\n", total, expected);
        exit(1);
    }
}

void test_nested_parallelism() {
    int i, j;
    int outer[N], inner[M];
    
    // Initialize
    for (i = 0; i < N; i++) outer[i] = i;
    for (j = 0; j < M; j++) inner[j] = j;
    
    // Nested parallel regions
    #pragma acc parallel copy(outer) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            outer[i] = outer[i] * 2;
            
            // Inner parallel region
            #pragma acc parallel copy(inner) num_workers(2) vector_length(32)
            {
                #pragma acc loop worker vector
                for (j = 0; j < M; j++) {
                    inner[j] = inner[j] + outer[i];
                }
            }
        }
    }
    
    // Verify outer
    for (i = 0; i < N; i++) {
        if (outer[i] != i * 2) {
            fprintf(stderr, "Error in test_nested_parallelism outer at %d\n", i);
            exit(1);
        }
    }
}

void test_conditional_partitioning() {
    int i;
    int data[N];
    int threshold = N / 2;
    
    // Initialize
    for (i = 0; i < N; i++) data[i] = i;
    
    // Conditional partitioning based on runtime value
    if (threshold > 100) {
        #pragma acc parallel copy(data) num_gangs(8) vector_length(64)
        {
            #pragma acc loop gang vector
            for (i = 0; i < N; i++) {
                data[i] = data[i] + 1000;
            }
        }
    } else {
        #pragma acc parallel copy(data) num_workers(4) vector_length(32)
        {
            #pragma acc loop worker vector
            for (i = 0; i < N; i++) {
                data[i] = data[i] + 500;
            }
        }
    }
    
    // Verify
    int expected_add = (threshold > 100) ? 1000 : 500;
    for (i = 0; i < N; i++) {
        if (data[i] != i + expected_add) {
            fprintf(stderr, "Error in test_conditional_partitioning at %d\n", i);
            exit(1);
        }
    }
}

void test_async_operations() {
    int i;
    int data[N];
    acc_device_t dev_type;
    
    // Initialize
    for (i = 0; i < N; i++) data[i] = i;
    
    // Get device type and set up async operations
    dev_type = acc_get_device_type();
    acc_set_device_num(0, dev_type);
    
    // Multiple async operations with different partitioning
    #pragma acc parallel copy(data) async(1) num_gangs(4)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data[i] = data[i] * 3;
        }
    }
    
    #pragma acc parallel copy(data) async(2) num_workers(2) vector_length(64)
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            data[i] = data[i] + 7;
        }
    }
    
    // Wait for all async operations
    #pragma acc wait
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != (i * 3) + 7) {
            fprintf(stderr, "Error in test_async_operations at %d\n", i);
            exit(1);
        }
    }
}

void test_multi_device() {
    // Try to trigger different device contexts
    acc_device_t devices[] = {acc_device_default, acc_device_host, acc_device_not_host};
    int num_devices = sizeof(devices) / sizeof(devices[0]);
    
    for (int d = 0; d < num_devices; d++) {
        if (acc_set_device_type(devices[d]) != 0) {
            // Device type not available, skip
            continue;
        }
        
        int data[10];
        for (int i = 0; i < 10; i++) data[i] = i;
        
        #pragma acc parallel copy(data)
        {
            #pragma acc loop
            for (int i = 0; i < 10; i++) {
                data[i] = data[i] * 2;
            }
        }
        
        // Verify
        for (int i = 0; i < 10; i++) {
            if (data[i] != i * 2) {
                fprintf(stderr, "Error in test_multi_device for device %d\n", d);
                exit(1);
            }
        }
    }
}

int main() {
    printf("Starting OpenACC partition coverage test...\n");
    
    // Test various partitioning scenarios
    test_gang_redundant();
    printf("✓ test_gang_redundant passed\n");
    
    test_gang_partitioned();
    printf("✓ test_gang_partitioned passed\n");
    
    test_worker_partitioned();
    printf("✓ test_worker_partitioned passed\n");
    
    test_gang_worker_partitioned();
    printf("✓ test_gang_worker_partitioned passed\n");
    
    test_vector_partitioned();
    printf("✓ test_vector_partitioned passed\n");
    
    test_gang_vector_partitioned();
    printf("✓ test_gang_vector_partitioned passed\n");
    
    test_worker_vector_partitioned();
    printf("✓ test_worker_vector_partitioned passed\n");
    
    test_fully_partitioned();
    printf("✓ test_fully_partitioned passed\n");
    
    test_nested_parallelism();
    printf("✓ test_nested_parallelism passed\n");
    
    test_conditional_partitioning();
    printf("✓ test_conditional_partitioning passed\n");
    
    test_async_operations();
    printf("✓ test_async_operations passed\n");
    
    test_multi_device();
    printf("✓ test_multi_device passed\n");
    
    printf("\nAll tests passed successfully!\n");
    printf("The partition code mapping function should have been called with values 0-7.\n");
    
    return 0;
}
