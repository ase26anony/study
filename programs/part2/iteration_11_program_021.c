/* test_openacc_partitions.c
 * Designed to trigger all 8 data partition codes in GCC's omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 64

/* Use volatile to prevent compile-time elimination */
static volatile int force_partition_mode = 0;

/* Test function for gang redundant (partition code 0) */
void test_gang_redundant(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test function for gang partitioned (partition code 1) */
void test_gang_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            #pragma acc cache(src[i:1])  /* Hint for gang partitioning */
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test function for worker partitioned (partition code 2) */
void test_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            #pragma acc cache(src[i:1]) worker  /* Explicit worker cache */
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test function for gang+worker partitioned (partition code 3) */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            #pragma acc cache(src[i:1]) gang worker  /* Both gang and worker */
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test function for vector partitioned (partition code 4) */
void test_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            #pragma acc cache(src[i:1]) vector  /* Vector partitioning */
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test function for gang+vector partitioned (partition code 5) */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            #pragma acc cache(src[i:1]) gang vector  /* Gang and vector */
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test function for worker+vector partitioned (partition code 6) */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            #pragma acc cache(src[i:1]) worker vector  /* Worker and vector */
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test function for fully partitioned (partition code 7) */
void test_fully_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            #pragma acc cache(src[i:1]) gang worker vector  /* Fully partitioned */
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Additional test using kernels construct with explicit data clauses */
void test_kernels_partitions(float *src, float *dest1, float *dest2, int n) {
    /* This should trigger various partition codes through different data clauses */
    
    /* gang partitioned */
    #pragma acc kernels copyin(src[0:n]) copyout(dest1[0:n]) \
                        num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            dest1[i] = src[i] * 10.0f;
        }
    }
    
    /* worker partitioned */
    #pragma acc kernels copyin(src[0:n]) copyout(dest2[0:n]) \
                        num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest2[i] = src[i] * 11.0f;
        }
    }
}

/* Test with multi-dimensional arrays to trigger complex partitioning */
void test_multi_dim_partitions(float src[M][M], float dest[M][M], int m) {
    float sum = 0.0f;
    
    /* This should generate gang+worker+vector partitioning */
    #pragma acc parallel copyin(src[0:m][0:m]) copyout(dest[0:m][0:m]) copyout(sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector collapse(2) reduction(+:sum)
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                dest[i][j] = src[i][j] * 12.0f;
                sum += src[i][j];
            }
        }
    }
    
    /* Force use of sum to prevent optimization */
    if (force_partition_mode) {
        printf("Multi-dim sum: %f\n", sum);
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float reductions[8] = {0};
    
    float src_2d[M][M];
    float dest_2d[M][M];
    
    /* Initialize data with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            src_2d[i][j] = (float)(i * M + j) * 0.01f;
        }
    }
    
    /* Test all 8 partition modes */
    test_gang_redundant(src, dest, N, &reductions[0]);
    test_gang_partitioned(src, dest, N, &reductions[1]);
    test_worker_partitioned(src, dest, N, &reductions[2]);
    test_gang_worker_partitioned(src, dest, N, &reductions[3]);
    test_vector_partitioned(src, dest, N, &reductions[4]);
    test_gang_vector_partitioned(src, dest, N, &reductions[5]);
    test_worker_vector_partitioned(src, dest, N, &reductions[6]);
    test_fully_partitioned(src, dest, N, &reductions[7]);
    
    /* Additional tests with different constructs */
    float *dest1 = (float*)malloc(N * sizeof(float));
    float *dest2 = (float*)malloc(N * sizeof(float));
    test_kernels_partitions(src, dest1, dest2, N);
    
    test_multi_dim_partitions(src_2d, dest_2d, M);
    
    /* Compute final checksum to ensure all computations happened */
    float final_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_checksum += dest[i] + dest1[i] + dest2[i];
    }
    
    for (int i = 0; i < 8; i++) {
        final_checksum += reductions[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            final_checksum += dest_2d[i][j];
        }
    }
    
    printf("Final checksum: %f\n", final_checksum);
    
    /* Cleanup */
    free(src);
    free(dest);
    free(dest1);
    free(dest2);
    
    return 0;
}
