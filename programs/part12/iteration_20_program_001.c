/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define NUM_CONFIGS 8

/* Structure to store queried execution configuration */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    long checksum;
} exec_config_t;

/* Host array to store results from device queries */
exec_config_t host_results[NUM_CONFIGS];

/* Device array for runtime queries */
#pragma acc declare create(host_results)

/* Initialize host_results on device */
void init_device_results() {
    #pragma acc enter data copyin(host_results[0:NUM_CONFIGS])
}

/* Update host with device results */
void sync_results_to_host() {
    #pragma acc update host(host_results[0:NUM_CONFIGS])
}

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant() {
    printf("Testing: gang redundant\n");
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(host_results[0:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        /* Only first thread in first gang stores results */
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[0].gangs = acc_get_num_gangs(acc_async_noval);
            host_results[0].workers = acc_get_num_workers(acc_async_noval);
            host_results[0].vector_len = acc_get_vector_length(acc_async_noval);
            
            /* Simple computation to ensure region executes */
            int local_sum = 0;
            #pragma acc loop vector reduction(+:local_sum)
            for (int i = 0; i < 32; i++) {
                local_sum += i;
            }
            host_results[0].checksum = local_sum;
        }
    }
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned() {
    printf("Testing: gang partitioned\n");
    
    int dynamic_gangs = 4;
    
    #pragma acc parallel num_gangs(dynamic_gangs) num_workers(1) vector_length(1) \
        copyout(host_results[1:1])
    {
        int gid = __pgi_gangidx();
        
        /* Each gang contributes to result */
        if (__pgi_workeridx() == 0 && __pgi_vectoridx() == 0) {
            /* Use atomic to avoid race conditions */
            #pragma acc atomic update
            host_results[1].gangs += 1;
            
            /* Each gang computes its contribution */
            int gang_sum = 0;
            #pragma acc loop vector
            for (int i = 0; i < 16; i++) {
                gang_sum += gid * 16 + i;
            }
            
            #pragma acc atomic update
            host_results[1].checksum += gang_sum;
        }
        
        /* Set workers and vector length once */
        if (gid == 0) {
            host_results[1].workers = acc_get_num_workers(acc_async_noval);
            host_results[1].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned() {
    printf("Testing: worker partitioned\n");
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(host_results[2:1])
    {
        int wid = __pgi_workeridx();
        
        if (__pgi_gangidx() == 0 && __pgi_vectoridx() == 0) {
            #pragma acc atomic update
            host_results[2].workers += 1;
            
            /* Worker-specific computation */
            int worker_sum = 0;
            #pragma acc loop vector
            for (int i = 0; i < 8; i++) {
                worker_sum += wid * 8 + i;
            }
            
            #pragma acc atomic update
            host_results[2].checksum += worker_sum;
        }
        
        if (wid == 0) {
            host_results[2].gangs = acc_get_num_gangs(acc_async_noval);
            host_results[2].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned() {
    printf("Testing: gang+worker partitioned\n");
    
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(host_results[3:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        
        if (__pgi_vectoridx() == 0) {
            /* Count unique gang-worker pairs */
            if (wid == 0) {
                #pragma acc atomic update
                host_results[3].gangs += 1;
            }
            if (gid == 0) {
                #pragma acc atomic update
                host_results[3].workers += 1;
            }
            
            /* Compute contribution from each worker */
            int contribution = gid * 100 + wid * 10;
            #pragma acc atomic update
            host_results[3].checksum += contribution;
        }
        
        if (gid == 0 && wid == 0) {
            host_results[3].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned() {
    printf("Testing: vector partitioned\n");
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(host_results[4:1])
    {
        int vid = __pgi_vectoridx();
        
        if (__pgi_gangidx() == 0 && __pgi_workeridx() == 0) {
            #pragma acc atomic update
            host_results[4].vector_len += 1;
            
            /* Vector lane computation */
            #pragma acc atomic update
            host_results[4].checksum += vid;
        }
        
        if (vid == 0) {
            host_results[4].gangs = acc_get_num_gangs(acc_async_noval);
            host_results[4].workers = acc_get_num_workers(acc_async_noval);
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned() {
    printf("Testing: gang+vector partitioned\n");
    
    #pragma acc parallel num_gangs(2) num_workers(1) vector_length(64) \
        copyout(host_results[5:1])
    {
        int gid = __pgi_gangidx();
        int vid = __pgi_vectoridx();
        
        if (__pgi_workeridx() == 0) {
            if (vid == 0) {
                #pragma acc atomic update
                host_results[5].gangs += 1;
            }
            if (gid == 0) {
                #pragma acc atomic update
                host_results[5].vector_len += 1;
            }
            
            /* Compute contribution */
            int contribution = gid * 1000 + vid;
            #pragma acc atomic update
            host_results[5].checksum += contribution;
        }
        
        if (gid == 0 && vid == 0) {
            host_results[5].workers = acc_get_num_workers(acc_async_noval);
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned() {
    printf("Testing: worker+vector partitioned\n");
    
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
        copyout(host_results[6:1])
    {
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (__pgi_gangidx() == 0) {
            if (vid == 0) {
                #pragma acc atomic update
                host_results[6].workers += 1;
            }
            if (wid == 0) {
                #pragma acc atomic update
                host_results[6].vector_len += 1;
            }
            
            /* Compute contribution */
            int contribution = wid * 100 + vid;
            #pragma acc atomic update
            host_results[6].checksum += contribution;
        }
        
        if (wid == 0 && vid == 0) {
            host_results[6].gangs = acc_get_num_gangs(acc_async_noval);
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned() {
    printf("Testing: fully partitioned\n");
    
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        copyout(host_results[7:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        /* Count each dimension */
        if (wid == 0 && vid == 0) {
            #pragma acc atomic update
            host_results[7].gangs += 1;
        }
        if (gid == 0 && vid == 0) {
            #pragma acc atomic update
            host_results[7].workers += 1;
        }
        if (gid == 0 && wid == 0) {
            #pragma acc atomic update
            host_results[7].vector_len += 1;
        }
        
        /* Fully unique contribution */
        int contribution = gid * 10000 + wid * 100 + vid;
        #pragma acc atomic update
        host_results[7].checksum += contribution;
    }
}

/* Test with nested parallelism using kernels directive */
void test_nested_partitioning() {
    printf("Testing nested parallelism with kernels\n");
    
    int data[N];
    long sum = 0;
    
    #pragma acc data copy(data[0:N], sum)
    {
        #pragma acc kernels num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (int i = 0; i < N/32; i++) {
                #pragma acc loop worker
                for (int j = 0; j < 2; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 16; k++) {
                        int idx = i*32 + j*16 + k;
                        if (idx < N) {
                            data[idx] = idx * 2;
                        }
                    }
                }
            }
            
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < N/16; i++) {
                #pragma acc loop worker reduction(+:sum)
                for (int j = 0; j < 2; j++) {
                    #pragma acc loop vector reduction(+:sum)
                    for (int k = 0; k < 8; k++) {
                        int idx = i*16 + j*8 + k;
                        if (idx < N) {
                            sum += data[idx];
                        }
                    }
                }
            }
        }
    }
    
    printf("Nested kernels sum: %ld\n", sum);
}

/* Test with unstructured data regions */
void test_unstructured_regions() {
    printf("Testing unstructured data regions\n");
    
    int *device_data = NULL;
    int host_data[64];
    long check = 0;
    
    /* Unstructured data enter */
    #pragma acc enter data copyin(host_data[0:64])
    
    /* Multiple parallel regions with same data */
    for (int iter = 0; iter < 3; iter++) {
        int gangs = 1 << iter;  /* 1, 2, 4 */
        int workers = 2;
        int vector_len = 16;
        
        #pragma acc parallel num_gangs(gangs) num_workers(workers) \
            vector_length(vector_len) present(host_data)
        {
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            
            int idx = gid * workers * vector_len + wid * vector_len + vid;
            if (idx < 64) {
                host_data[idx] = iter * 1000 + idx;
            }
        }
    }
    
    /* Exit data with update */
    #pragma acc update host(host_data[0:64])
    #pragma acc exit data delete(host_data)
    
    /* Verify on host */
    for (int i = 0; i < 64; i++) {
        check += host_data[i];
    }
    printf("Unstructured regions check: %ld\n", check);
}

/* Test that might trigger diagnostic paths */
void test_with_potential_diagnostics() {
    printf("Testing with potential diagnostic triggers\n");
    
    /* Try acc_malloc to potentially trigger diagnostic paths */
    void *dev_ptr = acc_malloc(1024);
    
    if (dev_ptr == NULL) {
        printf("acc_malloc failed (unexpected)\n");
    } else {
        /* Use the allocated memory in a parallel region */
        #pragma acc parallel num_gangs(2) num_workers(1) vector_length(64) \
            present_or_copyin(dev_ptr[0:1024])
        {
            int gid = __pgi_gangidx();
            int vid = __pgi_vectoridx();
            
            if (gid == 0 && vid == 0) {
                /* Just touch the memory */
                ((char*)dev_ptr)[0] = 42;
            }
        }
        
        acc_free(dev_ptr);
    }
    
    /* Another region with collapse clause */
    int matrix[32][32];
    #pragma acc data copy(matrix)
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang collapse(2)
            for (int i = 0; i < 32; i++) {
                for (int j = 0; j < 32; j++) {
                    matrix[i][j] = i * 32 + j;
                }
            }
        }
    }
}

int main() {
    printf("Starting OpenACC partition coverage test\n");
    
    /* Initialize device results array */
    init_device_results();
    
    /* Clear host results */
    for (int i = 0; i < NUM_CONFIGS; i++) {
        host_results[i].gangs = 0;
        host_results[i].workers = 0;
        host_results[i].vector_len = 0;
        host_results[i].checksum = 0;
    }
    
    /* Run all partition configuration tests */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Sync results from device */
    sync_results_to_host();
    
    /* Print results and compute final checksum */
    long total_checksum = 0;
    printf("\n=== Partition Configuration Results ===\n");
    for (int i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vector=%d, checksum=%ld\n",
               i, host_results[i].gangs, host_results[i].workers,
               host_results[i].vector_len, host_results[i].checksum);
        total_checksum += host_results[i].checksum;
    }
    printf("Total checksum: %ld\n", total_checksum);
    
    /* Additional tests for broader coverage */
    test_nested_partitioning();
    test_unstructured_regions();
    test_with_potential_diagnostics();
    
    printf("\nTest completed successfully\n");
    
    /* Final validation - ensure all configurations produced some output */
    int valid_configs = 0;
    for (int i = 0; i < NUM_CONFIGS; i++) {
        if (host_results[i].checksum != 0) {
            valid_configs++;
        }
    }
    
    if (valid_configs == NUM_CONFIGS) {
        printf("All %d partition configurations executed successfully\n", NUM_CONFIGS);
        return 0;
    } else {
        printf("Warning: Only %d/%d configurations produced results\n", 
               valid_configs, NUM_CONFIGS);
        return 1;
    }
}
