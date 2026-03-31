/* Test for OpenACC partition mapping coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128

int main() {
    int i, j, k;
    int config_results[NUM_CONFIGS][3] = {0};  // [gangs, workers, vector]
    int checksum = 0;
    
    /* Configuration 0: gang redundant */
    printf("Testing config 0: gang redundant\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(config_results[0:3])
    {
        config_results[0] = acc_get_num_gangs(0);
        config_results[1] = acc_get_num_workers(0);
        config_results[2] = acc_get_vector_length();
    }
    
    /* Configuration 1: gang partitioned */
    printf("Testing config 1: gang partitioned\n");
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(config_results[1:3])
    {
        config_results[0] = acc_get_num_gangs(0);
        config_results[1] = acc_get_num_workers(0);
        config_results[2] = acc_get_vector_length();
    }
    
    /* Configuration 2: worker partitioned */
    printf("Testing config 2: worker partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(config_results[2:3])
    {
        config_results[0] = acc_get_num_gangs(0);
        config_results[1] = acc_get_num_workers(0);
        config_results[2] = acc_get_vector_length();
    }
    
    /* Configuration 3: gang+worker partitioned */
    printf("Testing config 3: gang+worker partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(config_results[3:3])
    {
        config_results[0] = acc_get_num_gangs(0);
        config_results[1] = acc_get_num_workers(0);
        config_results[2] = acc_get_vector_length();
    }
    
    /* Configuration 4: vector partitioned */
    printf("Testing config 4: vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(config_results[4:3])
    {
        config_results[0] = acc_get_num_gangs(0);
        config_results[1] = acc_get_num_workers(0);
        config_results[2] = acc_get_vector_length();
    }
    
    /* Configuration 5: gang+vector partitioned */
    printf("Testing config 5: gang+vector partitioned\n");
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyout(config_results[5:3])
    {
        config_results[0] = acc_get_num_gangs(0);
        config_results[1] = acc_get_num_workers(0);
        config_results[2] = acc_get_vector_length();
    }
    
    /* Configuration 6: worker+vector partitioned */
    printf("Testing config 6: worker+vector partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
        copyout(config_results[6:3])
    {
        config_results[0] = acc_get_num_gangs(0);
        config_results[1] = acc_get_num_workers(0);
        config_results[2] = acc_get_vector_length();
    }
    
    /* Configuration 7: fully partitioned */
    printf("Testing config 7: fully partitioned\n");
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16) \
        copyout(config_results[7:3])
    {
        config_results[0] = acc_get_num_gangs(0);
        config_results[1] = acc_get_num_workers(0);
        config_results[2] = acc_get_vector_length();
    }
    
    /* Dynamic configuration with runtime variables */
    printf("Testing dynamic configurations\n");
    int dyn_gangs = 3;
    int dyn_workers = 5;
    int dyn_vector = 17;
    int dyn_results[3] = {0};
    
    #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) vector_length(dyn_vector) \
        copyout(dyn_results[0:3])
    {
        dyn_results[0] = acc_get_num_gangs(0);
        dyn_results[1] = acc_get_num_workers(0);
        dyn_results[2] = acc_get_vector_length();
    }
    
    /* Nested parallelism with collapse clause */
    printf("Testing nested parallelism with collapse\n");
    int nested_data[1000] = {0};
    
    #pragma acc data copy(nested_data[0:1000])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 10; i++) {
                for (j = 0; j < 10; j++) {
                    int idx = i * 10 + j;
                    if (idx < 1000) {
                        nested_data[idx] = acc_get_num_gangs(0) * 1000 + 
                                          acc_get_num_workers(0) * 100 + 
                                          acc_get_vector_length();
                    }
                }
            }
        }
    }
    
    /* Mixed structured/unstructured data regions */
    printf("Testing mixed data regions\n");
    float *device_data = NULL;
    int data_size = 1024;
    
    #pragma acc enter data create(device_data[0:data_size])
    
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(64) \
        present(device_data[0:data_size])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < data_size; i++) {
            device_data[i] = (float)i / data_size;
        }
    }
    
    #pragma acc exit data copyout(device_data[0:data_size]) delete(device_data[0:data_size])
    
    /* Diagnostic trigger with acc_malloc */
    printf("Testing diagnostic path with acc_malloc\n");
    void *test_ptr = acc_malloc(1024);
    if (test_ptr == NULL) {
        printf("Warning: acc_malloc returned NULL\n");
    } else {
        acc_free(test_ptr);
    }
    
    /* Kernels directive with automatic partitioning */
    printf("Testing kernels directive\n");
    int kernels_data[256];
    
    #pragma acc kernels num_gangs(8) num_workers(2) vector_length(32) copy(kernels_data[0:256])
    {
        #pragma acc loop gang
        for (i = 0; i < 8; i++) {
            #pragma acc loop worker
            for (j = 0; j < 2; j++) {
                #pragma acc loop vector
                for (k = 0; k < 32; k++) {
                    int idx = i * 64 + j * 32 + k;
                    if (idx < 256) {
                        kernels_data[idx] = i * 100 + j * 10 + k;
                    }
                }
            }
        }
    }
    
    /* Calculate checksum to ensure all regions executed */
    for (i = 0; i < NUM_CONFIGS; i++) {
        for (j = 0; j < 3; j++) {
            checksum += config_results[i][j];
        }
    }
    
    checksum += dyn_results[0] + dyn_results[1] + dyn_results[2];
    
    /* Verify nested data */
    for (i = 0; i < 100; i++) {
        checksum += nested_data[i];
    }
    
    /* Verify kernels data */
    for (i = 0; i < 256; i++) {
        checksum += kernels_data[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All partition configurations tested\n");
    
    return 0;
}
