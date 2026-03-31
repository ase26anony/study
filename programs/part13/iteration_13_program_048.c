/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O0 -fopenacc -foffload=disable -o test_partition test_partition.c
 * Run with: ACC_DEBUG=1 ./test_partition
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define M 32
#define P 8

void test_gang_redundant() {
    int data[N];
    int i;
    
    // Simple gang-level parallelism - likely maps to "gang redundant" or "gang partitioned"
    #pragma acc parallel loop gang copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 2;
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
    int data[N];
    int i;
    
    // Explicit gang partitioning with private data
    #pragma acc parallel loop gang private(i) copyout(data[0:N]) num_gangs(4)
    for (i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 3) {
            fprintf(stderr, "Error in test_gang_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_worker_partitioned() {
    int data[N];
    int i;
    
    // Worker-level parallelism
    #pragma acc parallel loop worker copyout(data[0:N]) num_workers(4)
    for (i = 0; i < N; i++) {
        data[i] = i * 4;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 4) {
            fprintf(stderr, "Error in test_worker_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_worker_partitioned() {
    int data[N][M];
    int i, j;
    
    // Nested parallelism: gang and worker levels
    #pragma acc parallel loop gang worker collapse(2) copyout(data[0:N][0:M]) \
        num_gangs(4) num_workers(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = i * M + j;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (data[i][j] != i * M + j) {
                fprintf(stderr, "Error in test_gang_worker_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_vector_partitioned() {
    float data[N];
    int i;
    
    // Vector-level parallelism
    #pragma acc parallel loop vector copyout(data[0:N]) vector_length(32)
    for (i = 0; i < N; i++) {
        data[i] = i * 1.5f;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 1.5f) {
            fprintf(stderr, "Error in test_vector_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_vector_partitioned() {
    int data[N];
    int i;
    
    // Gang and vector parallelism
    #pragma acc parallel loop gang vector copyout(data[0:N]) \
        num_gangs(4) vector_length(32)
    for (i = 0; i < N; i++) {
        data[i] = i * 5;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 5) {
            fprintf(stderr, "Error in test_gang_vector_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_worker_vector_partitioned() {
    float data[N][M];
    int i, j;
    
    // Worker and vector parallelism
    #pragma acc parallel loop worker vector collapse(2) copyout(data[0:N][0:M]) \
        num_workers(4) vector_length(16)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = (i + j) * 2.0f;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (data[i][j] != (i + j) * 2.0f) {
                fprintf(stderr, "Error in test_worker_vector_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_fully_partitioned() {
    int data[N][M][P];
    int i, j, k;
    
    // Fully partitioned: gang, worker, and vector
    #pragma acc parallel loop gang worker vector collapse(3) copyout(data[0:N][0:M][0:P]) \
        num_gangs(2) num_workers(2) vector_length(8)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                data[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                if (data[i][j][k] != i * M * P + j * P + k) {
                    fprintf(stderr, "Error in test_fully_partitioned at [%d][%d][%d]\n", i, j, k);
                    exit(1);
                }
            }
        }
    }
}

void test_reduction_partitioning() {
    int sum = 0;
    int data[N];
    int i;
    
    // Initialize data
    for (i = 0; i < N; i++) {
        data[i] = i + 1;
    }
    
    // Reduction with gang/worker/vector partitioning
    #pragma acc parallel loop gang worker vector reduction(+:sum) copyin(data[0:N]) \
        num_gangs(4) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        sum += data[i];
    }
    
    // Verify
    int expected = N * (N + 1) / 2;
    if (sum != expected) {
        fprintf(stderr, "Error in test_reduction_partitioning: sum=%d, expected=%d\n", 
                sum, expected);
        exit(1);
    }
}

void test_nested_parallelism() {
    int data[N];
    int i;
    
    // Nested parallel regions
    #pragma acc parallel copyout(data[0:N]) num_gangs(2)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data[i] = i * 7;
        }
        
        // Inner parallel region
        #pragma acc parallel loop worker vector
        for (i = 0; i < N; i++) {
            data[i] += 1;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 7 + 1) {
            fprintf(stderr, "Error in test_nested_parallelism at index %d\n", i);
            exit(1);
        }
    }
}

void test_conditional_partitioning() {
    int data[N];
    int i;
    int use_workers = 1;  // Runtime decision
    
    // Conditional partitioning based on runtime value
    if (use_workers) {
        #pragma acc parallel loop gang worker copyout(data[0:N]) \
            num_gangs(2) num_workers(4)
        for (i = 0; i < N; i++) {
            data[i] = i * 8;
        }
    } else {
        #pragma acc parallel loop gang vector copyout(data[0:N]) \
            num_gangs(4) vector_length(32)
        for (i = 0; i < N; i++) {
            data[i] = i * 9;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 8) {
            fprintf(stderr, "Error in test_conditional_partitioning at index %d\n", i);
            exit(1);
        }
    }
}

void test_async_partitioning() {
    int data[N];
    int i;
    int async_id = 1;
    
    // Async execution with partitioning
    #pragma acc parallel loop gang worker vector async(async_id) copyout(data[0:N]) \
        num_gangs(2) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        data[i] = i * 10;
    }
    
    // Wait for completion
    #pragma acc wait(async_id)
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 10) {
            fprintf(stderr, "Error in test_async_partitioning at index %d\n", i);
            exit(1);
        }
    }
}

void test_multi_device_partitioning() {
    // Test with different device types if available
    acc_device_t dev_type = acc_get_device_type();
    
    if (dev_type == acc_device_nvidia || dev_type == acc_device_radeon) {
        // Try to set device and test partitioning
        int num_devices = acc_get_num_devices(dev_type);
        if (num_devices > 0) {
            acc_set_device_num(0, dev_type);
            
            int data[N];
            int i;
            
            #pragma acc parallel loop gang worker copyout(data[0:N]) \
                num_gangs(4) num_workers(2)
            for (i = 0; i < N; i++) {
                data[i] = i * 11;
            }
            
            // Verify
            for (i = 0; i < N; i++) {
                if (data[i] != i * 11) {
                    fprintf(stderr, "Error in test_multi_device_partitioning at index %d\n", i);
                    exit(1);
                }
            }
        }
    }
}

void test_invalid_partition_attempt() {
    // This might trigger error paths that could use the default case
    // by passing invalid partition codes
    
    // Try with invalid device pointer (might trigger error handling)
    int *invalid_ptr = NULL;
    
    // This should fail gracefully or trigger error paths
    #pragma acc enter data copyin(invalid_ptr[0:1]) if(0)
    
    // Try with extremely large vector length (might cause unusual partitioning)
    int data[10];
    int i;
    
    #pragma acc parallel loop vector copyout(data[0:10]) vector_length(1024)
    for (i = 0; i < 10; i++) {
        data[i] = i;
    }
    
    // Verify
    for (i = 0; i < 10; i++) {
        if (data[i] != i) {
            fprintf(stderr, "Error in test_invalid_partition_attempt at index %d\n", i);
            exit(1);
        }
    }
}

int main() {
    printf("Starting partition code coverage test...\n");
    
    // Enable debug output to increase likelihood of calling the mapping function
    setenv("ACC_DEBUG", "1", 1);
    setenv("LIBGOMP_DEBUG", "1", 1);
    
    // Test various partitioning scenarios
    test_gang_redundant();
    printf("test_gang_redundant passed\n");
    
    test_gang_partitioned();
    printf("test_gang_partitioned passed\n");
    
    test_worker_partitioned();
    printf("test_worker_partitioned passed\n");
    
    test_gang_worker_partitioned();
    printf("test_gang_worker_partitioned passed\n");
    
    test_vector_partitioned();
    printf("test_vector_partitioned passed\n");
    
    test_gang_vector_partitioned();
    printf("test_gang_vector_partitioned passed\n");
    
    test_worker_vector_partitioned();
    printf("test_worker_vector_partitioned passed\n");
    
    test_fully_partitioned();
    printf("test_fully_partitioned passed\n");
    
    test_reduction_partitioning();
    printf("test_reduction_partitioning passed\n");
    
    test_nested_parallelism();
    printf("test_nested_parallelism passed\n");
    
    test_conditional_partitioning();
    printf("test_conditional_partitioning passed\n");
    
    test_async_partitioning();
    printf("test_async_partitioning passed\n");
    
    test_multi_device_partitioning();
    printf("test_multi_device_partitioning passed\n");
    
    test_invalid_partition_attempt();
    printf("test_invalid_partition_attempt passed\n");
    
    printf("All tests passed successfully!\n");
    
    return 0;
}
