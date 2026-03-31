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
    int gang_count;
    int worker_count;
    int vector_length;
    int test_id;
} partition_info_t;

/* Function to perform computation with specific partition configuration */
void run_partition_test(int test_id, int num_gangs, int num_workers, int vec_length, 
                       partition_info_t *host_results) {
    partition_info_t dev_result = {0, 0, 0, test_id};
    
    #pragma acc parallel copyin(num_gangs, num_workers, vec_length) \
                         copyout(dev_result) \
                         num_gangs(num_gangs) num_workers(num_workers) vector_length(vec_length)
    {
        dev_result.gang_count = acc_get_num_gangs(acc_async_noval);
        dev_result.worker_count = acc_get_num_workers(acc_async_noval);
        dev_result.vector_length = acc_get_vector_length(acc_async_noval);
        dev_result.test_id = test_id;
        
        /* Simple computation to prevent optimization */
        int idx = __pgi_gangidx() * __pgi_workeridx() + __pgi_vectoridx();
        volatile int dummy = idx * 2;
    }
    
    host_results[test_id] = dev_result;
}

/* Test with nested parallelism */
void run_nested_partition_test(int test_id, partition_info_t *host_results) {
    const int N = 1024;
    int *data = (int*)malloc(N * sizeof(int));
    partition_info_t dev_result = {0, 0, 0, test_id};
    
    #pragma acc data copyout(data[0:N]) copyout(dev_result)
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
                             present(data[0:N]) copyout(dev_result)
        {
            dev_result.gang_count = acc_get_num_gangs(acc_async_noval);
            dev_result.worker_count = acc_get_num_workers(acc_async_noval);
            dev_result.vector_length = acc_get_vector_length(acc_async_noval);
            
            #pragma acc loop gang
            for (int g = 0; g < dev_result.gang_count; g++) {
                #pragma acc loop worker
                for (int w = 0; w < dev_result.worker_count; w++) {
                    #pragma acc loop vector
                    for (int v = 0; v < dev_result.vector_length; v++) {
                        int idx = g * dev_result.worker_count * dev_result.vector_length +
                                 w * dev_result.vector_length + v;
                        if (idx < N) {
                            data[idx] = g * 1000 + w * 100 + v;
                        }
                    }
                }
            }
        }
    }
    
    host_results[test_id] = dev_result;
    
    /* Verify computation */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    printf("Nested test checksum: %d\n", sum);
    
    free(data);
}

/* Test with collapse clause */
void run_collapsed_partition_test(int test_id, partition_info_t *host_results) {
    const int N = 256;
    int *data = (int*)malloc(N * N * sizeof(int));
    partition_info_t dev_result = {0, 0, 0, test_id};
    
    #pragma acc data create(data[0:N*N])
    {
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(64) \
                             present(data[0:N*N]) copyout(dev_result)
        {
            dev_result.gang_count = acc_get_num_gangs(acc_async_noval);
            dev_result.worker_count = acc_get_num_workers(acc_async_noval);
            dev_result.vector_length = acc_get_vector_length(acc_async_noval);
            
            #pragma acc loop gang collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    int idx = i * N + j;
                    data[idx] = i + j;
                }
            }
        }
    }
    
    host_results[test_id] = dev_result;
    
    /* Cleanup */
    free(data);
}

/* Test with dynamic values */
void run_dynamic_partition_test(int dynamic_gangs, int dynamic_workers, 
                               int dynamic_vector, partition_info_t *result) {
    partition_info_t dev_result = {0, 0, 0, 7}; /* test_id = 7 for dynamic */
    
    #pragma acc parallel copyout(dev_result) \
                         num_gangs(dynamic_gangs) \
                         num_workers(dynamic_workers) \
                         vector_length(dynamic_vector)
    {
        dev_result.gang_count = acc_get_num_gangs(acc_async_noval);
        dev_result.worker_count = acc_get_num_workers(acc_async_noval);
        dev_result.vector_length = acc_get_vector_length(acc_async_noval);
        
        /* Force some computation */
        volatile int x = __pgi_gangidx() + __pgi_workeridx() + __pgi_vectoridx();
    }
    
    *result = dev_result;
}

