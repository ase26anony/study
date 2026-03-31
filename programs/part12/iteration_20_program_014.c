/* Test for OpenACC partition mapping coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=2 -foffload=nvptx-none -o test_partition test_partition.c */
/* Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128
#define ARRAY_SIZE 1024

/* Structure to store queried execution configuration */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    int partition_code;
} partition_config_t;

/* Helper to force runtime to compute partition mapping */
void query_partition_config(int config_id, partition_config_t *config) {
    int *d_gangs, *d_workers, *d_vector;
    int h_gangs = 0, h_workers = 0, h_vector = 0;
    
    /* Allocate device memory for results */
    #pragma acc enter data create(h_gangs, h_workers, h_vector)
    
    switch(config_id) {
        case 0: /* gang redundant */
            #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyin(config_id) present(h_gangs, h_workers, h_vector)
            {
                #pragma acc loop gang
                for (int g = 0; g < 1; g++) {
                    h_gangs = acc_get_num_gangs(acc_async_noval);
                    h_workers = acc_get_num_workers();
                    h_vector = acc_get_vector_length();
                }
            }
            break;
            
        case 1: /* gang partitioned */
            #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyin(config_id) present(h_gangs, h_workers, h_vector)
            {
                #pragma acc loop gang
                for (int g = 0; g < acc_get_num_gangs(acc_async_noval); g++) {
                    if (g == 0) {
                        h_gangs = acc_get_num_gangs(acc_async_noval);
                        h_workers = acc_get_num_workers();
                        h_vector = acc_get_vector_length();
                    }
                }
            }
            break;
            
        case 2: /* worker partitioned */
            #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyin(config_id) present(h_gangs, h_workers, h_vector)
            {
                #pragma acc loop worker
                for (int w = 0; w < acc_get_num_workers(); w++) {
                    if (w == 0) {
                        h_gangs = acc_get_num_gangs(acc_async_noval);
                        h_workers = acc_get_num_workers();
                        h_vector = acc_get_vector_length();
                    }
                }
            }
            break;
            
        case 3: /* gang+worker partitioned */
            #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyin(config_id) present(h_gangs, h_workers, h_vector)
            {
                #pragma acc loop gang worker
                for (int i = 0; i < 4; i++) {
                    if (acc_get_thread_id() == 0) {
                        h_gangs = acc_get_num_gangs(acc_async_noval);
                        h_workers = acc_get_num_workers();
                        h_vector = acc_get_vector_length();
                    }
                }
            }
            break;
            
        case 4: /* vector partitioned */
            #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                copyin(config_id) present(h_gangs, h_workers, h_vector)
            {
                #pragma acc loop vector
                for (int v = 0; v < acc_get_vector_length(); v++) {
                    if (v == 0) {
                        h_gangs = acc_get_num_gangs(acc_async_noval);
                        h_workers = acc_get_num_workers();
                        h_vector = acc_get_vector_length();
                    }
                }
            }
            break;
            
        case 5: /* gang+vector partitioned */
            #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                copyin(config_id) present(h_gangs, h_workers, h_vector)
            {
                #pragma acc loop gang vector
                for (int i = 0; i < 256; i++) {
                    if (acc_get_thread_id() == 0) {
                        h_gangs = acc_get_num_gangs(acc_async_noval);
                        h_workers = acc_get_num_workers();
                        h_vector = acc_get_vector_length();
                    }
                }
            }
            break;
            
        case 6: /* worker+vector partitioned */
            #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
                copyin(config_id) present(h_gangs, h_workers, h_vector)
            {
                #pragma acc loop worker vector
                for (int i = 0; i < 64; i++) {
                    if (acc_get_thread_id() == 0) {
                        h_gangs = acc_get_num_gangs(acc_async_noval);
                        h_workers = acc_get_num_workers();
                        h_vector = acc_get_vector_length();
                    }
                }
            }
            break;
            
        case 7: /* fully partitioned */
            #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                copyin(config_id) present(h_gangs, h_workers, h_vector)
            {
                #pragma acc loop gang worker vector
                for (int i = 0; i < 64; i++) {
                    if (acc_get_thread_id() == 0) {
                        h_gangs = acc_get_num_gangs(acc_async_noval);
                        h_workers = acc_get_num_workers();
                        h_vector = acc_get_vector_length();
                    }
                }
            }
            break;
    }
    
    /* Copy results back */
    #pragma acc update host(h_gangs, h_workers, h_vector)
    #pragma acc exit data delete(h_gangs, h_workers, h_vector)
    
    config->gangs = h_gangs;
    config->workers = h_workers;
    config->vector_len = h_vector;
    config->partition_code = config_id;
}

