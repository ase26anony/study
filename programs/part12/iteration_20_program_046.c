/* Test for OpenACC partition mapping coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

/* Structure to store queried execution configuration */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    long checksum;
} exec_config_t;

/* Host array to store results from device */
exec_config_t host_results[NUM_CONFIGS];

/* Device array for storing results */
#pragma acc declare create(host_results)

/* Function to initialize results array */
void init_results(void) {
    for (int i = 0; i < NUM_CONFIGS; i++) {
        host_results[i].gangs = 0;
        host_results[i].workers = 0;
        host_results[i].vector_len = 0;
        host_results[i].checksum = 0;
    }
    #pragma acc update device(host_results)
}

/* Function to print results */
void print_results(void) {
    long total_checksum = 0;
    printf("\n=== OpenACC Partition Configuration Results ===\n");
    for (int i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vector_len=%d, checksum=%ld\n",
               i, host_results[i].gangs, host_results[i].workers,
               host_results[i].vector_len, host_results[i].checksum);
        total_checksum += host_results[i].checksum;
    }
    printf("Total checksum: %ld\n", total_checksum);
}

int main(void) {
    int N = 1024;
    float *data = NULL;
    float *dev_data = NULL;
    int dynamic_gang_cnt = 4;
    int dynamic_worker_cnt = 2;
    int dynamic_vector_len = 64;
    
    /* Initialize host results array */
    init_results();
    
    /* Allocate and initialize test data */
    data = (float*)malloc(N * sizeof(float));
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    /* Test 0: Gang redundant (all 1s) */
    printf("Testing config 0: gang redundant...\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copy(host_results[0:1]) present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        long sum = 0;
        
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += (long)data[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            host_results[0].gangs = g;
            host_results[0].workers = w;
            host_results[0].vector_len = v;
            host_results[0].checksum = sum;
        }
    }
    
    /* Test 1: Gang partitioned */
    printf("Testing config 1: gang partitioned...\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copy(host_results[1:1]) present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        long sum = 0;
        
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += (long)data[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            host_results[1].gangs = g;
            host_results[1].workers = w;
            host_results[1].vector_len = v;
            host_results[1].checksum = sum;
        }
    }
    
    /* Test 2: Worker partitioned */
    printf("Testing config 2: worker partitioned...\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copy(host_results[2:1]) present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        long sum = 0;
        
        #pragma acc loop worker reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += (long)data[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            host_results[2].gangs = g;
            host_results[2].workers = w;
            host_results[2].vector_len = v;
            host_results[2].checksum = sum;
        }
    }
    
    /* Test 3: Gang+worker partitioned */
    printf("Testing config 3: gang+worker partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copy(host_results[3:1]) present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        long sum = 0;
        
        #pragma acc loop gang worker reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += (long)data[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            host_results[3].gangs = g;
            host_results[3].workers = w;
            host_results[3].vector_len = v;
            host_results[3].checksum = sum;
        }
    }
    
    /* Test 4: Vector partitioned */
    printf("Testing config 4: vector partitioned...\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copy(host_results[4:1]) present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        long sum = 0;
        
        #pragma acc loop vector reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += (long)data[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            host_results[4].gangs = g;
            host_results[4].workers = w;
            host_results[4].vector_len = v;
            host_results[4].checksum = sum;
        }
    }
    
    /* Test 5: Gang+vector partitioned */
    printf("Testing config 5: gang+vector partitioned...\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        copy(host_results[5:1]) present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        long sum = 0;
        
        #pragma acc loop gang vector reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += (long)data[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            host_results[5].gangs = g;
            host_results[5].workers = w;
            host_results[5].vector_len = v;
            host_results[5].checksum = sum;
        }
    }
    
    /* Test 6: Worker+vector partitioned */
    printf("Testing config 6: worker+vector partitioned...\n");
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        copy(host_results[6:1]) present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        long sum = 0;
        
        #pragma acc loop worker vector reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += (long)data[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            host_results[6].gangs = g;
            host_results[6].workers = w;
            host_results[6].vector_len = v;
            host_results[6].checksum = sum;
        }
    }
    
    /* Test 7: Fully partitioned */
    printf("Testing config 7: fully partitioned...\n");
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(8) \
        copy(host_results[7:1]) present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        long sum = 0;
        
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < N; i++) {
            sum += (long)data[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            host_results[7].gangs = g;
            host_results[7].workers = w;
            host_results[7].vector_len = v;
            host_results[7].checksum = sum;
        }
    }
    
    /* Test with dynamic values (may trigger different code paths) */
    printf("Testing with dynamic values...\n");
    #pragma acc parallel num_gangs(dynamic_gang_cnt) \
        num_workers(dynamic_worker_cnt) vector_length(dynamic_vector_len) \
        present(data[0:N])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length();
        
        /* Nested parallelism with collapse */
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                #pragma acc loop worker vector
                for (int k = 0; k < 4; k++) {
                    int idx = i * 64 + j * 4 + k;
                    if (idx < N) {
                        data[idx] *= 2.0f;
                    }
                }
            }
        }
    }
    
    /* Test with kernels construct (different code generation) */
    printf("Testing with kernels construct...\n");
    #pragma acc kernels copy(data[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N/128; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 4; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 32; k++) {
                    int idx = i * 128 + j * 32 + k;
                    if (idx < N) {
                        data[idx] += 1.0f;
                    }
                }
            }
        }
    }
    
    /* Unstructured data region test */
    printf("Testing unstructured data region...\n");
    dev_data = (float*)acc_malloc(N * sizeof(float));
    if (dev_data == NULL) {
        printf("acc_malloc failed - this may trigger diagnostic output\n");
    } else {
        #pragma acc enter data copyin(data[0:N])
        #pragma acc parallel present(data[0:N])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                data[i] = data[i] / 2.0f;
            }
        }
        #pragma acc exit data copyout(data[0:N])
        acc_free(dev_data);
    }
    
    /* Update all results from device */
    #pragma acc update host(host_results[0:NUM_CONFIGS])
    
    /* Print final results */
    print_results();
    
    /* Verify all configurations were executed */
    int configs_executed = 0;
    for (int i = 0; i < NUM_CONFIGS; i++) {
        if (host_results[i].checksum > 0) {
            configs_executed++;
        }
    }
    
    printf("\n%d out of %d configurations executed successfully\n", 
           configs_executed, NUM_CONFIGS);
    
    /* Cleanup */
    free(data);
    
    return 0;
}
