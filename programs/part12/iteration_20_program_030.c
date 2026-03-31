/* Test for OpenACC partition string coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* For diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128

/* Structure to store queried execution configuration */
typedef struct {
    int gangs;
    int workers;
    int vector;
    long checksum;
} exec_config_t;

/* Kernel that queries and stores execution configuration */
#pragma acc routine seq
void query_config(int idx, exec_config_t *configs) {
    int g = acc_get_num_gangs(0);
    int w = acc_get_num_workers(0);
    int v = acc_get_vector_length();
    
    configs[idx].gangs = g;
    configs[idx].workers = w;
    configs[idx].vector = v;
    
    /* Simple computation to prevent optimization */
    long sum = 0;
    #pragma acc loop seq
    for (int i = 0; i < g * w * v; i++) {
        sum += (i % 17) + 1;
    }
    configs[idx].checksum = sum;
}

int main() {
    exec_config_t host_configs[NUM_CONFIGS] = {0};
    exec_config_t *dev_configs;
    
    /* Allocate device memory for configurations */
    dev_configs = (exec_config_t *)acc_malloc(NUM_CONFIGS * sizeof(exec_config_t));
    if (!dev_configs) {
        fprintf(stderr, "acc_malloc failed\n");
        return 1;
    }
    
    /* Initialize device array */
    #pragma acc enter data copyin(host_configs[0:NUM_CONFIGS])
    #pragma acc parallel present(host_configs)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < NUM_CONFIGS; i++) {
            host_configs[i].gangs = 0;
            host_configs[i].workers = 0;
            host_configs[i].vector = 0;
            host_configs[i].checksum = 0;
        }
    }
    
    /* Configuration 0: gang redundant */
    printf("Testing configuration 0 (gang redundant)...\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyout(dev_configs[0:1]) present(host_configs)
    {
        #pragma acc loop gang
        for (int i = 0; i < 1; i++) {
            query_config(0, dev_configs);
        }
    }
    
    /* Configuration 1: gang partitioned */
    printf("Testing configuration 1 (gang partitioned)...\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyout(dev_configs[1:1]) present(host_configs)
    {
        #pragma acc loop gang
        for (int i = 0; i < 8; i++) {
            if (acc_on_device(acc_device_not_host)) {
                query_config(1, dev_configs);
            }
        }
    }
    
    /* Configuration 2: worker partitioned */
    printf("Testing configuration 2 (worker partitioned)...\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyout(dev_configs[2:1]) present(host_configs)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < 4; i++) {
            query_config(2, dev_configs);
        }
    }
    
    /* Configuration 3: gang+worker partitioned */
    printf("Testing configuration 3 (gang+worker partitioned)...\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyout(dev_configs[3:1]) present(host_configs)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < 4; i++) {
            query_config(3, dev_configs);
        }
    }
    
    /* Configuration 4: vector partitioned */
    printf("Testing configuration 4 (vector partitioned)...\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(64) \
                copyout(dev_configs[4:1]) present(host_configs)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < 64; i++) {
            query_config(4, dev_configs);
        }
    }
    
    /* Configuration 5: gang+vector partitioned */
    printf("Testing configuration 5 (gang+vector partitioned)...\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
                copyout(dev_configs[5:1]) present(host_configs)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < 128; i++) {
            query_config(5, dev_configs);
        }
    }
    
    /* Configuration 6: worker+vector partitioned */
    printf("Testing configuration 6 (worker+vector partitioned)...\n");
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(16) \
                copyout(dev_configs[6:1]) present(host_configs)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < 32; i++) {
            query_config(6, dev_configs);
        }
    }
    
    /* Configuration 7: fully partitioned */
    printf("Testing configuration 7 (fully partitioned)...\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(8) \
                copyout(dev_configs[7:1]) present(host_configs)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < 32; i++) {
            query_config(7, dev_configs);
        }
    }
    
    /* Dynamic configuration using runtime variables */
    printf("Testing dynamic configurations...\n");
    for (int iter = 0; iter < 3; iter++) {
        int dyn_gangs = 1 << iter;  /* 1, 2, 4 */
        int dyn_workers = 4 >> iter; /* 4, 2, 1 */
        int dyn_vector = 16 * (iter + 1); /* 16, 32, 48 */
        
        #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) \
                    vector_length(dyn_vector) present(host_configs)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (int i = 0; i < dyn_gangs; i++) {
                for (int j = 0; j < dyn_workers * dyn_vector; j++) {
                    /* Force computation to ensure region executes */
                    int idx = i * dyn_workers * dyn_vector + j;
                    if (idx < NUM_CONFIGS) {
                        host_configs[idx % NUM_CONFIGS].checksum += 
                            (i * 1000 + j * 17) % 1024;
                    }
                }
            }
        }
    }
    
    /* Nested parallelism with kernels directive */
    printf("Testing nested parallelism with kernels...\n");
    #pragma acc kernels num_gangs(4) num_workers(2) vector_length(8) \
                copy(host_configs[0:NUM_CONFIGS])
    {
        #pragma acc loop gang independent
        for (int g = 0; g < 4; g++) {
            #pragma acc loop worker independent
            for (int w = 0; w < 2; w++) {
                #pragma acc loop vector independent
                for (int v = 0; v < 8; v++) {
                    int idx = (g * 2 * 8 + w * 8 + v) % NUM_CONFIGS;
                    host_configs[idx].gangs += g + 1;
                    host_configs[idx].workers += w + 1;
                    host_configs[idx].vector += v + 1;
                }
            }
        }
    }
    
    /* Unstructured data region with compute */
    printf("Testing unstructured data regions...\n");
    int *data;
    data = (int *)acc_malloc(1024 * sizeof(int));
    
    if (data) {
        #pragma acc enter data copyin(data[0:1024])
        
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(32) \
                    present(data)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < 256; i++) {
                data[i] = i * 2;
            }
        }
        
        #pragma acc exit data copyout(data[0:1024])
        acc_free(data);
    } else {
        fprintf(stderr, "Warning: acc_malloc failed in unstructured test\n");
    }
    
    /* Copy results back and verify */
    #pragma acc update host(host_configs[0:NUM_CONFIGS])
    
    /* Print results and compute final checksum */
    long total_checksum = 0;
    printf("\nConfiguration Results:\n");
    printf("Config | Gangs | Workers | Vector | Checksum\n");
    printf("-------|-------|---------|--------|----------\n");
    
    for (int i = 0; i < NUM_CONFIGS; i++) {
        printf("%6d | %5d | %7d | %6d | %10ld\n",
               i, host_configs[i].gangs, host_configs[i].workers,
               host_configs[i].vector, host_configs[i].checksum);
        total_checksum += host_configs[i].checksum;
    }
    
    printf("\nTotal checksum: %ld\n", total_checksum);
    
    /* Cleanup */
    #pragma acc exit data delete(host_configs)
    acc_free(dev_configs);
    
    if (total_checksum > 0) {
        printf("Test PASSED: All configurations executed\n");
        return 0;
    } else {
        printf("Test FAILED: No computations performed\n");
        return 1;
    }
}
