/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128

/* Structure to store queried execution configuration */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    int partition_code;
} partition_config_t;

/* Host array to store results from device queries */
static partition_config_t host_results[NUM_CONFIGS];

/* Device array for runtime queries */
#pragma acc declare create(host_results)

/* Function to perform computation with specific partition configuration */
void test_partition_config(int config_id, int gang_cnt, int worker_cnt, int vector_cnt) {
    int i, j, k;
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    int sum = 0;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data[i] = i % 100;
    }
    
    /* Copy data to device */
    #pragma acc enter data copyin(data[0:N])
    
    /* Execute parallel region with specific partition configuration */
    #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(vector_cnt) \
        present(data[0:N]) copyout(host_results[config_id])
    {
        int g, w, v;
        
        /* Query runtime for actual execution configuration */
        g = acc_get_num_gangs(acc_async_noval);
        w = acc_get_num_workers(acc_async_noval);
        v = acc_get_vector_length(acc_async_noval);
        
        /* Store results in device memory */
        host_results[config_id].gangs = g;
        host_results[config_id].workers = w;
        host_results[config_id].vector_len = v;
        host_results[config_id].partition_code = config_id;
        
        /* Perform actual computation to ensure region isn't optimized away */
        #pragma acc loop gang reduction(+:sum)
        for (i = 0; i < N/vector_cnt; i++) {
            #pragma acc loop worker
            for (j = 0; j < worker_cnt; j++) {
                #pragma acc loop vector
                for (k = 0; k < vector_cnt; k++) {
                    int idx = i * vector_cnt + k;
                    if (idx < N) {
                        sum += data[idx];
                    }
                }
            }
        }
        
        /* Additional computation to stress partition mapping */
        #pragma acc loop gang
        for (i = 0; i < gang_cnt; i++) {
            #pragma acc loop worker
            for (j = 0; j < worker_cnt; j++) {
                #pragma acc loop vector
                for (k = 0; k < vector_cnt; k++) {
                    /* Dummy computation */
                    int temp = i * j * k;
                    host_results[config_id].gangs += (temp == 0) ? 0 : 0;
                }
            }
        }
    }
    
    /* Update host with results from device */
    #pragma acc update host(host_results[config_id])
    
    /* Clean up */
    #pragma acc exit data delete(data[0:N])
    free(data);
    
    /* Print diagnostic info */
    printf("Config %d: gangs=%d, workers=%d, vector=%d, sum=%d\n",
           config_id, host_results[config_id].gangs,
           host_results[config_id].workers,
           host_results[config_id].vector_len, sum);
}

/* Test nested parallelism with collapse clause */
void test_nested_partition(int config_id) {
    int i, j;
    const int N = 512;
    const int M = 256;
    int *matrix = (int*)malloc(N * M * sizeof(int));
    int total = 0;
    
    /* Initialize matrix */
    #pragma acc enter data create(matrix[0:N*M])
    #pragma acc parallel present(matrix[0:N*M])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                matrix[i * M + j] = (i + j) % 256;
            }
        }
    }
    
    /* Test different collapse factors to generate various partition patterns */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(64) \
        present(matrix[0:N*M]) reduction(+:total)
    {
        /* Collapse 2: gang+worker partitioned */
        #pragma acc loop gang worker collapse(2)
        for (i = 0; i < N/2; i++) {
            for (j = 0; j < M/2; j++) {
                total += matrix[i * M + j];
            }
        }
        
        /* Separate vector loop */
        #pragma acc loop vector
        for (i = 0; i < 64; i++) {
            total += i;
        }
    }
    
    /* Update one of the host_results entries */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copy(host_results[config_id])
    {
        host_results[config_id].gangs = acc_get_num_gangs(acc_async_noval);
        host_results[config_id].workers = acc_get_num_workers(acc_async_noval);
        host_results[config_id].vector_len = acc_get_vector_length(acc_async_noval);
    }
    
    #pragma acc exit data delete(matrix[0:N*M])
    free(matrix);
    
    printf("Nested test %d: total=%d\n", config_id, total);
}

/* Test dynamic partitioning with runtime variables */
void test_dynamic_partition() {
    int i;
    int dynamic_gangs[] = {1, 2, 4, 8};
    int dynamic_workers[] = {1, 2, 1, 2};
    int dynamic_vector[] = {1, 1, 64, 32};
    
    for (i = 0; i < 4; i++) {
        int g = dynamic_gangs[i];
        int w = dynamic_workers[i];
        int v = dynamic_vector[i];
        
        #pragma acc parallel num_gangs(g) num_workers(w) vector_length(v) \
            copy(host_results[7])
        {
            /* Query and modify results based on dynamic configuration */
            if (acc_get_num_gangs(acc_async_noval) > 1) {
                host_results[7].gangs = g;
            }
            if (acc_get_num_workers(acc_async_noval) > 1) {
                host_results[7].workers = w;
            }
            if (acc_get_vector_length(acc_async_noval) > 1) {
                host_results[7].vector_len = v;
            }
            
            /* Mixed computation pattern */
            #pragma acc loop gang
            for (int gi = 0; gi < g; gi++) {
                #pragma acc loop worker
                for (int wi = 0; wi < w; wi++) {
                    #pragma acc loop vector
                    for (int vi = 0; vi < v; vi++) {
                        /* Complex addressing to prevent optimization */
                        int idx = gi * w * v + wi * v + vi;
                        host_results[7].partition_code += (idx % 2);
                    }
                }
            }
        }
        
        #pragma acc update host(host_results[7])
        printf("Dynamic config %d: g=%d, w=%d, v=%d\n", i, g, w, v);
    }
}

