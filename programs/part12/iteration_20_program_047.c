/* Test case for covering partition string mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
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

/* Function to verify allocation (triggers diagnostic path) */
static void check_allocation(void) {
    void *ptr = acc_malloc(1024);
    if (ptr == NULL) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic\n");
    } else {
        acc_free(ptr);
    }
}

int main(void) {
    partition_result_t host_results[NUM_TESTS];
    partition_result_t *dev_results;
    int i, j;
    int checksum = 0;
    
    /* Initialize host results */
    for (i = 0; i < NUM_TESTS; i++) {
        host_results[i].gang_cnt = 0;
        host_results[i].worker_cnt = 0;
        host_results[i].vector_len = 0;
        host_results[i].test_id = i;
    }
    
    /* Allocate device memory for results */
    dev_results = (partition_result_t *)acc_malloc(NUM_TESTS * sizeof(partition_result_t));
    if (dev_results == NULL) {
        fprintf(stderr, "Device allocation failed\n");
        return 1;
    }
    
    /* Copy initialized results to device */
    #pragma acc enter data copyin(host_results[0:NUM_TESTS])
    #pragma acc parallel present(host_results[0:NUM_TESTS]) \
        copyout(dev_results[0:NUM_TESTS])
    {
        #pragma acc loop gang
        for (i = 0; i < NUM_TESTS; i++) {
            dev_results[i] = host_results[i];
        }
    }
    
    printf("Starting partition mapping tests...\n");
    
    /* Test 0: Gang redundant (all 1s) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(dev_results[0:NUM_TESTS])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            dev_results[0].gang_cnt = acc_get_num_gangs(acc_async_noval);
            dev_results[0].worker_cnt = acc_get_num_workers(acc_async_noval);
            dev_results[0].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 1: Gang partitioned */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        present(dev_results[0:NUM_TESTS])
    {
        int gid = __pgi_gangidx();
        if (gid == 0) {
            dev_results[1].gang_cnt = acc_get_num_gangs(acc_async_noval);
            dev_results[1].worker_cnt = acc_get_num_workers(acc_async_noval);
            dev_results[1].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 2: Worker partitioned */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        present(dev_results[0:NUM_TESTS])
    {
        int wid = __pgi_workeridx();
        if (wid == 0) {
            dev_results[2].gang_cnt = acc_get_num_gangs(acc_async_noval);
            dev_results[2].worker_cnt = acc_get_num_workers(acc_async_noval);
            dev_results[2].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 3: Gang+Worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        present(dev_results[0:NUM_TESTS])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        if (gid == 0 && wid == 0) {
            dev_results[3].gang_cnt = acc_get_num_gangs(acc_async_noval);
            dev_results[3].worker_cnt = acc_get_num_workers(acc_async_noval);
            dev_results[3].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 4: Vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        present(dev_results[0:NUM_TESTS])
    {
        int vid = __pgi_vectoridx();
        if (vid == 0) {
            dev_results[4].gang_cnt = acc_get_num_gangs(acc_async_noval);
            dev_results[4].worker_cnt = acc_get_num_workers(acc_async_noval);
            dev_results[4].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 5: Gang+Vector partitioned */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        present(dev_results[0:NUM_TESTS])
    {
        int gid = __pgi_gangidx();
        int vid = __pgi_vectoridx();
        if (gid == 0 && vid == 0) {
            dev_results[5].gang_cnt = acc_get_num_gangs(acc_async_noval);
            dev_results[5].worker_cnt = acc_get_num_workers(acc_async_noval);
            dev_results[5].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 6: Worker+Vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
        present(dev_results[0:NUM_TESTS])
    {
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        if (wid == 0 && vid == 0) {
            dev_results[6].gang_cnt = acc_get_num_gangs(acc_async_noval);
            dev_results[6].worker_cnt = acc_get_num_workers(acc_async_noval);
            dev_results[6].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test 7: Fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        present(dev_results[0:NUM_TESTS])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        if (gid == 0 && wid == 0 && vid == 0) {
            dev_results[7].gang_cnt = acc_get_num_gangs(acc_async_noval);
            dev_results[7].worker_cnt = acc_get_num_workers(acc_async_noval);
            dev_results[7].vector_len = acc_get_vector_length(acc_async_noval);
        }
    }
    
    /* Test with dynamic values (runtime variables) */
    {
        int dyn_gangs = 3;
        int dyn_workers = 2;
        int dyn_vector = 8;
        
        #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) \
            vector_length(dyn_vector) present(dev_results[0:NUM_TESTS])
        {
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            if (gid == 0 && wid == 0 && vid == 0) {
                /* This will be stored in a spare location */
                dev_results[0].gang_cnt += acc_get_num_gangs(acc_async_noval);
            }
        }
    }
    
    /* Nested parallelism test with collapse clause */
    {
        int data[1000];
        #pragma acc data copy(data[0:1000])
        {
            #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
            {
                #pragma acc loop gang collapse(2)
                for (i = 0; i < 10; i++) {
                    for (j = 0; j < 10; j++) {
                        int idx = i * 100 + j * 10;
                        #pragma acc loop worker vector
                        for (int k = 0; k < 10; k++) {
                            data[idx + k] = i + j + k;
                        }
                    }
                }
            }
        }
        
        /* Verify data to prevent optimization */
        int sum = 0;
        for (i = 0; i < 1000; i++) {
            sum += data[i];
        }
        checksum += sum;
    }
    
    /* Mixed structured/unstructured data regions */
    {
        float *host_array, *dev_array;
        host_array = (float*)malloc(1024 * sizeof(float));
        
        #pragma acc enter data create(dev_array[0:1024])
        
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
            present(dev_array[0:1024])
        {
            #pragma acc loop gang
            for (i = 0; i < 1024; i++) {
                dev_array[i] = i * 0.5f;
            }
        }
        
        #pragma acc update host(dev_array[0:1024])
        #pragma acc exit data delete(dev_array)
        
        /* Add to checksum */
        for (i = 0; i < 1024; i++) {
            checksum += (int)dev_array[i];
        }
        free(host_array);
    }
    
    /* Trigger diagnostic path with allocation check */
    check_allocation();
    
    /* Copy results back from device */
    #pragma acc update host(dev_results[0:NUM_TESTS])
    
    /* Print results and compute checksum */
    printf("\nPartition Configuration Results:\n");
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
               dev_results[i].gang_cnt,
               dev_results[i].worker_cnt,
               dev_results[i].vector_len,
               expected[i]);
        
        checksum += dev_results[i].gang_cnt;
        checksum += dev_results[i].worker_cnt;
        checksum += dev_results[i].vector_len;
    }
    
    printf("\nFinal checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates all regions executed)\n");
    
    /* Cleanup */
    #pragma acc exit data delete(host_results)
    acc_free(dev_results);
    
    return 0;
}
