/* Test case for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128

/* Structure to store runtime query results */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int partition_code;
} partition_info_t;

/* Function to force runtime to use partition mapping */
void test_partition_mapping(int test_id, partition_info_t *info) {
    int gang_val = 1, worker_val = 1, vector_val = 1;
    
    /* Set different configurations for each test case */
    switch(test_id) {
        case 0: /* gang redundant */
            gang_val = 1; worker_val = 1; vector_val = 1;
            break;
        case 1: /* gang partitioned */
            gang_val = 8; worker_val = 1; vector_val = 1;
            break;
        case 2: /* worker partitioned */
            gang_val = 1; worker_val = 4; vector_val = 1;
            break;
        case 3: /* gang+worker partitioned */
            gang_val = 2; worker_val = 2; vector_val = 1;
            break;
        case 4: /* vector partitioned */
            gang_val = 1; worker_val = 1; vector_val = 128;
            break;
        case 5: /* gang+vector partitioned */
            gang_val = 4; worker_val = 1; vector_val = 64;
            break;
        case 6: /* worker+vector partitioned */
            gang_val = 1; worker_val = 2; vector_val = 32;
            break;
        case 7: /* fully partitioned */
            gang_val = 4; worker_val = 2; vector_val = 16;
            break;
    }
    
    /* Dynamic values to prevent compile-time optimization */
    int dynamic_gang = gang_val + (test_id % 2);
    int dynamic_worker = worker_val + (test_id % 3);
    int dynamic_vector = vector_val + (test_id % 4);
    
    /* Query variables on device */
    int d_gang_cnt = 0, d_worker_cnt = 0, d_vector_len = 0;
    
    #pragma acc parallel num_gangs(dynamic_gang) num_workers(dynamic_worker) vector_length(dynamic_vector) \
                copyout(d_gang_cnt, d_worker_cnt, d_vector_len)
    {
        d_gang_cnt = acc_get_num_gangs(acc_async_noval);
        d_worker_cnt = acc_get_num_workers();
        d_vector_len = acc_get_vector_length();
        
        /* Force computation to prevent optimization */
        int idx = __pgi_gangidx() * __pgi_workeridx() * __pgi_vectoridx();
        volatile int dummy = idx;
    }
    
    info->gang_cnt = d_gang_cnt;
    info->worker_cnt = d_worker_cnt;
    info->vector_len = d_vector_len;
    info->partition_code = test_id;
}

/* Test with nested parallelism */
void test_nested_partition(int test_id, partition_info_t *info) {
    int data[256] = {0};
    int sum = 0;
    
    #pragma acc data copy(data[0:256])
    {
        /* Outer gang loop with inner worker/vector loops */
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
                    present(data[0:256])
        {
            #pragma acc loop gang
            for(int g = 0; g < 4; g++) {
                #pragma acc loop worker
                for(int w = 0; w < 2; w++) {
                    #pragma acc loop vector
                    for(int v = 0; v < 32; v++) {
                        int idx = g * 64 + w * 32 + v;
                        if(idx < 256) {
                            data[idx] = g * 1000 + w * 100 + v;
                        }
                    }
                }
            }
        }
        
        /* Different partition with collapse clause */
        #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16) \
                    copyout(sum)
        {
            #pragma acc loop gang worker vector collapse(2) reduction(+:sum)
            for(int i = 0; i < 16; i++) {
                for(int j = 0; j < 16; j++) {
                    sum += i * j;
                }
            }
        }
    }
    
    info->gang_cnt = acc_get_num_gangs(acc_async_noval);
    info->worker_cnt = acc_get_num_workers();
    info->vector_len = acc_get_vector_length();
}

/* Test with unstructured data regions */
void test_unstructured_partition(int test_id) {
    int *device_data = NULL;
    int host_data[100];
    
    /* Unstructured data region */
    device_data = (int*)acc_malloc(100 * sizeof(int));
    
    if(device_data == NULL) {
        /* This may trigger diagnostic paths */
        fprintf(stderr, "acc_malloc failed for test %d\n", test_id);
        return;
    }
    
    #pragma acc enter data copyin(host_data[0:100])
    
    /* Structured compute with different partition */
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(1) \
                present(device_data[0:100])
    {
        int gang_idx = __pgi_gangidx();
        int worker_idx = __pgi_workeridx();
        int vector_idx = __pgi_vectoridx();
        
        int idx = gang_idx * 100 + worker_idx * 10 + vector_idx;
        if(idx < 100) {
            device_data[idx] = idx * 2;
        }
    }
    
    #pragma acc exit data copyout(host_data[0:100])
    
    acc_free(device_data);
}

/* Mixed OpenACC constructs to stress partition mapping */
void test_mixed_constructs() {
    float a[1024], b[1024], c[1024];
    
    /* Initialize arrays */
    for(int i = 0; i < 1024; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
    }
    
    /* kernels construct with implicit partitioning */
    #pragma acc kernels copyin(a[0:1024], b[0:1024]) copyout(c[0:1024])
    {
        #pragma acc loop gang(32)
        for(int i = 0; i < 1024; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* parallel construct with explicit partitioning */
    #pragma acc parallel num_gangs(16) num_workers(1) vector_length(64) \
                present(a[0:1024], b[0:1024], c[0:1024])
    {
        #pragma acc loop gang vector
        for(int i = 0; i < 1024; i++) {
            c[i] = c[i] * 2.0f;
        }
    }
    
    /* Verify computation */
    float checksum = 0.0f;
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
                copyin(c[0:1024]) copyout(checksum)
    {
        float local_sum = 0.0f;
        #pragma acc loop worker vector reduction(+:local_sum)
        for(int i = 0; i < 1024; i++) {
            local_sum += c[i];
        }
        checksum = local_sum;
    }
    
    printf("Checksum: %f\n", checksum);
}

int main() {
    partition_info_t results[NUM_TESTS];
    int total_gangs = 0, total_workers = 0, total_vector = 0;
    
    printf("Testing OpenACC partition string mapping...\n");
    
    /* Test 1: Basic partition configurations */
    for(int i = 0; i < NUM_TESTS; i++) {
        test_partition_mapping(i, &results[i]);
        total_gangs += results[i].gang_cnt;
        total_workers += results[i].worker_cnt;
        total_vector += results[i].vector_len;
        
        printf("Test %d: gangs=%d, workers=%d, vector=%d\n",
               i, results[i].gang_cnt, results[i].worker_cnt, results[i].vector_len);
    }
    
    /* Test 2: Nested parallelism */
    partition_info_t nested_result;
    test_nested_partition(0, &nested_result);
    total_gangs += nested_result.gang_cnt;
    total_workers += nested_result.worker_cnt;
    total_vector += nested_result.vector_len;
    
    /* Test 3: Unstructured data regions */
    for(int i = 0; i < 2; i++) {
        test_unstructured_partition(i);
    }
    
    /* Test 4: Mixed constructs */
    test_mixed_constructs();
    
    /* Final checksum to ensure all regions executed */
    printf("\nTotal counts - Gangs: %d, Workers: %d, Vector: %d\n",
           total_gangs, total_workers, total_vector);
    
    /* Additional test with runtime condition to potentially trigger diagnostics */
    int *test_ptr = (int*)acc_malloc(10 * sizeof(int));
    if(test_ptr == NULL) {
        fprintf(stderr, "Final acc_malloc test failed - may trigger diagnostic\n");
    } else {
        acc_free(test_ptr);
    }
    
    /* Force device synchronization which may invoke partition logging */
    acc_wait_all();
    
    return 0;
}
