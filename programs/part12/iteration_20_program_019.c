/* Test case for covering partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
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
    int gang_count;
    int worker_count;
    int vector_length;
    int partition_code;
} partition_config_t;

/* Host function to validate and print results */
void validate_partition_codes(partition_config_t* configs, int num_configs) {
    int checksum = 0;
    
    for (int i = 0; i < num_configs; i++) {
        checksum += configs[i].gang_count;
        checksum += configs[i].worker_count;
        checksum += configs[i].vector_length;
        
        printf("Config %d: gangs=%d, workers=%d, vector=%d (code=%d)\n",
               i, configs[i].gang_count, configs[i].worker_count,
               configs[i].vector_length, configs[i].partition_code);
    }
    
    printf("Total checksum: %d\n", checksum);
}

int main() {
    int i, j;
    partition_config_t host_configs[NUM_CONFIGS] = {0};
    partition_config_t* dev_configs;
    
    /* Initialize host array */
    for (i = 0; i < NUM_CONFIGS; i++) {
        host_configs[i].partition_code = i;
    }
    
    /* Allocate device memory for results */
    dev_configs = (partition_config_t*)acc_malloc(NUM_CONFIGS * sizeof(partition_config_t));
    if (dev_configs == NULL) {
        fprintf(stderr, "acc_malloc failed - this may trigger diagnostic paths\n");
        return 1;
    }
    
    /* Copy initial data to device */
    #pragma acc enter data copyin(host_configs[0:NUM_CONFIGS])
    #pragma acc enter data create(dev_configs[0:NUM_CONFIGS])
    
    /* Test 1: Gang redundant (code 0) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(dev_configs[0:1]) present(host_configs)
    {
        int idx = 0;
        dev_configs[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        dev_configs[idx].worker_count = acc_get_num_workers(acc_async_noval);
        dev_configs[idx].vector_length = acc_get_vector_length(acc_async_noval);
        dev_configs[idx].partition_code = host_configs[idx].partition_code;
    }
    
    /* Test 2: Gang partitioned (code 1) */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(dev_configs[1:1]) present(host_configs)
    {
        int idx = 1;
        dev_configs[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        dev_configs[idx].worker_count = acc_get_num_workers(acc_async_noval);
        dev_configs[idx].vector_length = acc_get_vector_length(acc_async_noval);
        dev_configs[idx].partition_code = host_configs[idx].partition_code;
    }
    
    /* Test 3: Worker partitioned (code 2) */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(dev_configs[2:1]) present(host_configs)
    {
        int idx = 2;
        dev_configs[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        dev_configs[idx].worker_count = acc_get_num_workers(acc_async_noval);
        dev_configs[idx].vector_length = acc_get_vector_length(acc_async_noval);
        dev_configs[idx].partition_code = host_configs[idx].partition_code;
    }
    
    /* Test 4: Gang+worker partitioned (code 3) */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyout(dev_configs[3:1]) present(host_configs)
    {
        int idx = 3;
        dev_configs[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        dev_configs[idx].worker_count = acc_get_num_workers(acc_async_noval);
        dev_configs[idx].vector_length = acc_get_vector_length(acc_async_noval);
        dev_configs[idx].partition_code = host_configs[idx].partition_code;
    }
    
    /* Test 5: Vector partitioned (code 4) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(64) \
        copyout(dev_configs[4:1]) present(host_configs)
    {
        int idx = 4;
        dev_configs[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        dev_configs[idx].worker_count = acc_get_num_workers(acc_async_noval);
        dev_configs[idx].vector_length = acc_get_vector_length(acc_async_noval);
        dev_configs[idx].partition_code = host_configs[idx].partition_code;
    }
    
    /* Test 6: Gang+vector partitioned (code 5) */
    #pragma acc parallel num_gangs(2) num_workers(1) vector_length(32) \
        copyout(dev_configs[5:1]) present(host_configs)
    {
        int idx = 5;
        dev_configs[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        dev_configs[idx].worker_count = acc_get_num_workers(acc_async_noval);
        dev_configs[idx].vector_length = acc_get_vector_length(acc_async_noval);
        dev_configs[idx].partition_code = host_configs[idx].partition_code;
    }
    
    /* Test 7: Worker+vector partitioned (code 6) */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(16) \
        copyout(dev_configs[6:1]) present(host_configs)
    {
        int idx = 6;
        dev_configs[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        dev_configs[idx].worker_count = acc_get_num_workers(acc_async_noval);
        dev_configs[idx].vector_length = acc_get_vector_length(acc_async_noval);
        dev_configs[idx].partition_code = host_configs[idx].partition_code;
    }
    
    /* Test 8: Fully partitioned (code 7) */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(8) \
        copyout(dev_configs[7:1]) present(host_configs)
    {
        int idx = 7;
        dev_configs[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        dev_configs[idx].worker_count = acc_get_num_workers(acc_async_noval);
        dev_configs[idx].vector_length = acc_get_vector_length(acc_async_noval);
        dev_configs[idx].partition_code = host_configs[idx].partition_code;
    }
    
    /* Copy results back to host */
    #pragma acc update host(dev_configs[0:NUM_CONFIGS])
    
    /* Validate results */
    validate_partition_codes(dev_configs, NUM_CONFIGS);
    
    /* Additional test: Nested parallelism with collapse clause */
    int data[ARRAY_SIZE] = {0};
    int sum = 0;
    
    #pragma acc data copy(data[0:ARRAY_SIZE])
    {
        /* Nested parallelism with gang, worker, vector loops */
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
            present(data)
        {
            #pragma acc loop gang
            for (i = 0; i < 4; i++) {
                #pragma acc loop worker
                for (j = 0; j < 8; j++) {
                    int idx = i * 8 + j;
                    if (idx < ARRAY_SIZE) {
                        #pragma acc loop vector
                        for (int k = 0; k < 4; k++) {
                            data[idx] += k;
                        }
                    }
                }
            }
        }
        
        /* Collapsed loop with mixed partitioning */
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(64)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 32; i++) {
                for (j = 0; j < 32; j++) {
                    int idx = i * 32 + j;
                    if (idx < ARRAY_SIZE) {
                        data[idx] += 1;
                    }
                }
            }
        }
    }
    
    /* Compute checksum to ensure computations weren't optimized away */
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += data[i];
    }
    printf("Data array checksum: %d\n", sum);
    
    /* Dynamic partitioning based on runtime values */
    int dynamic_gangs = 3;
    int dynamic_workers = 5;
    int dynamic_vector = 16;
    
    #pragma acc parallel num_gangs(dynamic_gangs) \
        num_workers(dynamic_workers) vector_length(dynamic_vector)
    {
        /* Force runtime to handle dynamic partition configuration */
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        /* Use the values to prevent optimization */
        if (g * w * v == 0) {
            /* This should never happen, but prevents dead code elimination */
            printf("Error in dynamic partitioning\n");
        }
    }
    
    /* Test with kernels construct (different code path) */
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(4)
    {
        #pragma acc loop gang worker
        for (i = 0; i < 10; i++) {
            #pragma acc loop vector
            for (j = 0; j < 10; j++) {
                /* Dummy computation */
                int temp = i * j;
                (void)temp;
            }
        }
    }
    
    /* Cleanup */
    #pragma acc exit data delete(host_configs)
    #pragma acc exit data delete(dev_configs)
    acc_free(dev_configs);
    
    printf("Test completed successfully\n");
    return 0;
}
