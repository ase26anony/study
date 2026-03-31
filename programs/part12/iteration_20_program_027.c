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
#define VECTOR_LENGTH 128

int main() {
    int results[NUM_CONFIGS][4] = {0}; // [gangs, workers, vector, checksum]
    int i, j, k;
    
    // Initialize data
    int data_size = 1024;
    int *host_data = (int*)malloc(data_size * sizeof(int));
    int *device_data = NULL;
    
    #pragma acc enter data create(host_data[0:data_size])
    
    // Configuration 0: gang redundant (all 1)
    printf("Testing configuration 0: gang redundant\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(host_data[0:data_size])
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
        
        // Do some work
        #pragma acc loop gang
        for (i = 0; i < g; i++) {
            #pragma acc loop worker
            for (j = 0; j < w; j++) {
                #pragma acc loop vector
                for (k = 0; k < v; k++) {
                    int idx = i * w * v + j * v + k;
                    if (idx < data_size) {
                        #pragma acc atomic update
                        host_data[idx] += 1;
                    }
                }
            }
        }
    }
    
    // Configuration 1: gang partitioned
    printf("Testing configuration 1: gang partitioned\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        present(host_data[0:data_size])
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
        
        // Nested parallelism with gang-only partitioning
        #pragma acc loop gang
        for (i = 0; i < 64; i++) {
            host_data[i % data_size] += i;
        }
    }
    
    // Configuration 2: worker partitioned
    printf("Testing configuration 2: worker partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        present(host_data[0:data_size])
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
        
        // Worker-level parallelism
        #pragma acc loop worker
        for (j = 0; j < 32; j++) {
            host_data[j % data_size] += j * 2;
        }
    }
    
    // Configuration 3: gang+worker partitioned
    printf("Testing configuration 3: gang+worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        present(host_data[0:data_size])
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
        
        // Two-level nested parallelism
        #pragma acc loop gang
        for (i = 0; i < g; i++) {
            #pragma acc loop worker
            for (j = 0; j < w; j++) {
                int idx = i * w + j;
                if (idx < data_size) {
                    host_data[idx] += idx * 3;
                }
            }
        }
    }
    
    // Configuration 4: vector partitioned
    printf("Testing configuration 4: vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        present(host_data[0:data_size])
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
        
        // Vector-level parallelism
        #pragma acc loop vector
        for (k = 0; k < 256; k++) {
            if (k < data_size) {
                host_data[k] += k * 4;
            }
        }
    }
    
    // Configuration 5: gang+vector partitioned
    printf("Testing configuration 5: gang+vector partitioned\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        present(host_data[0:data_size])
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
        
        // Gang and vector parallelism with collapse
        #pragma acc loop gang vector collapse(2)
        for (i = 0; i < 8; i++) {
            for (k = 0; k < 16; k++) {
                int idx = i * 16 + k;
                if (idx < data_size) {
                    host_data[idx] += idx * 5;
                }
            }
        }
    }
    
    // Configuration 6: worker+vector partitioned
    printf("Testing configuration 6: worker+vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        present(host_data[0:data_size])
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
        
        // Worker and vector parallelism
        #pragma acc loop worker
        for (j = 0; j < w; j++) {
            #pragma acc loop vector
            for (k = 0; k < v; k++) {
                int idx = j * v + k;
                if (idx < data_size) {
                    host_data[idx] += idx * 6;
                }
            }
        }
    }
    
    // Configuration 7: fully partitioned
    printf("Testing configuration 7: fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
        present(host_data[0:data_size])
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
        
        // Three-level nested parallelism
        #pragma acc loop gang
        for (i = 0; i < g; i++) {
            #pragma acc loop worker
            for (j = 0; j < w; j++) {
                #pragma acc loop vector
                for (k = 0; k < v; k++) {
                    int idx = i * w * v + j * v + k;
                    if (idx < data_size) {
                        host_data[idx] += idx * 7;
                    }
                }
            }
        }
    }
    
    // Dynamic configuration using runtime variables
    printf("Testing dynamic configurations\n");
    for (int iter = 0; iter < 3; iter++) {
        int dyn_gangs = 1 << iter;  // 1, 2, 4
        int dyn_workers = 2;
        int dyn_vector = 32;
        
        #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) \
            vector_length(dyn_vector) present(host_data[0:data_size])
        {
            // This creates additional partition patterns
            #pragma acc loop gang worker vector
            for (int idx = 0; idx < 100; idx++) {
                if (idx < data_size) {
                    host_data[idx] += idx * iter;
                }
            }
        }
    }
    
    // Use kernels directive for different code path
    printf("Testing kernels directive\n");
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(32) \
        present(host_data[0:data_size])
    {
        #pragma acc loop gang
        for (i = 0; i < 10; i++) {
            #pragma acc loop worker
            for (j = 0; j < 10; j++) {
                #pragma acc loop vector
                for (k = 0; k < 10; k++) {
                    int idx = i * 100 + j * 10 + k;
                    if (idx < data_size) {
                        host_data[idx] += 1;
                    }
                }
            }
        }
    }
    
    // Force potential diagnostic path with acc_malloc
    printf("Testing diagnostic path with acc_malloc\n");
    void *test_ptr = acc_malloc(1024);
    if (test_ptr == NULL) {
        printf("Warning: acc_malloc returned NULL\n");
    } else {
        acc_free(test_ptr);
    }
    
    // Update results to host
    #pragma acc update host(results[0:NUM_CONFIGS][0:4])
    #pragma acc update host(host_data[0:data_size])
    
    // Calculate checksums
    int total_gangs = 0, total_workers = 0, total_vector = 0;
    int data_checksum = 0;
    
    for (i = 0; i < NUM_CONFIGS; i++) {
        total_gangs += results[i][0];
        total_workers += results[i][1];
        total_vector += results[i][2];
        printf("Config %d: gangs=%d, workers=%d, vector=%d\n",
               i, results[i][0], results[i][1], results[i][2]);
    }
    
    for (i = 0; i < data_size; i++) {
        data_checksum += host_data[i];
    }
    
    printf("\nTotals: gangs=%d, workers=%d, vector=%d\n",
           total_gangs, total_workers, total_vector);
    printf("Data checksum: %d\n", data_checksum);
    
    // Cleanup
    #pragma acc exit data delete(host_data[0:data_size])
    free(host_data);
    
    return 0;
}
