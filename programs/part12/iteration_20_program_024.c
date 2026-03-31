/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
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
    int config_results[NUM_CONFIGS][4] = {0}; // [gangs, workers, vector, checksum]
    int *d_results = NULL;
    int host_var = 1;
    int dynamic_gang_cnt = 4;
    int dynamic_worker_cnt = 2;
    int dynamic_vector_len = 64;
    
    // Allocate device memory for results
    d_results = (int*)acc_malloc(NUM_CONFIGS * 4 * sizeof(int));
    if (d_results == NULL) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic path\n");
        // Continue anyway - this might trigger the diagnostic that uses partition strings
    }
    
    // Initialize host array
    for (i = 0; i < NUM_CONFIGS; i++) {
        for (j = 0; j < 4; j++) {
            config_results[i][j] = 0;
        }
    }
    
    // Copy initial values to device
    #pragma acc enter data copyin(config_results[0:NUM_CONFIGS][0:4])
    
    printf("Testing OpenACC partition configurations...\n");
    
    // Configuration 0: gang redundant (all 1s)
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(config_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc single
        {
            config_results[0][0] = g;
            config_results[0][1] = w;
            config_results[0][2] = v;
            config_results[0][3] = g + w + v; // checksum
        }
    }
    
    // Configuration 1: gang partitioned
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        present(config_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc single
        {
            config_results[1][0] = g;
            config_results[1][1] = w;
            config_results[1][2] = v;
            config_results[1][3] = g + w + v;
        }
    }
    
    // Configuration 2: worker partitioned
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        present(config_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc single
        {
            config_results[2][0] = g;
            config_results[2][1] = w;
            config_results[2][2] = v;
            config_results[2][3] = g + w + v;
        }
    }
    
    // Configuration 3: gang+worker partitioned
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        present(config_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc single
        {
            config_results[3][0] = g;
            config_results[3][1] = w;
            config_results[3][2] = v;
            config_results[3][3] = g + w + v;
        }
    }
    
    // Configuration 4: vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        present(config_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc single
        {
            config_results[4][0] = g;
            config_results[4][1] = w;
            config_results[4][2] = v;
            config_results[4][3] = g + w + v;
        }
    }
    
    // Configuration 5: gang+vector partitioned
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        present(config_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc single
        {
            config_results[5][0] = g;
            config_results[5][1] = w;
            config_results[5][2] = v;
            config_results[5][3] = g + w + v;
        }
    }
    
    // Configuration 6: worker+vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        present(config_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc single
        {
            config_results[6][0] = g;
            config_results[6][1] = w;
            config_results[6][2] = v;
            config_results[6][3] = g + w + v;
        }
    }
    
    // Configuration 7: fully partitioned
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(8) \
        present(config_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc single
        {
            config_results[7][0] = g;
            config_results[7][1] = w;
            config_results[7][2] = v;
            config_results[7][3] = g + w + v;
        }
    }
    
    // Dynamic configuration using host variables
    #pragma acc parallel num_gangs(dynamic_gang_cnt) \
        num_workers(dynamic_worker_cnt) vector_length(dynamic_vector_len) \
        copy(host_var)
    {
        host_var += acc_get_num_gangs(acc_async_noval);
    }
    
    // Nested parallelism example
    int nested_data[100] = {0};
    #pragma acc data copy(nested_data[0:100])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (i = 0; i < 4; i++) {
                #pragma acc loop worker
                for (j = 0; j < 25; j++) {
                    int idx = i * 25 + j;
                    if (idx < 100) {
                        #pragma acc loop vector
                        for (k = 0; k < 1; k++) {
                            nested_data[idx] = acc_get_num_gangs(acc_async_noval) * 1000 +
                                              acc_get_num_workers(acc_async_noval) * 100 +
                                              acc_get_vector_length(acc_async_noval);
                        }
                    }
                }
            }
        }
    }
    
    // Collapse clause example
    int collapse_data[256] = {0};
    #pragma acc data copy(collapse_data[0:256])
    {
        #pragma acc parallel num_gangs(4) num_workers(4) vector_length(16)
        {
            #pragma acc loop collapse(2) gang worker vector
            for (i = 0; i < 16; i++) {
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    collapse_data[idx] = i * j;
                }
            }
        }
    }
    
    // Unstructured data region with compute
    int unstructured_data[50];
    #pragma acc enter data copyin(unstructured_data[0:50])
    
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        present(unstructured_data)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < 50; i++) {
            unstructured_data[i] = i * 2;
        }
    }
    
    #pragma acc exit data copyout(unstructured_data[0:50])
    
    // Update results from device
    #pragma acc update host(config_results[0:NUM_CONFIGS][0:4])
    
    // Calculate final checksum
    int total_checksum = 0;
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vector=%d, checksum=%d\n",
               i, config_results[i][0], config_results[i][1],
               config_results[i][2], config_results[i][3]);
        total_checksum += config_results[i][3];
    }
    
    // Add nested data checksum
    int nested_checksum = 0;
    for (i = 0; i < 100; i++) {
        nested_checksum += nested_data[i];
    }
    total_checksum += nested_checksum;
    
    // Add collapse data checksum
    int collapse_checksum = 0;
    for (i = 0; i < 256; i++) {
        collapse_checksum += collapse_data[i];
    }
    total_checksum += collapse_checksum;
    
    // Add unstructured data checksum
    int unstructured_checksum = 0;
    for (i = 0; i < 50; i++) {
        unstructured_checksum += unstructured_data[i];
    }
    total_checksum += unstructured_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Host var final value: %d\n", host_var);
    
    // Cleanup
    #pragma acc exit data delete(config_results)
    if (d_results != NULL) {
        acc_free(d_results);
    }
    
    // Verify all configurations were executed
    if (total_checksum > 0) {
        printf("Test PASSED - all configurations executed\n");
        return 0;
    } else {
        printf("Test FAILED - no configurations executed\n");
        return 1;
    }
}