/* Test that might trigger diagnostics */
void test_with_potential_diagnostics() {
    void *device_ptr = NULL;
    
    /* Allocate device memory - might trigger diagnostic paths */
    device_ptr = acc_malloc(1024);
    
    if (device_ptr == NULL) {
        printf("Warning: acc_malloc returned NULL\n");
    } else {
        /* Use the memory in a parallel region */
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
                             present(device_ptr[0:1024])
        {
            /* Access device memory */
            char *ptr = (char*)device_ptr;
            int idx = __pgi_gangidx() * __pgi_workeridx() + __pgi_vectoridx();
            if (idx < 1024) {
                ptr[idx] = idx % 256;
            }
        }
        
        acc_free(device_ptr);
    }
}

int main() {
    partition_info_t results[NUM_TESTS];
    int total_gangs = 0, total_workers = 0, total_vectors = 0;
    
    printf("Starting partition mapping coverage tests...\n");
    
    /* Test 0: gang redundant */
    printf("\nTest 0: gang redundant\n");
    run_partition_test(0, 1, 1, 1, results);
    
    /* Test 1: gang partitioned */
    printf("Test 1: gang partitioned\n");
    run_partition_test(1, 8, 1, 1, results);
    
    /* Test 2: worker partitioned */
    printf("Test 2: worker partitioned\n");
    run_partition_test(2, 1, 4, 1, results);
    
    /* Test 3: gang+worker partitioned */
    printf("Test 3: gang+worker partitioned\n");
    run_partition_test(3, 2, 2, 1, results);
    
    /* Test 4: vector partitioned */
    printf("Test 4: vector partitioned\n");
    run_partition_test(4, 1, 1, 128, results);
    
    /* Test 5: gang+vector partitioned */
    printf("Test 5: gang+vector partitioned\n");
    run_partition_test(5, 4, 1, 64, results);
    
    /* Test 6: worker+vector partitioned */
    printf("Test 6: worker+vector partitioned\n");
    run_partition_test(6, 1, 2, 64, results);
    
    /* Test 7: fully partitioned (using dynamic values) */
    printf("Test 7: fully partitioned (dynamic)\n");
    int dyn_gangs = 2, dyn_workers = 2, dyn_vector = 32;
    run_dynamic_partition_test(dyn_gangs, dyn_workers, dyn_vector, &results[7]);
    
    /* Additional nested parallelism test */
    printf("\nRunning nested parallelism test...\n");
    partition_info_t nested_result;
    run_nested_partition_test(8, &nested_result);
    
    /* Collapsed loop test */
    printf("Running collapsed loop test...\n");
    partition_info_t collapsed_result;
    run_collapsed_partition_test(9, &collapsed_result);
    
    /* Test with potential diagnostic paths */
    printf("\nTesting with potential diagnostic paths...\n");
    test_with_potential_diagnostics();
    
    /* Print results and compute checksum */
    printf("\n=== Partition Configuration Results ===\n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf("Test %d: gangs=%d, workers=%d, vector=%d\n",
               i, results[i].gang_count, results[i].worker_count, 
               results[i].vector_length);
        
        total_gangs += results[i].gang_count;
        total_workers += results[i].worker_count;
        total_vectors += results[i].vector_length;
    }
    
    printf("\n=== Checksums ===\n");
    printf("Total gangs: %d\n", total_gangs);
    printf("Total workers: %d\n", total_workers);
    printf("Total vector lengths: %d\n", total_vectors);
    printf("Overall checksum: %d\n", total_gangs + total_workers + total_vectors);
    
    /* Verify we got some non-zero results */
    if (total_gangs > 0 && total_workers > 0 && total_vectors > 0) {
        printf("\nAll partition tests completed successfully!\n");
        return 0;
    } else {
        printf("\nWarning: Some tests may have been optimized away\n");
        return 1;
    }
}