/* Test with kernels construct (different code path) */
void test_kernels_partition() {
    int i, j;
    const int N = 1024;
    int *array = (int*)malloc(N * sizeof(int));
    int checksum = 0;
    
    for (i = 0; i < N; i++) {
        array[i] = i;
    }
    
    #pragma acc data copy(array[0:N])
    {
        /* kernels construct with explicit gang/worker/vector clauses */
        #pragma acc kernels num_gangs(8) num_workers(1) vector_length(32)
        {
            #pragma acc loop gang
            for (i = 0; i < N/32; i++) {
                #pragma acc loop vector
                for (j = 0; j < 32; j++) {
                    int idx = i * 32 + j;
                    if (idx < N) {
                        array[idx] *= 2;
                    }
                }
            }
        }
        
        /* Another kernels with worker partitioning */
        #pragma acc kernels num_gangs(1) num_workers(4) vector_length(16)
        {
            #pragma acc loop worker
            for (i = 0; i < N/4; i++) {
                #pragma acc loop vector
                for (j = 0; j < 16; j++) {
                    int idx = i * 16 + j;
                    if (idx < N) {
                        array[idx] += 1;
                    }
                }
            }
        }
    }
    
    /* Compute checksum on host */
    for (i = 0; i < N; i++) {
        checksum += array[i];
    }
    
    printf("Kernels test checksum: %d\n", checksum);
    free(array);
}

/* Test diagnostic path with acc_malloc */
void test_diagnostic_path() {
    void *device_ptr;
    size_t alloc_size = 1024 * sizeof(int);
    
    /* This may trigger diagnostic output including partition strings */
    device_ptr = acc_malloc(alloc_size);
    
    if (device_ptr == NULL) {
        printf("acc_malloc failed - may trigger diagnostic\n");
    } else {
        /* Use the memory in a parallel region */
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(2) \
            present_device(device_ptr[0:alloc_size])
        {
            /* Dummy access to prevent optimization */
            int *ptr = (int*)device_ptr;
            #pragma acc loop gang
            for (int i = 0; i < 2; i++) {
                #pragma acc loop worker
                for (int j = 0; j < 2; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 2; k++) {
                        int idx = i * 4 + j * 2 + k;
                        if (idx < 1024) {
                            ptr[idx] = idx;
                        }
                    }
                }
            }
        }
        
        acc_free(device_ptr);
    }
}

int main() {
    int i;
    long total_checksum = 0;
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Initialize host_results on device */
    #pragma acc enter data copyin(host_results[0:NUM_CONFIGS])
    
    /* Test all 8 partition configurations (mapping to cases 0-7) */
    
    /* 0: gang redundant */
    test_partition_config(0, 1, 1, 1);
    
    /* 1: gang partitioned */
    test_partition_config(1, 8, 1, 1);
    
    /* 2: worker partitioned */
    test_partition_config(2, 1, 4, 1);
    
    /* 3: gang+worker partitioned */
    test_partition_config(3, 2, 2, 1);
    
    /* 4: vector partitioned */
    test_partition_config(4, 1, 1, 128);
    
    /* 5: gang+vector partitioned */
    test_partition_config(5, 4, 1, 64);
    
    /* 6: worker+vector partitioned */
    test_partition_config(6, 1, 2, 64);
    
    /* 7: fully partitioned */
    test_partition_config(7, 2, 2, 32);
    
    /* Test nested parallelism */
    test_nested_partition(3);  /* Should trigger gang+worker partitioned */
    
    /* Test dynamic partitioning */
    test_dynamic_partition();
    
    /* Test kernels construct */
    test_kernels_partition();
    
    /* Test diagnostic path */
    test_diagnostic_path();
    
    /* Final update of all results */
    #pragma acc update host(host_results[0:NUM_CONFIGS])
    
    /* Compute checksum to ensure all regions executed */
    for (i = 0; i < NUM_CONFIGS; i++) {
        total_checksum += host_results[i].gangs;
        total_checksum += host_results[i].workers;
        total_checksum += host_results[i].vector_len;
        total_checksum += host_results[i].partition_code;
    }
    
    /* Clean up */
    #pragma acc exit data delete(host_results[0:NUM_CONFIGS])
    
    printf("\nAll tests completed.\n");
    printf("Total checksum: %ld\n", total_checksum);
    printf("Partition configurations tested:\n");
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("  Config %d: %d gangs, %d workers, %d vector length\n",
               i, host_results[i].gangs, host_results[i].workers,
               host_results[i].vector_len);
    }
    
    return 0;
}
