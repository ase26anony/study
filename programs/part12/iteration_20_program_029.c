/* Test for OpenACC partition mapping coverage in GCC runtime */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c */
/* Or for diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_TESTS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128

int main() {
    int i, j;
    int results[NUM_TESTS][4] = {0}; /* [gangs, workers, vector, checksum] */
    int *d_results = NULL;
    int gang_cnt, worker_cnt, vector_len;
    
    /* Allocate device memory for results */
    d_results = (int*)acc_malloc(NUM_TESTS * 4 * sizeof(int));
    if (!d_results) {
        fprintf(stderr, "acc_malloc failed\n");
        return 1;
    }
    
    /* Initialize device results to zero */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        present(d_results[0:NUM_TESTS*4])
    {
        for (int idx = 0; idx < NUM_TESTS * 4; idx++) {
            d_results[idx] = 0;
        }
    }
    
    /* Test 0: gang redundant (all 1s) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(results[0:1][0:4]) present(d_results)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            d_results[0] = acc_get_num_gangs(acc_async_noval);
            d_results[1] = acc_get_num_workers(acc_async_noval);
            d_results[2] = acc_get_vector_length(acc_async_noval);
            d_results[3] = gid + wid + vid; /* checksum */
        }
    }
    
    /* Test 1: gang partitioned */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(results[1:1][0:4]) present(d_results)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            d_results[4] = acc_get_num_gangs(acc_async_noval);
            d_results[5] = acc_get_num_workers(acc_async_noval);
            d_results[6] = acc_get_vector_length(acc_async_noval);
            d_results[7] = gid; /* checksum */
        }
    }
    
    /* Test 2: worker partitioned */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(results[2:1][0:4]) present(d_results)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            d_results[8] = acc_get_num_gangs(acc_async_noval);
            d_results[9] = acc_get_num_workers(acc_async_noval);
            d_results[10] = acc_get_vector_length(acc_async_noval);
            d_results[11] = wid; /* checksum */
        }
    }
    
    /* Test 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(results[3:1][0:4]) present(d_results)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            d_results[12] = acc_get_num_gangs(acc_async_noval);
            d_results[13] = acc_get_num_workers(acc_async_noval);
            d_results[14] = acc_get_vector_length(acc_async_noval);
            d_results[15] = gid * 10 + wid; /* checksum */
        }
    }
    
    /* Test 4: vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyout(results[4:1][0:4]) present(d_results)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            d_results[16] = acc_get_num_gangs(acc_async_noval);
            d_results[17] = acc_get_num_workers(acc_async_noval);
            d_results[18] = acc_get_vector_length(acc_async_noval);
            d_results[19] = vid; /* checksum */
        }
    }
    
    /* Test 5: gang+vector partitioned */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        copyout(results[5:1][0:4]) present(d_results)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            d_results[20] = acc_get_num_gangs(acc_async_noval);
            d_results[21] = acc_get_num_workers(acc_async_noval);
            d_results[22] = acc_get_vector_length(acc_async_noval);
            d_results[23] = gid * 100 + vid; /* checksum */
        }
    }
    
    /* Test 6: worker+vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        copyout(results[6:1][0:4]) present(d_results)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            d_results[24] = acc_get_num_gangs(acc_async_noval);
            d_results[25] = acc_get_num_workers(acc_async_noval);
            d_results[26] = acc_get_vector_length(acc_async_noval);
            d_results[27] = wid * 10 + vid; /* checksum */
        }
    }
    
    /* Test 7: fully partitioned */
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(8) \
        copyout(results[7:1][0:4]) present(d_results)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        if (gid == 0 && wid == 0 && vid == 0) {
            d_results[28] = acc_get_num_gangs(acc_async_noval);
            d_results[29] = acc_get_num_workers(acc_async_noval);
            d_results[30] = acc_get_vector_length(acc_async_noval);
            d_results[31] = gid * 100 + wid * 10 + vid; /* checksum */
        }
    }
    
    /* Copy results back from device */
    #pragma acc update host(d_results[0:NUM_TESTS*4])
    
    /* Copy to host array for verification */
    for (i = 0; i < NUM_TESTS; i++) {
        for (j = 0; j < 4; j++) {
            results[i][j] = d_results[i*4 + j];
        }
    }
    
    /* Nested parallelism test with collapse clause */
    int data[1000] = {0};
    int sum = 0;
    
    #pragma acc data copy(data[0:1000])
    {
        /* Structured data region with nested parallelism */
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang
            for (i = 0; i < 10; i++) {
                #pragma acc loop worker
                for (j = 0; j < 10; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 10; k++) {
                        int idx = i*100 + j*10 + k;
                        if (idx < 1000) {
                            data[idx] = i + j + k;
                        }
                    }
                }
            }
        }
        
        /* Another region with collapse */
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(32)
        {
            #pragma acc loop gang collapse(2)
            for (i = 0; i < 20; i++) {
                for (j = 0; j < 50; j++) {
                    int idx = i*50 + j;
                    data[idx] += 1;
                }
            }
        }
    }
    
    /* Dynamic partitioning with runtime variables */
    for (int iter = 0; iter < 3; iter++) {
        gang_cnt = 1 << iter;      /* 1, 2, 4 */
        worker_cnt = 2 * (iter + 1); /* 2, 4, 6 */
        vector_len = 16 * (iter + 1); /* 16, 32, 48 */
        
        #pragma acc parallel num_gangs(gang_cnt) num_workers(worker_cnt) vector_length(vector_len) \
            copy(data[0:10])
        {
            int gid = __pgi_gangidx();
            int wid = __pgi_workeridx();
            int vid = __pgi_vectoridx();
            
            if (gid < 10 && gid == wid % gang_cnt && vid == 0) {
                data[gid] += gid + wid + vid;
            }
        }
    }
    
    /* Unstructured data region */
    int *unstructured_data = NULL;
    unstructured_data = (int*)acc_malloc(100 * sizeof(int));
    
    if (unstructured_data) {
        #pragma acc enter data copyin(unstructured_data[0:100])
        
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(2) \
            present(unstructured_data[0:100])
        {
            int idx = __pgi_gangidx() * 50 + __pgi_workeridx() * 25 + __pgi_vectoridx();
            if (idx < 100) {
                unstructured_data[idx] = idx;
            }
        }
        
        #pragma acc exit data copyout(unstructured_data[0:100])
        acc_free(unstructured_data);
    }
    
    /* Calculate checksum from all results */
    int total_checksum = 0;
    printf("Partition configuration results:\n");
    for (i = 0; i < NUM_TESTS; i++) {
        printf("Test %d: gangs=%d, workers=%d, vector=%d, checksum=%d\n",
               i, results[i][0], results[i][1], results[i][2], results[i][3]);
        total_checksum += results[i][0] + results[i][1] + results[i][2] + results[i][3];
    }
    
    /* Also checksum the data array */
    for (i = 0; i < 1000; i++) {
        sum += data[i];
    }
    
    printf("Total checksum from partition results: %d\n", total_checksum);
    printf("Data array checksum: %d\n", sum);
    
    /* Cleanup */
    acc_free(d_results);
    
    /* Force potential diagnostic output */
    if (total_checksum == 0) {
        fprintf(stderr, "Warning: All partition queries returned zero\n");
    }
    
    return 0;
}
