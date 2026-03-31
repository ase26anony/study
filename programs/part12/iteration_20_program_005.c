/* test-omp-acc-partition-strings.c */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test-omp-acc-partition-strings.c */
/* Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test-omp-acc-partition-strings.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

/* Structure to store partition query results */
typedef struct {
    int gang_count;
    int worker_count;
    int vector_length;
    int device_type;
} partition_result_t;

/* Host array to store results from all tests */
partition_result_t host_results[NUM_TESTS];

/* Device array for runtime queries */
#pragma acc declare create(device_results[NUM_TESTS])
partition_result_t device_results[NUM_TESTS];

/* Initialize device array */
void init_device_results() {
    #pragma acc enter data copyin(device_results[0:NUM_TESTS])
}

/* Cleanup device array */
void cleanup_device_results() {
    #pragma acc exit data delete(device_results[0:NUM_TESTS])
}

/* Test 0: Gang redundant (all 1s) */
void test_gang_redundant(int test_idx) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copy(device_results[test_idx:1])
    {
        device_results[test_idx].gang_count = acc_get_num_gangs(acc_async_noval);
        device_results[test_idx].worker_count = acc_get_num_workers(acc_async_noval);
        device_results[test_idx].vector_length = acc_get_vector_length(acc_async_noval);
        device_results[test_idx].device_type = acc_get_device_type();
    }
}

/* Test 1: Gang partitioned */
void test_gang_partitioned(int test_idx) {
    int gang_cnt = 8;
    #pragma acc parallel num_gangs(gang_cnt) num_workers(1) vector_length(1) \
        copy(device_results[test_idx:1])
    {
        device_results[test_idx].gang_count = acc_get_num_gangs(acc_async_noval);
        device_results[test_idx].worker_count = acc_get_num_workers(acc_async_noval);
        device_results[test_idx].vector_length = acc_get_vector_length(acc_async_noval);
        device_results[test_idx].device_type = acc_get_device_type();
    }
}

/* Test 2: Worker partitioned */
void test_worker_partitioned(int test_idx) {
    int worker_cnt = 4;
    #pragma acc parallel num_gangs(1) num_workers(worker_cnt) vector_length(1) \
        copy(device_results[test_idx:1])
    {
        device_results[test_idx].gang_count = acc_get_num_gangs(acc_async_noval);
        device_results[test_idx].worker_count = acc_get_num_workers(acc_async_noval);
        device_results[test_idx].vector_length = acc_get_vector_length(acc_async_noval);
        device_results[test_idx].device_type = acc_get_device_type();
    }
}

/* Test 3: Gang+Worker partitioned */
void test_gang_worker_partitioned(int test_idx) {
    int gang_cnt = 2;
    int worker_cnt = 2;
    #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(1) \
        copy(device_results[test_idx:1])
    {
        device_results[test_idx].gang_count = acc_get_num_gangs(acc_async_noval);
        device_results[test_idx].worker_count = acc_get_num_workers(acc_async_noval);
        device_results[test_idx].vector_length = acc_get_vector_length(acc_async_noval);
        device_results[test_idx].device_type = acc_get_device_type();
    }
}

/* Test 4: Vector partitioned */
void test_vector_partitioned(int test_idx) {
    int vec_len = 128;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(vec_len) \
        copy(device_results[test_idx:1])
    {
        device_results[test_idx].gang_count = acc_get_num_gangs(acc_async_noval);
        device_results[test_idx].worker_count = acc_get_num_workers(acc_async_noval);
        device_results[test_idx].vector_length = acc_get_vector_length(acc_async_noval);
        device_results[test_idx].device_type = acc_get_device_type();
    }
}

/* Test 5: Gang+Vector partitioned */
void test_gang_vector_partitioned(int test_idx) {
    int gang_cnt = 4;
    int vec_len = 64;
    #pragma acc parallel num_gangs(gang_cnt) num_workers(1) vector_length(vec_len) \
        copy(device_results[test_idx:1])
    {
        device_results[test_idx].gang_count = acc_get_num_gangs(acc_async_noval);
        device_results[test_idx].worker_count = acc_get_num_workers(acc_async_noval);
        device_results[test_idx].vector_length = acc_get_vector_length(acc_async_noval);
        device_results[test_idx].device_type = acc_get_device_type();
    }
}

/* Test 6: Worker+Vector partitioned */
void test_worker_vector_partitioned(int test_idx) {
    int worker_cnt = 8;
    int vec_len = 32;
    #pragma acc parallel num_gangs(1) num_workers(worker_cnt) vector_length(vec_len) \
        copy(device_results[test_idx:1])
    {
        device_results[test_idx].gang_count = acc_get_num_gangs(acc_async_noval);
        device_results[test_idx].worker_count = acc_get_num_workers(acc_async_noval);
        device_results[test_idx].vector_length = acc_get_vector_length(acc_async_noval);
        device_results[test_idx].device_type = acc_get_device_type();
    }
}

/* Test 7: Fully partitioned */
void test_fully_partitioned(int test_idx) {
    int gang_cnt = 4;
    int worker_cnt = 4;
    int vec_len = 32;
    #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(vec_len) \
        copy(device_results[test_idx:1])
    {
        device_results[test_idx].gang_count = acc_get_num_gangs(acc_async_noval);
        device_results[test_idx].worker_count = acc_get_num_workers(acc_async_noval);
        device_results[test_idx].vector_length = acc_get_vector_length(acc_async_noval);
        device_results[test_idx].device_type = acc_get_device_type();
    }
}

