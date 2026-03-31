/* Test for OpenACC partition string coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

/* Structure to store runtime query results */
struct partition_info {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int dynamic_value;
};

/* Function to force runtime to compute partition strings */
void test_partition_mapping(int test_id, struct partition_info *info) {
    int gang_val = info->gang_cnt;
    int worker_val = info->worker_cnt;
    int vector_val = info->vector_len;
    int dynamic = info->dynamic_value;
    
    /* Query variables that will be filled in device */
    int actual_gangs = 0, actual_workers = 0, actual_vector = 0;
    
    /* Test different partition configurations */
    switch(test_id) {
        case 0: /* gang redundant */
            #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                     copyout(actual_gangs, actual_workers, actual_vector)
            {
                actual_gangs = acc_get_num_gangs(acc_async_noval);
                actual_workers = acc_get_num_workers();
                actual_vector = acc_get_vector_length();
            }
            break;
            
        case 1: /* gang partitioned */
            #pragma acc parallel num_gangs(gang_val) num_workers(1) vector_length(1) \
                     copyout(actual_gangs, actual_workers, actual_vector)
            {
                actual_gangs = acc_get_num_gangs(acc_async_noval);
                actual_workers = acc_get_num_workers();
                actual_vector = acc_get_vector_length();
            }
            break;
            
        case 2: /* worker partitioned */
            #pragma acc parallel num_gangs(1) num_workers(worker_val) vector_length(1) \
                     copyout(actual_gangs, actual_workers, actual_vector)
            {
                actual_gangs = acc_get_num_gangs(acc_async_noval);
                actual_workers = acc_get_num_workers();
                actual_vector = acc_get_vector_length();
            }
            break;
            
        case 3: /* gang+worker partitioned */
            #pragma acc parallel num_gangs(gang_val) num_workers(worker_val) vector_length(1) \
                     copyout(actual_gangs, actual_workers, actual_vector)
            {
                actual_gangs = acc_get_num_gangs(acc_async_noval);
                actual_workers = acc_get_num_workers();
                actual_vector = acc_get_vector_length();
            }
            break;
            
        case 4: /* vector partitioned */
            #pragma acc parallel num_gangs(1) num_workers(1) vector_length(vector_val) \
                     copyout(actual_gangs, actual_workers, actual_vector)
            {
                actual_gangs = acc_get_num_gangs(acc_async_noval);
                actual_workers = acc_get_num_workers();
                actual_vector = acc_get_vector_length();
            }
            break;
            
        case 5: /* gang+vector partitioned */
            #pragma acc parallel num_gangs(gang_val) num_workers(1) vector_length(vector_val) \
                     copyout(actual_gangs, actual_workers, actual_vector)
            {
                actual_gangs = acc_get_num_gangs(acc_async_noval);
                actual_workers = acc_get_num_workers();
                actual_vector = acc_get_vector_length();
            }
            break;
            
        case 6: /* worker+vector partitioned */
            #pragma acc parallel num_gangs(1) num_workers(worker_val) vector_length(vector_val) \
                     copyout(actual_gangs, actual_workers, actual_vector)
            {
                actual_gangs = acc_get_num_gangs(acc_async_noval);
                actual_workers = acc_get_num_workers();
                actual_vector = acc_get_vector_length();
            }
            break;
            
        case 7: /* fully partitioned */
            #pragma acc parallel num_gangs(gang_val) num_workers(worker_val) vector_length(vector_val) \
                     copyout(actual_gangs, actual_workers, actual_vector)
            {
                actual_gangs = acc_get_num_gangs(acc_async_noval);
                actual_workers = acc_get_num_workers();
                actual_vector = acc_get_vector_length();
            }
            break;
    }
    
    /* Store results back */
    info->gang_cnt = actual_gangs;
    info->worker_cnt = actual_workers;
    info->vector_len = actual_vector;
}

/* Test nested parallelism with collapse */
void test_nested_partitions() {
    const int N = 1024;
    int data[N];
    int sum = 0;
    
    /* Initialize data on host */
    for (int i = 0; i < N; i++) {
        data[i] = i % 100;
    }
    
    /* Test 1: Gang-only partitioning with nested loops */
    #pragma acc data copy(data[0:N], sum)
    {
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1)
        {
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < N; i++) {
                sum += data[i];
            }
        }
        
        /* Test 2: Worker partitioning with collapse */
        #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1)
        {
            #pragma acc loop worker collapse(2)
            for (int i = 0; i < 32; i++) {
                for (int j = 0; j < 32; j++) {
                    int idx = i * 32 + j;
                    if (idx < N) {
                        data[idx] *= 2;
                    }
                }
            }
        }
        
        /* Test 3: Vector partitioning */
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128)
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                data[i] += 1;
            }
        }
        
        /* Test 4: Mixed gang+worker+vector with nested directives */
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(64)
        {
            #pragma acc loop gang
            for (int block = 0; block < 4; block++) {
                #pragma acc loop worker
                for (int w = 0; w < 2; w++) {
                    #pragma acc loop vector
                    for (int i = 0; i < 64; i++) {
                        int idx = block * 128 + w * 64 + i;
                        if (idx < N) {
                            data[idx] = data[idx] * 3 / 2;
                        }
                    }
                }
            }
        }
    }
    
    /* Force computation to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += data[i];
    }
    printf("Nested partitions checksum: %d\n", checksum);
}

/* Test with kernels directive (different code path) */
void test_kernels_partitions() {
    int array[256];
    int result = 0;
    
    #pragma acc data copy(array[0:256], result)
    {
        /* kernels with gang partitioning */
        #pragma acc kernels num_gangs(8) num_workers(1) vector_length(1)
        {
            #pragma acc loop gang
            for (int i = 0; i < 256; i++) {
                array[i] = i;
            }
        }
        
        /* kernels with vector partitioning */
        #pragma acc kernels num_gangs(1) num_workers(1) vector_length(64)
        {
            #pragma acc loop vector
            for (int i = 0; i < 256; i++) {
                array[i] += array[i % 64];
            }
        }
        
        /* kernels with mixed partitioning */
        #pragma acc kernels num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < 256; i++) {
                result += array[i];
            }
        }
    }
    
    printf("Kernels result: %d\n", result);
}

