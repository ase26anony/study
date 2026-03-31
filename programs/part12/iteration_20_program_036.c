/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128
#define ARRAY_SIZE 1024

/* Structure to store runtime query results */
typedef struct {
    int gang_count;
    int worker_count;
    int vector_length;
    int partition_code;
} partition_info_t;

/* Function to perform computation with specific partitioning */
void test_partition_scheme(int test_id, partition_info_t *info, float *data, int size) {
    int gang_cnt, worker_cnt, vector_len;
    
    /* Set partitioning based on test ID */
    switch(test_id) {
        case 0: /* gang redundant */
            gang_cnt = 1; worker_cnt = 1; vector_len = 1;
            info->partition_code = 0;
            break;
        case 1: /* gang partitioned */
            gang_cnt = 8; worker_cnt = 1; vector_len = 1;
            info->partition_code = 1;
            break;
        case 2: /* worker partitioned */
            gang_cnt = 1; worker_cnt = 4; vector_len = 1;
            info->partition_code = 2;
            break;
        case 3: /* gang+worker partitioned */
            gang_cnt = 2; worker_cnt = 2; vector_len = 1;
            info->partition_code = 3;
            break;
        case 4: /* vector partitioned */
            gang_cnt = 1; worker_cnt = 1; vector_len = VECTOR_LEN;
            info->partition_code = 4;
            break;
        case 5: /* gang+vector partitioned */
            gang_cnt = 4; worker_cnt = 1; vector_len = 64;
            info->partition_code = 5;
            break;
        case 6: /* worker+vector partitioned */
            gang_cnt = 1; worker_cnt = 2; vector_len = 32;
            info->partition_code = 6;
            break;
        case 7: /* fully partitioned */
            gang_cnt = 2; worker_cnt = 2; vector_len = 16;
            info->partition_code = 7;
            break;
        default:
            gang_cnt = 1; worker_cnt = 1; vector_len = 1;
            info->partition_code = -1;
    }
    
    /* Parallel region with explicit partitioning */
    #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(vector_len) \
                copyout(info[0:1]) present(data[0:size])
    {
        /* Query runtime for actual execution configuration */
        info->gang_count = acc_get_num_gangs(acc_async_noval);
        info->worker_count = acc_get_num_workers(acc_async_noval);
        info->vector_length = acc_get_vector_length(acc_async_noval);
        
        /* Perform actual computation to ensure region isn't optimized away */
        #pragma acc loop gang worker vector
        for(int i = 0; i < size; i++) {
            data[i] = data[i] * 2.0f + (float)test_id;
        }
    }
}

/* Test with nested parallelism */
void test_nested_partition(float *data, int size) {
    int gang_dynamic = 4;
    int worker_dynamic = 2;
    
    /* Dynamic partitioning using host variables */
    #pragma acc parallel num_gangs(gang_dynamic) num_workers(worker_dynamic) vector_length(32) \
                copy(data[0:size])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        /* Nested loop construct with collapse */
        #pragma acc loop gang collapse(2)
        for(int i = 0; i < g; i++) {
            for(int j = 0; j < w; j++) {
                #pragma acc loop worker vector
                for(int k = 0; k < size/(g*w); k++) {
                    int idx = i * (size/g) + j * (size/(g*w)) + k;
                    if(idx < size) {
                        data[idx] += (float)(g * w * v);
                    }
                }
            }
        }
    }
}

/* Test with kernels directive and automatic partitioning */
void test_kernels_partition(float *data, int size) {
    #pragma acc kernels copy(data[0:size]) num_gangs(8) num_workers(2) vector_length(64)
    {
        #pragma acc loop gang
        for(int i = 0; i < size/8; i++) {
            #pragma acc loop worker
            for(int j = 0; j < 2; j++) {
                #pragma acc loop vector
                for(int k = 0; k < 64; k++) {
                    int idx = i * 8 * 64 + j * 64 + k;
                    if(idx < size) {
                        data[idx] = data[idx] * 0.5f;
                    }
                }
            }
        }
    }
}

/* Test with unstructured data regions */
void test_unstructured_partition(float *data, int size) {
    float *device_data;
    
    /* Unstructured data region */
    #pragma acc enter data copyin(data[0:size])
    
    /* Multiple parallel regions with same data */
    for(int i = 0; i < 3; i++) {
        int gangs = 1 << i;  /* 1, 2, 4 */
        #pragma acc parallel num_gangs(gangs) num_workers(2) vector_length(16) \
                    present(data[0:size])
        {
            int g = acc_get_num_gangs(acc_async_noval);
            #pragma acc loop gang worker vector
            for(int j = 0; j < size; j++) {
                data[j] += (float)(g * i);
            }
        }
    }
    
    /* Exit data region */
    #pragma acc exit data copyout(data[0:size])
}

/* Test with potential diagnostic trigger */
void test_diagnostic_path(float *data, int size) {
    void *dev_ptr;
    
    /* Allocate device memory - may trigger diagnostics */
    dev_ptr = acc_malloc(size * sizeof(float));
    
    if(dev_ptr == NULL) {
        /* This might trigger diagnostic output including partition info */
        fprintf(stderr, "acc_malloc failed for size %d\n", size);
    } else {
        /* Use the allocated memory in a parallel region */
        #pragma acc parallel num_gangs(2) num_workers(4) vector_length(8) \
                    present(data[0:size]) deviceptr(dev_ptr)
        {
            #pragma acc loop gang worker vector
            for(int i = 0; i < size; i++) {
                ((float*)dev_ptr)[i] = data[i];
                data[i] = ((float*)dev_ptr)[i] * 3.0f;
            }
        }
        
        acc_free(dev_ptr);
    }
}

int main() {
    float *data;
    partition_info_t partition_results[NUM_TESTS];
    long checksum = 0;
    
    /* Allocate and initialize data */
    data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    for(int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i;
    }
    
    /* Copy data to device initially */
    #pragma acc data copy(data[0:ARRAY_SIZE]) copyout(partition_results[0:NUM_TESTS])
    {
        /* Test all partition schemes */
        for(int i = 0; i < NUM_TESTS; i++) {
            test_partition_scheme(i, &partition_results[i], data, ARRAY_SIZE);
        }
        
        /* Additional tests for coverage */
        test_nested_partition(data, ARRAY_SIZE);
        test_kernels_partition(data, ARRAY_SIZE);
        test_unstructured_partition(data, ARRAY_SIZE);
        test_diagnostic_path(data, ARRAY_SIZE);
    }
    
    /* Calculate checksum and print results */
    printf("Partition Test Results:\n");
    printf("=======================\n");
    
    for(int i = 0; i < NUM_TESTS; i++) {
        printf("Test %d: gangs=%d, workers=%d, vector=%d (code=%d)\n",
               i, partition_results[i].gang_count,
               partition_results[i].worker_count,
               partition_results[i].vector_length,
               partition_results[i].partition_code);
        
        checksum += partition_results[i].gang_count;
        checksum += partition_results[i].worker_count;
        checksum += partition_results[i].vector_length;
    }
    
    /* Verify data was actually computed on */
    float data_sum = 0.0f;
    for(int i = 0; i < ARRAY_SIZE; i++) {
        data_sum += data[i];
    }
    
    printf("\nChecksum: %ld\n", checksum);
    printf("Data sum: %f\n", data_sum);
    printf("All tests completed.\n");
    
    free(data);
    return 0;
}
