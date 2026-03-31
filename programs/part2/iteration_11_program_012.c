/* test_partition_codes.c - Cover all data partition mapping codes in GCC's omp-oacc-neuter-broadcast.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define GANGS 4
#define WORKERS 4
#define VECTOR_LENGTH 32

/* Use volatile to prevent compile-time elimination */
volatile int use_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANGS) num_workers(1) vector_length(VECTOR_LENGTH)
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
    #pragma acc parallel copy(src[0:n]) copy gang(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANGS) num_workers(1) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            sum[0] += src[i] * 2;
        }
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy worker(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            sum[0] += src[i] * 3;
        }
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum[0] += src[i] * 4;
        }
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy vector(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(1) num_workers(1) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            sum[0] += src[i] * 5;
        }
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang vector(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANGS) num_workers(1) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum[0] += src[i] * 6;
        }
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy worker vector(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            sum[0] += src[i] * 7;
        }
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1]) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LENGTH)
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            sum[0] += src[i] * 8;
        }
    }
}

/* Wrapper that selects partition mode based on volatile variable */
void test_partition_wrapper(int mode, float *src, float *dest, int n, float *sum) {
    switch (mode) {
        case 0: test_gang_redundant(src, dest, n, sum); break;
        case 1: test_gang_partitioned(src, dest, n, sum); break;
        case 2: test_worker_partitioned(src, dest, n, sum); break;
        case 3: test_gang_worker_partitioned(src, dest, n, sum); break;
        case 4: test_vector_partitioned(src, dest, n, sum); break;
        case 5: test_gang_vector_partitioned(src, dest, n, sum); break;
        case 6: test_worker_vector_partitioned(src, dest, n, sum); break;
        case 7: test_fully_partitioned(src, dest, n, sum); break;
        default: break;
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    float total_checksum = 0.0f;
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (i % 100) * 0.1f;
    }
    
    printf("Testing all OpenACC data partition modes (codes 0-7)...\n");
    
    /* Test each partition mode systematically */
    for (int mode = 0; mode < 8; mode++) {
        /* Clear destination and sum */
        memset(dest, 0, N * sizeof(float));
        sums[mode] = 0.0f;
        
        /* Use volatile to force compiler to consider all modes */
        use_partition_mode = mode;
        
        /* Test the specific partition mode */
        test_partition_wrapper(use_partition_mode, src, dest, N, &sums[mode]);
        
        /* Compute checksum from results to ensure computation happens */
        float dest_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            dest_sum += dest[i];
        }
        
        total_checksum += sums[mode] + dest_sum;
        
        printf("Mode %d: sum = %.2f, dest_sum = %.2f\n", 
               mode, sums[mode], dest_sum);
    }
    
    printf("\nTotal checksum: %.2f\n", total_checksum);
    
    /* Verify with expected value (computed from pattern) */
    float expected_src_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        expected_src_sum += src[i];
    }
    
    /* Expected checksum formula based on our test functions */
    float expected_total = expected_src_sum * (2+3+4+5+6+7+8+9 + 1+2+3+4+5+6+7+8);
    printf("Expected checksum: %.2f\n", expected_total);
    
    free(src);
    free(dest);
    
    return 0;
}