/* Test dynamic partitioning with runtime values */
void test_dynamic_partitions() {
    int dyn_gang = 2;
    int dyn_worker = 3;
    int dyn_vector = 64;
    
    int query_results[3] = {0, 0, 0};
    
    /* Loop with varying partition sizes */
    for (int iter = 0; iter < 4; iter++) {
        dyn_gang = 1 << iter;  /* 1, 2, 4, 8 */
        dyn_worker = (iter % 2) + 1;  /* 1, 2, 1, 2 */
        dyn_vector = 32 << (iter % 2);  /* 32, 64, 32, 64 */
        
        #pragma acc parallel num_gangs(dyn_gang) num_workers(dyn_worker) vector_length(dyn_vector) \
                 copyout(query_results[0:3])
        {
            query_results[0] = acc_get_num_gangs(acc_async_noval);
            query_results[1] = acc_get_num_workers();
            query_results[2] = acc_get_vector_length();
        }
        
        printf("Dynamic test %d: gangs=%d workers=%d vector=%d\n",
               iter, query_results[0], query_results[1], query_results[2]);
    }
}

/* Test to potentially trigger diagnostic paths */
void test_diagnostic_path() {
    void *device_ptr = NULL;
    size_t alloc_size = 1024;
    
    /* Allocate on device - may trigger diagnostic if fails */
    device_ptr = acc_malloc(alloc_size);
    
    if (device_ptr == NULL) {
        printf("acc_malloc failed - may trigger diagnostic\n");
    } else {
        /* Use the allocated memory in a parallel region */
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
                 present(device_ptr[0:alloc_size])
        {
            /* Simple computation using device memory */
            #pragma acc loop gang worker vector
            for (int i = 0; i < 256; i++) {
                /* Access device memory */
                char *ptr = (char *)device_ptr;
                ptr[i % alloc_size] = i & 0xFF;
            }
        }
        
        acc_free(device_ptr);
    }
}

/* Test unstructured data regions */
void test_unstructured_regions() {
    int host_data[512];
    int *device_data = NULL;
    
    /* Initialize host data */
    for (int i = 0; i < 512; i++) {
        host_data[i] = i;
    }
    
    /* Unstructured data enter */
    #pragma acc enter data copyin(host_data[0:512])
    
    /* Multiple compute regions with same data */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
             present(host_data)
    {
        #pragma acc loop gang
        for (int i = 0; i < 512; i++) {
            host_data[i] *= 2;
        }
    }
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
             present(host_data)
    {
        #pragma acc loop worker
        for (int i = 0; i < 512; i++) {
            host_data[i] += 1;
        }
    }
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
             present(host_data)
    {
        #pragma acc loop vector
        for (int i = 0; i < 512; i++) {
            host_data[i] = host_data[i] / 2;
        }
    }
    
    /* Unstructured data exit */
    #pragma acc exit data copyout(host_data[0:512])
    
    /* Verify some computation was done */
    int sum = 0;
    for (int i = 0; i < 512; i++) {
        sum += host_data[i];
    }
    printf("Unstructured regions sum: %d\n", sum);
}

int main() {
    struct partition_info tests[NUM_TESTS];
    int total_gangs = 0, total_workers = 0, total_vector = 0;
    
    printf("Starting OpenACC partition mapping tests...\n");
    
    /* Initialize test configurations */
    for (int i = 0; i < NUM_TESTS; i++) {
        tests[i].gang_cnt = (i & 1) ? 8 : 1;
        tests[i].worker_cnt = (i & 2) ? 4 : 1;
        tests[i].vector_len = (i & 4) ? VECTOR_LENGTH : 1;
        tests[i].dynamic_value = i * 10;
    }
    
    /* Run all partition mapping tests */
    for (int i = 0; i < NUM_TESTS; i++) {
        test_partition_mapping(i, &tests[i]);
        total_gangs += tests[i].gang_cnt;
        total_workers += tests[i].worker_cnt;
        total_vector += tests[i].vector_len;
        
        printf("Test %d: gangs=%d workers=%d vector=%d\n",
               i, tests[i].gang_cnt, tests[i].worker_cnt, tests[i].vector_len);
    }
    
    /* Run additional tests */
    test_nested_partitions();
    test_kernels_partitions();
    test_dynamic_partitions();
    test_unstructured_regions();
    test_diagnostic_path();
    
    /* Final checksum to ensure all tests executed */
    printf("\nTotal checksum - gangs: %d, workers: %d, vector: %d\n",
           total_gangs, total_workers, total_vector);
    printf("All partition mapping tests completed.\n");
    
    return 0;
}
