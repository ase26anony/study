/* Test for OpenACC partition mapping coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -fopt-info-omp-all -o test_partition test_partition.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128

/* Structure to store queried execution configuration */
struct exec_config {
    int gangs;
    int workers;
    int vector_len;
    int device_type;
};

/* Host array to store results from all configurations */
struct exec_config results[NUM_CONFIGS];

/* Initialize results array */
void init_results() {
    for (int i = 0; i < NUM_CONFIGS; i++) {
        results[i].gangs = 0;
        results[i].workers = 0;
        results[i].vector_len = 0;
        results[i].device_type = -1;
    }
}

/* Print results summary */
void print_results() {
    int total_gangs = 0, total_workers = 0, total_vector = 0;
    
    printf("\n=== OpenACC Partition Configuration Results ===\n");
    for (int i = 0; i < NUM_CONFIGS; i++) {
        printf("Config %d: gangs=%d, workers=%d, vector=%d, device=%d\n",
               i, results[i].gangs, results[i].workers, 
               results[i].vector_len, results[i].device_type);
        
        total_gangs += results[i].gangs;
        total_workers += results[i].workers;
        total_vector += results[i].vector_len;
    }
    
    printf("\nChecksum: total_gangs=%d, total_workers=%d, total_vector=%d\n",
           total_gangs, total_workers, total_vector);
}

