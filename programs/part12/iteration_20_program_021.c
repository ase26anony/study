/* Test case for OpenACC partition mapping coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* For diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c */

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
} partition_result_t;

/* Function to perform computation with specific partitioning */
void run_partition_test(int test_id, int num_gangs, int num_workers, int vec_length, 
                       partition_result_t *result, float *data, int data_size) {
    
    result->test_id = test_id;
    
    #pragma acc parallel copyin(num_gangs, num_workers, vec_length) \
                         copyout(result[0:1]) \
                         copy(data[0:data_size]) \
                         num_gangs(num_gangs) num_workers(num_workers) vector_length(vec_length)
    {
        int g, w, v;
        
        /* Query runtime for actual execution configuration */
        g = acc_get_num_gangs(0);
        w = acc_get_num_workers();
        v = acc_get_vector_length();
        
        /* Store results */
        result->gang_count = g;
        result->worker_count = w;
        result->vector_length = v;
        
        /* Perform actual computation to ensure region isn't optimized away */
        int idx = __pgi_gangidx() * __pgi_workeridx() + __pgi_vectoridx();
        if (idx < data_size) {
            data[idx] = (float)(g * 1000 + w * 100 + v + idx);
        }
    }
}

/* Test with nested parallelism */
void test_nested_partition(float *data, int size) {
    int i, j;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang
        for (i = 0; i < 4; i++) {
            #pragma acc loop worker
            for (j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < size) {
                    data[idx] += 1.0f;
                }
            }
        }
    }
}

/* Test with collapse clause */
void test_collapsed_partition(float *data, int size) {
    int i, j;
    
    #pragma acc parallel copy(data[0:size]) num_gangs(2) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker collapse(2)
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < size) {
                    data[idx] *= 2.0f;
                }
            }
        }
    }
}

/* Test with dynamic values */
void test_dynamic_partition(int dynamic_gangs, int dynamic_workers, 
                           partition_result_t *result, float *data, int size) {
    
    #pragma acc parallel copyin(dynamic_gangs, dynamic_workers) \
                         copyout(result[0:1]) \
                         copy(data[0:size]) \
                         num_gangs(dynamic_gangs) num_workers(dynamic_workers) vector_length(64)
    {
        result->gang_count = acc_get_num_gangs(0);
        result->worker_count = acc_get_num_workers();
        result->vector_length = acc_get_vector_length();
        
        /* Simple computation */
        int idx = __pgi_gangidx() * 64 + __pgi_vectoridx();
        if (idx < size) {
            data[idx] += result->gang_count * 10.0f + result->worker_count;
        }
    }
}

/* Force diagnostic path through memory allocation */
void test_diagnostic_path(int size) {
    float *device_data = NULL;
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1)
    {
        /* Empty region that might still trigger partition mapping */
    }
    
    /* Try device allocation - may trigger diagnostics */
    device_data = (float *)acc_malloc(size * sizeof(float));
    if (device_data == NULL) {
        /* This error message might include partition info in diagnostic mode */
        fprintf(stderr, "acc_malloc failed for size %d\n", size);
    } else {
        acc_free(device_data);
    }
}

int main() {
    int i;
    int data_size = 1024;
    float *data = (float *)malloc(data_size * sizeof(float));
    partition_result_t results[NUM_TESTS];
    
    /* Initialize data */
    for (i = 0; i < data_size; i++) {
        data[i] = (float)i;
    }
    
    printf("Starting OpenACC partition mapping tests...\n");
    
    /* Test 0: gang redundant (1,1,1) */
    run_partition_test(0, 1, 1, 1, &results[0], data, data_size);
    
    /* Test 1: gang partitioned (8,1,1) */
    run_partition_test(1, 8, 1, 1, &results[1], data, data_size);
    
    /* Test 2: worker partitioned (1,4,1) */
    run_partition_test(2, 1, 4, 1, &results[2], data, data_size);
    
    /* Test 3: gang+worker partitioned (2,2,1) */
    run_partition_test(3, 2, 2, 1, &results[3], data, data_size);
    
    /* Test 4: vector partitioned (1,1,128) */
    run_partition_test(4, 1, 1, VECTOR_LENGTH, &results[4], data, data_size);
    
    /* Test 5: gang+vector partitioned (4,1,64) */
    run_partition_test(5, 4, 1, 64, &results[5], data, data_size);
    
    /* Test 6: worker+vector partitioned (1,2,32) */
    run_partition_test(6, 1, 2, 32, &results[6], data, data_size);
    
    /* Test 7: fully partitioned (2,2,16) */
    run_partition_test(7, 2, 2, 16, &results[7], data, data_size);
    
    /* Test with dynamic values */
    int dyn_gangs = 3;
    int dyn_workers = 2;
    partition_result_t dyn_result;
    test_dynamic_partition(dyn_gangs, dyn_workers, &dyn_result, data, data_size);
    
    /* Test nested parallelism */
    test_nested_partition(data, data_size);
    
    /* Test collapsed loops */
    test_collapsed_partition(data, data_size);
    
    /* Try to trigger diagnostic path */
    test_diagnostic_path(data_size);
    
    /* Print results and compute checksum */
    printf("\nPartition Test Results:\n");
    printf("Test | Gangs | Workers | Vector\n");
    printf("-----|-------|---------|-------\n");
    
    long long checksum = 0;
    for (i = 0; i < NUM_TESTS; i++) {
        printf("%4d | %5d | %7d | %6d\n", 
               i, results[i].gang_count, results[i].worker_count, results[i].vector_length);
        checksum += results[i].gang_count + results[i].worker_count + results[i].vector_length;
    }
    
    printf("\nDynamic test: %d gangs, %d workers, %d vector\n",
           dyn_result.gang_count, dyn_result.worker_count, dyn_result.vector_length);
    checksum += dyn_result.gang_count + dyn_result.worker_count + dyn_result.vector_length;
    
    /* Verify data was actually modified */
    float data_sum = 0.0f;
    for (i = 0; i < data_size; i++) {
        data_sum += data[i];
    }
    
    printf("\nData checksum: %f\n", data_sum);
    printf("Partition checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(data);
    
    return 0;
}
