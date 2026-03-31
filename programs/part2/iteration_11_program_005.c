/* test_openacc_partitions.c
 * 
 * This program systematically tests OpenACC data partition modes
 * to trigger coverage of the partition code to string mapping
 * function in GCC's omp-oacc-neuter-broadcast.cc (lines 335-343).
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 32

/* Use volatile to prevent compile-time elimination of code paths */
static volatile int select_partition = 0;

/* Test function for gang redundant (partition code 0) */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) gang
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            sum[0] += src[i];
        }
    }
}

/* Test function for gang partitioned (partition code 1) */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            sum[0] += src[i] * 2.0f;
        }
    }
}

/* Test function for worker partitioned (partition code 2) */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy worker(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            sum[0] += src[i] * 3.0f;
        }
    }
}

/* Test function for gang+worker partitioned (partition code 3) */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum[0] += src[i] * 4.0f;
        }
    }
}

/* Test function for vector partitioned (partition code 4) */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            sum[0] += src[i] * 5.0f;
        }
    }
}

/* Test function for gang+vector partitioned (partition code 5) */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum[0] += src[i] * 6.0f;
        }
    }
}

/* Test function for worker+vector partitioned (partition code 6) */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy worker vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            sum[0] += src[i] * 7.0f;
        }
    }
}

/* Test function for fully partitioned (partition code 7) */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            sum[0] += src[i] * 8.0f;
        }
    }
}

/* Wrapper function that selects partition mode based on volatile variable */
void test_partition_wrapper(int mode, const float* src, float* dest, int n, float* sum) {
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
    /* Allocate and initialize source data */
    float* src = (float*)malloc(N * sizeof(float));
    float* dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0.0f};
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC data partition modes...\n");
    
    /* Test all 8 partition modes systematically */
    for (int mode = 0; mode < 8; mode++) {
        /* Reset destination and sum for each test */
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        sums[mode] = 0.0f;
        
        /* Use volatile to force compiler to consider all paths */
        select_partition = mode;
        
        /* Call wrapper with current mode */
        test_partition_wrapper(select_partition, src, dest, N, &sums[mode]);
        
        /* Compute checksum from destination array */
        float dest_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            dest_sum += dest[i];
        }
        
        printf("Mode %d: sum = %f, dest_sum = %f\n", mode, sums[mode], dest_sum);
    }
    
    /* Final checksum to ensure all computations are used */
    float final_checksum = 0.0f;
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    /* Cleanup */
    free(src);
    free(dest);
    
    return 0;
}
