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
    int sum = 0;
    
    // Initialize
    for (i = 0; i < N; i++) data[i] = i % 100;
    
    // Gang redundant - same data across all gangs
    #pragma acc parallel loop gang copy(data[0:N]) copy(sum) \
        num_gangs(4) vector_length(32)
    for (i = 0; i < N; i++) {
        #pragma acc atomic
        sum += data[i];
    }
    
    printf("Gang redundant test completed, sum = %d\n", sum);
}

void test_gang_partitioned() {
    int i, j;
    int matrix[N][M];
    int result[N] = {0};
    
    // Initialize matrix
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = (i + j) % 100;
        }
    }
    
    // Gang partitioned - each gang works on different rows
    #pragma acc parallel loop gang copy(matrix[0:N][0:M]) copyout(result[0:N]) \
        num_gangs(8) num_workers(1) vector_length(1)
    for (i = 0; i < N; i++) {
        int row_sum = 0;
        #pragma acc loop worker vector reduction(+:row_sum)
        for (j = 0; j < M; j++) {
            row_sum += matrix[i][j];
        }
        result[i] = row_sum;
    }
    
    printf("Gang partitioned test completed\n");
}

void test_worker_partitioned() {
    int i, j;
    int data[N][M];
    int partial_sums[4] = {0};  // One per worker
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = (i * j) % 100;
        }
    }
    
    // Worker partitioned - workers process different chunks
    #pragma acc parallel copy(data[0:N][0:M]) copyout(partial_sums[0:4]) \
        num_gangs(1) num_workers(4) vector_length(1)
    {
        int worker_id = acc_get_worker_num();
        int chunk_size = (N * M) / 4;
        int start = worker_id * chunk_size;
        int end = (worker_id == 3) ? (N * M) : start + chunk_size;
        int sum = 0;
        
        #pragma acc loop seq
        for (int idx = start; idx < end; idx++) {
            int i = idx / M;
            int j = idx % M;
            sum += data[i][j];
        }
        
        partial_sums[worker_id] = sum;
    }
    
    printf("Worker partitioned test completed\n");
}

void test_gang_worker_partitioned() {
    int i, j, k;
    int cube[N][M][P];
    int results[N][M] = {0};
    
    // Initialize 3D array
    #pragma acc parallel loop gang worker collapse(3) copyout(cube[0:N][0:M][0:P])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                cube[i][j][k] = (i + j + k) % 100;
            }
        }
    }
    
    // Gang+Worker partitioned - 2D decomposition
    #pragma acc parallel loop gang worker collapse(2) \
        copy(cube[0:N][0:M][0:P]) copyout(results[0:N][0:M]) \
        num_gangs(4) num_workers(8) vector_length(1)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            int sum = 0;
            #pragma acc loop vector reduction(+:sum)
            for (k = 0; k < P; k++) {
                sum += cube[i][j][k];
            }
            results[i][j] = sum;
        }
    }
    
    printf("Gang+Worker partitioned test completed\n");
}

void test_vector_partitioned() {
    int i, j;
    float a[N], b[N], c[N];
    
    // Initialize
    for (i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 0.5f;
    }
    
    // Vector partitioned - SIMD operations
    #pragma acc parallel loop vector copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(1) num_workers(1) vector_length(64)
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    printf("Vector partitioned test completed\n");
}

void test_gang_vector_partitioned() {
    int i, j;
    int matrix[N][M];
    int col_sums[M] = {0};
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = (i * M + j) % 100;
        }
    }
    
    // Gang+Vector partitioned - gangs with vector operations
    #pragma acc parallel loop gang vector collapse(2) \
        copy(matrix[0:N][0:M]) copyout(col_sums[0:M]) \
        num_gangs(8) num_workers(1) vector_length(32)
    for (j = 0; j < M; j++) {
        for (i = 0; i < N; i++) {
            #pragma acc atomic
            col_sums[j] += matrix[i][j];
        }
    }
    
    printf("Gang+Vector partitioned test completed\n");
}

void test_worker_vector_partitioned() {
    int i, j;
    int data[N][M];
    int row_sums[N] = {0};
    
    // Initialize
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = (i * j) % 100;
        }
    }
    
    // Worker+Vector partitioned
    #pragma acc parallel loop worker vector collapse(2) \
        copy(data[0:N][0:M]) copyout(row_sums[0:N]) \
        num_gangs(1) num_workers(4) vector_length(32)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            #pragma acc atomic
            row_sums[i] += data[i][j];
        }
    }
    
    printf("Worker+Vector partitioned test completed\n");
}

