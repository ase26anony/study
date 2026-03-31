/* Test for omp-oacc-neuter-broadcast.cc partition string coverage
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

/* Structure to store queried execution configuration */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int device_type;
} exec_config_t;

/* Host array to store results from all configurations */
exec_config_t host_results[NUM_CONFIGS];

/* Device array for storing results */
#pragma acc declare create(host_results)

/* Helper to initialize results */
void init_results() {
    for (int i = 0; i < NUM_CONFIGS; i++) {
        host_results[i].gang_cnt = 0;
        host_results[i].worker_cnt = 0;
        host_results[i].vector_len = 0;
        host_results[i].device_type = -1;
    }
    #pragma acc update device(host_results)
}

/* Test function for different partition configurations */
void test_partition_configs() {
    int i;
    
    /* Configuration 0: gang redundant (all 1) */
    printf("Testing config 0: gang redundant\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(host_results[0:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[0].gang_cnt = acc_get_num_gangs(acc_async_noval);
            host_results[0].worker_cnt = acc_get_num_workers(acc_async_noval);
            host_results[0].vector_len = acc_get_vector_length(acc_async_noval);
            host_results[0].device_type = acc_get_device_type();
        }
    }
    
    /* Configuration 1: gang partitioned */
    printf("Testing config 1: gang partitioned\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(host_results[1:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[1].gang_cnt = acc_get_num_gangs(acc_async_noval);
            host_results[1].worker_cnt = acc_get_num_workers(acc_async_noval);
            host_results[1].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Configuration 2: worker partitioned */
    printf("Testing config 2: worker partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(host_results[2:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[2].gang_cnt = acc_get_num_gangs(acc_async_noval);
            host_results[2].worker_cnt = acc_get_num_workers(acc_async_noval);
            host_results[2].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Configuration 3: gang+worker partitioned */
    printf("Testing config 3: gang+worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(host_results[3:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[3].gang_cnt = acc_get_num_gangs(acc_async_noval);
            host_results[3].worker_cnt = acc_get_num_workers(acc_async_noval);
            host_results[3].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Configuration 4: vector partitioned */
    printf("Testing config 4: vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(host_results[4:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[4].gang_cnt = acc_get_num_gangs(acc_async_noval);
            host_results[4].worker_cnt = acc_get_num_workers(acc_async_noval);
            host_results[4].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Configuration 5: gang+vector partitioned */
    printf("Testing config 5: gang+vector partitioned\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyout(host_results[5:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[5].gang_cnt = acc_get_num_gangs(acc_async_noval);
            host_results[5].worker_cnt = acc_get_num_workers(acc_async_noval);
            host_results[5].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Configuration 6: worker+vector partitioned */
    printf("Testing config 6: worker+vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
        copyout(host_results[6:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[6].gang_cnt = acc_get_num_gangs(acc_async_noval);
            host_results[6].worker_cnt = acc_get_num_workers(acc_async_noval);
            host_results[6].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Configuration 7: fully partitioned */
    printf("Testing config 7: fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        copyout(host_results[7:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[7].gang_cnt = acc_get_num_gangs(acc_async_noval);
            host_results[7].worker_cnt = acc_get_num_workers(acc_async_noval);
            host_results[7].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
}

/* Test with dynamic values for clauses */
void test_dynamic_partitions() {
    int dynamic_gangs = 3;
    int dynamic_workers = 2;
    int dynamic_vector = 8;
    
    printf("Testing dynamic configuration\n");
    #pragma acc parallel num_gangs(dynamic_gangs) \
        num_workers(dynamic_workers) vector_length(dynamic_vector)
    {
        /* Simple computation to ensure region isn't optimized away */
        int idx = __pgi_gangidx() * 100 + __pgi_workeridx() * 10 + __pgi_vectoridx();
        volatile int dummy = idx * 2;
    }
}

/* Test nested parallelism with collapse */
void test_nested_parallelism() {
    int N = 1000;
    float *data = (float*)malloc(N * sizeof(float));
    
    #pragma acc data create(data[0:N])
    {
        /* Outer gang loop with inner worker+vector loops */
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (int i = 0; i < N/4; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < 4; j++) {
                    int idx = i * 4 + j;
                    if (idx < N) {
                        data[idx] = __pgi_gangidx() * 100.0f + 
                                   __pgi_workeridx() * 10.0f + 
                                   __pgi_vectoridx();
                    }
                }
            }
        }
        
        /* Different collapse factor */
        #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16)
        {
            #pragma acc loop gang worker collapse(2)
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    int idx = i * 10 + j;
                    if (idx < N) {
                        data[idx] += 1.0f;
                    }
                }
            }
        }
    }
    
    free(data);
}

/* Test with kernels construct (different code path) */
void test_kernels_construct() {
    int N = 512;
    float *a = (float*)malloc(N * sizeof(float));
    float *b = (float*)malloc(N * sizeof(float));
    float *c = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
    }
    
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc kernels num_gangs(8) num_workers(1) vector_length(32)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    free(a);
    free(b);
    free(c);
}

/* Test diagnostic path with acc_malloc */
void test_diagnostic_path() {
    size_t size = 1024;
    
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16)
    {
        /* Force runtime to potentially generate diagnostic info */
        void *dev_ptr = acc_malloc(size);
        if (dev_ptr == NULL) {
            /* This might trigger diagnostic output including partition info */
            volatile int error = 1;
        } else {
            acc_free(dev_ptr);
        }
    }
}

/* Test with unstructured data regions */
void test_unstructured_regions() {
    int N = 100;
    float *host_data = (float*)malloc(N * sizeof(float));
    float *device_data;
    
    /* Unstructured data enter */
    #pragma acc enter data create(host_data[0:N])
    
    /* Multiple compute regions with same data */
    for (int iter = 0; iter < 3; iter++) {
        int gangs = 1 + (iter % 3);
        int workers = 1 + ((iter + 1) % 3);
        int vector = 8 << (iter % 2);
        
        #pragma acc parallel num_gangs(gangs) num_workers(workers) \
            vector_length(vector) present(host_data[0:N])
        {
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                host_data[i] += gid * 0.1f + wid * 0.01f + vid * 0.001f;
            }
        }
    }
    
    /* Unstructured data exit */
    #pragma acc exit data copyout(host_data[0:N])
    
    free(host_data);
}

int main() {
    int checksum = 0;
    
    printf("Starting partition configuration tests...\n");
    
    /* Initialize results */
    init_results();
    
    /* Test all static partition configurations */
    test_partition_configs();
    
    /* Update host with results */
    #pragma acc update host(host_results[0:NUM_CONFIGS])
    
    /* Calculate checksum and print results */
    printf("\nConfiguration Results:\n");
    printf("Config | Gangs | Workers | Vector\n");
    printf("-------|-------|---------|-------\n");
    for (int i = 0; i < NUM_CONFIGS; i++) {
        printf("%6d | %5d | %7d | %6d\n", 
               i, 
               host_results[i].gang_cnt,
               host_results[i].worker_cnt,
               host_results[i].vector_len);
        
        checksum += host_results[i].gang_cnt + 
                   host_results[i].worker_cnt + 
                   host_results[i].vector_len;
    }
    
    /* Additional tests to stress different code paths */
    test_dynamic_partitions();
    test_nested_parallelism();
    test_kernels_construct();
    test_unstructured_regions();
    
    /* Test diagnostic path last */
    test_diagnostic_path();
    
    printf("\nFinal checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return 0;
}
