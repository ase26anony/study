/* test-omp-acc-partition-coverage.c */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test-omp-acc-partition-coverage.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128

/* Structure to store runtime query results */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    long checksum;
} partition_result_t;

/* Function to force runtime to compute partition descriptor */
void run_partition_config(int config_id, partition_result_t *result) {
    int i, j;
    long local_sum = 0;
    int *device_array = NULL;
    int host_array[100] = {0};
    
    /* Initialize some data */
    for (i = 0; i < 100; i++) {
        host_array[i] = i + config_id;
    }
    
    /* Allocate device memory - may trigger diagnostics */
    device_array = (int *)acc_malloc(100 * sizeof(int));
    
    if (device_array == NULL) {
        /* This could trigger diagnostic path with partition string */
        fprintf(stderr, "Config %d: acc_malloc failed\n", config_id);
        return;
    }
    
    /* Copy data to device */
    #pragma acc enter data copyin(host_array[0:100])
    
    /* Different partition configurations based on config_id */
    switch(config_id) {
        case 0: /* gang redundant */
            #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyin(config_id) present(host_array) private(i)
            {
                int g = acc_get_num_gangs(0);
                int w = acc_get_num_workers(0);
                int v = acc_get_vector_length();
                #pragma acc loop gang
                for (i = 0; i < g; i++) {
                    local_sum += host_array[i % 100];
                }
                if (acc_on_device(acc_device_not_host)) {
                    result->gangs = g;
                    result->workers = w;
                    result->vector_len = v;
                }
            }
            break;
            
        case 1: /* gang partitioned */
            #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyin(config_id) present(host_array) private(i)
            {
                int g = acc_get_num_gangs(0);
                int w = acc_get_num_workers(0);
                int v = acc_get_vector_length();
                #pragma acc loop gang
                for (i = 0; i < g * 10; i++) {
                    local_sum += host_array[i % 100] * 2;
                }
                if (acc_on_device(acc_device_not_host)) {
                    result->gangs = g;
                    result->workers = w;
                    result->vector_len = v;
                }
            }
            break;
            
        case 2: /* worker partitioned */
            #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyin(config_id) present(host_array) private(i,j)
            {
                int g = acc_get_num_gangs(0);
                int w = acc_get_num_workers(0);
                int v = acc_get_vector_length();
                #pragma acc loop gang worker
                for (i = 0; i < w; i++) {
                    for (j = 0; j < 10; j++) {
                        local_sum += host_array[(i+j) % 100];
                    }
                }
                if (acc_on_device(acc_device_not_host)) {
                    result->gangs = g;
                    result->workers = w;
                    result->vector_len = v;
                }
            }
            break;
            
        case 3: /* gang+worker partitioned */
            #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyin(config_id) present(host_array) private(i,j)
            {
                int g = acc_get_num_gangs(0);
                int w = acc_get_num_workers(0);
                int v = acc_get_vector_length();
                #pragma acc loop gang worker
                for (i = 0; i < g * w; i++) {
                    local_sum += host_array[i % 100] * 3;
                }
                if (acc_on_device(acc_device_not_host)) {
                    result->gangs = g;
                    result->workers = w;
                    result->vector_len = v;
                }
            }
            break;
            
        case 4: /* vector partitioned */
            #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                copyin(config_id) present(host_array) private(i)
            {
                int g = acc_get_num_gangs(0);
                int w = acc_get_num_workers(0);
                int v = acc_get_vector_length();
                #pragma acc loop vector
                for (i = 0; i < v; i++) {
                    if (i < 100) {
                        local_sum += host_array[i];
                    }
                }
                if (acc_on_device(acc_device_not_host)) {
                    result->gangs = g;
                    result->workers = w;
                    result->vector_len = v;
                }
            }
            break;
            
        case 5: /* gang+vector partitioned */
            #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
                copyin(config_id) present(host_array) private(i,j)
            {
                int g = acc_get_num_gangs(0);
                int w = acc_get_num_workers(0);
                int v = acc_get_vector_length();
                #pragma acc loop gang vector
                for (i = 0; i < g * v; i++) {
                    if (i < 100) {
                        local_sum += host_array[i] * 4;
                    }
                }
                if (acc_on_device(acc_device_not_host)) {
                    result->gangs = g;
                    result->workers = w;
                    result->vector_len = v;
                }
            }
            break;
            
        case 6: /* worker+vector partitioned */
            #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
                copyin(config_id) present(host_array) private(i,j,k)
            {
                int g = acc_get_num_gangs(0);
                int w = acc_get_num_workers(0);
                int v = acc_get_vector_length();
                int k;
                #pragma acc loop worker vector
                for (i = 0; i < w; i++) {
                    for (j = 0; j < v; j++) {
                        k = i * v + j;
                        if (k < 100) {
                            local_sum += host_array[k];
                        }
                    }
                }
                if (acc_on_device(acc_device_not_host)) {
                    result->gangs = g;
                    result->workers = w;
                    result->vector_len = v;
                }
            }
            break;
            
        case 7: /* fully partitioned */
            #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
                copyin(config_id) present(host_array) private(i,j,k,l)
            {
                int g = acc_get_num_gangs(0);
                int w = acc_get_num_workers(0);
                int v = acc_get_vector_length();
                int k, l;
                #pragma acc loop gang worker vector collapse(2)
                for (i = 0; i < g; i++) {
                    for (j = 0; j < w; j++) {
                        for (k = 0; k < v; k++) {
                            l = i * w * v + j * v + k;
                            if (l < 100) {
                                local_sum += host_array[l] * 5;
                            }
                        }
                    }
                }
                if (acc_on_device(acc_device_not_host)) {
                    result->gangs = g;
                    result->workers = w;
                    result->vector_len = v;
                }
            }
            break;
    }
    
    /* Nested parallelism with kernels directive */
    if (config_id == 0 || config_id == 3) {
        int dynamic_gangs = 2 + config_id;
        int dynamic_workers = 1 + (config_id % 3);
        int dynamic_vector = 16 * (1 + config_id % 2);
        
        #pragma acc kernels num_gangs(dynamic_gangs) num_workers(dynamic_workers) \
            vector_length(dynamic_vector) copyin(config_id, dynamic_gangs, dynamic_workers, dynamic_vector)
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length();
            int idx = g * w * v;
            local_sum += idx;
            
            /* Store results back */
            if (acc_on_device(acc_device_not_host)) {
                result->gangs = g;
                result->workers = w;
                result->vector_len = v;
            }
        }
    }
    
    /* Update result with checksum */
    result->checksum = local_sum;
    
    /* Cleanup */
    #pragma acc exit data delete(host_array)
    if (device_array) {
        acc_free(device_array);
    }
}

