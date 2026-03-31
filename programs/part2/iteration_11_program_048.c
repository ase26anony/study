/* test_openacc_partitions.c
 * 
 * This program systematically tests all 8 OpenACC data partition modes
 * to trigger the partition code string lookup function in GCC's
 * omp-oacc-neuter-broadcast.cc (lines 335-343).
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_partitions
 * For NVIDIA offload: gcc -O2 -fopenacc -fdump-tree-omplower -foffload=nvptx-none test_openacc_partitions.c -o test_partitions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define GANG_SIZE 32
#define WORKER_SIZE 4
#define VECTOR_SIZE 32

/* Use volatile to prevent compile-time elimination */
volatile int use_partition_mode = 0;

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum[0:1]) \
        num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += dest[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            sum[0] = local_sum;
        }
    }
}

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1]) \
        num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += dest[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            sum[0] = local_sum;
        }
    }
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1]) \
        num_gangs(1) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += dest[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            sum[0] = local_sum;
        }
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1]) \
        num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += dest[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            sum[0] = local_sum;
        }
    }
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1]) \
        num_gangs(1) num_workers(1) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += dest[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            sum[0] = local_sum;
        }
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1]) \
        num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += dest[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            sum[0] = local_sum;
        }
    }
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1]) \
        num_gangs(1) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += dest[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            sum[0] = local_sum;
        }
    }
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1]) \
        num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += dest[i];
        }
        
        if (acc_on_device(acc_device_not_host)) {
            sum[0] = local_sum;
        }
    }
}

/* Alternative approach using data regions with explicit partition clauses */
void test_data_partition_modes(float *src, float *dest, int n, float *sum) {
    /* Test different data partition modifiers */
    
    /* gang partitioned */
    #pragma acc data copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc parallel num_gangs(GANG_SIZE)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 1.5f;
            }
        }
    }
    
    /* worker partitioned */
    #pragma acc data copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc parallel num_workers(WORKER_SIZE)
        {
            #pragma acc loop worker
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 2.5f;
            }
        }
    }
    
    /* vector partitioned */
    #pragma acc data copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc parallel vector_length(VECTOR_SIZE)
        {
            #pragma acc loop vector
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 3.5f;
            }
        }
    }
    
    /* gang+worker partitioned */
    #pragma acc data copy(src[0:n]) copy(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc parallel num_gangs(GANG_SIZE) num_workers(WORKER_SIZE)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 4.5f;
            }
        }
    }
}

/* Main function that calls all test cases */
int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    float total_sum = 0.0f;
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC data partition modes...\n");
    
    /* Use volatile variable to control which tests run */
    /* This prevents dead code elimination */
    volatile int mode = use_partition_mode;
    
    /* Test all 8 partition modes */
    if (mode == 0 || mode == 1) {
        memset(dest, 0, N * sizeof(float));
        test_gang_redundant(src, dest, N, &sums[0]);
        for (int i = 0; i < N; i++) total_sum += dest[i];
    }
    
    if (mode == 0 || mode == 2) {
        memset(dest, 0, N * sizeof(float));
        test_gang_partitioned(src, dest, N, &sums[1]);
        for (int i = 0; i < N; i++) total_sum += dest[i];
    }
    
    if (mode == 0 || mode == 3) {
        memset(dest, 0, N * sizeof(float));
        test_worker_partitioned(src, dest, N, &sums[2]);
        for (int i = 0; i < N; i++) total_sum += dest[i];
    }
    
    if (mode == 0 || mode == 4) {
        memset(dest, 0, N * sizeof(float));
        test_gang_worker_partitioned(src, dest, N, &sums[3]);
        for (int i = 0; i < N; i++) total_sum += dest[i];
    }
    
    if (mode == 0 || mode == 5) {
        memset(dest, 0, N * sizeof(float));
        test_vector_partitioned(src, dest, N, &sums[4]);
        for (int i = 0; i < N; i++) total_sum += dest[i];
    }
    
    if (mode == 0 || mode == 6) {
        memset(dest, 0, N * sizeof(float));
        test_gang_vector_partitioned(src, dest, N, &sums[5]);
        for (int i = 0; i < N; i++) total_sum += dest[i];
    }
    
    if (mode == 0 || mode == 7) {
        memset(dest, 0, N * sizeof(float));
        test_worker_vector_partitioned(src, dest, N, &sums[6]);
        for (int i = 0; i < N; i++) total_sum += dest[i];
    }
    
    if (mode == 0 || mode == 8) {
        memset(dest, 0, N * sizeof(float));
        test_fully_partitioned(src, dest, N, &sums[7]);
        for (int i = 0; i < N; i++) total_sum += dest[i];
    }
    
    /* Also test data region approach */
    test_data_partition_modes(src, dest, N, &sums[0]);
    
    /* Compute final checksum */
    float checksum = total_sum;
    for (int i = 0; i < 8; i++) {
        checksum += sums[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Verify results (simplified) */
    if (checksum > 0.0f) {
        printf("Test completed successfully.\n");
    } else {
        printf("Warning: Zero checksum detected.\n");
    }
    
    free(src);
    free(dest);
    
    return 0;
}