void test_fully_partitioned() {
    int i, j, k;
    int cube[N][M][P];
    int total_sum = 0;
    
    // Initialize
    #pragma acc parallel loop gang worker vector collapse(3) \
        copyout(cube[0:N][0:M][0:P])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                cube[i][j][k] = (i * j * k) % 100;
            }
        }
    }
    
    // Fully partitioned - using all levels
    #pragma acc parallel loop gang worker vector collapse(3) \
        copy(cube[0:N][0:M][0:P]) copy(total_sum) \
        reduction(+:total_sum) \
        num_gangs(4) num_workers(8) vector_length(16)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                total_sum += cube[i][j][k];
            }
        }
    }
    
    printf("Fully partitioned test completed, total_sum = %d\n", total_sum);
}

void test_nested_parallelism() {
    int i, j;
    int outer[N], inner[M];
    int result = 0;
    
    // Initialize
    for (i = 0; i < N; i++) outer[i] = i % 50;
    for (j = 0; j < M; j++) inner[j] = j % 30;
    
    // Nested parallelism
    #pragma acc parallel copy(outer[0:N], inner[0:M]) copy(result) \
        num_gangs(2)
    {
        int gang_sum = 0;
        
        #pragma acc loop gang reduction(+:gang_sum)
        for (i = 0; i < N; i++) {
            gang_sum += outer[i];
            
            // Inner parallel region
            #pragma acc parallel loop worker vector reduction(+:gang_sum) \
                copy(inner[0:M]) num_workers(2) vector_length(8)
            for (j = 0; j < M; j++) {
                gang_sum += inner[j];
            }
        }
        
        #pragma acc atomic
        result += gang_sum;
    }
    
    printf("Nested parallelism test completed, result = %d\n", result);
}

void test_runtime_determined_partitioning() {
    int i;
    int data[N];
    int sum = 0;
    int use_vector = 1;  // Runtime value
    
    // Initialize
    for (i = 0; i < N; i++) data[i] = i % 100;
    
    // Conditional partitioning based on runtime value
    if (use_vector) {
        #pragma acc parallel loop gang vector copy(data[0:N]) copy(sum) \
            reduction(+:sum) num_gangs(4) vector_length(32)
        for (i = 0; i < N; i++) {
            sum += data[i];
        }
    } else {
        #pragma acc parallel loop gang worker copy(data[0:N]) copy(sum) \
            reduction(+:sum) num_gangs(4) num_workers(2)
        for (i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    printf("Runtime-determined partitioning test completed, sum = %d\n", sum);
}

void test_async_operations() {
    int i;
    float a[N], b[N], c[N];
    acc_device_t dev_type = acc_get_device_type();
    
    // Initialize
    for (i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
    }
    
    // Multiple async operations with different partitioning
    #pragma acc parallel loop gang async(1) copyin(a[0:N], b[0:N]) copyout(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    #pragma acc parallel loop worker vector async(2) copy(c[0:N])
    for (i = 0; i < N; i++) {
        c[i] *= 2.0f;
    }
    
    #pragma acc wait
    
    printf("Async operations test completed\n");
}

void test_multi_device() {
    // Try to trigger different device paths
    acc_device_t devices[] = {acc_device_host, acc_device_not_host};
    int num_devices = sizeof(devices) / sizeof(devices[0]);
    
    for (int d = 0; d < num_devices; d++) {
        if (acc_set_device_type(devices[d]) == 0) {
            printf("Testing device type %d\n", devices[d]);
            
            int data[100];
            #pragma acc parallel loop copy(data[0:100])
            for (int i = 0; i < 100; i++) {
                data[i] = i;
            }
        }
    }
    
    // Reset to default
    acc_set_device_type(acc_get_device_type());
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to trigger logging paths
    char* debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        printf("Set ACC_DEBUG=1 for detailed partition logging\n");
    }
    
    // Test all partition types
    test_gang_redundant();           // Should trigger case 0
    test_gang_partitioned();         // Should trigger case 1
    test_worker_partitioned();       // Should trigger case 2
    test_gang_worker_partitioned();  // Should trigger case 3
    test_vector_partitioned();       // Should trigger case 4
    test_gang_vector_partitioned();  // Should trigger case 5
    test_worker_vector_partitioned(); // Should trigger case 6
    test_fully_partitioned();        // Should trigger case 7
    
    // Additional tests to explore more paths
    test_nested_parallelism();
    test_runtime_determined_partitioning();
    test_async_operations();
    test_multi_device();
    
    // Try to trigger error/illegal cases
    // Note: This might not directly hit the default case in the mapping function,
    // but could trigger other error paths that use the mapping
    printf("\nAttempting to trigger error conditions...\n");
    
    // Invalid data size (might trigger different internal handling)
    int small = 5;
    #pragma acc parallel copy(small)
    {
        small = 42;
    }
    
    // Uninitialized pointer (use with caution)
    int* ptr = NULL;
    #pragma acc parallel present_or_copy(ptr[0:10])
    {
        // This should fail or trigger special handling
    }
    
    printf("\nAll tests completed successfully!\n");
    printf("If compiled with coverage and run with ACC_DEBUG=1,\n");
    printf("the partition mapping function should have been called\n");
    printf("with codes 0-7 and potentially the default case.\n");
    
    return 0;
}
