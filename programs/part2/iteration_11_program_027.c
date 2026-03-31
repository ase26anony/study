/* test_partition_codes.c - Cover all data partition mapping cases in GCC's omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define GANG_SIZE 32
#define WORKER_SIZE 4
#define VECTOR_SIZE 8

/* Use volatile to prevent compile-time elimination */
volatile int use_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) /* gang redundant */
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            sum[0] += src[i];
        }
    }
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            sum[0] += src[i] * 2.0f;
        }
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy worker(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            sum[0] += src[i] * 3.0f;
        }
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum[0] += src[i] * 4.0f;
        }
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            sum[0] += src[i] * 5.0f;
        }
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum[0] += src[i] * 6.0f;
        }
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy worker vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            sum[0] += src[i] * 7.0f;
        }
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            sum[0] += src[i] * 8.0f;
        }
    }
}

/* Alternative approach using kernels construct */
void test_kernels_partition(float *src, float *dest, int n, float *sum) {
    #pragma acc kernels copy(src[0:n]) create gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 10.0f;
            sum[0] += src[i] * 9.0f;
        }
    }
}

/* Test with explicit data region and enter/exit data */
void test_data_region_partition(float *src, float *dest, int n, float *sum) {
    #pragma acc enter data copyin(src[0:n]) create gang(dest[0:n]) copyin(sum[0:1])
    
    #pragma acc parallel present(src[0:n], dest[0:n], sum[0:1])
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 11.0f;
            sum[0] += src[i] * 10.0f;
        }
    }
    
    #pragma acc exit data copyout(dest[0:n]) copyout(sum[0:1]) delete(src[0:n])
}

/* Main test driver */
int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};  /* One sum for each partition mode */
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing all OpenACC data partition modes...\n");
    
    /* Test each partition mode based on volatile variable */
    /* This prevents dead code elimination */
    if (use_partition_mode == 0 || use_partition_mode == 1) {
        memset(dest, 0, N * sizeof(float));
        test_gang_redundant(src, dest, N, &sums[0]);
        printf("Gang redundant sum: %f\n", sums[0]);
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 2) {
        memset(dest, 0, N * sizeof(float));
        test_gang_partitioned(src, dest, N, &sums[1]);
        printf("Gang partitioned sum: %f\n", sums[1]);
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 3) {
        memset(dest, 0, N * sizeof(float));
        test_worker_partitioned(src, dest, N, &sums[2]);
        printf("Worker partitioned sum: %f\n", sums[2]);
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 4) {
        memset(dest, 0, N * sizeof(float));
        test_gang_worker_partitioned(src, dest, N, &sums[3]);
        printf("Gang+Worker partitioned sum: %f\n", sums[3]);
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 5) {
        memset(dest, 0, N * sizeof(float));
        test_vector_partitioned(src, dest, N, &sums[4]);
        printf("Vector partitioned sum: %f\n", sums[4]);
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 6) {
        memset(dest, 0, N * sizeof(float));
        test_gang_vector_partitioned(src, dest, N, &sums[5]);
        printf("Gang+Vector partitioned sum: %f\n", sums[5]);
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 7) {
        memset(dest, 0, N * sizeof(float));
        test_worker_vector_partitioned(src, dest, N, &sums[6]);
        printf("Worker+Vector partitioned sum: %f\n", sums[6]);
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 8) {
        memset(dest, 0, N * sizeof(float));
        test_fully_partitioned(src, dest, N, &sums[7]);
        printf("Fully partitioned sum: %f\n", sums[7]);
    }
    
    /* Additional tests with different constructs */
    if (use_partition_mode == 0) {
        memset(dest, 0, N * sizeof(float));
        test_kernels_partition(src, dest, N, &sums[0]);
        test_data_region_partition(src, dest, N, &sums[1]);
    }
    
    /* Compute final checksum to ensure all computations are used */
    float final_checksum = 0.0f;
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    for (int i = 0; i < N; i++) {
        final_checksum += dest[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}
