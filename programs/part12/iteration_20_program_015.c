/* Test for omp-oacc-neuter-broadcast.cc partition string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

int main() {
    int results[NUM_TESTS][4] = {0};  // [gangs, workers, vector, checksum]
    int *d_results = NULL;
    int i, j;
    int dynamic_gang_cnt = 4;
    int dynamic_worker_cnt = 2;
    int dynamic_vector_len = 64;
    
    // Allocate device memory for results
    d_results = (int*)acc_malloc(NUM_TESTS * 4 * sizeof(int));
    if (d_results == NULL) {
        fprintf(stderr, "acc_malloc failed - this may trigger diagnostic path\n");
        // Continue anyway - host memory will be used
    }
    
    printf("Testing OpenACC partition configurations...\n");
    
    // Test 0: Gang redundant (all 1s)
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(results[0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        results[0] = g;
        results[1] = w;
        results[2] = v;
        results[3] = g + w + v;  // checksum
    }
    
    // Test 1: Gang partitioned
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(results[4:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        results[0] = g;
        results[1] = w;
        results[2] = v;
        results[3] = g * 100 + w * 10 + v;
    }
    
    // Test 2: Worker partitioned
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(results[8:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        results[0] = g;
        results[1] = w;
        results[2] = v;
        results[3] = g + w * 10 + v;
    }
    
    // Test 3: Gang+Worker partitioned
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(results[12:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        results[0] = g;
        results[1] = w;
        results[2] = v;
        results[3] = g * 100 + w * 10 + v;
    }
    
    // Test 4: Vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(results[16:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        results[0] = g;
        results[1] = w;
        results[2] = v;
        results[3] = g + w + v * 10;
    }
    
    // Test 5: Gang+Vector partitioned
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyout(results[20:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        results[0] = g;
        results[1] = w;
        results[2] = v;
        results[3] = g * 100 + w + v * 10;
    }
    
    // Test 6: Worker+Vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
        copyout(results[24:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        results[0] = g;
        results[1] = w;
        results[2] = v;
        results[3] = g + w * 100 + v * 10;
    }
    
    // Test 7: Fully partitioned
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) \
        copyout(results[28:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        results[0] = g;
        results[1] = w;
        results[2] = v;
        results[3] = g * 1000 + w * 100 + v * 10;
    }
    
    // Test with dynamic values (may trigger different code paths)
    printf("\nTesting with dynamic clause values...\n");
    int dyn_results[4] = {0};
    
    #pragma acc parallel num_gangs(dynamic_gang_cnt) \
        num_workers(dynamic_worker_cnt) vector_length(dynamic_vector_len) \
        copyout(dyn_results[0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        dyn_results[0] = g;
        dyn_results[1] = w;
        dyn_results[2] = v;
        dyn_results[3] = g * 100 + w * 10 + v;
    }
    
    // Test nested parallelism with collapse
    printf("\nTesting nested parallelism...\n");
    int nested_data[256] = {0};
    int sum = 0;
    
    #pragma acc data copy(nested_data[0:256])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang
            for (i = 0; i < 4; i++) {
                #pragma acc loop worker
                for (j = 0; j < 2; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 32; k++) {
                        int idx = i * 64 + j * 32 + k;
                        if (idx < 256) {
                            nested_data[idx] = i * 1000 + j * 100 + k;
                        }
                    }
                }
            }
        }
        
        // Another region with collapse
        #pragma acc parallel num_gangs(8) vector_length(32)
        {
            #pragma acc loop gang vector collapse(2)
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 4; j++) {
                    int idx = i * 4 + j;
                    if (idx < 32) {
                        nested_data[idx] += 1;
                    }
                }
            }
        }
    }
    
    // Calculate checksum from all results
    int total_checksum = 0;
    for (i = 0; i < NUM_TESTS; i++) {
        total_checksum += results[i][3];
        printf("Test %d: gangs=%d, workers=%d, vector=%d, checksum=%d\n",
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
    }
    
    total_checksum += dyn_results[3];
    printf("Dynamic: gangs=%d, workers=%d, vector=%d, checksum=%d\n",
           dyn_results[0], dyn_results[1], dyn_results[2], dyn_results[3]);
    
    // Also sum nested data
    for (i = 0; i < 256; i++) {
        sum += nested_data[i];
    }
    total_checksum += sum;
    
    printf("\nTotal checksum: %d (non-zero indicates execution)\n", total_checksum);
    
    // Test with unstructured data regions
    printf("\nTesting unstructured data regions...\n");
    int *unstructured_data = (int*)malloc(100 * sizeof(int));
    
    #pragma acc enter data copyin(unstructured_data[0:100])
    
    #pragma acc parallel present(unstructured_data[0:100]) \
        num_gangs(2) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < 100; i++) {
            unstructured_data[i] = i * 2;
        }
    }
    
    #pragma acc exit data copyout(unstructured_data[0:100])
    
    // Verify some values
    int unstructured_sum = 0;
    for (i = 0; i < 100; i++) {
        unstructured_sum += unstructured_data[i];
    }
    printf("Unstructured data sum: %d\n", unstructured_sum);
    
    free(unstructured_data);
    
    if (d_results != NULL) {
        acc_free(d_results);
    }
    
    // Force potential diagnostic path with NULL check
    void *test_alloc = acc_malloc(0);
    if (test_alloc == NULL) {
        printf("Note: acc_malloc(0) returned NULL (expected)\n");
    } else {
        acc_free(test_alloc);
    }
    
    return 0;
}
