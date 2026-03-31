/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Targets the switch statement mapping integer codes to partition descriptions
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 10
#define ARRAY_SIZE 1024

/* Structure to store partition query results */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int test_id;
} partition_info_t;

/* Helper to compute checksum */
static int compute_checksum(partition_info_t *results, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += results[i].gang_cnt;
        sum += results[i].worker_cnt;
        sum += results[i].vector_len;
        sum += results[i].test_id;
    }
    return sum;
}

int main() {
    partition_info_t host_results[NUM_TESTS] = {0};
    partition_info_t *dev_results;
    
    /* Allocate device memory for results */
    dev_results = (partition_info_t *)acc_malloc(NUM_TESTS * sizeof(partition_info_t));
    if (!dev_results) {
        fprintf(stderr, "acc_malloc failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    float data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (float)i;
    }
    
    /* Copy data to device */
    #pragma acc enter data copyin(data[0:ARRAY_SIZE])
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Test 0: Gang redundant (likely maps to case 0) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[0:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            dev_results[0].gang_cnt = acc_get_num_gangs(0);
            dev_results[0].worker_cnt = acc_get_num_workers(0);
            dev_results[0].vector_len = acc_get_vector_length();
            dev_results[0].test_id = 0;
        }
        
        /* Simple computation to ensure region isn't optimized away */
        int idx = gid * 64 + wid * 16 + vid;
        if (idx < ARRAY_SIZE) {
            data[idx] *= 1.001f;
        }
    }
    
    /* Test 1: Gang partitioned (case 1) */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[1:1])
    {
        int gid = __pgi_gangidx();
        if (gid == 0) {
            dev_results[1].gang_cnt = acc_get_num_gangs(0);
            dev_results[1].worker_cnt = acc_get_num_workers(0);
            dev_results[1].vector_len = acc_get_vector_length();
            dev_results[1].test_id = 1;
        }
        
        /* Distributed computation */
        for (int i = gid * 128; i < (gid + 1) * 128 && i < ARRAY_SIZE; i++) {
            data[i] += 0.5f;
        }
    }
    
    /* Test 2: Worker partitioned (case 2) */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[2:1])
    {
        int wid = __pgi_workeridx();
        if (wid == 0) {
            dev_results[2].gang_cnt = acc_get_num_gangs(0);
            dev_results[2].worker_cnt = acc_get_num_workers(0);
            dev_results[2].vector_len = acc_get_vector_length();
            dev_results[2].test_id = 2;
        }
        
        /* Worker-specific computation */
        for (int i = wid * 256; i < (wid + 1) * 256 && i < ARRAY_SIZE; i++) {
            data[i] -= 0.25f;
        }
    }
    
    /* Test 3: Gang+worker partitioned (case 3) */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[3:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        
        if (gid == 0 && wid == 0) {
            dev_results[3].gang_cnt = acc_get_num_gangs(0);
            dev_results[3].worker_cnt = acc_get_num_workers(0);
            dev_results[3].vector_len = acc_get_vector_length();
            dev_results[3].test_id = 3;
        }
        
        /* 2D distributed computation */
        int base = (gid * 2 + wid) * 128;
        for (int i = base; i < base + 128 && i < ARRAY_SIZE; i++) {
            data[i] *= 0.99f;
        }
    }
    
    /* Test 4: Vector partitioned (case 4) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[4:1])
    {
        int vid = __pgi_vectoridx();
        if (vid == 0) {
            dev_results[4].gang_cnt = acc_get_num_gangs(0);
            dev_results[4].worker_cnt = acc_get_num_workers(0);
            dev_results[4].vector_len = acc_get_vector_length();
            dev_results[4].test_id = 4;
        }
        
        /* Vectorized computation */
        if (vid < ARRAY_SIZE) {
            data[vid] = data[vid] * 2.0f - 1.0f;
        }
    }
    
    /* Test 5: Gang+vector partitioned (case 5) */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[5:1])
    {
        int gid = __pgi_gangidx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && vid == 0) {
            dev_results[5].gang_cnt = acc_get_num_gangs(0);
            dev_results[5].worker_cnt = acc_get_num_workers(0);
            dev_results[5].vector_len = acc_get_vector_length();
            dev_results[5].test_id = 5;
        }
        
        /* Gang+vector distributed computation */
        int idx = gid * 32 + vid;
        if (idx < ARRAY_SIZE) {
            data[idx] += sinf((float)idx * 0.01f);
        }
    }
    
    /* Test 6: Worker+vector partitioned (case 6) */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[6:1])
    {
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (wid == 0 && vid == 0) {
            dev_results[6].gang_cnt = acc_get_num_gangs(0);
            dev_results[6].worker_cnt = acc_get_num_workers(0);
            dev_results[6].vector_len = acc_get_vector_length();
            dev_results[6].test_id = 6;
        }
        
        /* Worker+vector distributed computation */
        int idx = wid * 64 + vid;
        if (idx < ARRAY_SIZE) {
            data[idx] = sqrtf(fabsf(data[idx]));
        }
    }
    
    /* Test 7: Fully partitioned (case 7) */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[7:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            dev_results[7].gang_cnt = acc_get_num_gangs(0);
            dev_results[7].worker_cnt = acc_get_num_workers(0);
            dev_results[7].vector_len = acc_get_vector_length();
            dev_results[7].test_id = 7;
        }
        
        /* Fully distributed computation */
        int idx = (gid * 2 + wid) * 16 + vid;
        if (idx < ARRAY_SIZE) {
            data[idx] = data[idx] * 0.5f + 0.5f;
        }
    }
    
    /* Test 8: Dynamic partitioning with runtime values */
    {
        int dyn_gangs = 3;
        int dyn_workers = 2;
        int dyn_vector = 8;
        
        #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) \
            vector_length(dyn_vector) present(data[0:ARRAY_SIZE]) \
            copyout(dev_results[8:1])
        {
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            
            if (gid == 0 && wid == 0 && vid == 0) {
                dev_results[8].gang_cnt = acc_get_num_gangs(0);
                dev_results[8].worker_cnt = acc_get_num_workers(0);
                dev_results[8].vector_len = acc_get_vector_length();
                dev_results[8].test_id = 8;
            }
            
            /* Dynamic computation pattern */
            int idx = (gid * dyn_workers + wid) * dyn_vector + vid;
            if (idx < ARRAY_SIZE) {
                data[idx] = cosf(data[idx] * 0.01f);
            }
        }
    }
    
    /* Test 9: Nested parallelism with collapse clause */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
        present(data[0:ARRAY_SIZE]) copyout(dev_results[9:1])
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                if (idx < ARRAY_SIZE) {
                    data[idx] = fmodf(data[idx], 10.0f);
                }
            }
        }
        
        /* Query in first active thread */
        if (__pgi_gangidx() == 0 && __pgi_workeridx() == 0 && __pgi_vectoridx() == 0) {
            dev_results[9].gang_cnt = acc_get_num_gangs(0);
            dev_results[9].worker_cnt = acc_get_num_workers(0);
            dev_results[9].vector_len = acc_get_vector_length();
            dev_results[9].test_id = 9;
        }
    }
    
    /* Copy results back to host */
    #pragma acc update host(dev_results[0:NUM_TESTS])
    
    /* Also test unstructured data regions */
    float *extra_data = (float *)acc_malloc(256 * sizeof(float));
    if (extra_data) {
        #pragma acc parallel num_gangs(2) num_workers(1) vector_length(64) \
            present(extra_data[0:256])
        {
            int gid = __pgi_gangidx();
            int vid = __pgi_vectoridx();
            int idx = gid * 64 + vid;
            if (idx < 256) {
                extra_data[idx] = (float)(gid * 1000 + vid);
            }
        }
        acc_free(extra_data);
    } else {
        fprintf(stderr, "Warning: acc_malloc for extra_data failed\n");
    }
    
    /* Copy device results to host array */
    for (int i = 0; i < NUM_TESTS; i++) {
        host_results[i] = dev_results[i];
    }
    
    /* Final data update */
    #pragma acc update host(data[0:ARRAY_SIZE])
    #pragma acc exit data delete(data[0:ARRAY_SIZE])
    
    /* Free device memory */
    acc_free(dev_results);
    
    /* Print results and compute checksum */
    printf("\nPartition query results:\n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("Test %d: gangs=%d, workers=%d, vector=%d\n",
               host_results[i].test_id,
               host_results[i].gang_cnt,
               host_results[i].worker_cnt,
               host_results[i].vector_len);
    }
    
    int checksum = compute_checksum(host_results, NUM_TESTS);
    printf("\nTotal checksum: %d\n", checksum);
    
    /* Verify data was actually modified */
    float data_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_sum += data[i];
    }
    printf("Final data sum: %f\n", data_sum);
    
    /* Force potential diagnostic output by checking device type */
    acc_device_t dev_type = acc_get_device_type();
    printf("Active device type: %d\n", (int)dev_type);
    
    return 0;
}