/* Test nested parallelism with collapse clause */
void test_nested_partitions() {
    int data[ARRAY_SIZE];
    int sum = 0;
    
    #pragma acc data copy(data, sum)
    {
        /* Nested gang-worker parallelism */
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1)
        {
            #pragma acc loop gang collapse(1)
            for (int g = 0; g < 4; g++) {
                #pragma acc loop worker
                for (int w = 0; w < 2; w++) {
                    int idx = g * 2 + w;
                    if (idx < ARRAY_SIZE) {
                        data[idx] = acc_get_num_gangs(acc_async_noval) * 1000 + 
                                   acc_get_num_workers() * 100 + 
                                   acc_get_vector_length();
                    }
                }
            }
        }
        
        /* Mixed partitioning with dynamic values */
        int dynamic_gangs = 3;
        int dynamic_workers = 3;
        int dynamic_vector = 8;
        
        #pragma acc parallel num_gangs(dynamic_gangs) \
                             num_workers(dynamic_workers) \
                             vector_length(dynamic_vector) \
                             present(data, sum)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (int i = 0; i < dynamic_gangs; i++) {
                for (int j = 0; j < dynamic_workers * dynamic_vector; j++) {
                    int idx = i * dynamic_workers * dynamic_vector + j;
                    if (idx < ARRAY_SIZE) {
                        #pragma acc atomic
                        data[idx] += acc_get_thread_id();
                    }
                }
            }
        }
        
        /* Compute checksum to ensure execution */
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                             present(data, sum)
        {
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < ARRAY_SIZE; i++) {
                sum += data[i];
            }
        }
    }
    
    printf("Nested partition checksum: %d\n", sum);
}

/* Test with unstructured data regions */
void test_unstructured_regions() {
    int *device_data;
    int host_data[100];
    
    /* Unstructured data region */
    #pragma acc enter data copyin(host_data[0:100])
    
    /* Multiple compute regions with same data */
    for (int i = 0; i < 4; i++) {
        int gangs = 1 << i;  /* 1, 2, 4, 8 */
        
        #pragma acc parallel num_gangs(gangs) num_workers(1) vector_length(1) \
                     present(host_data)
        {
            #pragma acc loop gang
            for (int j = 0; j < 100; j++) {
                host_data[j] += acc_get_num_gangs(acc_async_noval) + i;
            }
        }
    }
    
    #pragma acc exit data copyout(host_data[0:100])
    
    /* Verify some data was written */
    int verify = 0;
    for (int i = 0; i < 100; i++) {
        verify += host_data[i];
    }
    printf("Unstructured region checksum: %d\n", verify);
}

/* Force diagnostic path with acc_malloc */
void test_diagnostic_path() {
    size_t alloc_size = 1024;
    void *device_ptr = acc_malloc(alloc_size);
    
    if (device_ptr == NULL) {
        /* This should trigger diagnostic output if compiled with -fopenacc-diag */
        fprintf(stderr, "acc_malloc failed for size %zu\n", alloc_size);
    } else {
        /* Use the allocated memory in a parallel region */
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                     deviceptr(device_ptr)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < 64; i++) {
                ((int*)device_ptr)[i] = acc_get_thread_id();
            }
        }
        
        acc_free(device_ptr);
    }
}

int main() {
    partition_config_t configs[NUM_CONFIGS];
    int total_gangs = 0, total_workers = 0, total_vector = 0;
    
    printf("Testing OpenACC partition mapping coverage...\n");
    
    /* Test all 8 partition configurations */
    for (int i = 0; i < NUM_CONFIGS; i++) {
        query_partition_config(i, &configs[i]);
        
        printf("Config %d: gangs=%d, workers=%d, vector=%d\n",
               i, configs[i].gangs, configs[i].workers, configs[i].vector_len);
        
        total_gangs += configs[i].gangs;
        total_workers += configs[i].workers;
        total_vector += configs[i].vector_len;
    }
    
    printf("\nAggregate counts: gangs=%d, workers=%d, vector=%d\n",
           total_gangs, total_workers, total_vector);
    
    /* Test nested parallelism */
    test_nested_partitions();
    
    /* Test unstructured data regions */
    test_unstructured_regions();
    
    /* Test diagnostic path */
    test_diagnostic_path();
    
    /* Final verification that all configs were executed */
    if (total_gangs > 0 && total_workers > 0 && total_vector > 0) {
        printf("\nAll partition configurations tested successfully.\n");
        return 0;
    } else {
        printf("\nSome configurations may have been optimized away.\n");
        return 1;
    }
}