/* Test with nested parallelism */
void test_nested_partitioning(int test_idx) {
    int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    
    #pragma acc data copyout(data[0:N])
    {
        #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32)
        {
            #pragma acc loop gang
            for (int i = 0; i < N/32; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < 32; j++) {
                    int idx = i * 32 + j;
                    if (idx < N) {
                        data[idx] = i * 1000 + j;
                    }
                }
            }
        }
    }
    
    /* Verify some values to prevent optimization */
    int sum = 0;
    for (int i = 0; i < N; i += 64) {
        sum += data[i];
    }
    printf("Nested test checksum: %d\n", sum);
    
    free(data);
}

/* Test with kernels directive and collapse */
void test_kernels_collapse(int test_idx) {
    int M = 64, N = 64;
    int *matrix = (int*)malloc(M * N * sizeof(int));
    
    #pragma acc data copyout(matrix[0:M*N])
    {
        #pragma acc kernels num_gangs(16) num_workers(2) vector_length(64)
        {
            #pragma acc loop gang worker collapse(2)
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    matrix[i * N + j] = i * N + j;
                }
            }
        }
    }
    
    /* Force computation to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < M * N; i += 128) {
        checksum += matrix[i];
    }
    printf("Kernels collapse checksum: %d\n", checksum);
    
    free(matrix);
}

/* Test with unstructured data regions */
void test_unstructured_data(int test_idx) {
    int size = 1024;
    int *host_array = (int*)malloc(size * sizeof(int));
    int *device_array;
    
    /* Unstructured data region */
    device_array = (int*)acc_malloc(size * sizeof(int));
    if (device_array == NULL) {
        printf("acc_malloc failed - this may trigger diagnostic output\n");
        return;
    }
    
    #pragma acc enter data copyin(host_array[0:size])
    
    /* Structured compute on unstructured data */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(64) \
        present(host_array[0:size])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < size; i++) {
            host_array[i] = i * 2;
        }
    }
    
    #pragma acc exit data copyout(host_array[0:size])
    
    /* Verify */
    int sum = 0;
    for (int i = 0; i < size; i += 32) {
        sum += host_array[i];
    }
    printf("Unstructured data checksum: %d\n", sum);
    
    acc_free(device_array);
    free(host_array);
}

/* Test with dynamic values from runtime */
void test_dynamic_partitioning() {
    int max_gangs = 16;
    int max_workers = 8;
    int max_vector = 64;
    
    for (int g = 1; g <= max_gangs; g *= 2) {
        for (int w = 1; w <= max_workers; w *= 2) {
            for (int v = 1; v <= max_vector; v *= 2) {
                int result[3] = {0, 0, 0};
                
                #pragma acc parallel num_gangs(g) num_workers(w) vector_length(v) \
                    copyout(result[0:3])
                {
                    result[0] = acc_get_num_gangs(acc_async_noval);
                    result[1] = acc_get_num_workers(acc_async_noval);
                    result[2] = acc_get_vector_length(acc_async_noval);
                }
                
                /* Use results to prevent optimization */
                if (result[0] + result[1] + result[2] == 0) {
                    printf("Warning: All zero results\n");
                }
            }
        }
    }
}

int main() {
    printf("Starting OpenACC partition string coverage test...\n");
    
    /* Initialize device results array */
    init_device_results();
    
    /* Run all partition configuration tests */
    test_gang_redundant(0);
    test_gang_partitioned(1);
    test_worker_partitioned(2);
    test_gang_worker_partitioned(3);
    test_vector_partitioned(4);
    test_gang_vector_partitioned(5);
    test_worker_vector_partitioned(6);
    test_fully_partitioned(7);
    
    /* Copy results back to host */
    #pragma acc update host(device_results[0:NUM_TESTS])
    
    /* Calculate checksum to ensure all tests executed */
    long long total_checksum = 0;
    printf("\nPartition configuration results:\n");
    printf("Test | Gangs | Workers | Vector | Device Type\n");
    printf("-----|-------|---------|--------|------------\n");
    
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("%4d | %5d | %7d | %6d | %11d\n",
               i,
               device_results[i].gang_count,
               device_results[i].worker_count,
               device_results[i].vector_length,
               device_results[i].device_type);
        
        total_checksum += device_results[i].gang_count;
        total_checksum += device_results[i].worker_count;
        total_checksum += device_results[i].vector_length;
        total_checksum += device_results[i].device_type;
    }
    
    printf("\nTotal checksum: %lld\n", total_checksum);
    
    /* Additional tests to stress different code paths */
    printf("\nRunning additional tests...\n");
    
    /* Test nested parallelism */
    test_nested_partitioning(0);
    
    /* Test kernels directive with collapse */
    test_kernels_collapse(1);
    
    /* Test unstructured data regions */
    test_unstructured_data(2);
    
    /* Test dynamic partitioning with loops */
    printf("Testing dynamic partitioning variations...\n");
    test_dynamic_partitioning();
    
    /* Cleanup */
    cleanup_device_results();
    
    printf("\nTest completed successfully!\n");
    
    return 0;
}
