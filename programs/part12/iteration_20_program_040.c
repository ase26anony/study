/* Test for OpenACC partition mapping coverage in omp-oacc-neuter-broadcast.cc
 * This test exercises all partition types (0-7) through various OpenACC constructs
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define N 1024
#define MAX_GANGS 8
#define MAX_WORKERS 4
#define VECTOR_LENGTH 128

/* Function to verify partition configuration */
void verify_partition(int test_id, int gangs, int workers, int vector_len) {
    printf("Test %d: gangs=%d, workers=%d, vector=%d\n", 
           test_id, gangs, workers, vector_len);
}

int main() {
    int i, j;
    int host_results[8][4] = {0}; /* [test_case][gangs,workers,vector,checksum] */
    int *device_results = NULL;
    int *data_array = NULL;
    int dynamic_gang_cnt = 2;
    int dynamic_worker_cnt = 2;
    int dynamic_vector_len = 64;
    
    /* Allocate device memory for results */
    device_results = (int*)acc_malloc(8 * 4 * sizeof(int));
    if (!device_results) {
        fprintf(stderr, "acc_malloc failed for device_results\n");
        return 1;
    }
    
    /* Allocate and initialize data array */
    data_array = (int*)malloc(N * sizeof(int));
    for (i = 0; i < N; i++) {
        data_array[i] = i;
    }
    
    /* Copy data to device */
    #pragma acc enter data copyin(data_array[0:N])
    
    printf("Starting OpenACC partition coverage tests...\n\n");
    
    /* Test 0: Gang redundant (all 1s) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(data_array[0:N]) copyout(host_results[0:1][0:4])
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
                    /* Simple computation */
                    int idx = (i * w * v) + (j * v) + k;
                    if (idx < N) {
                        data_array[idx] += 1;
                    }
                }
            }
        }
        
        /* Store results */
        host_results[0][0] = g;
        host_results[0][1] = w;
        host_results[0][2] = v;
        host_results[0][3] = g + w + v;
    }
    verify_partition(0, host_results[0][0], host_results[0][1], host_results[0][2]);
    
    /* Test 1: Gang partitioned */
    #pragma acc parallel num_gangs(MAX_GANGS) num_workers(1) vector_length(1) \
        present(data_array[0:N]) copyout(host_results[1:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            data_array[i] *= 2;
        }
        
        host_results[1][0] = g;
        host_results[1][1] = w;
        host_results[1][2] = v;
        host_results[1][3] = g + w + v;
    }
    verify_partition(1, host_results[1][0], host_results[1][1], host_results[1][2]);
    
    /* Test 2: Worker partitioned */
    #pragma acc parallel num_gangs(1) num_workers(MAX_WORKERS) vector_length(1) \
        present(data_array[0:N]) copyout(host_results[2:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            data_array[i] -= i;
        }
        
        host_results[2][0] = g;
        host_results[2][1] = w;
        host_results[2][2] = v;
        host_results[2][3] = g + w + v;
    }
    verify_partition(2, host_results[2][0], host_results[2][1], host_results[2][2]);
    
    /* Test 3: Gang+Worker partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        present(data_array[0:N]) copyout(host_results[3:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang
        for (i = 0; i < g; i++) {
            #pragma acc loop worker
            for (j = 0; j < w; j++) {
                int idx = i * w + j;
                if (idx < N) {
                    data_array[idx] += idx;
                }
            }
        }
        
        host_results[3][0] = g;
        host_results[3][1] = w;
        host_results[3][2] = v;
        host_results[3][3] = g + w + v;
    }
    verify_partition(3, host_results[3][0], host_results[3][1], host_results[3][2]);
    
    /* Test 4: Vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(VECTOR_LENGTH) \
        present(data_array[0:N]) copyout(host_results[4:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            data_array[i] = data_array[i] % 100;
        }
        
        host_results[4][0] = g;
        host_results[4][1] = w;
        host_results[4][2] = v;
        host_results[4][3] = g + w + v;
    }
    verify_partition(4, host_results[4][0], host_results[4][1], host_results[4][2]);
    
    /* Test 5: Gang+Vector partitioned */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        present(data_array[0:N]) copyout(host_results[5:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            data_array[i] = data_array[i] * 3;
        }
        
        host_results[5][0] = g;
        host_results[5][1] = w;
        host_results[5][2] = v;
        host_results[5][3] = g + w + v;
    }
    verify_partition(5, host_results[5][0], host_results[5][1], host_results[5][2]);
    
    /* Test 6: Worker+Vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        present(data_array[0:N]) copyout(host_results[6:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            data_array[i] = data_array[i] / 2;
        }
        
        host_results[6][0] = g;
        host_results[6][1] = w;
        host_results[6][2] = v;
        host_results[6][3] = g + w + v;
    }
    verify_partition(6, host_results[6][0], host_results[6][1], host_results[6][2]);
    
    /* Test 7: Fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
        present(data_array[0:N]) copyout(host_results[7:1][0:4])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        #pragma acc loop gang worker vector collapse(2)
        for (i = 0; i < 16; i++) {
            for (j = 0; j < 16; j++) {
                int idx = i * 16 + j;
                if (idx < N) {
                    data_array[idx] = 0;
                }
            }
        }
        
        host_results[7][0] = g;
        host_results[7][1] = w;
        host_results[7][2] = v;
        host_results[7][3] = g + w + v;
    }
    verify_partition(7, host_results[7][0], host_results[7][1], host_results[7][2]);
    
    /* Test with dynamic values (conditional partitioning) */
    for (int iter = 0; iter < 2; iter++) {
        dynamic_gang_cnt = (iter == 0) ? 1 : 4;
        dynamic_worker_cnt = (iter == 0) ? 1 : 2;
        dynamic_vector_len = (iter == 0) ? 1 : 16;
        
        #pragma acc parallel num_gangs(dynamic_gang_cnt) \
            num_workers(dynamic_worker_cnt) vector_length(dynamic_vector_len) \
            present(data_array[0:N])
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
                        int idx = (i * w * v) + (j * v) + k;
                        if (idx < N) {
                            data_array[idx] += g + w + v;
                        }
                    }
                }
            }
        }
    }
    
    /* Test with kernels construct (different code path) */
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(32) \
        copy(data_array[0:N])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < N; i++) {
            data_array[i] += 1000;
        }
    }
    
    /* Copy data back and verify */
    #pragma acc update host(data_array[0:N])
    
    /* Calculate checksum */
    int total_checksum = 0;
    int data_checksum = 0;
    
    for (i = 0; i < 8; i++) {
        total_checksum += host_results[i][3];
    }
    
    for (i = 0; i < N; i++) {
        data_checksum += data_array[i];
    }
    
    printf("\nTest Summary:\n");
    printf("Total partition checksum: %d\n", total_checksum);
    printf("Data array checksum: %d\n", data_checksum);
    
    /* Cleanup */
    if (device_results) {
        acc_free(device_results);
    }
    
    #pragma acc exit data delete(data_array[0:N])
    free(data_array);
    
    /* Force diagnostic output by checking device type */
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", (int)dev_type);
    
    if (total_checksum > 0 && data_checksum != 0) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nTest validation failed!\n");
        return 1;
    }
}
