/* test_openacc_partitions.c
 * 
 * This program tests all 8 OpenACC data partition modes to trigger
 * the partition code string lookup in GCC's omp-oacc-neuter-broadcast.cc
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_partitions
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 32

/* Use volatile to prevent compile-time elimination */
static volatile int use_partition_mode = 0;

/* Test 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) /* gang redundant */
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy worker(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy worker vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

/* Additional test with create clause variations */
void test_create_partitions(float *src, float *dest, int n, float *sum) {
    /* Test create with different partition modes */
    #pragma acc data copyin(src[0:n]) create gang(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc parallel
        {
            float local_sum = 0.0f;
            #pragma acc loop gang reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 10.0f;
                local_sum += src[i];
            }
            if (acc_on_device(acc_device_not_host)) {
                *sum = local_sum;
            }
        }
    }
}

/* Test with present clause and partitioning */
void test_present_partitions(float *src, float *dest, int n, float *sum) {
    /* First ensure data is present */
    #pragma acc enter data copyin(src[0:n], dest[0:n], sum[0:1])
    
    #pragma acc parallel present(src[0:n]) present gang worker(dest[0:n]) present(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 11.0f;
            local_sum += src[i];
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
    
    #pragma acc exit data delete(src[0:n], dest[0:n]) copyout(sum[0:1])
}

/* Test with 2D arrays for more complex partitioning */
void test_2d_partitions(float (*src)[M], float (*dest)[M], int n, int m, float *sum) {
    #pragma acc parallel copy(src[0:n][0:m]) copy gang worker vector(dest[0:n][0:m]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum) collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                dest[i][j] = src[i][j] * 12.0f;
                local_sum += src[i][j];
            }
        }
        if (acc_on_device(acc_device_not_host)) {
            *sum = local_sum;
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    float total_sum = 0.0f;
    
    /* Initialize source array */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Use volatile to force all partition modes to be considered */
    for (int mode = 0; mode < 8; mode++) {
        use_partition_mode = mode;
        
        /* Clear destination */
        for (int i = 0; i < N; i++) {
            dest[i] = 0.0f;
        }
        
        /* Select partition mode based on volatile variable */
        switch (use_partition_mode) {
            case 0:
                test_gang_redundant(src, dest, N, &sums[0]);
                break;
            case 1:
                test_gang_partitioned(src, dest, N, &sums[1]);
                break;
            case 2:
                test_worker_partitioned(src, dest, N, &sums[2]);
                break;
            case 3:
                test_gang_worker_partitioned(src, dest, N, &sums[3]);
                break;
            case 4:
                test_vector_partitioned(src, dest, N, &sums[4]);
                break;
            case 5:
                test_gang_vector_partitioned(src, dest, N, &sums[5]);
                break;
            case 6:
                test_worker_vector_partitioned(src, dest, N, &sums[6]);
                break;
            case 7:
                test_fully_partitioned(src, dest, N, &sums[7]);
                break;
        }
        
        /* Verify computation on host side */
        float host_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            host_sum += src[i];
        }
        
        /* Accumulate to total sum */
        total_sum += sums[mode];
        
        /* Print result to prevent dead code elimination */
        printf("Mode %d: device sum = %f, host sum = %f\n", 
               mode, sums[mode], host_sum);
    }
    
    /* Additional tests with different clauses */
    test_create_partitions(src, dest, N, &sums[0]);
    test_present_partitions(src, dest, N, &sums[1]);
    
    /* Test with 2D arrays */
    float (*src2d)[M] = (float(*)[M])malloc(N * M * sizeof(float));
    float (*dest2d)[M] = (float(*)[M])malloc(N * M * sizeof(float));
    float sum2d = 0.0f;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            src2d[i][j] = (float)((i * M + j) % 100) * 0.01f;
        }
    }
    
    test_2d_partitions(src2d, dest2d, N, M, &sum2d);
    total_sum += sum2d;
    
    printf("Total checksum: %f\n", total_sum);
    
    /* Cleanup */
    free(src);
    free(dest);
    free(src2d);
    free(dest2d);
    
    return 0;
}
