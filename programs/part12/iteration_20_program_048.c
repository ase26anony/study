/* test_partition_mapping.c - Cover partition string mapping in omp-oacc-neuter-broadcast.cc */
/* Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=2 -foffload=nvptx-none -o test_partition test_partition_mapping.c */
/* Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition_mapping.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

/* Structure to store runtime query results */
typedef struct {
    int gang_count;
    int worker_count;
    int vector_length;
    int device_type;
} partition_config_t;

/* Host array to store results from device */
static partition_config_t host_results[NUM_CONFIGS];

/* Device array for runtime queries */
#pragma acc declare create(host_results)

/* Function to force runtime to map partition codes */
void test_partition_mapping(void) {
    int i, j, k;
    int sum = 0;
    
    /* Initialize host results */
    for (i = 0; i < NUM_CONFIGS; i++) {
        host_results[i].gang_count = 0;
        host_results[i].worker_count = 0;
        host_results[i].vector_length = 0;
        host_results[i].device_type = -1;
    }
    
    /* Copy to device */
    #pragma acc update device(host_results)
    
    printf("Testing OpenACC partition mapping...\n");
    
    /* Configuration 0: gang redundant (all 1s) */
    printf("\nConfig 0: gang redundant\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(host_results[0:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        /* Only first thread in first gang writes */
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[0].gang_count = acc_get_num_gangs(acc_get_device_type());
            host_results[0].worker_count = acc_get_num_workers();
            host_results[0].vector_length = acc_get_vector_length();
            host_results[0].device_type = acc_get_device_type();
        }
    }
    
    /* Configuration 1: gang partitioned */
    printf("\nConfig 1: gang partitioned\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(host_results[1:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[1].gang_count = acc_get_num_gangs(acc_get_device_type());
            host_results[1].worker_count = acc_get_num_workers();
            host_results[1].vector_length = acc_get_vector_length();
        }
    }
    
    /* Configuration 2: worker partitioned */
    printf("\nConfig 2: worker partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(host_results[2:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[2].gang_count = acc_get_num_gangs(acc_get_device_type());
            host_results[2].worker_count = acc_get_num_workers();
            host_results[2].vector_length = acc_get_vector_length();
        }
    }
    
    /* Configuration 3: gang+worker partitioned */
    printf("\nConfig 3: gang+worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(host_results[3:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[3].gang_count = acc_get_num_gangs(acc_get_device_type());
            host_results[3].worker_count = acc_get_num_workers();
            host_results[3].vector_length = acc_get_vector_length();
        }
    }
    
    /* Configuration 4: vector partitioned */
    printf("\nConfig 4: vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(host_results[4:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[4].gang_count = acc_get_num_gangs(acc_get_device_type());
            host_results[4].worker_count = acc_get_num_workers();
            host_results[4].vector_length = acc_get_vector_length();
        }
    }
    
    /* Configuration 5: gang+vector partitioned */
    printf("\nConfig 5: gang+vector partitioned\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyout(host_results[5:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[5].gang_count = acc_get_num_gangs(acc_get_device_type());
            host_results[5].worker_count = acc_get_num_workers();
            host_results[5].vector_length = acc_get_vector_length();
        }
    }
    
    /* Configuration 6: worker+vector partitioned */
    printf("\nConfig 6: worker+vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyout(host_results[6:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[6].gang_count = acc_get_num_gangs(acc_get_device_type());
            host_results[6].worker_count = acc_get_num_workers();
            host_results[6].vector_length = acc_get_vector_length();
        }
    }
    
    /* Configuration 7: fully partitioned */
    printf("\nConfig 7: fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
        copyout(host_results[7:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[7].gang_count = acc_get_num_gangs(acc_get_device_type());
            host_results[7].worker_count = acc_get_num_workers();
            host_results[7].vector_length = acc_get_vector_length();
        }
    }
    
    /* Update results from device */
    #pragma acc update host(host_results)
    
    /* Print results and compute checksum */
    printf("\n=== Partition Configuration Results ===\n");
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vector=%d\n",
               i, host_results[i].gang_count,
               host_results[i].worker_count,
               host_results[i].vector_length);
        sum += host_results[i].gang_count + 
               host_results[i].worker_count + 
               host_results[i].vector_length;
    }
    printf("\nChecksum (sum of all counts): %d\n", sum);
}

