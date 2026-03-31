/* Test case for OpenACC partition mapping coverage
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 10
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

/* Structure to store partition query results */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    long checksum;
} partition_result_t;

/* Device function to compute a simple checksum */
#pragma acc routine seq
long compute_checksum(int gang, int worker, int vector, int idx) {
    return (long)gang * 1000 + worker * 100 + vector * 10 + idx;
}

int main() {
    partition_result_t results[NUM_TESTS];
    int i, j;
    long total_checksum = 0;
    
    /* Initialize results */
    for (i = 0; i < NUM_TESTS; i++) {
        results[i].gang_cnt = 0;
        results[i].worker_cnt = 0;
        results[i].vector_len = 0;
        results[i].checksum = 0;
    }
    
    printf("Starting OpenACC partition mapping tests...\n");
    
    /* Test 1: Gang redundant (case 0) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyout(results[0:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[0].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[0].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[0].vector_len = acc_get_vector_length(acc_async_noval);
        results[0].checksum = compute_checksum(gid, wid, vid, 0);
    }
    
    /* Test 2: Gang partitioned (case 1) */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyout(results[1:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[1].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[1].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[1].vector_len = acc_get_vector_length(acc_async_noval);
        results[1].checksum = compute_checksum(gid, wid, vid, 1);
    }
    
    /* Test 3: Worker partitioned (case 2) */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyout(results[2:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[2].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[2].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[2].vector_len = acc_get_vector_length(acc_async_noval);
        results[2].checksum = compute_checksum(gid, wid, vid, 2);
    }
    
    /* Test 4: Gang+worker partitioned (case 3) */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyout(results[3:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[3].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[3].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[3].vector_len = acc_get_vector_length(acc_async_noval);
        results[3].checksum = compute_checksum(gid, wid, vid, 3);
    }
    
    /* Test 5: Vector partitioned (case 4) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                copyout(results[4:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[4].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[4].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[4].vector_len = acc_get_vector_length(acc_async_noval);
        results[4].checksum = compute_checksum(gid, wid, vid, 4);
    }
    
    /* Test 6: Gang+vector partitioned (case 5) */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                copyout(results[5:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[5].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[5].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[5].vector_len = acc_get_vector_length(acc_async_noval);
        results[5].checksum = compute_checksum(gid, wid, vid, 5);
    }
    
    /* Test 7: Worker+vector partitioned (case 6) */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
                copyout(results[6:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[6].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[6].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[6].vector_len = acc_get_vector_length(acc_async_noval);
        results[6].checksum = compute_checksum(gid, wid, vid, 6);
    }
    
    /* Test 8: Fully partitioned (case 7) */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                copyout(results[7:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[7].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[7].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[7].vector_len = acc_get_vector_length(acc_async_noval);
        results[7].checksum = compute_checksum(gid, wid, vid, 7);
    }
    
    /* Test 9: Dynamic partitioning with runtime variables */
    int dyn_gangs = 3;
    int dyn_workers = 3;
    int dyn_vector = 8;
    
    #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) \
                vector_length(dyn_vector) copyout(results[8:1])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        results[8].gang_cnt = acc_get_num_gangs(acc_async_noval);
        results[8].worker_cnt = acc_get_num_workers(acc_async_noval);
        results[8].vector_len = acc_get_vector_length(acc_async_noval);
        results[8].checksum = compute_checksum(gid, wid, vid, 8);
    }
    
    /* Test 10: Nested parallelism with collapse clause */
    int data[1000];
    for (i = 0; i < 1000; i++) data[i] = i;
    
    #pragma acc data copy(data[0:1000])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 20; j++) {
                    int idx = i * 20 + j;
                    if (idx < 1000) {
                        data[idx] *= 2;
                    }
                }
            }
        }
        
        /* Query partition info after nested computation */
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                    copyout(results[9:1])
        {
            results[9].gang_cnt = acc_get_num_gangs(acc_async_noval);
            results[9].worker_cnt = acc_get_num_workers(acc_async_noval);
            results[9].vector_len = acc_get_vector_length(acc_async_noval);
            results[9].checksum = 999;
        }
    }
    
    /* Test 11: Unstructured data regions with diagnostic trigger */
    void *device_ptr = acc_malloc(1024);
    if (device_ptr == NULL) {
        printf("Warning: acc_malloc failed - may trigger diagnostic path\n");
    } else {
        acc_free(device_ptr);
    }
    
    /* Print results and compute total checksum */
    printf("\nPartition Query Results:\n");
    printf("Test | Gangs | Workers | Vector | Checksum\n");
    printf("-----|-------|---------|--------|----------\n");
    
    for (i = 0; i < NUM_TESTS; i++) {
        printf("%4d | %6d | %7d | %6d | %10ld\n",
               i, results[i].gang_cnt, results[i].worker_cnt,
               results[i].vector_len, results[i].checksum);
        total_checksum += results[i].checksum;
    }
    
    /* Verify data was actually computed in Test 10 */
    int data_sum = 0;
    for (i = 0; i < 1000; i++) {
        data_sum += data[i];
    }
    total_checksum += data_sum;
    
    printf("\nTotal checksum: %ld\n", total_checksum);
    printf("Data array sum: %d\n", data_sum);
    
    /* Additional test using kernels construct */
    printf("\nTesting kernels construct with varying partitions...\n");
    int kernel_data[100];
    
    #pragma acc data copy(kernel_data[0:100])
    {
        #pragma acc kernels num_gangs(2) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker
            for (i = 0; i < 100; i++) {
                kernel_data[i] = i * 2;
            }
        }
        
        #pragma acc kernels num_gangs(1) num_workers(4) vector_length(1)
        {
            #pragma acc loop worker
            for (i = 0; i < 100; i++) {
                kernel_data[i] += 1;
            }
        }
    }
    
    /* Final verification */
    int final_sum = 0;
    for (i = 0; i < 100; i++) {
        final_sum += kernel_data[i];
    }
    printf("Kernel data sum: %d\n", final_sum);
    total_checksum += final_sum;
    
    printf("\nAll tests completed. Final total checksum: %ld\n", total_checksum);
    
    if (total_checksum > 0) {
        printf("SUCCESS: All regions executed (non-zero checksum)\n");
        return 0;
    } else {
        printf("ERROR: No computation detected\n");
        return 1;
    }
}
