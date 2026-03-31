/* test_openacc_partitions.c
 * 
 * This program systematically tests all 8 OpenACC data partition modes
 * to trigger coverage of the partition code string mapping function
 * in omp-oacc-neuter-broadcast.cc lines 335-343.
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_partitions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32

/* Use volatile to prevent compile-time elimination */
volatile int use_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* Code 0: gang redundant - no partition modifier */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* Code 1: gang partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 2.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* Code 2: worker partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i] * 3.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* Code 3: gang+worker partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i] * 4.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* Code 4: vector partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 5.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* Code 5: gang+vector partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 6.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* Code 6: worker+vector partitioned */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i] * 7.0f;
        }
    }
    
    *sum = local_sum;
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* Code 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 8.0f;
        }
    }
    
    *sum = local_sum;
}

/* Additional tests with explicit data clauses and partition modifiers */
void test_explicit_data_partitions(float *src, float *dest1, float *dest2, int n) {
    /* Test various explicit data partition modifiers */
    
    /* gang partitioned data clause */
    #pragma acc data copyin(src[0:n]) create(dest1[0:n]) copyout(dest2[0:n])
    {
        #pragma acc parallel present(src, dest1, dest2)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest1[i] = src[i] * 10.0f;
            }
            
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest2[i] = dest1[i] + src[i];
            }
        }
    }
    
    /* worker partitioned data clause */
    float temp[M];
    #pragma acc data copyin(src[0:n]) create(temp[0:M])
    {
        #pragma acc parallel present(src, temp)
        {
            #pragma acc loop worker
            for (int i = 0; i < M; i++) {
                temp[i] = src[i % n] * (i + 1);
            }
        }
    }
    
    /* vector partitioned data clause with nested loops */
    #pragma acc data copy(src[0:n]) copy(dest1[0:n])
    {
        #pragma acc parallel present(src, dest1)
        {
            #pragma acc loop gang
            for (int block = 0; block < n/32; block++) {
                #pragma acc loop worker vector
                for (int i = 0; i < 32; i++) {
                    int idx = block * 32 + i;
                    if (idx < n) {
                        dest1[idx] = src[idx] * (block + 1) + i;
                    }
                }
            }
        }
    }
}

/* Test with kernels construct for different code generation paths */
void test_kernels_partitions(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    /* kernels region with gang partitioned data */
    #pragma acc kernels copy(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 11.0f;
            local_sum += src[i] * 9.0f;
        }
        
        /* Additional nested loop to trigger worker partitioning */
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest[i] += i * 0.1f;
        }
    }
    
    *sum = local_sum;
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float *dest2 = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    float total_sum = 0.0f;
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (i % 100) * 0.1f + 1.0f;
    }
    
    /* Clear destination arrays */
    memset(dest, 0, N * sizeof(float));
    memset(dest2, 0, N * sizeof(float));
    
    printf("Testing OpenACC data partition modes...\n");
    
    /* Test all 8 partition modes */
    test_gang_redundant(src, dest, N, &sums[0]);
    test_gang_partitioned(src, dest, N, &sums[1]);
    test_worker_partitioned(src, dest, N, &sums[2]);
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    test_vector_partitioned(src, dest, N, &sums[4]);
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    test_fully_partitioned(src, dest, N, &sums[7]);
    
    /* Test explicit data clauses with partition modifiers */
    test_explicit_data_partitions(src, dest, dest2, N);
    
    /* Test kernels construct */
    float kernels_sum = 0.0f;
    test_kernels_partitions(src, dest, N, &kernels_sum);
    
    /* Compute final checksum to ensure all computations are used */
    for (int i = 0; i < 8; i++) {
        total_sum += sums[i];
    }
    total_sum += kernels_sum;
    
    /* Also use dest arrays in checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        total_sum += dest[i] * 0.001f;
        total_sum += dest2[i] * 0.001f;
    }
    
    printf("Total checksum: %f\n", total_sum);
    
    /* Use volatile to control which test runs (prevents optimization) */
    if (use_partition_mode) {
        printf("Partition mode forced: %d\n", use_partition_mode);
    }
    
    free(src);
    free(dest);
    free(dest2);
    
    return 0;
}
