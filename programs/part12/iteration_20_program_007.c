/* Test for OpenACC partition mapping coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128
#define ARRAY_SIZE 1024

int main() {
    int i, j, config;
    int results[NUM_CONFIGS][4] = {0};  // [gangs, workers, vector, checksum]
    int host_array[ARRAY_SIZE];
    int *device_array;
    int checksum = 0;
    
    // Initialize host array
    for (i = 0; i < ARRAY_SIZE; i++) {
        host_array[i] = i % 100;
    }
    
    // Allocate device memory
    device_array = (int*)acc_malloc(ARRAY_SIZE * sizeof(int));
    if (device_array == NULL) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic path\n");
        // Continue anyway to test other paths
    }
    
    // Copy data to device
    #pragma acc enter data copyin(host_array[0:ARRAY_SIZE])
    
    printf("Testing OpenACC partition configurations...\n");
    
    // Configuration 0: gang redundant (all 1s)
    config = 0;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyout(results[config][0:4]) present(host_array)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[config][0] = acc_get_num_gangs(acc_async_noval);
        results[config][1] = acc_get_num_workers(acc_async_noval);
        results[config][2] = acc_get_vector_length(acc_async_noval);
        
        // Simple computation to prevent optimization
        if (gid == 0 && wid == 0 && vid == 0) {
            int sum = 0;
            #pragma acc loop gang worker vector reduction(+:sum)
            for (int idx = 0; idx < 64; idx++) {
                sum += idx % 16;
            }
            results[config][3] = sum;
        }
    }
    
    // Configuration 1: gang partitioned
    config = 1;
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyout(results[config][0:4]) present(host_array)
    {
        int gid = __pgi_gangidx();
        results[config][0] = acc_get_num_gangs(acc_async_noval);
        results[config][1] = acc_get_num_workers(acc_async_noval);
        results[config][2] = acc_get_vector_length(acc_async_noval);
        
        // Gang-level work distribution
        #pragma acc loop gang
        for (i = 0; i < 8; i++) {
            if (gid == i) {
                results[config][3] += i * 10;
            }
        }
    }
    
    // Configuration 2: worker partitioned
    config = 2;
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyout(results[config][0:4]) present(host_array)
    {
        int wid = __pgi_workeridx();
        results[config][0] = acc_get_num_gangs(acc_async_noval);
        results[config][1] = acc_get_num_workers(acc_async_noval);
        results[config][2] = acc_get_vector_length(acc_async_noval);
        
        // Worker-level work
        #pragma acc loop worker
        for (i = 0; i < 4; i++) {
            if (wid == i) {
                results[config][3] += i * 5;
            }
        }
    }
    
    // Configuration 3: gang+worker partitioned
    config = 3;
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyout(results[config][0:4]) present(host_array)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        results[config][0] = acc_get_num_gangs(acc_async_noval);
        results[config][1] = acc_get_num_workers(acc_async_noval);
        results[config][2] = acc_get_vector_length(acc_async_noval);
        
        // Nested gang-worker parallelism
        #pragma acc loop gang
        for (i = 0; i < 2; i++) {
            #pragma acc loop worker
            for (j = 0; j < 2; j++) {
                if (gid == i && wid == j) {
                    results[config][3] += i * 10 + j * 5;
                }
            }
        }
    }
    
    // Configuration 4: vector partitioned
    config = 4;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(64) \
                copyout(results[config][0:4]) present(host_array)
    {
        int vid = __pgi_vectoridx();
        results[config][0] = acc_get_num_gangs(acc_async_noval);
        results[config][1] = acc_get_num_workers(acc_async_noval);
        results[config][2] = acc_get_vector_length(acc_async_noval);
        
        // Vector-level work
        #pragma acc loop vector
        for (i = 0; i < 64; i++) {
            if (vid == i % 64) {
                results[config][3] += i % 8;
            }
        }
    }
    
    // Configuration 5: gang+vector partitioned
    config = 5;
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
                copyout(results[config][0:4]) present(host_array)
    {
        int gid = __pgi_gangidx();
        int vid = __pgi_vectoridx();
        results[config][0] = acc_get_num_gangs(acc_async_noval);
        results[config][1] = acc_get_num_workers(acc_async_noval);
        results[config][2] = acc_get_vector_length(acc_async_noval);
        
        // Mixed gang-vector parallelism
        #pragma acc loop gang vector
        for (i = 0; i < 128; i++) {
            if (gid == (i / 32) % 4 && vid == i % 32) {
                results[config][3] += (i % 16);
            }
        }
    }
    
    // Configuration 6: worker+vector partitioned
    config = 6;
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(16) \
                copyout(results[config][0:4]) present(host_array)
    {
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        results[config][0] = acc_get_num_gangs(acc_async_noval);
        results[config][1] = acc_get_num_workers(acc_async_noval);
        results[config][2] = acc_get_vector_length(acc_async_noval);
        
        // Worker-vector nested loops
        #pragma acc loop worker
        for (i = 0; i < 2; i++) {
            #pragma acc loop vector
            for (j = 0; j < 16; j++) {
                if (wid == i && vid == j) {
                    results[config][3] += i * 8 + j;
                }
            }
        }
    }
    
    // Configuration 7: fully partitioned
    config = 7;
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(8) \
                copyout(results[config][0:4]) present(host_array)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        results[config][0] = acc_get_num_gangs(acc_async_noval);
        results[config][1] = acc_get_num_workers(acc_async_noval);
        results[config][2] = acc_get_vector_length(acc_async_noval);
        
        // Fully nested parallelism
        #pragma acc loop gang
        for (i = 0; i < 2; i++) {
            #pragma acc loop worker
            for (j = 0; j < 2; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 8; k++) {
                    if (gid == i && wid == j && vid == k) {
                        results[config][3] += i * 100 + j * 10 + k;
                    }
                }
            }
        }
    }
    
    // Test with dynamic values (runtime-determined partitions)
    printf("\nTesting dynamic partition configurations...\n");
    for (int iter = 0; iter < 3; iter++) {
        int dyn_gangs = 1 + (iter * 2);
        int dyn_workers = 1 + iter;
        int dyn_vector = 1 << (iter + 3);  // 8, 16, 32
        
        #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) vector_length(dyn_vector) \
                    present(host_array)
        {
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            
            // Use collapse clause to generate different partition patterns
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < dyn_gangs; i++) {
                for (j = 0; j < dyn_workers * 4; j++) {
                    int idx = i * dyn_workers * 4 + j;
                    if (idx < ARRAY_SIZE) {
                        #pragma acc atomic update
                        host_array[idx] += gid * 1000 + wid * 100 + vid;
                    }
                }
            }
        }
    }
    
    // Update host with modified data
    #pragma acc update host(host_array[0:ARRAY_SIZE])
    
    // Test kernels directive with automatic partitioning
    printf("\nTesting kernels directive...\n");
    #pragma acc kernels copy(host_array[0:ARRAY_SIZE])
    {
        #pragma acc loop gang(4)
        for (i = 0; i < ARRAY_SIZE / 4; i++) {
            #pragma acc loop worker(2)
            for (j = 0; j < 4; j++) {
                #pragma acc loop vector(16)
                for (int k = 0; k < 4; k++) {
                    int idx = i * 16 + j * 4 + k;
                    if (idx < ARRAY_SIZE) {
                        host_array[idx] *= 2;
                    }
                }
            }
        }
    }
    
    // Calculate final checksum
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += host_array[i];
    }
    
    // Also add results checksums
    for (config = 0; config < NUM_CONFIGS; config++) {
        checksum += results[config][0] + results[config][1] + 
                   results[config][2] + results[config][3];
    }
    
    // Print summary
    printf("\nPartition Configuration Results:\n");
    printf("Config | Gangs | Workers | Vector | Checksum\n");
    printf("-------|-------|---------|--------|----------\n");
    for (config = 0; config < NUM_CONFIGS; config++) {
        printf("  %d    |  %3d  |   %3d   |  %4d  |  %6d\n",
               config, results[config][0], results[config][1],
               results[config][2], results[config][3]);
    }
    
    printf("\nFinal array checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates all regions executed)\n");
    
    // Cleanup
    if (device_array) {
        acc_free(device_array);
    }
    #pragma acc exit data delete(host_array)
    
    return 0;
}
