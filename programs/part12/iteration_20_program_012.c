/* Test case for omp-oacc-neuter-broadcast.cc partition string coverage
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LENGTH 128

int main() {
    int results[NUM_CONFIGS][4] = {0}; // [gangs, workers, vector, checksum]
    int *d_results = NULL;
    int i, j, k;
    
    // Allocate device memory for results
    d_results = (int*)acc_malloc(NUM_CONFIGS * 4 * sizeof(int));
    if (!d_results) {
        fprintf(stderr, "acc_malloc failed\n");
        return 1;
    }
    
    // Initialize device results to zero
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(d_results[0:NUM_CONFIGS*4])
    {
        for (int idx = 0; idx < NUM_CONFIGS * 4; idx++) {
            d_results[idx] = 0;
        }
    }
    
    // Configuration 0: gang redundant (all 1s)
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(d_results[0:4])
    {
        d_results[0] = acc_get_num_gangs(0);  // dimension 0
        d_results[1] = acc_get_num_workers();
        d_results[2] = acc_get_vector_length();
        d_results[3] = 1;  // checksum
    }
    
    // Configuration 1: gang partitioned
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(d_results[4:4])
    {
        d_results[4] = acc_get_num_gangs(0);
        d_results[5] = acc_get_num_workers();
        d_results[6] = acc_get_vector_length();
        d_results[7] = 2;
    }
    
    // Configuration 2: worker partitioned
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(d_results[8:4])
    {
        d_results[8] = acc_get_num_gangs(0);
        d_results[9] = acc_get_num_workers();
        d_results[10] = acc_get_vector_length();
        d_results[11] = 3;
    }
    
    // Configuration 3: gang+worker partitioned
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyout(d_results[12:4])
    {
        d_results[12] = acc_get_num_gangs(0);
        d_results[13] = acc_get_num_workers();
        d_results[14] = acc_get_vector_length();
        d_results[15] = 4;
    }
    
    // Configuration 4: vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(d_results[16:4])
    {
        d_results[16] = acc_get_num_gangs(0);
        d_results[17] = acc_get_num_workers();
        d_results[18] = acc_get_vector_length();
        d_results[19] = 5;
    }
    
    // Configuration 5: gang+vector partitioned
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyout(d_results[20:4])
    {
        d_results[20] = acc_get_num_gangs(0);
        d_results[21] = acc_get_num_workers();
        d_results[22] = acc_get_vector_length();
        d_results[23] = 6;
    }
    
    // Configuration 6: worker+vector partitioned
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
        copyout(d_results[24:4])
    {
        d_results[24] = acc_get_num_gangs(0);
        d_results[25] = acc_get_num_workers();
        d_results[26] = acc_get_vector_length();
        d_results[27] = 7;
    }
    
    // Configuration 7: fully partitioned
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
        copyout(d_results[28:4])
    {
        d_results[28] = acc_get_num_gangs(0);
        d_results[29] = acc_get_num_workers();
        d_results[30] = acc_get_vector_length();
        d_results[31] = 8;
    }
    
    // Copy results back to host
    #pragma acc update host(d_results[0:NUM_CONFIGS*4])
    
    // Process and print results
    int total_checksum = 0;
    printf("Partition configuration results:\n");
    printf("Config | Gangs | Workers | Vector | Checksum\n");
    printf("-------|-------|---------|--------|----------\n");
    
    for (i = 0; i < NUM_CONFIGS; i++) {
        for (j = 0; j < 4; j++) {
            results[i][j] = d_results[i*4 + j];
        }
        printf("%6d | %5d | %7d | %6d | %8d\n", 
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
        total_checksum += results[i][3];
    }
    
    printf("\nTotal checksum: %d\n", total_checksum);
    
    // Test with dynamic values (runtime variables)
    printf("\nTesting dynamic configurations:\n");
    
    int dynamic_gangs[] = {1, 2, 4, 8};
    int dynamic_workers[] = {1, 2, 4};
    int dynamic_vector[] = {1, 32, 64};
    
    int dyn_results[3] = {0};
    
    for (i = 0; i < 3; i++) {
        int g = dynamic_gangs[i];
        int w = dynamic_workers[i];
        int v = dynamic_vector[i];
        
        #pragma acc parallel num_gangs(g) num_workers(w) vector_length(v) \
            copyout(dyn_results[0:3])
        {
            dyn_results[0] = acc_get_num_gangs(0);
            dyn_results[1] = acc_get_num_workers();
            dyn_results[2] = acc_get_vector_length();
        }
        
        printf("Dynamic config %d: gangs=%d, workers=%d, vector=%d\n",
               i, dyn_results[0], dyn_results[1], dyn_results[2]);
    }
    
    // Test nested parallelism with collapse
    printf("\nTesting nested parallelism with collapse:\n");
    
    int data[1000] = {0};
    int sum = 0;
    
    #pragma acc data copy(data[0:1000])
    {
        // Nested parallelism: gang outer, worker inner
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
            copyout(sum)
        {
            #pragma acc loop gang
            for (i = 0; i < 4; i++) {
                #pragma acc loop worker
                for (j = 0; j < 250; j++) {
                    int idx = i * 250 + j;
                    #pragma acc loop vector
                    for (k = 0; k < 1; k++) {
                        data[idx] = idx * 2;
                    }
                }
            }
            
            // Reduction
            #pragma acc loop gang reduction(+:sum)
            for (i = 0; i < 1000; i++) {
                sum += data[i];
            }
        }
        
        // Collapsed loop
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(64)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 20; j++) {
                    int idx = i * 20 + j;
                    data[idx] += 1;
                }
            }
        }
    }
    
    printf("Final sum after nested operations: %d\n", sum);
    
    // Test kernels directive with different partitionings
    printf("\nTesting kernels directive:\n");
    
    int kernels_data[256];
    
    #pragma acc kernels num_gangs(4) num_workers(2) vector_length(32) \
        copy(kernels_data[0:256])
    {
        #pragma acc loop gang
        for (i = 0; i < 4; i++) {
            #pragma acc loop worker
            for (j = 0; j < 2; j++) {
                #pragma acc loop vector
                for (k = 0; k < 32; k++) {
                    int idx = i * 64 + j * 32 + k;
                    kernels_data[idx] = idx * 3;
                }
            }
        }
    }
    
    // Verify kernels execution
    int kernels_sum = 0;
    for (i = 0; i < 256; i++) {
        kernels_sum += kernels_data[i];
    }
    printf("Kernels data sum: %d\n", kernels_sum);
    
    // Cleanup
    acc_free(d_results);
    
    // Force potential diagnostic output with a NULL check
    void *test_ptr = acc_malloc(0);
    if (!test_ptr) {
        printf("Note: acc_malloc(0) returned NULL (expected)\n");
    }
    
    return 0;
}
