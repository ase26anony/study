/* Test case for covering partition string mapping in omp-oacc-neuter-broadcast.cc
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

/* Structure to store runtime query results */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    long checksum;
} partition_result_t;

/* Helper function to verify results aren't optimized away */
long compute_checksum(int *array, int size) {
    long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

int main() {
    int i, j;
    long total_checksum = 0;
    partition_result_t results[NUM_CONFIGS] = {0};
    int *host_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *device_array;
    
    /* Initialize host array */
    for (i = 0; i < ARRAY_SIZE; i++) {
        host_array[i] = i % 100;
    }
    
    /* Allocate device memory */
    device_array = (int*)acc_malloc(ARRAY_SIZE * sizeof(int));
    if (device_array == NULL) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic path\n");
        /* Continue anyway - this might trigger the diagnostic that uses partition strings */
    }
    
    /* Copy data to device */
    #pragma acc enter data copyin(host_array[0:ARRAY_SIZE])
    #pragma acc enter data create(device_array[0:ARRAY_SIZE])
    
    printf("Testing various OpenACC partition configurations...\n");
    
    /* Configuration 0: gang redundant (all 1s) */
    {
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            /* Perform some computation to ensure region isn't optimized away */
            #pragma acc loop gang worker vector
            for (i = 0; i < ARRAY_SIZE; i++) {
                device_array[i] = host_array[i] * 2;
            }
            
            /* Simple reduction for checksum */
            #pragma acc loop gang worker vector reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        results[0].gangs = local_gangs;
        results[0].workers = local_workers;
        results[0].vector_len = local_vector;
        results[0].checksum = local_checksum;
        total_checksum += local_checksum;
    }
    
    /* Configuration 1: gang partitioned */
    {
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            #pragma acc loop gang
            for (i = 0; i < ARRAY_SIZE; i++) {
                device_array[i] = host_array[i] + i;
            }
            
            #pragma acc loop gang reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        results[1].gangs = local_gangs;
        results[1].workers = local_workers;
        results[1].vector_len = local_vector;
        results[1].checksum = local_checksum;
        total_checksum += local_checksum;
    }
    
    /* Configuration 2: worker partitioned */
    {
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            #pragma acc loop worker
            for (i = 0; i < ARRAY_SIZE; i++) {
                device_array[i] = host_array[i] * 3;
            }
            
            #pragma acc loop worker reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        results[2].gangs = local_gangs;
        results[2].workers = local_workers;
        results[2].vector_len = local_vector;
        results[2].checksum = local_checksum;
        total_checksum += local_checksum;
    }
    
    /* Configuration 3: gang+worker partitioned */
    {
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            /* Nested parallelism */
            #pragma acc loop gang
            for (i = 0; i < ARRAY_SIZE/2; i++) {
                #pragma acc loop worker
                for (j = 0; j < 2; j++) {
                    int idx = i * 2 + j;
                    if (idx < ARRAY_SIZE) {
                        device_array[idx] = host_array[idx] - j;
                    }
                }
            }
            
            #pragma acc loop gang worker reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        results[3].gangs = local_gangs;
        results[3].workers = local_workers;
        results[3].vector_len = local_vector;
        results[3].checksum = local_checksum;
        total_checksum += local_checksum;
    }
    
    /* Configuration 4: vector partitioned */
    {
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            #pragma acc loop vector
            for (i = 0; i < ARRAY_SIZE; i++) {
                device_array[i] = host_array[i] / 2;
            }
            
            #pragma acc loop vector reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        results[4].gangs = local_gangs;
        results[4].workers = local_workers;
        results[4].vector_len = local_vector;
        results[4].checksum = local_checksum;
        total_checksum += local_checksum;
    }
    
    /* Configuration 5: gang+vector partitioned */
    {
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            #pragma acc loop gang vector
            for (i = 0; i < ARRAY_SIZE; i++) {
                device_array[i] = host_array[i] * host_array[i];
            }
            
            #pragma acc loop gang vector reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        results[5].gangs = local_gangs;
        results[5].workers = local_workers;
        results[5].vector_len = local_vector;
        results[5].checksum = local_checksum;
        total_checksum += local_checksum;
    }
    
    /* Configuration 6: worker+vector partitioned */
    {
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            #pragma acc loop worker vector
            for (i = 0; i < ARRAY_SIZE; i++) {
                device_array[i] = host_array[i] % 50;
            }
            
            #pragma acc loop worker vector reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        results[6].gangs = local_gangs;
        results[6].workers = local_workers;
        results[6].vector_len = local_vector;
        results[6].checksum = local_checksum;
        total_checksum += local_checksum;
    }
    
    /* Configuration 7: fully partitioned (gang+worker+vector) */
    {
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            /* Triple nested parallelism */
            #pragma acc loop gang
            for (i = 0; i < ARRAY_SIZE/4; i++) {
                #pragma acc loop worker
                for (j = 0; j < 2; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 2; k++) {
                        int idx = i * 4 + j * 2 + k;
                        if (idx < ARRAY_SIZE) {
                            device_array[idx] = host_array[idx] + idx;
                        }
                    }
                }
            }
            
            #pragma acc loop gang worker vector reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        results[7].gangs = local_gangs;
        results[7].workers = local_workers;
        results[7].vector_len = local_vector;
        results[7].checksum = local_checksum;
        total_checksum += local_checksum;
    }
    
    /* Additional test: Dynamic configuration using runtime variables */
    {
        int dynamic_gangs = 3;
        int dynamic_workers = 3;
        int dynamic_vector = 8;
        int local_gangs, local_workers, local_vector;
        long local_checksum = 0;
        
        #pragma acc parallel num_gangs(dynamic_gangs) num_workers(dynamic_workers) vector_length(dynamic_vector) \
                    copyout(local_gangs, local_workers, local_vector, local_checksum) \
                    present(host_array, device_array)
        {
            local_gangs = acc_get_num_gangs(acc_async_noval);
            local_workers = acc_get_num_workers(acc_async_noval);
            local_vector = acc_get_vector_length();
            
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 64; i++) {
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    if (idx < ARRAY_SIZE) {
                        device_array[idx] = host_array[idx] * 5;
                    }
                }
            }
            
            #pragma acc loop gang worker vector reduction(+:local_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                local_checksum += device_array[i];
            }
        }
        
        total_checksum += local_checksum;
        printf("Dynamic config: gangs=%d, workers=%d, vector=%d, checksum=%ld\n",
               local_gangs, local_workers, local_vector, local_checksum);
    }
    
    /* Test with kernels construct (different code path) */
    {
        long kernels_checksum = 0;
        
        #pragma acc kernels num_gangs(2) num_workers(2) vector_length(32) \
                    copyout(kernels_checksum) present(host_array, device_array)
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < ARRAY_SIZE; i++) {
                device_array[i] = host_array[i] * 7;
            }
            
            #pragma acc loop gang worker vector reduction(+:kernels_checksum)
            for (i = 0; i < ARRAY_SIZE; i++) {
                kernels_checksum += device_array[i];
            }
        }
        
        total_checksum += kernels_checksum;
        printf("Kernels construct checksum: %ld\n", kernels_checksum);
    }
    
    /* Clean up */
    #pragma acc exit data delete(host_array, device_array)
    if (device_array) {
        acc_free(device_array);
    }
    
    /* Print summary to ensure no optimization */
    printf("\nPartition Configuration Results:\n");
    printf("Config | Gangs | Workers | Vector | Checksum\n");
    printf("-------+-------+---------+--------+----------\n");
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("%6d | %5d | %7d | %6d | %10ld\n",
               i, results[i].gangs, results[i].workers, 
               results[i].vector_len, results[i].checksum);
    }
    
    printf("\nTotal checksum (verification): %ld\n", total_checksum);
    
    free(host_array);
    
    /* Force potential diagnostic by checking device type */
    acc_device_t dev_type = acc_get_device_type();
    printf("Active device type: %d\n", (int)dev_type);
    
    return 0;
}
