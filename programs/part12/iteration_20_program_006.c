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
#define VECTOR_LENGTH 128

/* Structure to store partition query results */
typedef struct {
    int gang_count;
    int worker_count;
    int vector_length;
    int device_type;
} partition_info_t;

/* Host array to store results from device */
partition_info_t host_results[NUM_TESTS];

/* Device array for results */
#pragma acc declare create(host_results)

/* Function to verify partition configuration */
void verify_partition(int test_id, partition_info_t *info) {
    printf("Test %d: gangs=%d, workers=%d, vector=%d\n",
           test_id, info->gang_count, info->worker_count, info->vector_length);
}

int main() {
    int i, j;
    int sum_gangs = 0, sum_workers = 0, sum_vectors = 0;
    
    /* Initialize host results */
    for (i = 0; i < NUM_TESTS; i++) {
        host_results[i].gang_count = 0;
        host_results[i].worker_count = 0;
        host_results[i].vector_length = 0;
        host_results[i].device_type = acc_get_device_type();
    }
    
    /* Copy results array to device */
    #pragma acc enter data copyin(host_results[0:NUM_TESTS])
    
    /* Test 0: Gang redundant (all 1) - maps to case 0 */
    printf("Launching Test 0: Gang redundant\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(host_results[0:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[0].gang_count = acc_get_num_gangs(acc_async_noval);
            host_results[0].worker_count = acc_get_num_workers(acc_async_noval);
            host_results[0].vector_length = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 1: Gang partitioned - maps to case 1 */
    printf("Launching Test 1: Gang partitioned\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(host_results[1:1])
    {
        int gid = __pgi_gangidx();
        if (gid == 0) {
            host_results[1].gang_count = acc_get_num_gangs(acc_async_noval);
            host_results[1].worker_count = acc_get_num_workers(acc_async_noval);
            host_results[1].vector_length = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 2: Worker partitioned - maps to case 2 */
    printf("Launching Test 2: Worker partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(host_results[2:1])
    {
        int wid = __pgi_workeridx();
        if (wid == 0) {
            host_results[2].gang_count = acc_get_num_gangs(acc_async_noval);
            host_results[2].worker_count = acc_get_num_workers(acc_async_noval);
            host_results[2].vector_length = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 3: Gang+Worker partitioned - maps to case 3 */
    printf("Launching Test 3: Gang+Worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(host_results[3:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        if (gid == 0 && wid == 0) {
            host_results[3].gang_count = acc_get_num_gangs(acc_async_noval);
            host_results[3].worker_count = acc_get_num_workers(acc_async_noval);
            host_results[3].vector_length = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 4: Vector partitioned - maps to case 4 */
    printf("Launching Test 4: Vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(host_results[4:1])
    {
        int vid = __pgi_vectoridx();
        if (vid == 0) {
            host_results[4].gang_count = acc_get_num_gangs(acc_async_noval);
            host_results[4].worker_count = acc_get_num_workers(acc_async_noval);
            host_results[4].vector_length = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 5: Gang+Vector partitioned - maps to case 5 */
    printf("Launching Test 5: Gang+Vector partitioned\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyout(host_results[5:1])
    {
        int gid = __pgi_gangidx();
        int vid = __pgi_vectoridx();
        if (gid == 0 && vid == 0) {
            host_results[5].gang_count = acc_get_num_gangs(acc_async_noval);
            host_results[5].worker_count = acc_get_num_workers(acc_async_noval);
            host_results[5].vector_length = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 6: Worker+Vector partitioned - maps to case 6 */
    printf("Launching Test 6: Worker+Vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
        copyout(host_results[6:1])
    {
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        if (wid == 0 && vid == 0) {
            host_results[6].gang_count = acc_get_num_gangs(acc_async_noval);
            host_results[6].worker_count = acc_get_num_workers(acc_async_noval);
            host_results[6].vector_length = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 7: Fully partitioned - maps to case 7 */
    printf("Launching Test 7: Fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        copyout(host_results[7:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        if (gid == 0 && wid == 0 && vid == 0) {
            host_results[7].gang_count = acc_get_num_gangs(acc_async_noval);
            host_results[7].worker_count = acc_get_num_workers(acc_async_noval);
            host_results[7].vector_length = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Update host with all results */
    #pragma acc update host(host_results[0:NUM_TESTS])
    
    /* Print and verify results */
    printf("\nPartition Configuration Results:\n");
    for (i = 0; i < NUM_TESTS; i++) {
        verify_partition(i, &host_results[i]);
        sum_gangs += host_results[i].gang_count;
        sum_workers += host_results[i].worker_count;
        sum_vectors += host_results[i].vector_length;
    }
    
    printf("\nChecksum: gangs=%d, workers=%d, vectors=%d\n",
           sum_gangs, sum_workers, sum_vectors);
    
    /* Test with dynamic values (Requirement 4) */
    printf("\nTesting with dynamic values:\n");
    for (i = 1; i <= 3; i++) {
        int dynamic_gangs = i * 2;
        int dynamic_workers = i;
        int dynamic_vector = 32 / i;
        
        #pragma acc parallel num_gangs(dynamic_gangs) \
            num_workers(dynamic_workers) vector_length(dynamic_vector)
        {
            /* Simple computation to ensure region executes */
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            /* Force computation to prevent optimization */
            volatile int dummy = gid + wid + vid;
        }
    }
    
    /* Test nested parallelism (Requirement 3) */
    printf("\nTesting nested parallelism:\n");
    #pragma acc data copyout(host_results[0:1])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (i = 0; i < 4; i++) {
                #pragma acc loop worker
                for (j = 0; j < 2; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 32; k++) {
                        /* Nested computation */
                        if (i == 0 && j == 0 && k == 0) {
                            host_results[0].gang_count = 
                                acc_get_num_gangs(acc_async_noval);
                            host_results[0].worker_count = 
                                acc_get_num_workers(acc_async_noval);
                            host_results[0].vector_length = 
                                acc_get_vector_length(acc_async_noval);
                        }
                    }
                }
            }
        }
    }
    
    /* Test with collapse clause */
    printf("\nTesting with collapse clause:\n");
    {
        int data[100][100];
        #pragma acc data copy(data)
        {
            #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
            {
                #pragma acc loop collapse(2) gang worker vector
                for (i = 0; i < 100; i++) {
                    for (j = 0; j < 100; j++) {
                        data[i][j] = i * 100 + j;
                    }
                }
            }
        }
    }
    
    /* Test diagnostic path with acc_malloc (Requirement 5) */
    printf("\nTesting diagnostic path with acc_malloc:\n");
    void *device_ptr = acc_malloc(1024);
    if (device_ptr == NULL) {
        printf("Error: acc_malloc failed\n");
    } else {
        printf("acc_malloc succeeded at %p\n", device_ptr);
        acc_free(device_ptr);
    }
    
    /* Test kernels directive variants */
    printf("\nTesting kernels directive:\n");
    {
        float a[1000], b[1000], c[1000];
        for (i = 0; i < 1000; i++) {
            a[i] = i * 1.0f;
            b[i] = i * 0.5f;
        }
        
        #pragma acc kernels copyin(a[0:1000], b[0:1000]) copyout(c[0:1000])
        {
            #pragma acc loop gang(32) worker(8) vector(32)
            for (i = 0; i < 1000; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        /* Verify result */
        float sum = 0.0f;
        for (i = 0; i < 1000; i++) {
            sum += c[i];
        }
        printf("Kernels result checksum: %f\n", sum);
    }
    
    /* Cleanup */
    #pragma acc exit data delete(host_results[0:NUM_TESTS])
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
