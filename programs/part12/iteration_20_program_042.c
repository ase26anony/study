/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Targets lines 335-343: partition code to string mapping
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

int main() {
    int i, j, test_id;
    int results[NUM_TESTS][4] = {0}; // [gangs, workers, vector, checksum]
    int *d_results = NULL;
    int host_var_gangs = 4;
    int host_var_workers = 2;
    int host_var_vector = 64;
    
    // Allocate device memory for results
    d_results = (int*)acc_malloc(NUM_TESTS * 4 * sizeof(int));
    if (d_results == NULL) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic path\n");
        // Continue anyway - this might trigger the diagnostic that uses partition strings
    }
    
    // Initialize results on host
    for (i = 0; i < NUM_TESTS; i++) {
        for (j = 0; j < 4; j++) {
            results[i][j] = -1;
        }
    }
    
    // Copy initial results to device
    #pragma acc enter data copyin(results)
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    // Test 0: gang redundant (likely code 0)
    test_id = 0;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyin(test_id) present(results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[test_id][0] = g;
        #pragma acc atomic update
        results[test_id][1] = w;
        #pragma acc atomic update
        results[test_id][2] = v;
        #pragma acc atomic update
        results[test_id][3] += g + w + v; // checksum
    }
    
    // Test 1: gang partitioned (likely code 1)
    test_id = 1;
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyin(test_id) present(results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[test_id][0] = g;
        #pragma acc atomic update
        results[test_id][1] = w;
        #pragma acc atomic update
        results[test_id][2] = v;
        #pragma acc atomic update
        results[test_id][3] += g + w + v;
    }
    
    // Test 2: worker partitioned (likely code 2)
    test_id = 2;
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyin(test_id) present(results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[test_id][0] = g;
        #pragma acc atomic update
        results[test_id][1] = w;
        #pragma acc atomic update
        results[test_id][2] = v;
        #pragma acc atomic update
        results[test_id][3] += g + w + v;
    }
    
    // Test 3: gang+worker partitioned (likely code 3)
    test_id = 3;
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyin(test_id) present(results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[test_id][0] = g;
        #pragma acc atomic update
        results[test_id][1] = w;
        #pragma acc atomic update
        results[test_id][2] = v;
        #pragma acc atomic update
        results[test_id][3] += g + w + v;
    }
    
    // Test 4: vector partitioned (likely code 4)
    test_id = 4;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                copyin(test_id) present(results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[test_id][0] = g;
        #pragma acc atomic update
        results[test_id][1] = w;
        #pragma acc atomic update
        results[test_id][2] = v;
        #pragma acc atomic update
        results[test_id][3] += g + w + v;
    }
    
    // Test 5: gang+vector partitioned (likely code 5)
    test_id = 5;
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                copyin(test_id) present(results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[test_id][0] = g;
        #pragma acc atomic update
        results[test_id][1] = w;
        #pragma acc atomic update
        results[test_id][2] = v;
        #pragma acc atomic update
        results[test_id][3] += g + w + v;
    }
    
    // Test 6: worker+vector partitioned (likely code 6)
    test_id = 6;
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
                copyin(test_id) present(results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[test_id][0] = g;
        #pragma acc atomic update
        results[test_id][1] = w;
        #pragma acc atomic update
        results[test_id][2] = v;
        #pragma acc atomic update
        results[test_id][3] += g + w + v;
    }
    
    // Test 7: fully partitioned (likely code 7) with dynamic values
    test_id = 7;
    #pragma acc parallel num_gangs(host_var_gangs) \
                num_workers(host_var_workers) \
                vector_length(host_var_vector) \
                copyin(test_id, host_var_gangs, host_var_workers, host_var_vector) \
                present(results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc atomic update
        results[test_id][0] = g;
        #pragma acc atomic update
        results[test_id][1] = w;
        #pragma acc atomic update
        results[test_id][2] = v;
        #pragma acc atomic update
        results[test_id][3] += g + w + v;
    }
    
    // Additional test: Nested parallelism with collapse
    printf("Testing nested parallelism with collapse...\n");
    int nested_results[3] = {0};
    #pragma acc data copy(nested_results)
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang collapse(1)
            for (int g = 0; g < 4; g++) {
                #pragma acc loop worker
                for (int w = 0; w < 2; w++) {
                    #pragma acc loop vector
                    for (int v = 0; v < 32; v++) {
                        #pragma acc atomic update
                        nested_results[0] += 1;
                    }
                }
            }
        }
    }
    
    // Additional test: kernels directive with automatic partitioning
    printf("Testing kernels directive...\n");
    int kernels_sum = 0;
    int arr[1000];
    
    #pragma acc data copy(arr, kernels_sum)
    {
        #pragma acc kernels num_gangs(8) num_workers(2) vector_length(64)
        {
            #pragma acc loop gang worker
            for (i = 0; i < 1000; i++) {
                arr[i] = i;
                #pragma acc atomic update
                kernels_sum += i % 100;
            }
        }
    }
    
    // Copy results back from device
    #pragma acc update host(results)
    #pragma acc exit data delete(results)
    
    // Print results and compute final checksum
    int total_checksum = 0;
    printf("\nPartition query results:\n");
    printf("Test | Gangs | Workers | Vector | Checksum\n");
    printf("-----|-------|---------|--------|----------\n");
    
    for (i = 0; i < NUM_TESTS; i++) {
        printf("%4d | %5d | %7d | %6d | %8d\n", 
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
        total_checksum += results[i][3];
    }
    
    total_checksum += nested_results[0] + kernels_sum;
    
    printf("\nNested parallelism count: %d\n", nested_results[0]);
    printf("Kernels sum: %d\n", kernels_sum);
    printf("Total checksum: %d\n", total_checksum);
    
    // Force potential diagnostic output by using acc_malloc with size 0
    // This might trigger error reporting that includes partition strings
    void *null_ptr = acc_malloc(0);
    if (null_ptr == NULL) {
        printf("Note: acc_malloc(0) returned NULL (expected)\n");
    }
    
    // Clean up
    if (d_results != NULL) {
        acc_free(d_results);
    }
    
    // Verify we got some meaningful results
    if (total_checksum > 0) {
        printf("\nTest completed successfully.\n");
        return 0;
    } else {
        printf("\nWarning: Total checksum is zero - regions may have been optimized away\n");
        return 1;
    }
}
