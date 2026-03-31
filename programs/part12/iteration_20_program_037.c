/* Test for omp-oacc-neuter-broadcast.cc partition string coverage
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
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
    int results[NUM_TESTS][4] = {0};  // [gangs, workers, vector, checksum]
    int *d_results;
    int dynamic_gang_cnt = 4;
    int dynamic_worker_cnt = 2;
    int dynamic_vector_len = 64;
    
    // Allocate device memory for results
    d_results = (int*)acc_malloc(NUM_TESTS * 4 * sizeof(int));
    if (!d_results) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic path\n");
        // Continue anyway - this might trigger the diagnostic that uses partition strings
    }
    
    // Initialize host results array
    for (i = 0; i < NUM_TESTS; i++) {
        for (j = 0; j < 4; j++) {
            results[i][j] = -1;
        }
    }
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    // Test 0: gang redundant (all 1s)
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(results[0:4])
    {
        if (acc_on_device(acc_device_not_host)) {
            results[0] = acc_get_num_gangs(0);
            results[1] = acc_get_num_workers();
            results[2] = acc_get_vector_length();
            results[3] = 1;  // checksum
        }
    }
    
    // Test 1: gang partitioned
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(results[4:4])
    {
        if (acc_on_device(acc_device_not_host)) {
            results[0] = acc_get_num_gangs(0);
            results[1] = acc_get_num_workers();
            results[2] = acc_get_vector_length();
            results[3] = 2;
        }
    }
    
    // Test 2: worker partitioned
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(results[8:4])
    {
        if (acc_on_device(acc_device_not_host)) {
            results[0] = acc_get_num_gangs(0);
            results[1] = acc_get_num_workers();
            results[2] = acc_get_vector_length();
            results[3] = 3;
        }
    }
    
    // Test 3: gang+worker partitioned
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(results[12:4])
    {
        if (acc_on_device(acc_device_not_host)) {
            results[0] = acc_get_num_gangs(0);
            results[1] = acc_get_num_workers();
            results[2] = acc_get_vector_length();
            results[3] = 4;
        }
    }
    
    // Test 4: vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(results[16:4])
    {
        if (acc_on_device(acc_device_not_host)) {
            results[0] = acc_get_num_gangs(0);
            results[1] = acc_get_num_workers();
            results[2] = acc_get_vector_length();
            results[3] = 5;
        }
    }
    
    // Test 5: gang+vector partitioned
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyout(results[20:4])
    {
        if (acc_on_device(acc_device_not_host)) {
            results[0] = acc_get_num_gangs(0);
            results[1] = acc_get_num_workers();
            results[2] = acc_get_vector_length();
            results[3] = 6;
        }
    }
    
    // Test 6: worker+vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
        copyout(results[24:4])
    {
        if (acc_on_device(acc_device_not_host)) {
            results[0] = acc_get_num_gangs(0);
            results[1] = acc_get_num_workers();
            results[2] = acc_get_vector_length();
            results[3] = 7;
        }
    }
    
    // Test 7: fully partitioned
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16) \
        copyout(results[28:4])
    {
        if (acc_on_device(acc_device_not_host)) {
            results[0] = acc_get_num_gangs(0);
            results[1] = acc_get_num_workers();
            results[2] = acc_get_vector_length();
            results[3] = 8;
        }
    }
    
    // Test with dynamic values (may trigger different code paths)
    for (test_id = 0; test_id < 3; test_id++) {
        int dyn_results[4] = {0};
        
        #pragma acc parallel num_gangs(dynamic_gang_cnt) \
            num_workers(dynamic_worker_cnt) vector_length(dynamic_vector_len) \
            copyout(dyn_results[0:4])
        {
            if (acc_on_device(acc_device_not_host)) {
                dyn_results[0] = acc_get_num_gangs(0);
                dyn_results[1] = acc_get_num_workers();
                dyn_results[2] = acc_get_vector_length();
                dyn_results[3] = 100 + test_id;
            }
        }
        
        // Rotate dynamic values
        dynamic_gang_cnt = (dynamic_gang_cnt % 8) + 1;
        dynamic_worker_cnt = (dynamic_worker_cnt % 4) + 1;
        dynamic_vector_len = (dynamic_vector_len % 128) + 1;
    }
    
    // Test nested parallelism with collapse clause
    {
        int nested_data[256] = {0};
        int sum = 0;
        
        #pragma acc data copy(nested_data[0:256])
        {
            #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
            {
                #pragma acc loop gang collapse(1)
                for (i = 0; i < 4; i++) {
                    #pragma acc loop worker
                    for (j = 0; j < 2; j++) {
                        #pragma acc loop vector
                        for (int k = 0; k < 32; k++) {
                            int idx = i * 64 + j * 32 + k;
                            if (idx < 256) {
                                nested_data[idx] = i + j + k;
                            }
                        }
                    }
                }
            }
            
            // Another region with different collapse factor
            #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16)
            {
                #pragma acc loop gang collapse(2)
                for (i = 0; i < 2; i++) {
                    for (int m = 0; m < 2; m++) {
                        #pragma acc loop worker
                        for (j = 0; j < 4; j++) {
                            #pragma acc loop vector
                            for (int k = 0; k < 16; k++) {
                                int idx = (i * 2 + m) * 64 + j * 16 + k;
                                if (idx < 256) {
                                    nested_data[idx] += 1;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Compute checksum on host
        for (i = 0; i < 256; i++) {
            sum += nested_data[i];
        }
        printf("Nested parallelism checksum: %d\n", sum);
    }
    
    // Test kernels directive with automatic partitioning
    {
        int kernels_data[1024];
        int kernels_sum = 0;
        
        #pragma acc data copy(kernels_data[0:1024])
        {
            #pragma acc kernels num_gangs(8) num_workers(2) vector_length(64)
            {
                #pragma acc loop gang
                for (i = 0; i < 8; i++) {
                    #pragma acc loop worker
                    for (j = 0; j < 2; j++) {
                        #pragma acc loop vector
                        for (int k = 0; k < 64; k++) {
                            int idx = i * 128 + j * 64 + k;
                            kernels_data[idx] = idx % 256;
                        }
                    }
                }
            }
            
            #pragma acc kernels num_gangs(1) num_workers(1) vector_length(256)
            {
                #pragma acc loop vector
                for (i = 0; i < 1024; i++) {
                    kernels_data[i] *= 2;
                }
            }
        }
        
        for (i = 0; i < 1024; i++) {
            kernels_sum += kernels_data[i];
        }
        printf("Kernels checksum: %d\n", kernels_sum);
    }
    
    // Print results summary
    printf("\nPartition test results:\n");
    printf("Test | Gangs | Workers | Vector | Checksum\n");
    printf("-----|-------|---------|--------|---------\n");
    
    int total_checksum = 0;
    for (i = 0; i < NUM_TESTS; i++) {
        printf("%4d | %5d | %7d | %6d | %8d\n",
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
        total_checksum += results[i][3];
    }
    
    printf("\nTotal checksum: %d\n", total_checksum);
    
    // Clean up
    if (d_results) {
        acc_free(d_results);
    }
    
    // Force potential diagnostic output by creating a condition
    // that might trigger runtime error reporting
    {
        void *test_ptr = acc_malloc(0);  // Zero-size allocation might fail
        if (!test_ptr) {
            printf("Note: Zero-size acc_malloc returned NULL (expected)\n");
        } else {
            acc_free(test_ptr);
        }
    }
    
    return 0;
}
