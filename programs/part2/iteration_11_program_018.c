/* test_openacc_partitions.c
 * 
 * This program systematically exercises OpenACC data partition codes 0-7
 * by creating compute regions with different data clause modifiers.
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -foffload=nvptx-none test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define VAL(i) ((i) * 3.14159f)

/* Helper to prevent optimization */
static volatile int use_partition_mode = 0;

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 0;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) 
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            sum[0] += src[i];
        }
    }
}

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 1;
    
    #pragma acc parallel copy(src[0:n]) copy gang(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            sum[0] += src[i] * 2.0f;
        }
    }
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 2;
    
    #pragma acc parallel copy(src[0:n]) copy worker(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            sum[0] += src[i] * 3.0f;
        }
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 3;
    
    #pragma acc parallel copy(src[0:n]) copy gang worker(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum[0] += src[i] * 4.0f;
        }
    }
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 4;
    
    #pragma acc parallel copy(src[0:n]) copy vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            sum[0] += src[i] * 5.0f;
        }
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 5;
    
    #pragma acc parallel copy(src[0:n]) copy gang vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum[0] += src[i] * 6.0f;
        }
    }
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 6;
    
    #pragma acc parallel copy(src[0:n]) copy worker vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            sum[0] += src[i] * 7.0f;
        }
    }
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 7;
    
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) copy(sum[0:1])
    {
        #pragma acc loop gang worker vector reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            sum[0] += src[i] * 8.0f;
        }
    }
}

/* Additional test using kernels construct for variety */
void test_kernels_partition(float *src, float *dest, int n, float *sum) {
    use_partition_mode = 3; /* gang+worker */
    
    #pragma acc kernels copyin(src[0:n]) copyout(dest[0:n]) copy(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 10.0f;
            local_sum += src[i];
        }
        sum[0] = local_sum;
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    
    /* Initialize source array */
    for (int i = 0; i < N; i++) {
        src[i] = VAL(i);
    }
    
    printf("Testing OpenACC data partition modes 0-7...\n");
    
    /* Test all 8 partition modes */
    test_gang_redundant(src, dest, N, &sums[0]);
    test_gang_partitioned(src, dest, N, &sums[1]);
    test_worker_partitioned(src, dest, N, &sums[2]);
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    test_vector_partitioned(src, dest, N, &sums[4]);
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    test_fully_partitioned(src, dest, N, &sums[7]);
    
    /* Also test kernels construct */
    float kernel_sum = 0.0f;
    test_kernels_partition(src, dest, N, &kernel_sum);
    
    /* Compute final checksum to ensure all computations happened */
    float total_checksum = kernel_sum;
    for (int i = 0; i < 8; i++) {
        total_checksum += sums[i];
    }
    
    /* Add dest array values to checksum */
    for (int i = 0; i < N; i++) {
        total_checksum += dest[i];
    }
    
    printf("Total checksum: %f\n", total_checksum);
    printf("Partition mode tests completed.\n");
    
    /* Verify against expected value (computed analytically) */
    float expected_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        float val = VAL(i);
        expected_sum += val * (2.0f + 3.0f + 4.0f + 5.0f + 6.0f + 7.0f + 8.0f + 9.0f + 10.0f);
        expected_sum += val * (0.0f + 2.0f + 3.0f + 4.0f + 5.0f + 6.0f + 7.0f + 8.0f);
    }
    
    printf("Expected checksum: %f\n", expected_sum);
    printf("Difference: %e\n", total_checksum - expected_sum);
    
    free(src);
    free(dest);
    
    return 0;
}
