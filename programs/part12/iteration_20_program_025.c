/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

int main() {
    int results[NUM_TESTS][4] = {0}; // [gangs, workers, vector, checksum]
    int i, j, k;
    int dynamic_gang_cnt = 4;
    int dynamic_worker_cnt = 2;
    int dynamic_vector_len = 64;
    
    // Initialize host arrays
    int host_array[1000];
    int *device_array;
    
    #pragma acc data copy(host_array[0:1000])
    {
        // Test 0: gang redundant (all 1s)
        printf("Test 0: gang redundant\n");
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                    copyout(results[0:1][0:4])
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length(0);
            results[0][0] = g;
            results[0][1] = w;
            results[0][2] = v;
            results[0][3] = g + w + v;
        }
        
        // Test 1: gang partitioned
        printf("Test 1: gang partitioned\n");
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                    copyout(results[1:1][0:4])
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length(0);
            results[1][0] = g;
            results[1][1] = w;
            results[1][2] = v;
            results[1][3] = g + w + v;
        }
        
        // Test 2: worker partitioned
        printf("Test 2: worker partitioned\n");
        #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                    copyout(results[2:1][0:4])
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length(0);
            results[2][0] = g;
            results[2][1] = w;
            results[2][2] = v;
            results[2][3] = g + w + v;
        }
        
        // Test 3: gang+worker partitioned
        printf("Test 3: gang+worker partitioned\n");
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                    copyout(results[3:1][0:4])
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length(0);
            results[3][0] = g;
            results[3][1] = w;
            results[3][2] = v;
            results[3][3] = g + w + v;
        }
        
        // Test 4: vector partitioned
        printf("Test 4: vector partitioned\n");
        #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                    copyout(results[4:1][0:4])
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length(0);
            results[4][0] = g;
            results[4][1] = w;
            results[4][2] = v;
            results[4][3] = g + w + v;
        }
        
        // Test 5: gang+vector partitioned
        printf("Test 5: gang+vector partitioned\n");
        #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                    copyout(results[5:1][0:4])
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length(0);
            results[5][0] = g;
            results[5][1] = w;
            results[5][2] = v;
            results[5][3] = g + w + v;
        }
        
        // Test 6: worker+vector partitioned
        printf("Test 6: worker+vector partitioned\n");
        #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
                    copyout(results[6:1][0:4])
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length(0);
            results[6][0] = g;
            results[6][1] = w;
            results[6][2] = v;
            results[6][3] = g + w + v;
        }
        
        // Test 7: fully partitioned
        printf("Test 7: fully partitioned\n");
        #pragma acc parallel num_gangs(4) num_workers(4) vector_length(32) \
                    copyout(results[7:1][0:4])
        {
            int g = acc_get_num_gangs(0);
            int w = acc_get_num_workers(0);
            int v = acc_get_vector_length(0);
            results[7][0] = g;
            results[7][1] = w;
            results[7][2] = v;
            results[7][3] = g + w + v;
        }
        
        // Test with dynamic values (triggers different code paths)
        printf("Test with dynamic values\n");
        #pragma acc parallel num_gangs(dynamic_gang_cnt) \
                    num_workers(dynamic_worker_cnt) \
                    vector_length(dynamic_vector_len) \
                    copyout(host_array[0:100])
        {
            int idx = acc_get_num_gangs(0) * acc_get_num_workers(0) * 
                     acc_get_vector_length(0);
            #pragma acc loop gang worker vector
            for (i = 0; i < 100; i++) {
                host_array[i] = idx + i;
            }
        }
        
        // Nested parallelism test with collapse
        printf("Nested parallelism test\n");
        #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
                    copy(host_array[0:256])
        {
            #pragma acc loop gang collapse(2)
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 4; j++) {
                    #pragma acc loop worker vector
                    for (k = 0; k < 8; k++) {
                        int idx = i * 32 + j * 8 + k;
                        if (idx < 256) {
                            host_array[idx] = acc_get_num_gangs(0) * 1000 + 
                                            acc_get_num_workers(0) * 100 + 
                                            acc_get_vector_length(0);
                        }
                    }
                }
            }
        }
        
        // Unstructured data region with compute
        printf("Unstructured data region test\n");
        #pragma acc enter data copyin(host_array[500:100])
        
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                    present(host_array[500:100])
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < 100; i++) {
                host_array[500 + i] = i * 2;
            }
        }
        
        #pragma acc exit data copyout(host_array[500:100])
        
        // Diagnostic trigger: memory allocation check
        printf("Memory allocation diagnostic test\n");
        device_array = (int*)acc_malloc(1000 * sizeof(int));
        if (device_array == NULL) {
            printf("Warning: acc_malloc returned NULL\n");
        } else {
            #pragma acc parallel num_gangs(2) num_workers(1) vector_length(32) \
                        deviceptr(device_array)
            {
                #pragma acc loop gang worker vector
                for (i = 0; i < 1000; i++) {
                    device_array[i] = i;
                }
            }
            acc_free(device_array);
        }
    }
    
    // Print results and compute checksum
    int total_checksum = 0;
    printf("\nPartition query results:\n");
    printf("Test | Gangs | Workers | Vector | Checksum\n");
    printf("-----|-------|---------|--------|---------\n");
    for (i = 0; i < NUM_TESTS; i++) {
        printf("%4d | %5d | %7d | %6d | %8d\n", 
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
        total_checksum += results[i][3];
    }
    
    // Additional checksum from host_array
    int array_checksum = 0;
    for (i = 0; i < 600; i++) {
        array_checksum += host_array[i];
    }
    
    printf("\nTotal checksum: %d\n", total_checksum + array_checksum);
    printf("All tests completed.\n");
    
    return 0;
}
