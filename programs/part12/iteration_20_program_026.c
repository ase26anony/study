/* Test for omp-oacc-neuter-broadcast.cc partition string coverage
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128

int main() {
    int results[NUM_TESTS][4] = {0}; // [gangs, workers, vector, checksum]
    int *d_results = NULL;
    int i, j;
    
    // Allocate device memory for results
    d_results = (int*)acc_malloc(NUM_TESTS * 4 * sizeof(int));
    if (!d_results) {
        fprintf(stderr, "acc_malloc failed\n");
        return 1;
    }
    
    // Initialize device results to zero
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(d_results[0:NUM_TESTS*4])
    {
        for (int idx = 0; idx < NUM_TESTS * 4; idx++) {
            d_results[idx] = 0;
        }
    }
    
    // Test 0: Gang redundant (all 1s)
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyin(d_results[0:NUM_TESTS*4]) \
        present(d_results[0:NUM_TESTS*4])
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length(0);
        d_results[0] = g;
        d_results[1] = w;
        d_results[2] = v;
        d_results[3] = g + w + v; // checksum
    }
    
    // Test 1: Gang partitioned
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(d_results[0:NUM_TESTS*4]) \
        present(d_results[0:NUM_TESTS*4])
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length(0);
        d_results[4] = g;
        d_results[5] = w;
        d_results[6] = v;
        d_results[7] = g + w + v;
    }
    
    // Test 2: Worker partitioned
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(d_results[0:NUM_TESTS*4]) \
        present(d_results[0:NUM_TESTS*4])
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length(0);
        d_results[8] = g;
        d_results[9] = w;
        d_results[10] = v;
        d_results[11] = g + w + v;
    }
    
    // Test 3: Gang+Worker partitioned
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyin(d_results[0:NUM_TESTS*4]) \
        present(d_results[0:NUM_TESTS*4])
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length(0);
        d_results[12] = g;
        d_results[13] = w;
        d_results[14] = v;
        d_results[15] = g + w + v;
    }
    
    // Test 4: Vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyin(d_results[0:NUM_TESTS*4]) \
        present(d_results[0:NUM_TESTS*4])
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length(0);
        d_results[16] = g;
        d_results[17] = w;
        d_results[18] = v;
        d_results[19] = g + w + v;
    }
    
    // Test 5: Gang+Vector partitioned
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyin(d_results[0:NUM_TESTS*4]) \
        present(d_results[0:NUM_TESTS*4])
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length(0);
        d_results[20] = g;
        d_results[21] = w;
        d_results[22] = v;
        d_results[23] = g + w + v;
    }
    
    // Test 6: Worker+Vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
        copyin(d_results[0:NUM_TESTS*4]) \
        present(d_results[0:NUM_TESTS*4])
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length(0);
        d_results[24] = g;
        d_results[25] = w;
        d_results[26] = v;
        d_results[27] = g + w + v;
    }
    
    // Test 7: Fully partitioned
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16) \
        copyin(d_results[0:NUM_TESTS*4]) \
        present(d_results[0:NUM_TESTS*4])
    {
        int g = acc_get_num_gangs(0);
        int w = acc_get_num_workers(0);
        int v = acc_get_vector_length(0);
        d_results[28] = g;
        d_results[29] = w;
        d_results[30] = v;
        d_results[31] = g + w + v;
    }
    
    // Copy results back to host
    #pragma acc update host(d_results[0:NUM_TESTS*4])
    
    // Copy to local array
    for (i = 0; i < NUM_TESTS; i++) {
        for (j = 0; j < 4; j++) {
            results[i][j] = d_results[i*4 + j];
        }
    }
    
    // Print results and calculate total checksum
    int total_checksum = 0;
    printf("Partition Test Results:\n");
    printf("Test | Gangs | Workers | Vector | Checksum\n");
    printf("-----|-------|---------|--------|----------\n");
    for (i = 0; i < NUM_TESTS; i++) {
        printf("%4d | %5d | %7d | %6d | %8d\n",
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
        total_checksum += results[i][3];
    }
    printf("Total checksum: %d\n", total_checksum);
    
    // Additional test with dynamic values and nested parallelism
    printf("\nTesting nested parallelism with dynamic values:\n");
    
    int dynamic_gangs = 3;
    int dynamic_workers = 5;
    int dynamic_vector = 8;
    
    int nested_results[3] = {0};
    int *d_nested = NULL;
    
    d_nested = (int*)acc_malloc(3 * sizeof(int));
    if (!d_nested) {
        fprintf(stderr, "acc_malloc for nested test failed - this may trigger diagnostics\n");
    } else {
        #pragma acc parallel num_gangs(dynamic_gangs) \
            num_workers(dynamic_workers) vector_length(dynamic_vector) \
            copyout(d_nested[0:3])
        {
            // Outer gang loop
            #pragma acc loop gang
            for (int g = 0; g < acc_get_num_gangs(0); g++) {
                // Inner worker loop
                #pragma acc loop worker
                for (int w = 0; w < acc_get_num_workers(0); w++) {
                    // Innermost vector loop
                    #pragma acc loop vector
                    for (int v = 0; v < acc_get_vector_length(0); v++) {
                        if (g == 0 && w == 0 && v == 0) {
                            d_nested[0] = acc_get_num_gangs(0);
                            d_nested[1] = acc_get_num_workers(0);
                            d_nested[2] = acc_get_vector_length(0);
                        }
                    }
                }
            }
        }
        
        #pragma acc update host(d_nested[0:3])
        printf("Dynamic nested: gangs=%d, workers=%d, vector=%d\n",
               d_nested[0], d_nested[1], d_nested[2]);
        acc_free(d_nested);
    }
    
    // Test with kernels construct (different code path)
    printf("\nTesting kernels construct:\n");
    int kernels_data[100] = {0};
    int kernels_sum = 0;
    
    #pragma acc data copy(kernels_data[0:100])
    {
        #pragma acc kernels num_gangs(2) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < 100; i++) {
                kernels_data[i] = i * 2;
            }
        }
        
        #pragma acc kernels num_gangs(1) num_workers(8) vector_length(1)
        {
            #pragma acc loop gang worker
            for (i = 0; i < 100; i++) {
                kernels_data[i] += 1;
            }
        }
    }
    
    for (i = 0; i < 100; i++) {
        kernels_sum += kernels_data[i];
    }
    printf("Kernels data checksum: %d\n", kernels_sum);
    
    // Cleanup
    acc_free(d_results);
    
    // Force potential diagnostic output by using runtime API
    acc_set_device_num(0, acc_device_default);
    printf("Using device type: %d\n", acc_get_device_type());
    
    return 0;
}
