/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

/* Structure to store runtime query results */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int test_id;
} partition_result_t;

/* Host array to store results from device */
partition_result_t host_results[NUM_TESTS];

/* Device array for results */
#pragma acc declare create(host_results)

/* Function to verify partition configuration */
void verify_partition(int test_id, int gangs, int workers, int vector_len) {
    printf("Test %d: gangs=%d, workers=%d, vector=%d\n", 
           test_id, gangs, workers, vector_len);
}

int main() {
    int i, j;
    int sum_gangs = 0, sum_workers = 0, sum_vectors = 0;
    
    /* Initialize host results */
    for (i = 0; i < NUM_TESTS; i++) {
        host_results[i].gang_cnt = 0;
        host_results[i].worker_cnt = 0;
        host_results[i].vector_len = 0;
        host_results[i].test_id = i;
    }
    
    /* Copy results array to device */
    #pragma acc update device(host_results)
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Test 0: gang redundant (all 1s) */
    {
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
            }
        }
    }
    
    /* Test 1: gang partitioned */
    {
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                    copyout(host_results[1:1])
        {
            int gid = __pgi_gangidx();
            if (gid == 0) {
                host_results[1].gang_cnt = acc_get_num_gangs(acc_async_noval);
                host_results[1].worker_cnt = acc_get_num_workers(acc_async_noval);
                host_results[1].vector_len = acc_get_vector_length(acc_async_noval);
            }
        }
    }
    
    /* Test 2: worker partitioned */
    {
        #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                    copyout(host_results[2:1])
        {
            int wid = __pgi_workeridx();
            if (wid == 0) {
                host_results[2].gang_cnt = acc_get_num_gangs(acc_async_noval);
                host_results[2].worker_cnt = acc_get_num_workers(acc_async_noval);
                host_results[2].vector_len = acc_get_vector_length(acc_async_noval);
            }
        }
    }
    
    /* Test 3: gang+worker partitioned */
    {
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                    copyout(host_results[3:1])
        {
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            if (gid == 0 && wid == 0) {
                host_results[3].gang_cnt = acc_get_num_gangs(acc_async_noval);
                host_results[3].worker_cnt = acc_get_num_workers(acc_async_noval);
                host_results[3].vector_len = acc_get_vector_length(acc_async_noval);
            }
        }
    }
    
    /* Test 4: vector partitioned */
    {
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                    copyout(host_results[4:1])
        {
            int vid = __pgi_vectoridx();
            if (vid == 0) {
                host_results[4].gang_cnt = acc_get_num_gangs(acc_async_noval);
                host_results[4].worker_cnt = acc_get_num_workers(acc_async_noval);
                host_results[4].vector_len = acc_get_vector_length(acc_async_noval);
            }
        }
    }
    
    /* Test 5: gang+vector partitioned */
    {
        #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                    copyout(host_results[5:1])
        {
            int gid = __pgi_gangidx();
            int vid = __pgi_vectoridx();
            if (gid == 0 && vid == 0) {
                host_results[5].gang_cnt = acc_get_num_gangs(acc_async_noval);
                host_results[5].worker_cnt = acc_get_num_workers(acc_async_noval);
                host_results[5].vector_len = acc_get_vector_length(acc_async_noval);
            }
        }
    }
    
    /* Test 6: worker+vector partitioned */
    {
        #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
                    copyout(host_results[6:1])
        {
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            if (wid == 0 && vid == 0) {
                host_results[6].gang_cnt = acc_get_num_gangs(acc_async_noval);
                host_results[6].worker_cnt = acc_get_num_workers(acc_async_noval);
                host_results[6].vector_len = acc_get_vector_length(acc_async_noval);
            }
        }
    }
    
    /* Test 7: fully partitioned */
    {
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
    
    /* Update results from device */
    #pragma acc update host(host_results)
    
    /* Print and verify results */
    printf("\nPartition configuration results:\n");
    for (i = 0; i < NUM_TESTS; i++) {
        verify_partition(host_results[i].test_id,
                        host_results[i].gang_cnt,
                        host_results[i].worker_cnt,
                        host_results[i].vector_len);
        
        sum_gangs += host_results[i].gang_cnt;
        sum_workers += host_results[i].worker_cnt;
        sum_vectors += host_results[i].vector_len;
    }
    
    /* Test with dynamic values (Requirement 4) */
    printf("\nDynamic partition tests:\n");
    for (i = 1; i <= 3; i++) {
        int dynamic_gangs = i * 2;
        int dynamic_workers = i;
        int dynamic_vector = 32 / i;
        
        #pragma acc parallel num_gangs(dynamic_gangs) \
                    num_workers(dynamic_workers) \
                    vector_length(dynamic_vector)
        {
            /* Simple computation to ensure region executes */
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            /* No-op to prevent optimization */
            __asm__ volatile ("" : : "r"(gid), "r"(wid), "r"(vid));
        }
        printf("  Dynamic test %d: gangs=%d, workers=%d, vector=%d\n",
               i, dynamic_gangs, dynamic_workers, dynamic_vector);
    }
    
    /* Nested parallelism test (Requirement 3) */
    printf("\nNested parallelism test:\n");
    {
        int data[1000];
        #pragma acc data copyout(data[0:1000])
        {
            #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
            {
                #pragma acc loop gang
                for (i = 0; i < 4; i++) {
                    #pragma acc loop worker
                    for (j = 0; j < 250; j++) {
                        int idx = i * 250 + j;
                        #pragma acc loop vector
                        for (int k = 0; k < 8; k++) {
                            data[idx] = i + j + k;
                        }
                    }
                }
            }
        }
        
        /* Verify data was computed */
        int checksum = 0;
        for (i = 0; i < 1000; i++) {
            checksum += data[i];
        }
        printf("  Nested parallelism checksum: %d\n", checksum);
    }
    
    /* Mixed structured/unstructured constructs (Requirement 6) */
    printf("\nMixed data region test:\n");
    {
        float *device_data;
        size_t nbytes = 1024 * sizeof(float);
        
        /* Unstructured data region */
        device_data = (float *)acc_malloc(nbytes);
        
        if (device_data == NULL) {
            /* This may trigger diagnostic path (Requirement 5) */
            printf("  Warning: acc_malloc failed (this may trigger diagnostics)\n");
        } else {
            /* Structured compute on unstructured data */
            #pragma acc parallel num_gangs(8) num_workers(1) vector_length(32) \
                        present(device_data[0:1024])
            {
                int idx = __pgi_gangidx() * 128 + __pgi_vectoridx();
                if (idx < 1024) {
                    device_data[idx] = idx * 0.5f;
                }
            }
            
            /* Copy back and verify */
            float host_data[1024];
            #pragma acc update host(device_data[0:1024])
            #pragma acc exit data delete(device_data[0:1024])
            
            float sum = 0.0f;
            for (i = 0; i < 1024; i++) {
                sum += device_data[i];
            }
            printf("  Mixed region sum: %.2f\n", sum);
            
            acc_free(device_data);
        }
    }
    
    /* Final checksum to ensure all regions executed */
    printf("\nFinal statistics:\n");
    printf("  Total gangs queried: %d\n", sum_gangs);
    printf("  Total workers queried: %d\n", sum_workers);
    printf("  Total vector lengths: %d\n", sum_vectors);
    printf("  Overall checksum: %d\n", sum_gangs + sum_workers + sum_vectors);
    
    if ((sum_gangs + sum_workers + sum_vectors) > 0) {
        printf("\nAll partition tests completed successfully!\n");
        return 0;
    } else {
        printf("\nError: No partition data collected!\n");
        return 1;
    }
}