int main() {
    int i, j;
    int *host_array;
    int *device_array;
    int array_size = 1024;
    int dynamic_gang_cnt = 4;
    int dynamic_worker_cnt = 2;
    int dynamic_vector_len = 64;
    
    /* Initialize host array */
    host_array = (int*)malloc(array_size * sizeof(int));
    for (i = 0; i < array_size; i++) {
        host_array[i] = i;
    }
    
    /* Allocate device array */
    device_array = (int*)acc_malloc(array_size * sizeof(int));
    if (device_array == NULL) {
        printf("Error: acc_malloc failed\n");
        return 1;
    }
    
    /* Initialize results */
    init_results();
    
    /* =========================================== */
    /* Configuration 0: Gang Redundant */
    /* =========================================== */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(results[0:1]) copyin(host_array[0:array_size])
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        /* Query runtime configuration */
        results[0].gangs = acc_get_num_gangs(0);
        results[0].workers = acc_get_num_workers(0);
        results[0].vector_len = acc_get_vector_length();
        results[0].device_type = acc_get_device_type();
        
        /* Simple computation to prevent optimization */
        if (gid == 0 && wid == 0 && vid == 0) {
            for (int idx = 0; idx < array_size; idx++) {
                host_array[idx] += 1;
            }
        }
    }
    
    /* =========================================== */
    /* Configuration 1: Gang Partitioned */
    /* =========================================== */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(results[1:1])
    {
        results[1].gangs = acc_get_num_gangs(0);
        results[1].workers = acc_get_num_workers(0);
        results[1].vector_len = acc_get_vector_length();
        results[1].device_type = acc_get_device_type();
    }
    
    /* =========================================== */
    /* Configuration 2: Worker Partitioned */
    /* =========================================== */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(results[2:1])
    {
        results[2].gangs = acc_get_num_gangs(0);
        results[2].workers = acc_get_num_workers(0);
        results[2].vector_len = acc_get_vector_length();
        results[2].device_type = acc_get_device_type();
    }
    
    /* =========================================== */
    /* Configuration 3: Gang+Worker Partitioned */
    /* =========================================== */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(results[3:1])
    {
        results[3].gangs = acc_get_num_gangs(0);
        results[3].workers = acc_get_num_workers(0);
        results[3].vector_len = acc_get_vector_length();
        results[3].device_type = acc_get_device_type();
    }
    
    /* =========================================== */
    /* Configuration 4: Vector Partitioned */
    /* =========================================== */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(results[4:1])
    {
        results[4].gangs = acc_get_num_gangs(0);
        results[4].workers = acc_get_num_workers(0);
        results[4].vector_len = acc_get_vector_length();
        results[4].device_type = acc_get_device_type();
    }
    
    /* =========================================== */
    /* Configuration 5: Gang+Vector Partitioned */
    /* =========================================== */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        copyout(results[5:1])
    {
        results[5].gangs = acc_get_num_gangs(0);
        results[5].workers = acc_get_num_workers(0);
        results[5].vector_len = acc_get_vector_length();
        results[5].device_type = acc_get_device_type();
    }
    
    /* =========================================== */
    /* Configuration 6: Worker+Vector Partitioned */
    /* =========================================== */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyout(results[6:1])
    {
        results[6].gangs = acc_get_num_gangs(0);
        results[6].workers = acc_get_num_workers(0);
        results[6].vector_len = acc_get_vector_length();
        results[6].device_type = acc_get_device_type();
    }
    
    /* =========================================== */
    /* Configuration 7: Fully Partitioned */
    /* =========================================== */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
        copyout(results[7:1])
    {
        results[7].gangs = acc_get_num_gangs(0);
        results[7].workers = acc_get_num_workers(0);
        results[7].vector_len = acc_get_vector_length();
        results[7].device_type = acc_get_device_type();
    }
    
    /* =========================================== */
    /* Dynamic Configuration with Runtime Values */
    /* =========================================== */
    {
        int dyn_results[3] = {0, 0, 0};
        
        #pragma acc parallel num_gangs(dynamic_gang_cnt) \
            num_workers(dynamic_worker_cnt) vector_length(dynamic_vector_len) \
            copyout(dyn_results[0:3])
        {
            dyn_results[0] = acc_get_num_gangs(0);
            dyn_results[1] = acc_get_num_workers(0);
            dyn_results[2] = acc_get_vector_length();
        }
        
        printf("Dynamic config: gangs=%d, workers=%d, vector=%d\n",
               dyn_results[0], dyn_results[1], dyn_results[2]);
    }
    
    /* =========================================== */
    /* Nested Parallelism with Collapse */
    /* =========================================== */
    {
        int nested_results[100] = {0};
        int sum = 0;
        
        #pragma acc data copyin(host_array[0:array_size]) \
            copyout(nested_results[0:100])
        {
            #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16)
            {
                #pragma acc loop gang worker vector collapse(2)
                for (i = 0; i < 10; i++) {
                    for (j = 0; j < 10; j++) {
                        int idx = i * 10 + j;
                        if (idx < 100) {
                            nested_results[idx] = 
                                __pgi_gangidx() * 1000 + 
                                __pgi_workeridx() * 100 + 
                                __pgi_vectoridx();
                        }
                    }
                }
            }
        }
        
        /* Compute checksum on host */
        for (i = 0; i < 100; i++) {
            sum += nested_results[i];
        }
        printf("Nested parallelism checksum: %d\n", sum);
    }
    
    /* =========================================== */
    /* Kernels Construct with Different Partitions */
    /* =========================================== */
    {
        int kernels_array[256];
        int kernels_sum = 0;
        
        #pragma acc kernels copyout(kernels_array[0:256]) \
            num_gangs(8) num_workers(1) vector_length(32)
        {
            #pragma acc loop gang
            for (i = 0; i < 256; i++) {
                kernels_array[i] = i * 2;
            }
        }
        
        /* Verify on host */
        for (i = 0; i < 256; i++) {
            kernels_sum += kernels_array[i];
        }
        printf("Kernels construct checksum: %d\n", kernels_sum);
    }
    
    /* =========================================== */
    /* Unstructured Data Region with Compute */
    /* =========================================== */
    {
        int *unstructured_data;
        int unstructured_size = 512;
        
        unstructured_data = (int*)acc_malloc(unstructured_size * sizeof(int));
        if (unstructured_data == NULL) {
            printf("Warning: Second acc_malloc returned NULL\n");
        } else {
            #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) \
                present(unstructured_data[0:unstructured_size])
            {
                #pragma acc loop gang worker vector
                for (i = 0; i < unstructured_size; i++) {
                    unstructured_data[i] = i * 3;
                }
            }
            
            /* Copy back and verify */
            int *host_verify = (int*)malloc(unstructured_size * sizeof(int));
            #pragma acc update host(unstructured_data[0:unstructured_size])
            
            for (i = 0; i < unstructured_size; i++) {
                host_verify[i] = unstructured_data[i];
            }
            
            acc_free(unstructured_data);
            free(host_verify);
        }
    }
    
    /* =========================================== */
    /* Mixed OpenACC Constructs in Loop */
    /* =========================================== */
    for (int iter = 0; iter < 3; iter++) {
        int loop_gangs = 1 << iter;  /* 1, 2, 4 */
        int loop_workers = 2;
        int loop_vector = 32;
        
        #pragma acc parallel num_gangs(loop_gangs) \
            num_workers(loop_workers) vector_length(loop_vector)
        {
            /* Force runtime to map partition for each iteration */
            int temp = acc_get_num_gangs(0) + acc_get_num_workers(0);
            if (temp > 0) {
                /* Do nothing, just reference the values */
            }
        }
    }
    
    /* Print all results */
    print_results();
    
    /* Cleanup */
    acc_free(device_array);
    free(host_array);
    
    return 0;
}
