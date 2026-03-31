/* Test case for OpenACC partition mapping coverage
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128
#define ARRAY_SIZE 1024

int main() {
    int i, j, config;
    int results[NUM_CONFIGS][4] = {0}; // [gangs, workers, vectors, checksum]
    int *device_results;
    int *data_array;
    int host_checksum = 0;
    
    // Allocate device memory for results
    device_results = (int*)acc_malloc(NUM_CONFIGS * 4 * sizeof(int));
    if (!device_results) {
        fprintf(stderr, "acc_malloc failed for device_results\n");
        return 1;
    }
    
    // Initialize device results to zero
    #pragma acc enter data copyin(results[0:NUM_CONFIGS][0:4])
    
    // Allocate and initialize data array
    data_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data_array[i] = i % 100;
    }
    
    #pragma acc enter data copyin(data_array[0:ARRAY_SIZE])
    
    printf("Testing OpenACC partition configurations...\n");
    
    // Configuration 0: gang redundant (all 1s)
    config = 0;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copy(device_results[0:NUM_CONFIGS*4]) \
        present(data_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            device_results[config*4 + 0] = g;
            device_results[config*4 + 1] = w;
            device_results[config*4 + 2] = v;
            
            // Simple computation to ensure region isn't optimized away
            #pragma acc loop gang
            for (i = 0; i < g; i++) {
                device_results[config*4 + 3] += i;
            }
        }
    }
    
    // Configuration 1: gang partitioned
    config = 1;
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copy(device_results[0:NUM_CONFIGS*4]) \
        present(data_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            device_results[config*4 + 0] = g;
            device_results[config*4 + 1] = w;
            device_results[config*4 + 2] = v;
            
            #pragma acc loop gang
            for (i = 0; i < ARRAY_SIZE; i++) {
                if (i % g == acc_get_gang_num()) {
                    device_results[config*4 + 3] += data_array[i];
                }
            }
        }
    }
    
    // Configuration 2: worker partitioned
    config = 2;
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copy(device_results[0:NUM_CONFIGS*4]) \
        present(data_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            device_results[config*4 + 0] = g;
            device_results[config*4 + 1] = w;
            device_results[config*4 + 2] = v;
            
            #pragma acc loop worker
            for (i = 0; i < ARRAY_SIZE; i++) {
                if (i % w == acc_get_worker_num()) {
                    device_results[config*4 + 3] += data_array[i];
                }
            }
        }
    }
    
    // Configuration 3: gang+worker partitioned
    config = 3;
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copy(device_results[0:NUM_CONFIGS*4]) \
        present(data_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            device_results[config*4 + 0] = g;
            device_results[config*4 + 1] = w;
            device_results[config*4 + 2] = v;
            
            #pragma acc loop gang worker
            for (i = 0; i < ARRAY_SIZE; i++) {
                int gang_id = acc_get_gang_num();
                int worker_id = acc_get_worker_num();
                if ((i % g == gang_id) && ((i / g) % w == worker_id)) {
                    device_results[config*4 + 3] += data_array[i];
                }
            }
        }
    }
    
    // Configuration 4: vector partitioned
    config = 4;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copy(device_results[0:NUM_CONFIGS*4]) \
        present(data_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            device_results[config*4 + 0] = g;
            device_results[config*4 + 1] = w;
            device_results[config*4 + 2] = v;
            
            #pragma acc loop vector
            for (i = 0; i < ARRAY_SIZE; i++) {
                if (i % v == acc_get_vector_num()) {
                    device_results[config*4 + 3] += data_array[i];
                }
            }
        }
    }
    
    // Configuration 5: gang+vector partitioned (nested parallelism)
    config = 5;
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copy(device_results[0:NUM_CONFIGS*4]) \
        present(data_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            device_results[config*4 + 0] = g;
            device_results[config*4 + 1] = w;
            device_results[config*4 + 2] = v;
            
            #pragma acc loop gang
            for (i = 0; i < g; i++) {
                #pragma acc loop vector
                for (j = 0; j < v; j++) {
                    int idx = i * v + j;
                    if (idx < ARRAY_SIZE) {
                        device_results[config*4 + 3] += data_array[idx];
                    }
                }
            }
        }
    }
    
    // Configuration 6: worker+vector partitioned
    config = 6;
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
        copy(device_results[0:NUM_CONFIGS*4]) \
        present(data_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            device_results[config*4 + 0] = g;
            device_results[config*4 + 1] = w;
            device_results[config*4 + 2] = v;
            
            #pragma acc loop worker
            for (i = 0; i < w; i++) {
                #pragma acc loop vector
                for (j = 0; j < v; j++) {
                    int idx = i * v + j;
                    if (idx < ARRAY_SIZE) {
                        device_results[config*4 + 3] += data_array[idx];
                    }
                }
            }
        }
    }
    
    // Configuration 7: fully partitioned (all dimensions > 1)
    config = 7;
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16) \
        copy(device_results[0:NUM_CONFIGS*4]) \
        present(data_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            device_results[config*4 + 0] = g;
            device_results[config*4 + 1] = w;
            device_results[config*4 + 2] = v;
            
            #pragma acc loop gang worker vector collapse(3)
            for (int gi = 0; gi < g; gi++) {
                for (int wi = 0; wi < w; wi++) {
                    for (int vi = 0; vi < v; vi++) {
                        int idx = (gi * w * v) + (wi * v) + vi;
                        if (idx < ARRAY_SIZE) {
                            device_results[config*4 + 3] += data_array[idx];
                        }
                    }
                }
            }
        }
    }
    
    // Dynamic configuration using runtime variables
    printf("\nTesting dynamic configurations...\n");
    for (int dynamic_gangs = 1; dynamic_gangs <= 4; dynamic_gangs *= 2) {
        int dynamic_workers = 8 / dynamic_gangs;
        int dynamic_vector = 32;
        
        #pragma acc parallel num_gangs(dynamic_gangs) \
                            num_workers(dynamic_workers) \
                            vector_length(dynamic_vector) \
                 present(data_array[0:ARRAY_SIZE])
        {
            // Force computation with dynamic values
            #pragma acc loop gang worker vector
            for (i = 0; i < ARRAY_SIZE; i++) {
                data_array[i] = data_array[i] * 2 - data_array[i];
            }
        }
    }
    
    // Test with kernels construct (different code path)
    printf("\nTesting kernels construct...\n");
    #pragma acc kernels num_gangs(4) num_workers(2) vector_length(32) \
        copy(data_array[0:ARRAY_SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            data_array[i] = data_array[i] + 1;
        }
    }
    
    // Copy results back from device
    #pragma acc update host(results[0:NUM_CONFIGS][0:4])
    
    // Print results and calculate checksum
    printf("\nPartition Configuration Results:\n");
    printf("Config | Gangs | Workers | Vectors | Checksum\n");
    printf("-------|-------|---------|---------|----------\n");
    
    for (config = 0; config < NUM_CONFIGS; config++) {
        printf("%6d | %5d | %7d | %7d | %8d\n",
               config,
               results[config][0],
               results[config][1],
               results[config][2],
               results[config][3]);
        host_checksum += results[config][0] + results[config][1] + 
                        results[config][2] + results[config][3];
    }
    
    printf("\nTotal checksum: %d\n", host_checksum);
    
    // Test diagnostic path with acc_malloc error check
    printf("\nTesting diagnostic path...\n");
    void *test_alloc = acc_malloc(0);  // Zero-size allocation might fail
    if (!test_alloc) {
        printf("Note: acc_malloc(0) returned NULL (expected for diagnostics)\n");
    } else {
        acc_free(test_alloc);
    }
    
    // Cleanup
    #pragma acc exit data delete(data_array[0:ARRAY_SIZE])
    #pragma acc exit data delete(results[0:NUM_CONFIGS][0:4])
    acc_free(device_results);
    free(data_array);
    
    printf("\nTest completed successfully.\n");
    return 0;
}