/* Mixed OpenACC directives for varied code paths */
void test_mixed_directives() {
    int data[1000];
    int i;
    
    /* Unstructured data region */
    #pragma acc enter data copyin(data[0:1000])
    
    /* Structured compute region inside data region */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) present(data)
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length();
        
        #pragma acc loop gang worker vector collapse(2)
        for (i = 0; i < g * w * v; i++) {
            if (i < 1000) {
                data[i] = i * 2;
            }
        }
    }
    
    /* Another region with different partitioning */
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(1) present(data)
    {
        #pragma acc loop worker
        for (i = 0; i < 1000; i++) {
            data[i] += i;
        }
    }
    
    #pragma acc exit data copyout(data[0:1000])
    
    /* Verify data was processed */
    long sum = 0;
    for (i = 0; i < 1000; i++) {
        sum += data[i];
    }
    printf("Mixed directives checksum: %ld\n", sum);
}

int main() {
    partition_result_t results[NUM_CONFIGS];
    int i;
    long total_checksum = 0;
    
    printf("Testing OpenACC partition configurations...\n");
    
    /* Initialize results */
    for (i = 0; i < NUM_CONFIGS; i++) {
        results[i].gangs = 0;
        results[i].workers = 0;
        results[i].vector_len = 0;
        results[i].checksum = 0;
    }
    
    /* Test all partition configurations */
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("Running config %d...\n", i);
        run_partition_config(i, &results[i]);
        total_checksum += results[i].checksum;
    }
    
    /* Test mixed directives */
    test_mixed_directives();
    
    /* Print results summary */
    printf("\nPartition Configuration Results:\n");
    printf("Config | Gangs | Workers | Vector | Checksum\n");
    printf("--------------------------------------------\n");
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("%6d | %5d | %7d | %6d | %10ld\n",
               i, results[i].gangs, results[i].workers,
               results[i].vector_len, results[i].checksum);
    }
    
    printf("\nTotal checksum: %ld\n", total_checksum);
    
    /* Force potential diagnostic output */
    if (total_checksum == 0) {
        fprintf(stderr, "Warning: All checksums are zero!\n");
    }
    
    /* Additional test with runtime-determined values */
    {
        int dyn_gang = 3, dyn_worker = 2, dyn_vector = 64;
        int test_array[100];
        
        for (i = 0; i < 100; i++) test_array[i] = i;
        
        #pragma acc data copy(test_array)
        #pragma acc parallel num_gangs(dyn_gang) num_workers(dyn_worker) \
            vector_length(dyn_vector)
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length();
            
            #pragma acc loop gang worker vector
            for (i = 0; i < g * w * v; i++) {
                if (i < 100) {
                    test_array[i] *= 2;
                }
            }
        }
        
        /* Final verification sum */
        long final_sum = 0;
        for (i = 0; i < 100; i++) {
            final_sum += test_array[i];
        }
        printf("Dynamic config final sum: %ld\n", final_sum);
    }
    
    return 0;
}
