/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128

int main() {
    int i, j, k;
    int results[NUM_CONFIGS][4] = {0}; // [gangs, workers, vector, checksum]
    int dynamic_gang = 4;
    int dynamic_worker = 2;
    int dynamic_vector = 64;
    
    // Initialize results array on host
    for (i = 0; i < NUM_CONFIGS; i++) {
        for (j = 0; j < 4; j++) {
            results[i][j] = 0;
        }
    }
    
    // Copy results to device
    #pragma acc enter data copyin(results[0:NUM_CONFIGS][0:4])
    
    printf("Testing OpenACC partition configurations...\n");
    
    // Configuration 0: gang redundant (all 1)
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(results[0:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[0][0] = g;
        #pragma acc atomic update
        results[0][1] = w;
        #pragma acc atomic update
        results[0][2] = v;
        #pragma acc atomic update
        results[0][3] += 1; // checksum
    }
    
    // Configuration 1: gang partitioned
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(results[1:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[1][0] = g;
        #pragma acc atomic update
        results[1][1] = w;
        #pragma acc atomic update
        results[1][2] = v;
        #pragma acc atomic update
        results[1][3] += 1;
    }
    
    // Configuration 2: worker partitioned
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(results[2:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[2][0] = g;
        #pragma acc atomic update
        results[2][1] = w;
        #pragma acc atomic update
        results[2][2] = v;
        #pragma acc atomic update
        results[2][3] += 1;
    }
    
    // Configuration 3: gang+worker partitioned
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(results[3:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[3][0] = g;
        #pragma acc atomic update
        results[3][1] = w;
        #pragma acc atomic update
        results[3][2] = v;
        #pragma acc atomic update
        results[3][3] += 1;
    }
    
    // Configuration 4: vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(results[4:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[4][0] = g;
        #pragma acc atomic update
        results[4][1] = w;
        #pragma acc atomic update
        results[4][2] = v;
        #pragma acc atomic update
        results[4][3] += 1;
    }
    
    // Configuration 5: gang+vector partitioned
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        copyout(results[5:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[5][0] = g;
        #pragma acc atomic update
        results[5][1] = w;
        #pragma acc atomic update
        results[5][2] = v;
        #pragma acc atomic update
        results[5][3] += 1;
    }
    
    // Configuration 6: worker+vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        copyout(results[6:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[6][0] = g;
        #pragma acc atomic update
        results[6][1] = w;
        #pragma acc atomic update
        results[6][2] = v;
        #pragma acc atomic update
        results[6][3] += 1;
    }
    
    // Configuration 7: fully partitioned
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(8) \
        copyout(results[7:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[7][0] = g;
        #pragma acc atomic update
        results[7][1] = w;
        #pragma acc atomic update
        results[7][2] = v;
        #pragma acc atomic update
        results[7][3] += 1;
    }
    
    // Dynamic configuration using host variables
    #pragma acc parallel num_gangs(dynamic_gang) num_workers(dynamic_worker) \
        vector_length(dynamic_vector) copyout(results[0:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[0][3] += g + w + v; // Additional checksum
    }
    
    // Nested parallelism example
    int nested_data[100] = {0};
    #pragma acc data copy(nested_data[0:100])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang
            for (i = 0; i < 4; i++) {
                #pragma acc loop worker
                for (j = 0; j < 25; j++) {
                    int idx = i * 25 + j;
                    if (idx < 100) {
                        #pragma acc loop vector
                        for (k = 0; k < 16; k++) {
                            // Simulate some work
                            if (k == 0) {
                                #pragma acc atomic update
                                nested_data[idx] += 1;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Collapse clause example
    int collapsed_data[256] = {0};
    #pragma acc data copy(collapsed_data[0:256])
    {
        #pragma acc parallel num_gangs(8) num_workers(2) vector_length(4)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 16; i++) {
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    collapsed_data[idx] = i + j;
                }
            }
        }
    }
    
    // Unstructured data region with compute
    int *device_ptr = NULL;
    #pragma acc enter data create(device_ptr[0:10])
    
    #pragma acc parallel present(device_ptr[0:10]) \
        num_gangs(2) num_workers(1) vector_length(1)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        #pragma acc loop gang
        for (i = 0; i < 2; i++) {
            if (acc_on_device(acc_device_not_host)) {
                device_ptr[i] = g;
            }
        }
    }
    
    #pragma acc exit data copyout(device_ptr[0:10]) delete(device_ptr[0:10])
    
    // Update results from device to host
    #pragma acc update host(results[0:NUM_CONFIGS][0:4])
    
    // Print results and calculate checksum
    int total_checksum = 0;
    printf("\nPartition configuration results:\n");
    printf("Config | Gangs | Workers | Vector | Checksum\n");
    printf("---------------------------------------------\n");
    
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("%6d | %6d | %7d | %6d | %8d\n", 
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
        total_checksum += results[i][0] + results[i][1] + results[i][2] + results[i][3];
    }
    
    // Add nested data checksum
    int nested_checksum = 0;
    for (i = 0; i < 100; i++) {
        nested_checksum += nested_data[i];
    }
    total_checksum += nested_checksum;
    
    // Add collapsed data checksum
    int collapsed_checksum = 0;
    for (i = 0; i < 256; i++) {
        collapsed_checksum += collapsed_data[i];
    }
    total_checksum += collapsed_checksum;
    
    // Add device_ptr checksum
    int ptr_checksum = 0;
    for (i = 0; i < 10; i++) {
        ptr_checksum += device_ptr[i];
    }
    total_checksum += ptr_checksum;
    
    printf("\nTotal checksum: %d\n", total_checksum);
    
    // Force potential diagnostic path with acc_malloc
    void *test_alloc = acc_malloc(1024);
    if (test_alloc == NULL) {
        printf("Warning: acc_malloc returned NULL\n");
    } else {
        acc_free(test_alloc);
    }
    
    // Clean up
    #pragma acc exit data delete(results[0:NUM_CONFIGS][0:4])
    
    if (total_checksum > 0) {
        printf("\nTest completed successfully.\n");
        return 0;
    } else {
        printf("\nTest failed - no computations performed.\n");
        return 1;
    }
}
