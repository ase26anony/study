/* test_openacc_partitions.c
 * 
 * This program exercises OpenACC data partitioning modes to trigger
 * the partition code string mapping function in GCC's omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 32

/* Use volatile to prevent compile-time elimination */
static volatile int force_partition_mode = 0;

/* Test function for gang redundant (partition code 0) */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
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
    
    *sum = local_sum;
}

/* Test function for gang partitioned (partition code 1) */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Use gang-partitioned data access */
            #pragma acc cache(src[i:1])  /* Hint for gang partitioning */
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for worker partitioned (partition code 2) */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Explicit worker partitioning */
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                if (idx < n) {
                    dest[idx] = src[idx] * 4.0f;
                    local_sum += src[idx];
                }
            }
        }
    }
    
    *sum = local_sum;
}

/* Test function for gang+worker partitioned (partition code 3) */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for vector partitioned (partition code 4) */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for gang+vector partitioned (partition code 5) */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Nested loops to force gang+vector partitioning */
            #pragma acc loop vector
            for (int j = 0; j < 4; j++) {
                int idx = i * 4 + j;
                if (idx < n) {
                    dest[idx] = src[idx] * 7.0f;
                    local_sum += src[idx];
                }
            }
        }
    }
    
    *sum = local_sum;
}

/* Test function for worker+vector partitioned (partition code 6) */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
    }
    
    *sum = local_sum;
}

/* Test function for fully partitioned (partition code 7) */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        /* Triple-nested loops to exercise full partitioning */
        #pragma acc loop gang
        for (int g = 0; g < 4; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 2; w++) {
                #pragma acc loop vector reduction(+:local_sum)
                for (int i = g*2*32 + w*32; i < g*2*32 + (w+1)*32 && i < n; i++) {
                    dest[i] = src[i] * 9.0f;
                    local_sum += src[i];
                }
            }
        }
    }
    
    *sum = local_sum;
}

/* Function that uses data clauses with explicit partition modifiers */
void test_data_clause_partitions(const float* src, float* dest, int n, float* sums) {
    float sum0 = 0.0f, sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    float sum4 = 0.0f, sum5 = 0.0f, sum6 = 0.0f, sum7 = 0.0f;
    
    /* Test different data clause partition modifiers */
    
    /* Code 0: gang redundant (implicit) */
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum0) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:sum0)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i];
            sum0 += src[i];
        }
    }
    
    /* Code 1: gang partitioned */
    #pragma acc parallel copy gang(src[0:n], dest[0:n]) copyout(sum1) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:sum1)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            sum1 += src[i];
        }
    }
    
    /* Code 2: worker partitioned */
    #pragma acc parallel copy worker(src[0:n], dest[0:n]) copyout(sum2) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:sum2)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            sum2 += src[i];
        }
    }
    
    /* Code 3: gang+worker partitioned */
    #pragma acc parallel copy gang worker(src[0:n], dest[0:n]) copyout(sum3) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:sum3)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            sum3 += src[i];
        }
    }
    
    /* Code 4: vector partitioned */
    #pragma acc parallel copy vector(src[0:n], dest[0:n]) copyout(sum4) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:sum4)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum4 += src[i];
        }
    }
    
    /* Code 5: gang+vector partitioned */
    #pragma acc parallel copy gang vector(src[0:n], dest[0:n]) copyout(sum5) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:sum5)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            sum5 += src[i];
        }
    }
    
    /* Code 6: worker+vector partitioned */
    #pragma acc parallel copy worker vector(src[0:n], dest[0:n]) copyout(sum6) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:sum6)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum6 += src[i];
        }
    }
    
    /* Code 7: fully partitioned */
    #pragma acc parallel copy gang worker vector(src[0:n], dest[0:n]) copyout(sum7) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:sum7)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            sum7 += src[i];
        }
    }
    
    sums[0] = sum0; sums[1] = sum1; sums[2] = sum2; sums[3] = sum3;
    sums[4] = sum4; sums[5] = sum5; sums[6] = sum6; sums[7] = sum7;
}

int main() {
    const int total_size = N * M;
    float *src = (float*)malloc(total_size * sizeof(float));
    float *dest = (float*)malloc(total_size * sizeof(float));
    float sums[8] = {0};
    float total_checksum = 0.0f;
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < total_size; i++) {
        src[i] = (i % 100) * 0.01f;
    }
    
    printf("Testing OpenACC data partitioning modes...\n");
    
    /* Test individual partition modes */
    test_gang_redundant(src, dest, total_size, &sums[0]);
    test_gang_partitioned(src, dest, total_size, &sums[1]);
    test_worker_partitioned(src, dest, total_size, &sums[2]);
    test_gang_worker_partitioned(src, dest, total_size, &sums[3]);
    test_vector_partitioned(src, dest, total_size, &sums[4]);
    test_gang_vector_partitioned(src, dest, total_size, &sums[5]);
    test_worker_vector_partitioned(src, dest, total_size, &sums[6]);
    test_fully_partitioned(src, dest, total_size, &sums[7]);
    
    /* Test data clause partition modifiers */
    float clause_sums[8];
    test_data_clause_partitions(src, dest, total_size, clause_sums);
    
    /* Compute final checksum */
    for (int i = 0; i < 8; i++) {
        total_checksum += sums[i] + clause_sums[i];
    }
    
    /* Use volatile to prevent dead code elimination */
    if (force_partition_mode > 1000) {
        printf("Debug output (should not appear): %f\n", total_checksum);
    }
    
    printf("Final checksum: %f\n", total_checksum);
    printf("Test completed.\n");
    
    free(src);
    free(dest);
    
    return 0;
}
