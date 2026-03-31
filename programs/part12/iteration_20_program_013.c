/* Test for OpenACC partition mapping coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -ftree-parallelize-loops=2 -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128

/* Structure to store partition query results */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int test_id;
} partition_result_t;

/* Helper to force runtime to compute partition strings */
void force_partition_mapping(int test_id, partition_result_t *result) {
    switch(test_id) {
        case 0: /* gang redundant */
            #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyout(result[0:1])
            {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length();
                result->test_id = test_id;
            }
            break;
            
        case 1: /* gang partitioned */
            #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyout(result[0:1])
            {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length();
                result->test_id = test_id;
            }
            break;
            
        case 2: /* worker partitioned */
            #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyout(result[0:1])
            {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length();
                result->test_id = test_id;
            }
            break;
            
        case 3: /* gang+worker partitioned */
            #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyout(result[0:1])
            {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length();
                result->test_id = test_id;
            }
            break;
            
        case 4: /* vector partitioned */
            #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                copyout(result[0:1])
            {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length();
                result->test_id = test_id;
            }
            break;
            
        case 5: /* gang+vector partitioned */
            #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                copyout(result[0:1])
            {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length();
                result->test_id = test_id;
            }
            break;
            
        case 6: /* worker+vector partitioned */
            #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
                copyout(result[0:1])
            {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length();
                result->test_id = test_id;
            }
            break;
            
        case 7: /* fully partitioned */
            #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                copyout(result[0:1])
            {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length();
                result->test_id = test_id;
            }
            break;
    }
}

/* Test nested parallelism with collapse */
void test_nested_partitions() {
    const int N = 1024;
    int data[N];
    
    #pragma acc data copyout(data[0:N])
    {
        /* Nested gang/worker/vector loops */
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (int i = 0; i < N/4; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < 4; j++) {
                    int idx = i*4 + j;
                    if (idx < N) {
                        data[idx] = acc_get_num_gangs(0) * 1000 + 
                                   acc_get_num_workers(0) * 100 + 
                                   acc_get_vector_length();
                    }
                }
            }
        }
        
        /* Collapsed loops with different factors */
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(64)
        {
            #pragma acc loop gang collapse(2)
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 4; j++) {
                    int idx = i*4 + j;
                    if (idx < N) {
                        data[idx] += acc_get_num_gangs(0);
                    }
                }
            }
        }
    }
    
    /* Verify some computation was done */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    printf("Nested partition checksum: %d\n", sum);
}

/* Test dynamic partitioning with runtime variables */
void test_dynamic_partitions() {
    partition_result_t dyn_results[3];
    
    for (int iter = 0; iter < 3; iter++) {
        int gang_cnt = 1 << iter;  /* 1, 2, 4 */
        int worker_cnt = 2 - iter; /* 2, 1, 0 (will use 1) */
        if (worker_cnt < 1) worker_cnt = 1;
        int vec_len = 32 >> iter;  /* 32, 16, 8 */
        
        #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(vec_len) \
            copyout(dyn_results[iter:1])
        {
            dyn_results[iter].gang_cnt = acc_get_num_gangs(0);
            dyn_results[iter].worker_cnt = acc_get_num_workers(0);
            dyn_results[iter].vector_len = acc_get_vector_length();
            dyn_results[iter].test_id = 100 + iter;
        }
    }
    
    /* Force update to ensure runtime executes */
    #pragma acc update host(dyn_results[0:3])
    
    printf("Dynamic partition results:\n");
    for (int i = 0; i < 3; i++) {
        printf("  Test %d: gangs=%d workers=%d vector=%d\n",
               dyn_results[i].test_id,
               dyn_results[i].gang_cnt,
               dyn_results[i].worker_cnt,
               dyn_results[i].vector_len);
    }
}

/* Test with unstructured data regions */
void test_unstructured_regions() {
    int *device_data = NULL;
    const int N = 256;
    
    /* Unstructured data region */
    device_data = (int*)acc_malloc(N * sizeof(int));
    
    if (device_data == NULL) {
        /* This may trigger diagnostic paths */
        fprintf(stderr, "acc_malloc failed - may generate diagnostic\n");
        return;
    }
    
    /* Structured compute on unstructured data */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) \
        present_deviceptr(device_data)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            device_data[i] = i * acc_get_num_gangs(0);
        }
    }
    
    /* Copy back and verify */
    int host_data[N];
    #pragma acc update host(device_data[0:N])
    acc_memcpy_from_device(host_data, device_data, N * sizeof(int));
    
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += host_data[i];
    }
    printf("Unstructured region checksum: %d\n", sum);
    
    acc_free(device_data);
}

/* Test kernels directive with automatic partitioning */
void test_kernels_partition() {
    const int N = 512;
    int data[N];
    
    #pragma acc kernels copyout(data[0:N]) num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang
        for (int i = 0; i < N/2; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < 2; j++) {
                int idx = i*2 + j;
                data[idx] = acc_get_num_gangs(0) + acc_get_num_workers(0);
            }
        }
    }
    
    /* Force computation */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    printf("Kernels partition checksum: %d\n", sum);
}

int main() {
    partition_result_t results[NUM_TESTS];
    int total_gangs = 0, total_workers = 0, total_vector = 0;
    
    printf("Testing OpenACC partition mapping coverage...\n");
    
    /* Test all 8 partition configurations */
    for (int i = 0; i < NUM_TESTS; i++) {
        force_partition_mapping(i, &results[i]);
        
        /* Force update to ensure execution */
        #pragma acc update host(results[i:1])
        
        total_gangs += results[i].gang_cnt;
        total_workers += results[i].worker_cnt;
        total_vector += results[i].vector_len;
        
        printf("Test %d: gangs=%d workers=%d vector=%d\n",
               i, results[i].gang_cnt, results[i].worker_cnt, results[i].vector_len);
    }
    
    printf("\nTotal checksum: gangs=%d workers=%d vector=%d\n",
           total_gangs, total_workers, total_vector);
    
    /* Additional tests to stress different code paths */
    test_nested_partitions();
    test_dynamic_partitions();
    test_unstructured_regions();
    test_kernels_partition();
    
    /* Final verification that all tests executed */
    if (total_gangs > 0 && total_vector > 0) {
        printf("\nAll partition tests completed successfully.\n");
    } else {
        printf("\nWarning: Some tests may have been optimized away.\n");
    }
    
    return 0;
}
