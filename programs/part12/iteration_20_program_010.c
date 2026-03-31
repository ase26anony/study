/* Test case for OpenACC partition mapping coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* For diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition_diag test_partition.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128
#define ARRAY_SIZE 1024

/* Structure to store runtime query results */
typedef struct {
    int gang_cnt;
    int worker_cnt;
    int vector_len;
    int partition_code;
} partition_config_t;

/* Helper function to validate allocation (triggers diagnostic paths) */
static void check_acc_malloc(void) {
    void *ptr = acc_malloc(1024);
    if (ptr == NULL) {
        fprintf(stderr, "acc_malloc failed - may trigger diagnostic\n");
    } else {
        acc_free(ptr);
    }
}

int main() {
    int i, j;
    int host_array[ARRAY_SIZE];
    int device_array[ARRAY_SIZE];
    partition_config_t results[NUM_CONFIGS];
    int checksum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < ARRAY_SIZE; i++) {
        host_array[i] = i;
        device_array[i] = 0;
    }
    
    /* Copy to device */
    #pragma acc enter data copyin(host_array[0:ARRAY_SIZE]) \
                         create(device_array[0:ARRAY_SIZE])
    
    printf("Testing OpenACC partition mapping...\n");
    
    /* Configuration 0: gang redundant (all 1s) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
                copyout(results[0:1])
    {
        results[0].gang_cnt = acc_get_num_gangs(0);
        results[0].worker_cnt = acc_get_num_workers(0);
        results[0].vector_len = acc_get_vector_length();
        results[0].partition_code = 0;
    }
    
    /* Configuration 1: gang partitioned */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyout(results[1:1])
    {
        results[1].gang_cnt = acc_get_num_gangs(0);
        results[1].worker_cnt = acc_get_num_workers(0);
        results[1].vector_len = acc_get_vector_length();
        results[1].partition_code = 1;
    }
    
    /* Configuration 2: worker partitioned */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyout(results[2:1])
    {
        results[2].gang_cnt = acc_get_num_gangs(0);
        results[2].worker_cnt = acc_get_num_workers(0);
        results[2].vector_len = acc_get_vector_length();
        results[2].partition_code = 2;
    }
    
    /* Configuration 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
                copyout(results[3:1])
    {
        results[3].gang_cnt = acc_get_num_gangs(0);
        results[3].worker_cnt = acc_get_num_workers(0);
        results[3].vector_len = acc_get_vector_length();
        results[3].partition_code = 3;
    }
    
    /* Configuration 4: vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
                copyout(results[4:1])
    {
        results[4].gang_cnt = acc_get_num_gangs(0);
        results[4].worker_cnt = acc_get_num_workers(0);
        results[4].vector_len = acc_get_vector_length();
        results[4].partition_code = 4;
    }
    
    /* Configuration 5: gang+vector partitioned */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
                copyout(results[5:1])
    {
        results[5].gang_cnt = acc_get_num_gangs(0);
        results[5].worker_cnt = acc_get_num_workers(0);
        results[5].vector_len = acc_get_vector_length();
        results[5].partition_code = 5;
    }
    
    /* Configuration 6: worker+vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(32) \
                copyout(results[6:1])
    {
        results[6].gang_cnt = acc_get_num_gangs(0);
        results[6].worker_cnt = acc_get_num_workers(0);
        results[6].vector_len = acc_get_vector_length();
        results[6].partition_code = 6;
    }
    
    /* Configuration 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(16) \
                copyout(results[7:1])
    {
        results[7].gang_cnt = acc_get_num_gangs(0);
        results[7].worker_cnt = acc_get_num_workers(0);
        results[7].vector_len = acc_get_vector_length();
        results[7].partition_code = 7;
    }
    
    /* Test with dynamic values (runtime variables) */
    int dyn_gangs = 3;
    int dyn_workers = 5;
    int dyn_vector = 8;
    
    #pragma acc parallel num_gangs(dyn_gangs) num_workers(dyn_workers) \
                vector_length(dyn_vector)
    {
        #pragma acc loop gang
        for (i = 0; i < dyn_gangs * 10; i++) {
            #pragma acc loop worker
            for (j = 0; j < dyn_workers * 10; j++) {
                int idx = i * (dyn_workers * 10) + j;
                if (idx < ARRAY_SIZE) {
                    #pragma acc atomic
                    device_array[idx] += 1;
                }
            }
        }
    }
    
    /* Nested parallelism with collapse clause */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang collapse(2)
        for (i = 0; i < 16; i++) {
            for (j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < ARRAY_SIZE) {
                    #pragma acc loop vector
                    for (int k = 0; k < 4; k++) {
                        #pragma acc atomic
                        device_array[idx] += k;
                    }
                }
            }
        }
    }
    
    /* Mixed structured/unstructured data regions */
    #pragma acc data present(host_array[0:ARRAY_SIZE], device_array[0:ARRAY_SIZE])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < ARRAY_SIZE; i++) {
            device_array[i] = host_array[i] * 2;
        }
        
        #pragma acc kernels loop gang(16) worker(8) vector(32)
        for (i = 0; i < ARRAY_SIZE; i++) {
            device_array[i] += 1;
        }
    }
    
    /* Trigger potential diagnostic path */
    check_acc_malloc();
    
    /* Copy results back and compute checksum */
    #pragma acc update host(device_array[0:ARRAY_SIZE])
    
    for (i = 0; i < NUM_CONFIGS; i++) {
        checksum += results[i].gang_cnt + results[i].worker_cnt + results[i].vector_len;
        printf("Config %d: gangs=%d, workers=%d, vector=%d\n", 
               i, results[i].gang_cnt, results[i].worker_cnt, results[i].vector_len);
    }
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += device_array[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates all regions executed)\n");
    
    /* Cleanup */
    #pragma acc exit data delete(host_array[0:ARRAY_SIZE], \
                                 device_array[0:ARRAY_SIZE])
    
    return 0;
}
