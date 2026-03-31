/* test_openacc_partitions.c
 * 
 * This program tests all 8 OpenACC data partition modes to trigger
 * the partition code string lookup function in GCC's omp-oacc-neuter-broadcast.cc
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 32

/* Use volatile to prevent compile-time elimination */
volatile int use_partition_mode = 0;

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
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
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
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
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
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
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
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
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
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
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
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
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
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
        }
    }
}

/* Test 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
        }
    }
}

/* Additional test with nested loops to trigger complex partitioning */
void test_nested_partitions(float *src, float *dest, int n, int m, float *sum) {
    #pragma acc parallel copy(src[0:n*m]) create gang worker vector(dest[0:n*m]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                #pragma acc loop vector reduction(+:local_sum)
                for (int k = 0; k < 4; k++) {
                    int idx = i*m + j;
                    dest[idx] = src[idx] * (i + j + k);
                    local_sum += src[idx];
                }
            }
        }
        if (__builtin_acc_on_device()) {
            sum[0] = local_sum;
        }
    }
}

/* Test with present clause to trigger different code paths */
void test_with_present(float *src, float *dest, int n, float *sum) {
    #pragma acc enter data copyin(src[0:n]) create(dest[0:n], sum[0:1])
    
    /* Force different partition modes based on volatile */
    if (use_partition_mode % 2 == 0) {
        #pragma acc parallel present(src[0:n]) present gang(dest[0:n]) present(sum[0:1])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 2.5f;
            }
        }
    } else {
        #pragma acc parallel present(src[0:n]) present worker vector(dest[0:n]) present(sum[0:1])
        {
            #pragma acc loop worker vector
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 3.5f;
            }
        }
    }
    
    #pragma acc exit data copyout(dest[0:n], sum[0:1]) delete(src[0:n])
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC data partition modes...\n");
    
    /* Test all 8 partition modes systematically */
    test_gang_redundant(src, dest, N, &sums[0]);
    printf("Test 0 (gang redundant) completed, sum = %f\n", sums[0]);
    
    test_gang_partitioned(src, dest, N, &sums[1]);
    printf("Test 1 (gang partitioned) completed, sum = %f\n", sums[1]);
    
    test_worker_partitioned(src, dest, N, &sums[2]);
    printf("Test 2 (worker partitioned) completed, sum = %f\n", sums[2]);
    
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    printf("Test 3 (gang+worker partitioned) completed, sum = %f\n", sums[3]);
    
    test_vector_partitioned(src, dest, N, &sums[4]);
    printf("Test 4 (vector partitioned) completed, sum = %f\n", sums[4]);
    
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    printf("Test 5 (gang+vector partitioned) completed, sum = %f\n", sums[5]);
    
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    printf("Test 6 (worker+vector partitioned) completed, sum = %f\n", sums[6]);
    
    test_fully_partitioned(src, dest, N, &sums[7]);
    printf("Test 7 (fully partitioned) completed, sum = %f\n", sums[7]);
    
    /* Test nested loops with complex partitioning */
    float *src2d = (float*)malloc(N * M * sizeof(float));
    float *dest2d = (float*)malloc(N * M * sizeof(float));
    float sum_nested = 0.0f;
    
    for (int i = 0; i < N * M; i++) {
        src2d[i] = (float)(i % 50) * 0.05f;
    }
    
    test_nested_partitions(src2d, dest2d, N, M, &sum_nested);
    printf("Nested partition test completed, sum = %f\n", sum_nested);
    
    /* Test with present clause */
    test_with_present(src, dest, N, &sums[0]);
    printf("Present clause test completed\n");
    
    /* Compute final checksum */
    float final_checksum = 0.0f;
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    final_checksum += sum_nested;
    
    /* Add some values from dest arrays to ensure they're used */
    for (int i = 0; i < 10; i++) {
        final_checksum += dest[i];
        if (i < N*M) final_checksum += dest2d[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    free(src);
    free(dest);
    free(src2d);
    free(dest2d);
    
    return 0;
}
