/* test_openacc_partitions.c
 * Designed to trigger coverage of omp-oacc-neuter-broadcast.cc lines 335-343
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_acc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define CHUNK_SIZE 128

/* Use volatile to prevent compile-time elimination */
static volatile int force_partition_mode = 0;

/* Test function for gang redundant (code 0) */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum[0:1]) 
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for gang partitioned (code 1) */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for worker partitioned (code 2) */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for gang+worker partitioned (code 3) */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for vector partitioned (code 4) */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for gang+vector partitioned (code 5) */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for worker+vector partitioned (code 6) */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test function for fully partitioned (code 7) */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Additional tests with explicit data clauses and partition modifiers */
void test_explicit_partition_modifiers(float* data, int n, float* results) {
    /* Test 1: gang partitioned with explicit modifier */
    #pragma acc parallel copyin(data[0:n]) copyout(results[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            results[i] = data[i] + 1.0f;
        }
    }
    
    /* Test 2: worker partitioned with explicit modifier */
    float* temp = (float*)malloc(n * sizeof(float));
    #pragma acc enter data create(temp[0:n])
    #pragma acc parallel present(data[0:n], temp[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            temp[i] = data[i] * 2.0f;
        }
    }
    #pragma acc exit data copyout(temp[0:n])
    free(temp);
    
    /* Test 3: vector partitioned with explicit modifier */
    #pragma acc parallel copy(data[0:n]) copy(results[0:n])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            results[i] = data[i] / 2.0f;
        }
    }
}

/* Complex test with nested data regions and multiple partition schemes */
void test_complex_partition_schemes(float* src1, float* src2, float* dest, int n) {
    /* Use volatile to force runtime selection of partition scheme */
    int mode = force_partition_mode;
    
    if (mode == 0) {
        /* gang redundant */
        #pragma acc data copy(src1[0:n], src2[0:n], dest[0:n])
        {
            #pragma acc parallel
            {
                #pragma acc loop gang
                for (int i = 0; i < n; i++) {
                    dest[i] = src1[i] + src2[i];
                }
            }
        }
    } else if (mode == 1) {
        /* gang partitioned */
        #pragma acc data copyin(src1[0:n], src2[0:n]) copyout(dest[0:n])
        {
            #pragma acc parallel
            {
                #pragma acc loop gang
                for (int i = 0; i < n; i++) {
                    dest[i] = src1[i] - src2[i];
                }
            }
        }
    } else if (mode == 2) {
        /* worker partitioned */
        #pragma acc data copy(src1[0:n], src2[0:n], dest[0:n])
        {
            #pragma acc parallel
            {
                #pragma acc loop gang worker
                for (int i = 0; i < n; i++) {
                    dest[i] = src1[i] * src2[i];
                }
            }
        }
    }
    /* Additional modes would continue here... */
}

int main() {
    const int n = N;
    float *src, *dest, *results;
    float sums[8] = {0};
    float total_checksum = 0.0f;
    
    /* Allocate and initialize data */
    src = (float*)malloc(n * sizeof(float));
    dest = (float*)malloc(n * sizeof(float));
    results = (float*)malloc(n * sizeof(float));
    
    if (!src || !dest || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < n; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC partition modes...\n");
    
    /* Test all partition modes 0-7 */
    test_gang_redundant(src, dest, n, &sums[0]);
    test_gang_partitioned(src, dest, n, &sums[1]);
    test_worker_partitioned(src, dest, n, &sums[2]);
    test_gang_worker_partitioned(src, dest, n, &sums[3]);
    test_vector_partitioned(src, dest, n, &sums[4]);
    test_gang_vector_partitioned(src, dest, n, &sums[5]);
    test_worker_vector_partitioned(src, dest, n, &sums[6]);
    test_fully_partitioned(src, dest, n, &sums[7]);
    
    /* Test explicit partition modifiers */
    test_explicit_partition_modifiers(src, n, results);
    
    /* Test complex partition schemes */
    test_complex_partition_schemes(src, dest, results, n);
    
    /* Compute final checksum to ensure computations aren't optimized away */
    for (int i = 0; i < 8; i++) {
        total_checksum += sums[i];
    }
    
    for (int i = 0; i < n; i++) {
        total_checksum += dest[i] + results[i];
    }
    
    printf("Total checksum: %f\n", total_checksum);
    printf("Partition mode tests completed.\n");
    
    /* Cleanup */
    free(src);
    free(dest);
    free(results);
    
    return 0;
}
