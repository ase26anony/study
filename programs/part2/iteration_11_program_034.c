/* test_openacc_partitions.c
 * 
 * This program systematically tests OpenACC data partition modes
 * to trigger coverage of the partition code to string mapping
 * function in GCC's omp-oacc-neuter-broadcast.cc.
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_partitions
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 128

/* Use volatile to prevent compile-time elimination of code paths */
static volatile int force_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) 
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        if (force_partition_mode == 0) {
            *sum = local_sum;
        }
    }
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        if (force_partition_mode == 1) {
            *sum = local_sum;
        }
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy worker(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        if (force_partition_mode == 2) {
            *sum = local_sum;
        }
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        if (force_partition_mode == 3) {
            *sum = local_sum;
        }
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        if (force_partition_mode == 4) {
            *sum = local_sum;
        }
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        if (force_partition_mode == 5) {
            *sum = local_sum;
        }
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy worker vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        if (force_partition_mode == 6) {
            *sum = local_sum;
        }
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        if (force_partition_mode == 7) {
            *sum = local_sum;
        }
    }
}

/* Additional test with kernels construct for different code path */
void test_kernels_partition(const float* src, float* dest, int n, float* sum) {
    #pragma acc kernels copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                if (idx < n) {
                    dest[idx] = src[idx] * 10.0f;
                    local_sum += src[idx];
                }
            }
        }
        *sum = local_sum;
    }
}

/* Test with present clause to trigger different data mapping */
void test_present_partition(const float* src, float* dest, int n, float* sum) {
    #pragma acc data copyin(src[0:n]) create(dest[0:n]) copyout(sum[0:1])
    {
        #pragma acc parallel present(src, dest, sum) copy gang worker(dest[0:n])
        {
            float local_sum = 0.0f;
            #pragma acc loop gang worker reduction(+:local_sum)
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 11.0f;
                local_sum += src[i];
            }
            *sum = local_sum;
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[12] = {0};
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC data partition modes...\n");
    
    /* Test all partition modes 0-7 */
    force_partition_mode = 0;
    test_gang_redundant(src, dest, N, &sums[0]);
    
    force_partition_mode = 1;
    test_gang_partitioned(src, dest, N, &sums[1]);
    
    force_partition_mode = 2;
    test_worker_partitioned(src, dest, N, &sums[2]);
    
    force_partition_mode = 3;
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    
    force_partition_mode = 4;
    test_vector_partitioned(src, dest, N, &sums[4]);
    
    force_partition_mode = 5;
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    
    force_partition_mode = 6;
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    
    force_partition_mode = 7;
    test_fully_partitioned(src, dest, N, &sums[7]);
    
    /* Additional tests for different code paths */
    test_kernels_partition(src, dest, N, &sums[8]);
    test_present_partition(src, dest, N, &sums[9]);
    
    /* Compute final checksum to ensure code isn't optimized away */
    float final_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_checksum += dest[i];
    }
    for (int i = 0; i < 10; i++) {
        final_checksum += sums[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}
