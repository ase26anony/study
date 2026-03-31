/* test-omp-acc-partition-codes.c
 * 
 * This program systematically exercises OpenACC data partition codes 0-7
 * to trigger coverage of the string mapping function in GCC's
 * omp-oacc-neuter-broadcast.cc (lines 335-343).
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized \
 *               -foffload=nvptx-none -o test_partition test-omp-acc-partition-codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define CHUNK_SIZE 128

/* Helper to initialize arrays with patterned data */
void init_array(float *arr, int size, float base) {
    for (int i = 0; i < size; i++) {
        arr[i] = base + (i % 100) * 0.1f;
    }
}

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(local_sum) \
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

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] + 1.0f;
            local_sum += dest[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel create(dest[0:n]) copy(src[0:n], local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * src[i];
            local_sum += dest[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += dest[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] / 2.0f;
            local_sum += dest[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] + i * 0.01f;
            local_sum += dest[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] - i * 0.005f;
            local_sum += dest[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * src[i] / 100.0f;
            local_sum += dest[i];
        }
    }
    
    *reduction = local_sum;
}

/* Additional test using kernels construct with explicit data clauses */
void test_kernels_mixed_partitions(float *src1, float *src2, float *dest, int n, float *reduction) {
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Use volatile to prevent compile-time elimination of different paths */
    volatile int mode = 0;
    
    #pragma acc kernels copy(src1[0:n], src2[0:n], dest[0:n]) \
                        copyout(sum1, sum2) \
                        num_gangs(4) num_workers(2) vector_length(32)
    {
        /* This may generate multiple partition codes */
        #pragma acc loop gang reduction(+:sum1)
        for (int i = 0; i < n; i++) {
            sum1 += src1[i];
        }
        
        #pragma acc loop worker reduction(+:sum2)
        for (int i = 0; i < n; i++) {
            sum2 += src2[i];
        }
        
        /* Conditional to force consideration of different partition modes */
        if (mode == 0) {
            #pragma acc loop vector
            for (int i = 0; i < n; i++) {
                dest[i] = src1[i] + src2[i];
            }
        } else {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                dest[i] = src1[i] - src2[i];
            }
        }
    }
    
    *reduction = sum1 + sum2;
}

/* Test with nested data regions and explicit data clauses */
void test_nested_data_regions(float *src, float *dest1, float *dest2, int n, float *reduction) {
    float sum = 0.0f;
    
    /* Outer data region with gang partitioning */
    #pragma acc data copy(src[0:n]) copy(dest1[0:n]) copyout(sum)
    {
        #pragma acc parallel num_gangs(4)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                dest1[i] = src[i] * 1.5f;
            }
        }
        
        /* Inner data region with worker partitioning */
        #pragma acc data copy(dest1[0:n]) create(dest2[0:n])
        {
            #pragma acc parallel num_workers(2)
            {
                #pragma acc loop worker reduction(+:sum)
                for (int i = 0; i < n; i++) {
                    dest2[i] = dest1[i] + i * 0.1f;
                    sum += dest2[i];
                }
            }
        }
    }
    
    *reduction = sum;
}

int main() {
    float *src1, *src2, *dest1, *dest2, *dest3;
    float reductions[10];
    float final_checksum = 0.0f;
    
    /* Allocate and initialize arrays */
    src1 = (float*)malloc(N * sizeof(float));
    src2 = (float*)malloc(N * sizeof(float));
    dest1 = (float*)malloc(N * sizeof(float));
    dest2 = (float*)malloc(N * sizeof(float));
    dest3 = (float*)malloc(N * sizeof(float));
    
    if (!src1 || !src2 || !dest1 || !dest2 || !dest3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(src1, N, 1.0f);
    init_array(src2, N, 2.0f);
    memset(dest1, 0, N * sizeof(float));
    memset(dest2, 0, N * sizeof(float));
    memset(dest3, 0, N * sizeof(float));
    
    printf("Testing OpenACC data partition codes 0-7...\n");
    
    /* Test all 8 partition codes systematically */
    test_gang_redundant(src1, dest1, N, &reductions[0]);
    test_gang_partitioned(src1, dest2, N, &reductions[1]);
    test_worker_partitioned(src1, dest3, N, &reductions[2]);
    test_gang_worker_partitioned(src2, dest1, N, &reductions[3]);
    test_vector_partitioned(src2, dest2, N, &reductions[4]);
    test_gang_vector_partitioned(src2, dest3, N, &reductions[5]);
    test_worker_vector_partitioned(src1, dest1, N, &reductions[6]);
    test_fully_partitioned(src1, dest2, N, &reductions[7]);
    
    /* Additional tests to increase coverage probability */
    test_kernels_mixed_partitions(src1, src2, dest3, N, &reductions[8]);
    test_nested_data_regions(src1, dest1, dest2, N, &reductions[9]);
    
    /* Compute final checksum to ensure all computations are observable */
    for (int i = 0; i < 10; i++) {
        final_checksum += reductions[i];
    }
    
    /* Add array elements to checksum */
    for (int i = 0; i < N; i++) {
        final_checksum += dest1[i] + dest2[i] + dest3[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("(Coverage of partition string mapping should be triggered during compilation with -fdump-tree-omplower)\n");
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(dest1);
    free(dest2);
    free(dest3);
    
    return 0;
}
