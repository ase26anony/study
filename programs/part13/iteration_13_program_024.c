/* Test program to cover partition code mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -ftree-parallelize-loops=0 -o test_partition test_partition.c
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
    int i;
    int data[N];
    
    #pragma acc parallel loop gang copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 2) {
            printf("Error in test_gang_redundant at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_partitioned() {
    int i, j;
    int matrix[N][M];
    
    #pragma acc parallel loop gang copyout(matrix[0:N][0:M]) num_gangs(8)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = i + j;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (matrix[i][j] != i + j) {
                printf("Error in test_gang_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_worker_partitioned() {
    int i, j;
    float array[N][M];
    
    #pragma acc parallel loop gang worker copyout(array[0:N][0:M]) \
        num_gangs(4) num_workers(8)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            array[i][j] = (float)(i * j) / 100.0f;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            float expected = (float)(i * j) / 100.0f;
            if (array[i][j] != expected) {
                printf("Error in test_worker_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_gang_worker_partitioned() {
    int i, j, k;
    int cube[N][M][P];
    
    #pragma acc parallel loop gang worker copyout(cube[0:N][0:M][0:P]) \
        num_gangs(4) num_workers(4)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                cube[i][j][k] = i * 1000000 + j * 1000 + k;
            }
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                int expected = i * 1000000 + j * 1000 + k;
                if (cube[i][j][k] != expected) {
                    printf("Error in test_gang_worker_partitioned at [%d][%d][%d]\n", i, j, k);
                    exit(1);
                }
            }
        }
    }
}

void test_vector_partitioned() {
    int i;
    double vec[N];
    
    #pragma acc parallel loop vector copyout(vec[0:N]) vector_length(128)
    for (i = 0; i < N; i++) {
        vec[i] = (double)i * 3.14159;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (vec[i] != (double)i * 3.14159) {
            printf("Error in test_vector_partitioned at index %d\n", i);
            exit(1);
        }
    }
}

void test_gang_vector_partitioned() {
    int i, j;
    float matrix[N][M];
    float sum = 0.0f;
    
    #pragma acc parallel loop gang vector reduction(+:sum) \
        copy(matrix[0:N][0:M]) copyin(sum) copyout(sum) \
        num_gangs(8) vector_length(64)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix[i][j] = (float)(i + j);
            sum += matrix[i][j];
        }
    }
    
    // Verify sum
    float expected_sum = 0.0f;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            expected_sum += (float)(i + j);
        }
    }
    
    if (sum != expected_sum) {
        printf("Error in test_gang_vector_partitioned: sum mismatch\n");
        exit(1);
    }
}

void test_worker_vector_partitioned() {
    int i, j;
    int array[N][M];
    
    #pragma acc parallel loop worker vector copy(array[0:N][0:M]) \
        num_workers(4) vector_length(32)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            array[i][j] = i * M + j;
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if (array[i][j] != i * M + j) {
                printf("Error in test_worker_vector_partitioned at [%d][%d]\n", i, j);
                exit(1);
            }
        }
    }
}

void test_fully_partitioned() {
    int i, j, k;
    int result = 0;
    int data[N][M];
    
    #pragma acc parallel loop gang worker vector reduction(+:result) \
        copy(data[0:N][0:M]) copyin(result) copyout(result) \
        num_gangs(4) num_workers(2) vector_length(16)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            data[i][j] = i * j;
            result += data[i][j];
        }
    }
    
    // Verify result
    int expected_result = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            expected_result += i * j;
        }
    }
    
    if (result != expected_result) {
        printf("Error in test_fully_partitioned: result mismatch\n");
        exit(1);
    }
}

void test_nested_parallelism() {
    int i, j;
    int outer[N];
    int inner[M];
    
    #pragma acc parallel copyout(outer[0:N])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            outer[i] = i * 10;
            
            #pragma acc parallel loop worker vector copyout(inner[0:M])
            for (j = 0; j < M; j++) {
                inner[j] = outer[i] + j;
            }
            
            // Verify inner results
            for (j = 0; j < M; j++) {
                if (inner[j] != outer[i] + j) {
                    printf("Error in test_nested_parallelism at outer[%d], inner[%d]\n", i, j);
                    exit(1);
                }
            }
        }
    }
}

void test_combined_directives() {
    int i, j, k;
    int tile_result = 0;
    
    #pragma acc parallel loop tile(32, 16, 8) gang worker vector \
        reduction(+:tile_result) copyin(tile_result) copyout(tile_result)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                tile_result += i + j + k;
            }
        }
    }
    
    // Verify
    int expected_tile = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                expected_tile += i + j + k;
            }
        }
    }
    
    if (tile_result != expected_tile) {
        printf("Error in test_combined_directives\n");
        exit(1);
    }
}

void test_runtime_determined_partitioning() {
    int i;
    int dynamic[N];
    int chunk_size = 64;  // Runtime value
    
    #pragma acc parallel loop gang copyout(dynamic[0:N]) \
        num_gangs(N / chunk_size)
    for (i = 0; i < N; i++) {
        dynamic[i] = i * chunk_size;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (dynamic[i] != i * chunk_size) {
            printf("Error in test_runtime_determined_partitioning at index %d\n", i);
            exit(1);
        }
    }
}

void test_multi_device() {
    int original_device = acc_get_device_num(acc_get_device_type());
    
    // Try to switch devices (may trigger different partitioning paths)
    acc_set_device_num(0, acc_device_default);
    
    int i;
    int data[N];
    
    #pragma acc parallel loop gang worker copyout(data[0:N])
    for (i = 0; i < N; i++) {
        data[i] = i * 3;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (data[i] != i * 3) {
            printf("Error in test_multi_device at index %d\n", i);
            exit(1);
        }
    }
    
    // Restore original device
    acc_set_device_num(original_device, acc_device_default);
}

void test_private_firstprivate() {
    int i;
    int private_var = 100;
    int firstprivate_var = 200;
    int result[N];
    
    #pragma acc parallel loop gang private(private_var) \
        firstprivate(firstprivate_var) copyout(result[0:N])
    for (i = 0; i < N; i++) {
        private_var = i;
        result[i] = private_var + firstprivate_var;
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (result[i] != i + 200) {
            printf("Error in test_private_firstprivate at index %d\n", i);
            exit(1);
        }
    }
}

void test_async_operations() {
    int i;
    int async_data[N];
    acc_handle_t async_handle;
    
    #pragma acc parallel loop gang async copyout(async_data[0:N])
    for (i = 0; i < N; i++) {
        async_data[i] = i * 5;
    }
    
    #pragma acc wait
    
    // Verify
    for (i = 0; i < N; i++) {
        if (async_data[i] != i * 5) {
            printf("Error in test_async_operations at index %d\n", i);
            exit(1);
        }
    }
}

int main() {
    printf("Starting comprehensive OpenACC partition coverage test...\n");
    
    // Enable debug output to trigger partition string mapping
    char* debug_env = getenv("ACC_DEBUG");
    if (!debug_env) {
        putenv("ACC_DEBUG=1");
    }
    
    // Test each partitioning scenario
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
    
    test_combined_directives();
    printf("✓ test_combined_directives passed\n");
    
    test_runtime_determined_partitioning();
    printf("✓ test_runtime_determined_partitioning passed\n");
    
    test_multi_device();
    printf("✓ test_multi_device passed\n");
    
    test_private_firstprivate();
    printf("✓ test_private_firstprivate passed\n");
    
    test_async_operations();
    printf("✓ test_async_operations passed\n");
    
    printf("\nAll tests passed successfully!\n");
    printf("The partition code mapping function should have been called with values 0-7.\n");
    
    return 0;
}
