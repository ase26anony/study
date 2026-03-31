/* Test case for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128

int main() {
    int i, j, k;
    int config_results[NUM_CONFIGS][4] = {0}; // [gangs, workers, vectors, checksum]
    int *d_results = NULL;
    int host_checksum = 0;
    
    // Allocate device memory for results
    d_results = (int*)acc_malloc(NUM_CONFIGS * 4 * sizeof(int));
    if (d_results == NULL) {
        fprintf(stderr, "acc_malloc failed - this may trigger diagnostic paths\n");
        return 1;
    }
    
    // Initialize device results to zero
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        for (int idx = 0; idx < NUM_CONFIGS * 4; idx++) {
            d_results[idx] = 0;
        }
    }
    
    // Configuration 0: gang redundant (all 1s)
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyin(config_results[0][0:4]) present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        d_results[0] = g;
        d_results[1] = w;
        d_results[2] = v;
        d_results[3] = g + w + v; // checksum
    }
    
    // Configuration 1: gang partitioned
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        d_results[4] = g;
        d_results[5] = w;
        d_results[6] = v;
        d_results[7] = g + w + v;
    }
    
    // Configuration 2: worker partitioned
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        d_results[8] = g;
        d_results[9] = w;
        d_results[10] = v;
        d_results[11] = g + w + v;
    }
    
    // Configuration 3: gang+worker partitioned
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        d_results[12] = g;
        d_results[13] = w;
        d_results[14] = v;
        d_results[15] = g + w + v;
    }
    
    // Configuration 4: vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        d_results[16] = g;
        d_results[17] = w;
        d_results[18] = v;
        d_results[19] = g + w + v;
    }
    
    // Configuration 5: gang+vector partitioned
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        d_results[20] = g;
        d_results[21] = w;
        d_results[22] = v;
        d_results[23] = g + w + v;
    }
    
    // Configuration 6: worker+vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        d_results[24] = g;
        d_results[25] = w;
        d_results[26] = v;
        d_results[27] = g + w + v;
    }
    
    // Configuration 7: fully partitioned
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(16) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        d_results[28] = g;
        d_results[29] = w;
        d_results[30] = v;
        d_results[31] = g + w + v;
    }
    
    // Copy results back to host
    #pragma acc update host(d_results[0:NUM_CONFIGS*4])
    
    // Process results and calculate checksum
    for (i = 0; i < NUM_CONFIGS; i++) {
        for (j = 0; j < 4; j++) {
            config_results[i][j] = d_results[i*4 + j];
        }
        host_checksum += config_results[i][3]; // Sum all checksums
    }
    
    // Test with dynamic values (runtime variables)
    int dyn_gangs = 3;
    int dyn_workers = 5;
    int dyn_vector = 8;
    
    #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) vector_length(dyn_vector) \
        present(d_results[0:NUM_CONFIGS*4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        // Store in a different location to avoid overwriting
        if (acc_on_device(acc_device_not_host)) {
            d_results[0] += g;  // Add to existing value
            d_results[1] += w;
            d_results[2] += v;
        }
    }
    
    // Test nested parallelism with collapse clause
    int data[1000] = {0};
    
    #pragma acc data copy(data[0:1000])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 10; j++) {
                    int idx = i * 10 + j;
                    if (idx < 1000) {
                        data[idx] = acc_get_num_gangs(acc_async_noval) + 
                                   acc_get_num_workers(acc_async_noval) +
                                   acc_get_vector_length(acc_async_noval);
                    }
                }
            }
        }
    }
    
    // Test kernels directive with automatic partitioning
    int kernels_data[500] = {0};
    
    #pragma acc data copy(kernels_data[0:500])
    {
        #pragma acc kernels num_gangs(8) num_workers(2) vector_length(64)
        {
            #pragma acc loop gang
            for (i = 0; i < 8; i++) {
                #pragma acc loop worker
                for (j = 0; j < 2; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < 64; k++) {
                        int idx = i * 128 + j * 64 + k;
                        if (idx < 500) {
                            kernels_data[idx] = i * 100 + j * 10 + k;
                        }
                    }
                }
            }
        }
    }
    
    // Verify data was processed
    int data_sum = 0;
    for (i = 0; i < 500; i++) {
        data_sum += kernels_data[i];
    }
    host_checksum += data_sum;
    
    // Test unstructured data regions
    int *unstructured_data = NULL;
    unstructured_data = (int*)acc_malloc(100 * sizeof(int));
    
    if (unstructured_data != NULL) {
        #pragma acc enter data copyin(unstructured_data[0:100])
        
        #pragma acc parallel present(unstructured_data[0:100]) \
            num_gangs(2) num_workers(4) vector_length(8)
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < 100; i++) {
                unstructured_data[i] = i * 2;
            }
        }
        
        #pragma acc exit data copyout(unstructured_data[0:100])
        
        // Verify
        for (i = 0; i < 100; i++) {
            host_checksum += unstructured_data[i];
        }
        
        acc_free(unstructured_data);
    }
    
    // Print final checksum to ensure no optimization removed computations
    printf("Final checksum: %d\n", host_checksum);
    printf("Partition configurations tested: %d\n", NUM_CONFIGS);
    
    // Print some configuration results for verification
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vectors=%d, checksum=%d\n",
               i, config_results[i][0], config_results[i][1], 
               config_results[i][2], config_results[i][3]);
    }
    
    acc_free(d_results);
    
    return 0;
}
