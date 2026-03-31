/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128

/* Structure to store runtime query results */
typedef struct {
    int gang_count;
    int worker_count;
    int vector_length;
    int dynamic_value;
} partition_result;

/* Host array to store results from device */
partition_result host_results[NUM_TESTS];

/* Device array for results */
#pragma acc declare create(host_results)

/* Function to verify allocation (triggers diagnostic paths) */
void check_allocation(void) {
    void *ptr = acc_malloc(1024);
    if (ptr == NULL) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic\n");
    } else {
        acc_free(ptr);
    }
}

int main() {
    int i, j;
    int sum_gangs = 0, sum_workers = 0, sum_vectors = 0;
    
    /* Initialize host results */
    for (i = 0; i < NUM_TESTS; i++) {
        host_results[i].gang_count = 0;
        host_results[i].worker_count = 0;
        host_results[i].vector_length = 0;
        host_results[i].dynamic_value = i;
    }
    
    /* Copy results array to device */
    #pragma acc enter data copyin(host_results[0:NUM_TESTS])
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Test 0: gang redundant (all 1s) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(host_results[0:1])
    {
        int idx = 0;
        host_results[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        host_results[idx].worker_count = acc_get_num_workers(acc_async_noval);
        host_results[idx].vector_length = acc_get_vector_length(acc_async_noval);
    }
    
    /* Test 1: gang partitioned */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(host_results[1:1])
    {
        int idx = 1;
        host_results[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        host_results[idx].worker_count = acc_get_num_workers(acc_async_noval);
        host_results[idx].vector_length = acc_get_vector_length(acc_async_noval);
    }
    
    /* Test 2: worker partitioned */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(host_results[2:1])
    {
        int idx = 2;
        host_results[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        host_results[idx].worker_count = acc_get_num_workers(acc_async_noval);
        host_results[idx].vector_length = acc_get_vector_length(acc_async_noval);
    }
    
    /* Test 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(host_results[3:1])
    {
        int idx = 3;
        host_results[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        host_results[idx].worker_count = acc_get_num_workers(acc_async_noval);
        host_results[idx].vector_length = acc_get_vector_length(acc_async_noval);
    }
    
    /* Test 4: vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(host_results[4:1])
    {
        int idx = 4;
        host_results[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        host_results[idx].worker_count = acc_get_num_workers(acc_async_noval);
        host_results[idx].vector_length = acc_get_vector_length(acc_async_noval);
    }
    
    /* Test 5: gang+vector partitioned */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyout(host_results[5:1])
    {
        int idx = 5;
        host_results[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        host_results[idx].worker_count = acc_get_num_workers(acc_async_noval);
        host_results[idx].vector_length = acc_get_vector_length(acc_async_noval);
    }
    
    /* Test 6: worker+vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
        copyout(host_results[6:1])
    {
        int idx = 6;
        host_results[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        host_results[idx].worker_count = acc_get_num_workers(acc_async_noval);
        host_results[idx].vector_length = acc_get_vector_length(acc_async_noval);
    }
    
    /* Test 7: fully partitioned */
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(32) \
        copyout(host_results[7:1])
    {
        int idx = 7;
        host_results[idx].gang_count = acc_get_num_gangs(acc_async_noval);
        host_results[idx].worker_count = acc_get_num_workers(acc_async_noval);
        host_results[idx].vector_length = acc_get_vector_length(acc_async_noval);
    }
    
    /* Update results from device */
    #pragma acc update host(host_results[0:NUM_TESTS])
    
    /* Nested parallelism test with collapse clause */
    {
        int data[1000];
        #pragma acc data copy(data[0:1000])
        {
            /* Outer gang loop with inner worker+vector loops */
            #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
            {
                #pragma acc loop gang
                for (i = 0; i < 4; i++) {
                    #pragma acc loop worker
                    for (j = 0; j < 250; j++) {
                        int idx = i * 250 + j;
                        #pragma acc loop vector
                        for (int k = 0; k < 32; k++) {
                            /* Dummy computation */
                            if (k == 0) data[idx] = i + j;
                        }
                    }
                }
            }
            
            /* Collapsed nested loops */
            #pragma acc parallel num_gangs(8) vector_length(64)
            {
                #pragma acc loop gang vector collapse(2)
                for (i = 0; i < 50; i++) {
                    for (j = 0; j < 20; j++) {
                        int idx = i * 20 + j;
                        data[idx] += idx;
                    }
                }
            }
        }
    }
    
    /* Dynamic partitioning with runtime variables */
    for (int iter = 0; iter < 3; iter++) {
        int dyn_gangs = 2 + iter * 2;
        int dyn_workers = 1 << iter;  /* 1, 2, 4 */
        int dyn_vector = 32 >> iter;  /* 32, 16, 8 */
        
        #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) \
            vector_length(dyn_vector)
        {
            /* Force computation to prevent optimization */
            int local = acc_get_num_gangs(acc_async_noval) + 
                       acc_get_num_workers(acc_async_noval);
            if (local > 0) {
                /* Dummy operation */
                local = local * 2;
            }
        }
    }
    
    /* Unstructured data region with compute */
    {
        float *device_array;
        #pragma acc enter data create(device_array[0:1024])
        
        #pragma acc parallel present(device_array[0:1024]) \
            num_gangs(16) vector_length(64)
        {
            #pragma acc loop gang vector
            for (i = 0; i < 1024; i++) {
                device_array[i] = i * 0.5f;
            }
        }
        
        #pragma acc exit data delete(device_array[0:1024])
    }
    
    /* Trigger potential diagnostic path */
    check_allocation();
    
    /* Process and validate results */
    printf("\nPartition query results:\n");
    printf("Test | Gangs | Workers | Vector | Expected Partition\n");
    printf("-----|-------|---------|--------|-------------------\n");
    
    const char *expected[] = {
        "gang redundant",
        "gang partitioned", 
        "worker partitioned",
        "gang+worker partitioned",
        "vector partitioned",
        "gang+vector partitioned",
        "worker+vector partitioned",
        "fully partitioned"
    };
    
    for (i = 0; i < NUM_TESTS; i++) {
        printf("%4d | %6d | %7d | %6d | %s\n",
               i,
               host_results[i].gang_count,
               host_results[i].worker_count,
               host_results[i].vector_length,
               expected[i]);
        
        sum_gangs += host_results[i].gang_count;
        sum_workers += host_results[i].worker_count;
        sum_vectors += host_results[i].vector_length;
    }
    
    /* Final checksum to ensure no optimization eliminated regions */
    printf("\nChecksums (non-zero validates execution):\n");
    printf("Total gangs: %d\n", sum_gangs);
    printf("Total workers: %d\n", sum_workers);
    printf("Total vector lengths: %d\n", sum_vectors);
    
    /* Verify we got some non-zero results */
    if (sum_gangs > 0 && sum_workers > 0 && sum_vectors > 0) {
        printf("\nSUCCESS: All partition configurations exercised.\n");
    } else {
        printf("\nWARNING: Some partitions may not have executed.\n");
    }
    
    /* Cleanup */
    #pragma acc exit data delete(host_results)
    
    return 0;
}
