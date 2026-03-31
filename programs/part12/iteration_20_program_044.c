/* Test for OpenACC partition mapping coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

int main() {
    int i, j, k;
    int config_results[NUM_CONFIGS][4] = {0}; // [gangs, workers, vector, checksum]
    int *d_results = NULL;
    int host_checksum = 0;
    
    // Allocate device memory for results
    d_results = (int*)acc_malloc(NUM_CONFIGS * 4 * sizeof(int));
    if (d_results == NULL) {
        fprintf(stderr, "acc_malloc failed\n");
        return 1;
    }
    
    // Initialize device results to zero
    #pragma acc parallel deviceptr(d_results)
    {
        for (int idx = 0; idx < NUM_CONFIGS * 4; idx++) {
            d_results[idx] = 0;
        }
    }
    
    printf("Testing OpenACC partition configurations...\n");
    
    // Configuration 0: gang redundant (all 1s)
    {
        int config_id = 0;
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                    copyin(config_id) deviceptr(d_results)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            int idx = config_id * 4;
            d_results[idx] = g;
            d_results[idx + 1] = w;
            d_results[idx + 2] = v;
            d_results[idx + 3] = g + w + v; // checksum
        }
    }
    
    // Configuration 1: gang partitioned
    {
        int config_id = 1;
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                    copyin(config_id) deviceptr(d_results)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            int idx = config_id * 4;
            d_results[idx] = g;
            d_results[idx + 1] = w;
            d_results[idx + 2] = v;
            d_results[idx + 3] = g + w + v;
        }
    }
    
    // Configuration 2: worker partitioned
    {
        int config_id = 2;
        #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                    copyin(config_id) deviceptr(d_results)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            int idx = config_id * 4;
            d_results[idx] = g;
            d_results[idx + 1] = w;
            d_results[idx + 2] = v;
            d_results[idx + 3] = g + w + v;
        }
    }
    
    // Configuration 3: gang+worker partitioned
    {
        int config_id = 3;
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                    copyin(config_id) deviceptr(d_results)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            int idx = config_id * 4;
            d_results[idx] = g;
            d_results[idx + 1] = w;
            d_results[idx + 2] = v;
            d_results[idx + 3] = g + w + v;
        }
    }
    
    // Configuration 4: vector partitioned
    {
        int config_id = 4;
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                    copyin(config_id) deviceptr(d_results)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            int idx = config_id * 4;
            d_results[idx] = g;
            d_results[idx + 1] = w;
            d_results[idx + 2] = v;
            d_results[idx + 3] = g + w + v;
        }
    }
    
    // Configuration 5: gang+vector partitioned
    {
        int config_id = 5;
        #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                    copyin(config_id) deviceptr(d_results)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            int idx = config_id * 4;
            d_results[idx] = g;
            d_results[idx + 1] = w;
            d_results[idx + 2] = v;
            d_results[idx + 3] = g + w + v;
        }
    }
    
    // Configuration 6: worker+vector partitioned
    {
        int config_id = 6;
        #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
                    copyin(config_id) deviceptr(d_results)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            int idx = config_id * 4;
            d_results[idx] = g;
            d_results[idx + 1] = w;
            d_results[idx + 2] = v;
            d_results[idx + 3] = g + w + v;
        }
    }
    
    // Configuration 7: fully partitioned
    {
        int config_id = 7;
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                    copyin(config_id) deviceptr(d_results)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            int idx = config_id * 4;
            d_results[idx] = g;
            d_results[idx + 1] = w;
            d_results[idx + 2] = v;
            d_results[idx + 3] = g + w + v;
        }
    }
    
    // Copy results back from device
    #pragma acc update host(d_results[0:NUM_CONFIGS*4])
    
    // Process results
    for (i = 0; i < NUM_CONFIGS; i++) {
        for (j = 0; j < 4; j++) {
            config_results[i][j] = d_results[i * 4 + j];
        }
        host_checksum += config_results[i][3];
    }
    
    // Test nested parallelism with collapse clause
    {
        int data[1000] = {0};
        int sum = 0;
        
        #pragma acc data copy(data[0:1000])
        {
            // Nested parallelism with gang, worker, vector
            #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
                        copyout(sum)
            {
                int local_sum = 0;
                
                #pragma acc loop gang reduction(+:local_sum)
                for (i = 0; i < 10; i++) {
                    #pragma acc loop worker
                    for (j = 0; j < 10; j++) {
                        #pragma acc loop vector
                        for (k = 0; k < 10; k++) {
                            int idx = i * 100 + j * 10 + k;
                            if (idx < 1000) {
                                local_sum += idx;
                            }
                        }
                    }
                }
                sum = local_sum;
            }
            
            // Another region with collapse
            #pragma acc parallel num_gangs(8) vector_length(64)
            {
                #pragma acc loop gang vector collapse(2)
                for (i = 0; i < 20; i++) {
                    for (j = 0; j < 50; j++) {
                        int idx = i * 50 + j;
                        if (idx < 1000) {
                            data[idx] = idx * 2;
                        }
                    }
                }
            }
        }
        
        host_checksum += sum;
    }
    
    // Test dynamic configuration using host variables
    {
        int dynamic_gangs = 3;
        int dynamic_workers = 5;
        int dynamic_vector = 8;
        int dynamic_result[3] = {0};
        
        #pragma acc parallel num_gangs(dynamic_gangs) \
                    num_workers(dynamic_workers) \
                    vector_length(dynamic_vector) \
                    copyout(dynamic_result[0:3])
        {
            dynamic_result[0] = acc_get_num_gangs(acc_async_noval);
            dynamic_result[1] = acc_get_num_workers(acc_async_noval);
            dynamic_result[2] = acc_get_vector_length(acc_async_noval);
        }
        
        host_checksum += dynamic_result[0] + dynamic_result[1] + dynamic_result[2];
    }
    
    // Test kernels directive with different partitions
    {
        float array[256];
        float result = 0.0f;
        
        #pragma acc data copy(array[0:256])
        {
            #pragma acc kernels num_gangs(4) num_workers(1) vector_length(32)
            {
                #pragma acc loop gang
                for (i = 0; i < 4; i++) {
                    #pragma acc loop vector
                    for (j = 0; j < 64; j++) {
                        array[i * 64 + j] = (float)(i * 64 + j);
                    }
                }
            }
            
            #pragma acc kernels num_gangs(1) num_workers(2) vector_length(16)
            {
                #pragma acc loop worker
                for (i = 0; i < 2; i++) {
                    #pragma acc loop vector
                    for (j = 0; j < 128; j++) {
                        int idx = i * 128 + j;
                        if (idx < 256) {
                            result += array[idx];
                        }
                    }
                }
            }
        }
        
        host_checksum += (int)result;
    }
    
    // Test unstructured data regions
    {
        int *unstructured_data = NULL;
        int unstructured_size = 1024;
        
        unstructured_data = (int*)acc_malloc(unstructured_size * sizeof(int));
        if (unstructured_data != NULL) {
            #pragma acc enter data copyin(unstructured_size)
            
            #pragma acc parallel present(unstructured_data[0:unstructured_size]) \
                        num_gangs(8) num_workers(1) vector_length(1)
            {
                #pragma acc loop gang
                for (i = 0; i < unstructured_size; i++) {
                    unstructured_data[i] = i * 3;
                }
            }
            
            int verify_sum = 0;
            #pragma acc parallel present(unstructured_data[0:unstructured_size]) \
                        num_gangs(1) num_workers(4) vector_length(32) \
                        reduction(+:verify_sum)
            {
                #pragma acc loop worker vector
                for (i = 0; i < unstructured_size; i++) {
                    verify_sum += unstructured_data[i];
                }
            }
            
            host_checksum += verify_sum;
            
            #pragma acc exit data delete(unstructured_data[0:unstructured_size])
            acc_free(unstructured_data);
        }
    }
    
    // Print summary
    printf("Configuration results:\n");
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vector=%d, checksum=%d\n",
               i, config_results[i][0], config_results[i][1], 
               config_results[i][2], config_results[i][3]);
    }
    
    printf("Total host checksum: %d\n", host_checksum);
    
    // Cleanup
    acc_free(d_results);
    
    // Force potential diagnostic output by checking device type
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", (int)dev_type);
    
    return 0;
}
