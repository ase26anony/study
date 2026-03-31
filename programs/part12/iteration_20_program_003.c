/* Test for OpenACC partition mapping coverage in omp-oacc-neuter-broadcast.cc
 * Covers all 8 partition cases (0-7) plus error case
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 10
#define ARRAY_SIZE 1024

/* Structure to store runtime query results */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int test_id;
} partition_info_t;

/* Host array to store results from device */
partition_info_t host_results[NUM_TESTS];

/* Device array for runtime queries */
#pragma acc declare create(device_results[NUM_TESTS])

/* Initialize device array */
void init_device_results() {
    #pragma acc enter data copyin(host_results)
    #pragma acc parallel present(host_results)
    {
        for (int i = 0; i < NUM_TESTS; i++) {
            host_results[i].gang_cnt = 0;
            host_results[i].worker_cnt = 0;
            host_results[i].vector_len = 0;
            host_results[i].test_id = -1;
        }
    }
    #pragma acc update device(host_results)
}

/* Test 0: Gang redundant (all 1s) */
void test_gang_redundant(int test_idx) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 0;
    }
}

/* Test 1: Gang partitioned */
void test_gang_partitioned(int test_idx) {
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 1;
    }
}

/* Test 2: Worker partitioned */
void test_worker_partitioned(int test_idx) {
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 2;
    }
}

/* Test 3: Gang+worker partitioned */
void test_gang_worker_partitioned(int test_idx) {
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 3;
    }
}

/* Test 4: Vector partitioned */
void test_vector_partitioned(int test_idx) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 4;
    }
}

/* Test 5: Gang+vector partitioned */
void test_gang_vector_partitioned(int test_idx) {
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 5;
    }
}

/* Test 6: Worker+vector partitioned */
void test_worker_vector_partitioned(int test_idx) {
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 6;
    }
}

/* Test 7: Fully partitioned */
void test_fully_partitioned(int test_idx) {
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 7;
    }
}

/* Test with dynamic values (runtime variables) */
void test_dynamic_partition(int test_idx, int g, int w, int v) {
    #pragma acc parallel num_gangs(g) num_workers(w) vector_length(v) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 8;  /* Dynamic test */
    }
}

/* Test with nested parallelism */
void test_nested_partition(int test_idx) {
    int data[ARRAY_SIZE];
    
    #pragma acc data copy(data[0:ARRAY_SIZE])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (int g = 0; g < 4; g++) {
                #pragma acc loop worker
                for (int w = 0; w < 2; w++) {
                    #pragma acc loop vector
                    for (int v = 0; v < 32; v++) {
                        int idx = g * 64 + w * 32 + v;
                        if (idx < ARRAY_SIZE) {
                            data[idx] = g * 1000 + w * 100 + v;
                        }
                    }
                }
            }
        }
    }
    
    /* Query in a separate region to ensure partition info is captured */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
        copy(host_results[test_idx:1])
    {
        host_results[test_idx].gang_cnt = acc_get_num_gangs(0);
        host_results[test_idx].worker_cnt = acc_get_num_workers(0);
        host_results[test_idx].vector_len = acc_get_vector_length();
        host_results[test_idx].test_id = 9;  /* Nested test */
    }
}

/* Test with kernels construct (different code path) */
void test_kernels_partition(int test_idx) {
    float a[ARRAY_SIZE], b[ARRAY_SIZE], c[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
    }
    
    #pragma acc data copyin(a[0:ARRAY_SIZE], b[0:ARRAY_SIZE]) \
                     copyout(c[0:ARRAY_SIZE])
    {
        #pragma acc kernels num_gangs(8) num_workers(1) vector_length(64)
        {
            #pragma acc loop gang
            for (int i = 0; i < ARRAY_SIZE; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    /* Force runtime to process partition info */
    void* dev_ptr = acc_malloc(ARRAY_SIZE * sizeof(float));
    if (dev_ptr == NULL) {
        /* This may trigger diagnostic path with partition string */
        fprintf(stderr, "acc_malloc failed for test %d\n", test_idx);
    } else {
        acc_free(dev_ptr);
    }
}

int main() {
    int checksum = 0;
    
    /* Initialize device results */
    init_device_results();
    
    /* Run all partition tests */
    test_gang_redundant(0);
    test_gang_partitioned(1);
    test_worker_partitioned(2);
    test_gang_worker_partitioned(3);
    test_vector_partitioned(4);
    test_gang_vector_partitioned(5);
    test_worker_vector_partitioned(6);
    test_fully_partitioned(7);
    
    /* Test with dynamic values */
    test_dynamic_partition(8, 3, 2, 8);
    
    /* Test nested parallelism */
    test_nested_partition(9);
    
    /* Test kernels construct (separate code path) */
    test_kernels_partition(0);  /* Reuse index 0 for kernels test */
    
    /* Update results from device */
    #pragma acc update host(host_results[0:NUM_TESTS])
    
    /* Calculate checksum and print results */
    printf("Partition Test Results:\n");
    printf("=======================\n");
    
    for (int i = 0; i < NUM_TESTS; i++) {
        if (host_results[i].test_id >= 0) {
            printf("Test %d (ID=%d): gangs=%d, workers=%d, vector=%d\n",
                   i, host_results[i].test_id,
                   host_results[i].gang_cnt,
                   host_results[i].worker_cnt,
                   host_results[i].vector_len);
            
            checksum += host_results[i].gang_cnt;
            checksum += host_results[i].worker_cnt;
            checksum += host_results[i].vector_len;
            checksum += host_results[i].test_id;
        }
    }
    
    printf("\nChecksum: %d\n", checksum);
    
    /* Additional test with collapse clause */
    {
        int arr[100][100];
        #pragma acc data copy(arr)
        #pragma acc parallel num_gangs(10) num_workers(5) vector_length(16)
        #pragma acc loop collapse(2) gang worker vector
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                arr[i][j] = i * 100 + j;
            }
        }
        
        /* Verify some values */
        int verify_sum = 0;
        #pragma acc parallel reduction(+:verify_sum) \
            num_gangs(4) num_workers(2) vector_length(8)
        #pragma acc loop gang worker vector collapse(2) reduction(+:verify_sum)
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                verify_sum += arr[i][j];
            }
        }
        
        printf("Collapse test verification sum: %d\n", verify_sum);
        checksum += verify_sum;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Test with enter/exit data (unstructured data regions) */
    {
        float* device_data;
        int data_size = 1000;
        
        device_data = (float*)acc_malloc(data_size * sizeof(float));
        if (device_data) {
            #pragma acc enter data copyin(device_data[0:data_size])
            
            #pragma acc parallel present(device_data[0:data_size]) \
                num_gangs(5) num_workers(3) vector_length(4)
            {
                #pragma acc loop gang worker vector
                for (int i = 0; i < data_size; i++) {
                    device_data[i] = i * 0.5f;
                }
            }
            
            #pragma acc exit data copyout(device_data[0:data_size])
            acc_free(device_data);
        }
    }
    
    return 0;
}
