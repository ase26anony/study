/* Test case for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
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
#define ARRAY_SIZE 1024

/* Structure to store queried execution configuration */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    int device_type;
} exec_config_t;

/* Helper function to verify allocation */
static void check_acc_malloc(void *ptr, const char *msg) {
    if (ptr == NULL) {
        fprintf(stderr, "ACC malloc failed: %s\n", msg);
    }
}

int main() {
    int i, j, k;
    int sum = 0;
    int *host_array = NULL;
    int *device_array = NULL;
    exec_config_t configs[NUM_CONFIGS];
    
    /* Initialize host array */
    host_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    for (i = 0; i < ARRAY_SIZE; i++) {
        host_array[i] = i % 100;
    }
    
    /* Allocate device array */
    device_array = (int*)acc_malloc(ARRAY_SIZE * sizeof(int));
    check_acc_malloc(device_array, "device_array allocation");
    
    if (device_array == NULL) {
        fprintf(stderr, "Failed to allocate device memory\n");
        free(host_array);
        return 1;
    }
    
    /* Copy data to device */
    #pragma acc enter data copyin(host_array[0:ARRAY_SIZE])
    #pragma acc enter data create(device_array[0:ARRAY_SIZE])
    
    /* Configuration 0: gang redundant (all 1s) */
    printf("Testing configuration 0: gang redundant\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyin(host_array[0:ARRAY_SIZE]) \
        present(device_array[0:ARRAY_SIZE])
    {
        int gang_id = acc_get_num_gangs(acc_async_noval);
        int worker_id = acc_get_num_workers(acc_async_noval);
        int vector_len = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < 1; i++) {
            #pragma acc loop worker
            for (j = 0; j < 1; j++) {
                #pragma acc loop vector
                for (k = 0; k < ARRAY_SIZE; k++) {
                    if (k < ARRAY_SIZE) {
                        device_array[k] = host_array[k] + gang_id + worker_id;
                    }
                }
            }
        }
        
        /* Store configuration - first thread writes */
        if (acc_on_device(acc_device_not_host) && 
            __builtin_acc_get_thread_id() == 0) {
            configs[0].gangs = gang_id;
            configs[0].workers = worker_id;
            configs[0].vector_len = vector_len;
            configs[0].device_type = acc_get_device_type();
        }
    }
    
    /* Configuration 1: gang partitioned */
    printf("Testing configuration 1: gang partitioned\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    {
        int gang_id = acc_get_num_gangs(acc_async_noval);
        int worker_id = acc_get_num_workers(acc_async_noval);
        int vector_len = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < gang_id; i++) {
            int start = i * (ARRAY_SIZE / gang_id);
            int end = (i + 1) * (ARRAY_SIZE / gang_id);
            if (i == gang_id - 1) end = ARRAY_SIZE;
            
            #pragma acc loop worker
            for (j = 0; j < 1; j++) {
                #pragma acc loop vector
                for (k = start; k < end; k++) {
                    device_array[k] = host_array[k] * 2;
                }
            }
        }
        
        if (acc_on_device(acc_device_not_host) && 
            __builtin_acc_get_thread_id() == 0) {
            configs[1].gangs = gang_id;
            configs[1].workers = worker_id;
            configs[1].vector_len = vector_len;
        }
    }
    
    /* Configuration 2: worker partitioned */
    printf("Testing configuration 2: worker partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    {
        int gang_id = acc_get_num_gangs(acc_async_noval);
        int worker_id = acc_get_num_workers(acc_async_noval);
        int vector_len = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < 1; i++) {
            #pragma acc loop worker
            for (j = 0; j < worker_id; j++) {
                int start = j * (ARRAY_SIZE / worker_id);
                int end = (j + 1) * (ARRAY_SIZE / worker_id);
                if (j == worker_id - 1) end = ARRAY_SIZE;
                
                #pragma acc loop vector
                for (k = start; k < end; k++) {
                    device_array[k] = host_array[k] + j;
                }
            }
        }
        
        if (acc_on_device(acc_device_not_host) && 
            __builtin_acc_get_thread_id() == 0) {
            configs[2].gangs = gang_id;
            configs[2].workers = worker_id;
            configs[2].vector_len = vector_len;
        }
    }
    
    /* Configuration 3: gang+worker partitioned */
    printf("Testing configuration 3: gang+worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    {
        int gang_id = acc_get_num_gangs(acc_async_noval);
        int worker_id = acc_get_num_workers(acc_async_noval);
        int vector_len = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < gang_id; i++) {
            #pragma acc loop worker
            for (j = 0; j < worker_id; j++) {
                int idx = i * worker_id + j;
                int start = idx * (ARRAY_SIZE / (gang_id * worker_id));
                int end = (idx + 1) * (ARRAY_SIZE / (gang_id * worker_id));
                if (idx == gang_id * worker_id - 1) end = ARRAY_SIZE;
                
                #pragma acc loop vector
                for (k = start; k < end; k++) {
                    device_array[k] = host_array[k] * 3;
                }
            }
        }
        
        if (acc_on_device(acc_device_not_host) && 
            __builtin_acc_get_thread_id() == 0) {
            configs[3].gangs = gang_id;
            configs[3].workers = worker_id;
            configs[3].vector_len = vector_len;
        }
    }
    
    /* Configuration 4: vector partitioned */
    printf("Testing configuration 4: vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    {
        int gang_id = acc_get_num_gangs(acc_async_noval);
        int worker_id = acc_get_num_workers(acc_async_noval);
        int vector_len = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < 1; i++) {
            #pragma acc loop worker
            for (j = 0; j < 1; j++) {
                #pragma acc loop vector
                for (k = 0; k < ARRAY_SIZE; k++) {
                    if (k < ARRAY_SIZE) {
                        device_array[k] = host_array[k] / 2;
                    }
                }
            }
        }
        
        if (acc_on_device(acc_device_not_host) && 
            __builtin_acc_get_thread_id() == 0) {
            configs[4].gangs = gang_id;
            configs[4].workers = worker_id;
            configs[4].vector_len = vector_len;
        }
    }
    
    /* Configuration 5: gang+vector partitioned */
    printf("Testing configuration 5: gang+vector partitioned\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    {
        int gang_id = acc_get_num_gangs(acc_async_noval);
        int worker_id = acc_get_num_workers(acc_async_noval);
        int vector_len = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < gang_id; i++) {
            int start = i * (ARRAY_SIZE / gang_id);
            int end = (i + 1) * (ARRAY_SIZE / gang_id);
            if (i == gang_id - 1) end = ARRAY_SIZE;
            
            #pragma acc loop worker
            for (j = 0; j < 1; j++) {
                #pragma acc loop vector
                for (k = start; k < end; k++) {
                    device_array[k] = host_array[k] + i;
                }
            }
        }
        
        if (acc_on_device(acc_device_not_host) && 
            __builtin_acc_get_thread_id() == 0) {
            configs[5].gangs = gang_id;
            configs[5].workers = worker_id;
            configs[5].vector_len = vector_len;
        }
    }
    
    /* Configuration 6: worker+vector partitioned */
    printf("Testing configuration 6: worker+vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
        present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    {
        int gang_id = acc_get_num_gangs(acc_async_noval);
        int worker_id = acc_get_num_workers(acc_async_noval);
        int vector_len = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < 1; i++) {
            #pragma acc loop worker
            for (j = 0; j < worker_id; j++) {
                int start = j * (ARRAY_SIZE / worker_id);
                int end = (j + 1) * (ARRAY_SIZE / worker_id);
                if (j == worker_id - 1) end = ARRAY_SIZE;
                
                #pragma acc loop vector
                for (k = start; k < end; k++) {
                    device_array[k] = host_array[k] * j;
                }
            }
        }
        
        if (acc_on_device(acc_device_not_host) && 
            __builtin_acc_get_thread_id() == 0) {
            configs[6].gangs = gang_id;
            configs[6].workers = worker_id;
            configs[6].vector_len = vector_len;
        }
    }
    
    /* Configuration 7: fully partitioned */
    printf("Testing configuration 7: fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    {
        int gang_id = acc_get_num_gangs(acc_async_noval);
        int worker_id = acc_get_num_workers(acc_async_noval);
        int vector_len = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < gang_id; i++) {
            #pragma acc loop worker
            for (j = 0; j < worker_id; j++) {
                #pragma acc loop vector
                for (k = 0; k < ARRAY_SIZE / (gang_id * worker_id); k++) {
                    int idx = i * worker_id * (ARRAY_SIZE / (gang_id * worker_id)) +
                              j * (ARRAY_SIZE / (gang_id * worker_id)) + k;
                    if (idx < ARRAY_SIZE) {
                        device_array[idx] = host_array[idx] + i + j;
                    }
                }
            }
        }
        
        if (acc_on_device(acc_device_not_host) && 
            __builtin_acc_get_thread_id() == 0) {
            configs[7].gangs = gang_id;
            configs[7].workers = worker_id;
            configs[7].vector_len = vector_len;
        }
    }
    
    /* Dynamic configuration using host variables */
    printf("Testing dynamic configuration\n");
    for (int iter = 0; iter < 3; iter++) {
        int dyn_gangs = 1 << iter;  /* 1, 2, 4 */
        int dyn_workers = 4 >> iter; /* 4, 2, 1 */
        int dyn_vector = 32 >> iter; /* 32, 16, 8 */
        
        #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) vector_length(dyn_vector) \
            present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
        {
            int gang_id = acc_get_num_gangs(acc_async_noval);
            int worker_id = acc_get_num_workers(acc_async_noval);
            int vector_len = acc_get_vector_length(acc_async_noval);
            
            #pragma acc loop gang collapse(2)
            for (i = 0; i < gang_id; i++) {
                for (j = 0; j < worker_id; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < ARRAY_SIZE / (gang_id * worker_id); k++) {
                        int idx = i * worker_id * (ARRAY_SIZE / (gang_id * worker_id)) +
                                  j * (ARRAY_SIZE / (gang_id * worker_id)) + k;
                        if (idx < ARRAY_SIZE) {
                            device_array[idx] = host_array[idx] * (iter + 1);
                        }
                    }
                }
            }
        }
    }
    
    /* Copy results back and compute checksum */
    #pragma acc update host(device_array[0:ARRAY_SIZE])
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += device_array[i];
    }
    
    printf("Checksum of device array: %d\n", sum);
    
    /* Print queried configurations */
    printf("\nQueried execution configurations:\n");
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vector=%d\n",
               i, configs[i].gangs, configs[i].workers, configs[i].vector_len);
    }
    
    /* Cleanup */
    #pragma acc exit data delete(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    acc_free(device_array);
    free(host_array);
    
    /* Additional test with kernels construct */
    printf("\nTesting kernels construct with varying partitions\n");
    {
        int *kernels_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
        for (i = 0; i < ARRAY_SIZE; i++) kernels_array[i] = i;
        
        #pragma acc data copy(kernels_array[0:ARRAY_SIZE])
        {
            #pragma acc kernels num_gangs(8) num_workers(1) vector_length(1)
            {
                #pragma acc loop gang
                for (i = 0; i < ARRAY_SIZE; i++) {
                    kernels_array[i] *= 2;
                }
            }
            
            #pragma acc kernels num_gangs(1) num_workers(4) vector_length(1)
            {
                #pragma acc loop worker
                for (i = 0; i < ARRAY_SIZE; i++) {
                    kernels_array[i] += 1;
                }
            }
            
            #pragma acc kernels num_gangs(1) num_workers(1) vector_length(128)
            {
                #pragma acc loop vector
                for (i = 0; i < ARRAY_SIZE; i++) {
                    kernels_array[i] /= 2;
                }
            }
        }
        
        int kernels_sum = 0;
        for (i = 0; i < ARRAY_SIZE; i++) kernels_sum += kernels_array[i];
        printf("Kernels checksum: %d\n", kernels_sum);
        
        free(kernels_array);
    }
    
    return 0;
}
