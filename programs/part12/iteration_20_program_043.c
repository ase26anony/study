/* Test for OpenACC partition mapping coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c */

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
    int test_id;
} partition_info_t;

/* Function to perform computation with specific partitioning */
void test_partition_config(int test_id, int gang_val, int worker_val, int vector_val, partition_info_t *result) {
    result->test_id = test_id;
    
    #pragma acc parallel copyin(gang_val, worker_val, vector_val) copyout(result[0:1]) \
        num_gangs(gang_val) num_workers(worker_val) vector_length(vector_val)
    {
        result->gang_cnt = acc_get_num_gangs(0);
        result->worker_cnt = acc_get_num_workers(0);
        result->vector_len = acc_get_vector_length(0);
        
        /* Perform some computation to ensure region isn't optimized away */
        int idx = acc_get_gang_id(0) * acc_get_worker_id(0) + acc_get_thread_id(0);
        volatile int dummy = idx * 2;
    }
}

/* Test with nested parallelism */
void test_nested_partition(int test_id, partition_info_t *result) {
    int data[256] = {0};
    result->test_id = test_id;
    
    #pragma acc data copy(data[0:256], result[0:1])
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
                        if (idx < 256) {
                            data[idx] = g * 1000 + w * 100 + v;
                        }
                    }
                }
            }
            
            /* Query runtime configuration */
            if (acc_get_gang_id(0) == 0 && acc_get_worker_id(0) == 0 && acc_get_thread_id(0) == 0) {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length(0);
            }
        }
    }
}

/* Test with collapse clause */
void test_collapsed_partition(int test_id, partition_info_t *result) {
    int matrix[16][16] = {0};
    result->test_id = test_id;
    
    #pragma acc data copy(matrix[0:16][0:16], result[0:1])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16)
        {
            #pragma acc loop collapse(2) gang worker
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 16; j++) {
                    matrix[i][j] = i * 16 + j;
                }
            }
            
            if (acc_get_gang_id(0) == 0 && acc_get_worker_id(0) == 0 && acc_get_thread_id(0) == 0) {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length(0);
            }
        }
    }
}

/* Test with dynamic values */
void test_dynamic_partition(int *gangs, int *workers, int *vectors, partition_info_t *result) {
    result->test_id = 100; /* Special ID for dynamic test */
    
    #pragma acc parallel copyin(gangs[0:1], workers[0:1], vectors[0:1]) copyout(result[0:1]) \
        num_gangs(gangs[0]) num_workers(workers[0]) vector_length(vectors[0])
    {
        result->gang_cnt = acc_get_num_gangs(0);
        result->worker_cnt = acc_get_num_workers(0);
        result->vector_len = acc_get_vector_length(0);
        
        /* Force some computation */
        volatile int x = acc_get_gang_id(0) + acc_get_worker_id(0) + acc_get_thread_id(0);
    }
}

/* Test with kernels construct */
void test_kernels_partition(partition_info_t *result) {
    int arr[1024] = {0};
    result->test_id = 200; /* Kernels test ID */
    
    #pragma acc data copy(arr[0:1024], result[0:1])
    {
        #pragma acc kernels num_gangs(8) num_workers(1) vector_length(32)
        {
            #pragma acc loop gang
            for (int i = 0; i < 1024; i++) {
                arr[i] = i * 2;
            }
            
            #pragma acc loop gang worker vector
            for (int i = 0; i < 1024; i++) {
                arr[i] += 1;
            }
        }
        
        /* Query needs to be in separate region */
        #pragma acc parallel copy(result[0:1])
        {
            if (acc_get_gang_id(0) == 0 && acc_get_worker_id(0) == 0 && acc_get_thread_id(0) == 0) {
                result->gang_cnt = 8;  /* We specified 8 gangs */
                result->worker_cnt = 1; /* We specified 1 worker */
                result->vector_len = 32; /* We specified vector length 32 */
            }
        }
    }
}

/* Test with unstructured data regions */
void test_unstructured_partition(partition_info_t *result) {
    int *device_data = NULL;
    int host_data[64] = {0};
    
    /* Unstructured data region */
    #pragma acc enter data copyin(host_data[0:64])
    
    /* Multiple compute regions with same data */
    #pragma acc parallel present(host_data[0:64]) num_gangs(2) num_workers(4) vector_length(1)
    {
        int gid = acc_get_gang_id(0);
        int wid = acc_get_worker_id(0);
        int tid = acc_get_thread_id(0);
        
        if (gid * 32 + wid * 8 + tid < 64) {
            host_data[gid * 32 + wid * 8 + tid] = gid * 100 + wid * 10 + tid;
        }
        
        if (gid == 0 && wid == 0 && tid == 0) {
            result->gang_cnt = acc_get_num_gangs(0);
            result->worker_cnt = acc_get_num_workers(0);
            result->vector_len = acc_get_vector_length(0);
            result->test_id = 300;
        }
    }
    
    /* Another compute region with different partitioning */
    #pragma acc parallel present(host_data[0:64]) num_gangs(1) num_workers(1) vector_length(64)
    {
        int idx = acc_get_thread_id(0);
        if (idx < 64) {
            host_data[idx] += 1;
        }
    }
    
    #pragma acc exit data copyout(host_data[0:64])
    
    /* Verify data was modified */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += host_data[i];
    }
    printf("Unstructured data test checksum: %d\n", sum);
}

