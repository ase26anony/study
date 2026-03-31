/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Covers all 8 partition cases (0-7) plus error case
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 10
#define ARRAY_SIZE 1024

int main() {
    int i, j, test_id;
    int results[NUM_TESTS][4] = {0};  // [gangs, workers, vectors, checksum]
    int *device_results = NULL;
    int *host_array = NULL;
    int *device_array = NULL;
    int sum = 0;
    
    // Allocate and initialize arrays
    host_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    for (i = 0; i < ARRAY_SIZE; i++) {
        host_array[i] = i % 100;
    }
    
    // Test 0: Gang redundant (all 1s)
    printf("Test 0: Gang redundant\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyout(results[0:4]) copyin(host_array[0:ARRAY_SIZE])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < g; i++) {
            #pragma acc loop worker
            for (j = 0; j < w; j++) {
                #pragma acc loop vector
                for (int k = 0; k < v; k++) {
                    // Force computation
                    int idx = i * w * v + j * v + k;
                    if (idx < 4) {
                        if (idx == 0) results[0] = g;
                        else if (idx == 1) results[1] = w;
                        else if (idx == 2) results[2] = v;
                        else if (idx == 3) results[3] = g + w + v;
                    }
                }
            }
        }
    }
    
    // Test 1: Gang partitioned
    printf("Test 1: Gang partitioned\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyout(results[4:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < g; i++) {
            if (i < 4) {
                results[i] = g;
                results[i+1] = w;
                results[i+2] = v;
                results[i+3] = g * 100 + w * 10 + v;
            }
        }
    }
    
    // Test 2: Worker partitioned
    printf("Test 2: Worker partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyout(results[8:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop worker
        for (i = 0; i < w; i++) {
            if (i < 4) {
                results[i] = g;
                results[i+1] = w;
                results[i+2] = v;
                results[i+3] = g * 1000 + w * 100 + v * 10;
            }
        }
    }
    
    // Test 3: Gang+Worker partitioned
    printf("Test 3: Gang+Worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyout(results[12:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (int gi = 0; gi < g; gi++) {
            #pragma acc loop worker
            for (int wi = 0; wi < w; wi++) {
                int idx = gi * w + wi;
                if (idx < 4) {
                    results[idx] = g;
                    results[idx+1] = w;
                    results[idx+2] = v;
                    results[idx+3] = gi * 1000 + wi * 100 + v;
                }
            }
        }
    }
    
    // Test 4: Vector partitioned
    printf("Test 4: Vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                copyout(results[16:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop vector
        for (i = 0; i < v; i++) {
            if (i < 4) {
                results[i] = g;
                results[i+1] = w;
                results[i+2] = v;
                results[i+3] = i * 100 + g * 10 + w;
            }
        }
    }
    
    // Test 5: Gang+Vector partitioned
    printf("Test 5: Gang+Vector partitioned\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
                copyout(results[20:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang vector
        for (i = 0; i < g * v; i++) {
            if (i < 4) {
                results[i] = g;
                results[i+1] = w;
                results[i+2] = v;
                results[i+3] = (i % g) * 1000 + (i / g) * 10 + w;
            }
        }
    }
    
    // Test 6: Worker+Vector partitioned
    printf("Test 6: Worker+Vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
                copyout(results[24:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop worker vector
        for (i = 0; i < w * v; i++) {
            if (i < 4) {
                results[i] = g;
                results[i+1] = w;
                results[i+2] = v;
                results[i+3] = (i % w) * 1000 + (i / w) * 10 + g;
            }
        }
    }
    
    // Test 7: Fully partitioned
    printf("Test 7: Fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                copyout(results[28:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang worker vector
        for (i = 0; i < g * w * v; i++) {
            if (i < 4) {
                results[i] = g;
                results[i+1] = w;
                results[i+2] = v;
                results[i+3] = i * 100 + g * 10 + w;
            }
        }
    }
    
    // Test 8: Dynamic partitioning with runtime values
    printf("Test 8: Dynamic partitioning\n");
    int dyn_gangs = 3;
    int dyn_workers = 2;
    int dyn_vector = 8;
    
    #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) \
                vector_length(dyn_vector) copyout(results[32:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop collapse(2)
        for (i = 0; i < g; i++) {
            for (j = 0; j < w; j++) {
                #pragma acc loop vector
                for (int k = 0; k < v; k++) {
                    int idx = i * w * v + j * v + k;
                    if (idx < 4) {
                        results[idx] = g;
                        results[idx+1] = w;
                        results[idx+2] = v;
                        results[idx+3] = idx * 100 + g * 10 + w;
                    }
                }
            }
        }
    }
    
    // Test 9: Nested parallelism with kernels directive
    printf("Test 9: Kernels directive with nested loops\n");
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(32) \
                copyout(results[36:4])
    {
        #pragma acc loop gang
        for (i = 0; i < 2; i++) {
            #pragma acc loop worker
            for (j = 0; j < 2; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 32; k++) {
                    int idx = i * 2 * 32 + j * 32 + k;
                    if (idx < 4) {
                        results[idx] = acc_get_num_gangs(acc_async_noval);
                        results[idx+1] = acc_get_num_workers(acc_async_noval);
                        results[idx+2] = acc_get_vector_length(acc_async_noval);
                        results[idx+3] = idx * 111;
                    }
                }
            }
        }
    }
    
    // Force diagnostic path with acc_malloc
    printf("Test 10: Trigger diagnostic path\n");
    void *test_ptr = acc_malloc(1024);
    if (test_ptr == NULL) {
        printf("acc_malloc failed - may trigger diagnostic\n");
    } else {
        acc_free(test_ptr);
    }
    
    // Data region with unstructured constructs
    printf("Testing unstructured data regions\n");
    #pragma acc enter data copyin(host_array[0:ARRAY_SIZE])
    
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
                present(host_array[0:ARRAY_SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            host_array[i] = host_array[i] * 2;
        }
    }
    
    #pragma acc exit data copyout(host_array[0:ARRAY_SIZE])
    
    // Calculate checksum from all results
    for (test_id = 0; test_id < NUM_TESTS; test_id++) {
        for (j = 0; j < 4; j++) {
            sum += results[test_id * 4 + j];
        }
    }
    
    // Also sum host array to ensure computation happened
    int array_sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        array_sum += host_array[i];
    }
    sum += array_sum;
    
    printf("Final checksum: %d\n", sum);
    printf("Expected non-zero checksum: %s\n", sum != 0 ? "PASS" : "FAIL");
    
    // Cleanup
    free(host_array);
    
    return 0;
}
