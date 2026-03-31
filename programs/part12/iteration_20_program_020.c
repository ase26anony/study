/* Test for OpenACC partition mapping coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* For diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

int main() {
    int i, j, k;
    int results[NUM_CONFIGS][4] = {0}; /* [gangs, workers, vector, checksum] */
    int *d_results = NULL;
    int dynamic_gang_cnt = 4;
    int dynamic_worker_cnt = 2;
    int dynamic_vector_len = 64;
    
    /* Initialize host array */
    for (i = 0; i < NUM_CONFIGS; i++) {
        for (j = 0; j < 4; j++) {
            results[i][j] = 0;
        }
    }
    
    /* Allocate device memory for results */
    d_results = (int*)acc_malloc(NUM_CONFIGS * 4 * sizeof(int));
    if (d_results == NULL) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic path\n");
        return 1;
    }
    
    /* Copy initial values to device */
    #pragma acc enter data copyin(results[0:NUM_CONFIGS][0:4])
    
    /* Configuration 0: gang redundant (all 1s) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            results[0][0] = g;
            results[0][1] = w;
            results[0][2] = v;
            results[0][3] = g + w + v; /* checksum */
        }
    }
    
    /* Configuration 1: gang partitioned */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            results[1][0] = g;
            results[1][1] = w;
            results[1][2] = v;
            results[1][3] = g + w + v;
        }
    }
    
    /* Configuration 2: worker partitioned */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            results[2][0] = g;
            results[2][1] = w;
            results[2][2] = v;
            results[2][3] = g + w + v;
        }
    }
    
    /* Configuration 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            results[3][0] = g;
            results[3][1] = w;
            results[3][2] = v;
            results[3][3] = g + w + v;
        }
    }
    
    /* Configuration 4: vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(VECTOR_LENGTH) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            results[4][0] = g;
            results[4][1] = w;
            results[4][2] = v;
            results[4][3] = g + w + v;
        }
    }
    
    /* Configuration 5: gang+vector partitioned */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            results[5][0] = g;
            results[5][1] = w;
            results[5][2] = v;
            results[5][3] = g + w + v;
        }
    }
    
    /* Configuration 6: worker+vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            results[6][0] = g;
            results[6][1] = w;
            results[6][2] = v;
            results[6][3] = g + w + v;
        }
    }
    
    /* Configuration 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            results[7][0] = g;
            results[7][1] = w;
            results[7][2] = v;
            results[7][3] = g + w + v;
        }
    }
    
    /* Dynamic configuration using host variables */
    #pragma acc parallel num_gangs(dynamic_gang_cnt) \
        num_workers(dynamic_worker_cnt) vector_length(dynamic_vector_len) \
        present(results[0:NUM_CONFIGS][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        if (acc_on_device(acc_device_not_host)) {
            /* Store in results[0] to overwrite - shows dynamic behavior */
            results[0][0] += g;
            results[0][1] += w;
            results[0][2] += v;
            results[0][3] += g + w + v;
        }
    }
    
    /* Nested parallelism example with collapse */
    int data[1000];
    #pragma acc data copy(data[0:1000])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (j = 0; j < 10; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < 10; k++) {
                        int idx = i*100 + j*10 + k;
                        if (idx < 1000) {
                            data[idx] = i + j + k;
                        }
                    }
                }
            }
        }
        
        /* Collapsed loop with mixed partitioning */
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(16)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 20; i++) {
                for (j = 0; j < 20; j++) {
                    int idx = i*20 + j;
                    if (idx < 1000) {
                        data[idx] += 1;
                    }
                }
            }
        }
    }
    
    /* Unstructured data region with compute */
    int *unstructured_data = (int*)malloc(100 * sizeof(int));
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
    
    /* Copy results back to host */
    #pragma acc update host(results[0:NUM_CONFIGS][0:4])
    
    /* Print results and calculate checksum */
    int total_checksum = 0;
    printf("Partition configuration results:\n");
    printf("Config | Gangs | Workers | Vector | Checksum\n");
    printf("-------|-------|---------|--------|----------\n");
    
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("%6d | %5d | %7d | %6d | %8d\n", 
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
        total_checksum += results[i][3];
    }
    
    printf("\nTotal checksum: %d\n", total_checksum);
    
    /* Verify unstructured data */
    int unstructured_sum = 0;
    for (i = 0; i < 100; i++) {
        unstructured_sum += unstructured_data[i];
    }
    printf("Unstructured data sum: %d\n", unstructured_sum);
    
    /* Cleanup */
    free(unstructured_data);
    acc_free(d_results);
    
    /* Final check - trigger potential diagnostic with invalid acc_malloc */
    void *test_ptr = acc_malloc(0);  /* Zero-size allocation might trigger edge case */
    if (test_ptr == NULL) {
        printf("Zero-size acc_malloc returned NULL (expected)\n");
    }
    
    return 0;
}