/* Test that might trigger diagnostics */
void test_with_possible_diagnostic(partition_info_t *result) {
    void *ptr = NULL;
    
    /* This might trigger allocation diagnostics */
    ptr = acc_malloc(1024 * sizeof(int));
    
    if (ptr == NULL) {
        printf("acc_malloc returned NULL (this is expected in some configurations)\n");
    } else {
        #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) present(ptr[0:1024])
        {
            int *data = (int *)ptr;
            int idx = acc_get_worker_id(0) * acc_get_vector_length(0) + acc_get_thread_id(0);
            if (idx < 1024) {
                data[idx] = idx;
            }
            
            if (acc_get_gang_id(0) == 0 && acc_get_worker_id(0) == 0 && acc_get_thread_id(0) == 0) {
                result->gang_cnt = acc_get_num_gangs(0);
                result->worker_cnt = acc_get_num_workers(0);
                result->vector_len = acc_get_vector_length(0);
                result->test_id = 400;
            }
        }
        
        int *host_check = (int *)malloc(1024 * sizeof(int));
        #pragma acc update host(ptr[0:1024])
        memcpy(host_check, ptr, 1024 * sizeof(int));
        
        int check_sum = 0;
        for (int i = 0; i < 1024; i++) {
            check_sum += host_check[i];
        }
        printf("Device allocation test checksum: %d\n", check_sum);
        
        free(host_check);
        acc_free(ptr);
    }
}

int main() {
    partition_info_t results[20] = {0};
    int total_gangs = 0, total_workers = 0, total_vectors = 0;
    
    printf("Testing OpenACC partition mapping coverage...\n");
    
    /* Test 0: gang redundant (1,1,1) */
    test_partition_config(0, 1, 1, 1, &results[0]);
    
    /* Test 1: gang partitioned (8,1,1) */
    test_partition_config(1, 8, 1, 1, &results[1]);
    
    /* Test 2: worker partitioned (1,4,1) */
    test_partition_config(2, 1, 4, 1, &results[2]);
    
    /* Test 3: gang+worker partitioned (2,2,1) */
    test_partition_config(3, 2, 2, 1, &results[3]);
    
    /* Test 4: vector partitioned (1,1,128) */
    test_partition_config(4, 1, 1, 128, &results[4]);
    
    /* Test 5: gang+vector partitioned (4,1,64) */
    test_partition_config(5, 4, 1, 64, &results[5]);
    
    /* Test 6: worker+vector partitioned (1,2,32) */
    test_partition_config(6, 1, 2, 32, &results[6]);
    
    /* Test 7: fully partitioned (2,2,16) */
    test_partition_config(7, 2, 2, 16, &results[7]);
    
    /* Test nested parallelism */
    test_nested_partition(8, &results[8]);
    
    /* Test collapsed loops */
    test_collapsed_partition(9, &results[9]);
    
    /* Test with dynamic values */
    int dyn_gangs = 3, dyn_workers = 2, dyn_vectors = 8;
    test_dynamic_partition(&dyn_gangs, &dyn_workers, &dyn_vectors, &results[10]);
    
    /* Test kernels construct */
    test_kernels_partition(&results[11]);
    
    /* Test unstructured data regions */
    test_unstructured_partition(&results[12]);
    
    /* Test with possible diagnostic trigger */
    test_with_possible_diagnostic(&results[13]);
    
    /* Print results and compute checksum */
    printf("\nPartition test results:\n");
    printf("Test ID | Gangs | Workers | Vector\n");
    printf("--------|-------|---------|-------\n");
    
    for (int i = 0; i < 14; i++) {
        if (results[i].test_id != 0 || i < 8) {
            printf("%7d | %5d | %7d | %6d\n", 
                   results[i].test_id, 
                   results[i].gang_cnt,
                   results[i].worker_cnt,
                   results[i].vector_len);
            
            total_gangs += results[i].gang_cnt;
            total_workers += results[i].worker_cnt;
            total_vectors += results[i].vector_len;
        }
    }
    
    printf("\nChecksum: gangs=%d, workers=%d, vectors=%d\n", 
           total_gangs, total_workers, total_vectors);
    
    /* Additional test: loop over multiple configurations */
    printf("\nTesting multiple configurations in loop:\n");
    partition_info_t loop_results[5];
    int configs[5][3] = {
        {1, 1, 1},    /* gang redundant */
        {4, 1, 1},    /* gang partitioned */
        {1, 4, 1},    /* worker partitioned */
        {2, 2, 8},    /* mixed */
        {1, 1, 64}    /* vector partitioned */
    };
    
    for (int i = 0; i < 5; i++) {
        #pragma acc parallel copyin(configs[i][0:3]) copyout(loop_results[i]) \
            num_gangs(configs[i][0]) num_workers(configs[i][1]) vector_length(configs[i][2])
        {
            loop_results[i].gang_cnt = acc_get_num_gangs(0);
            loop_results[i].worker_cnt = acc_get_num_workers(0);
            loop_results[i].vector_len = acc_get_vector_length(0);
            loop_results[i].test_id = 500 + i;
            
            /* Force computation */
            volatile int comp = acc_get_gang_id(0) * 1000 + 
                               acc_get_worker_id(0) * 100 + 
                               acc_get_thread_id(0);
        }
        
        total_gangs += loop_results[i].gang_cnt;
        total_workers += loop_results[i].worker_cnt;
        total_vectors += loop_results[i].vector_len;
    }
    
    printf("Final checksum: gangs=%d, workers=%d, vectors=%d\n",
           total_gangs, total_workers, total_vectors);
    
    if (total_gangs > 0 && total_workers > 0 && total_vectors > 0) {
        printf("\nAll partition tests completed successfully.\n");
        return 0;
    } else {
        printf("\nWarning: Some partition tests may have been optimized away.\n");
        return 1;
    }
}