/* Test with nested parallelism */
void test_nested_partitions(void) {
    int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    int sum = 0;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data[i] = i % 100;
    }
    
    printf("\n=== Testing Nested Parallelism ===\n");
    
    /* Nested: gang outer, worker inner */
    #pragma acc data copy(data[0:N])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (int g = 0; g < 4; g++) {
                #pragma acc loop worker
                for (int w = 0; w < 2; w++) {
                    #pragma acc loop vector
                    for (int i = 0; i < 32; i++) {
                        int idx = g * 64 + w * 32 + i;
                        if (idx < N) {
                            data[idx] *= 2;
                        }
                    }
                }
            }
        }
        
        /* Collapsed loops */
        #pragma acc parallel num_gangs(8) vector_length(64)
        {
            #pragma acc loop gang vector collapse(2)
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 4; j++) {
                    int idx = i * 4 + j;
                    if (idx < N) {
                        data[idx] += 1;
                    }
                }
            }
        }
    }
    
    /* Verify results */
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    printf("Nested parallelism checksum: %d\n", sum);
    
    free(data);
}

/* Test with dynamic values */
void test_dynamic_partitions(void) {
    int dynamic_gangs = 3;
    int dynamic_workers = 2;
    int dynamic_vector = 16;
    
    printf("\n=== Testing Dynamic Partitions ===\n");
    
    /* Use runtime variables in clauses */
    #pragma acc parallel num_gangs(dynamic_gangs) \
        num_workers(dynamic_workers) vector_length(dynamic_vector)
    {
        /* Simple computation to ensure region executes */
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        /* Force runtime to allocate partition structure */
        if (gid == 0 && wid == 0 && vid == 0) {
            int gangs = acc_get_num_gangs(acc_get_device_type());
            int workers = acc_get_num_workers();
            int vector = acc_get_vector_length();
            printf("Dynamic config: gangs=%d, workers=%d, vector=%d\n",
                   gangs, workers, vector);
        }
    }
    
    /* Loop with varying configurations */
    for (int i = 1; i <= 4; i++) {
        int g = i * 2;
        int w = i;
        int v = 32 / i;
        
        #pragma acc parallel num_gangs(g) num_workers(w) vector_length(v)
        {
            /* Empty but forces partition setup */
            __asm__ volatile ("");
        }
    }
}

/* Test diagnostic path with acc_malloc */
void test_diagnostic_path(void) {
    printf("\n=== Testing Diagnostic Path ===\n");
    
    /* Allocate device memory - may trigger diagnostics */
    void *dev_ptr = acc_malloc(1024);
    
    if (dev_ptr == NULL) {
        printf("acc_malloc failed - may trigger diagnostic\n");
    } else {
        /* Use the memory in a parallel region */
        #pragma acc parallel num_gangs(2) num_workers(1) vector_length(32) \
            present(dev_ptr[0:1024])
        {
            /* Simple access */
            char *ptr = (char*)dev_ptr;
            int idx = __pgi_gangidx() * 32 + __pgi_vectoridx();
            if (idx < 1024) {
                ptr[idx] = (char)idx;
            }
        }
        
        acc_free(dev_ptr);
    }
}

/* Mixed structured/unstructured data regions */
void test_mixed_data_regions(void) {
    int N = 512;
    int *host_data = (int*)malloc(N * sizeof(int));
    int *device_data;
    
    printf("\n=== Testing Mixed Data Regions ===\n");
    
    /* Unstructured data region */
    device_data = (int*)acc_malloc(N * sizeof(int));
    
    if (device_data) {
        /* Structured compute on unstructured data */
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) \
            present(device_data[0:N])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                device_data[i] = i * 2;
            }
        }
        
        /* Copy back */
        #pragma acc update host(device_data[0:N])
        
        /* Structured data region */
        #pragma acc data copy(host_data[0:N])
        {
            #pragma acc parallel num_gangs(2) num_workers(4) vector_length(8)
            {
                #pragma acc loop gang worker vector
                for (int i = 0; i < N; i++) {
                    host_data[i] += device_data[i];
                }
            }
        }
        
        acc_free(device_data);
    }
    
    free(host_data);
}

int main(void) {
    printf("Starting OpenACC partition mapping coverage test...\n");
    
    /* Initialize OpenACC */
    acc_init(acc_get_device_type());
    
    /* Run all test suites */
    test_partition_mapping();
    test_nested_partitions();
    test_dynamic_partitions();
    test_diagnostic_path();
    test_mixed_data_regions();
    
    printf("\n=== Test Complete ===\n");
    
    return 0;
}
