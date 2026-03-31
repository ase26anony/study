/* test-omp-acc-partition-codes.c
 * 
 * This program systematically exercises GCC's OpenACC data partition
 * mapping logic to cover the string lookup cases in omp-oacc-neuter-broadcast.cc
 * lines 335-343.
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized 
 *                -foffload=nvptx-none -o test_partition test-omp-acc-partition-codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define NUM_CASES 8

/* Use volatile to prevent compile-time elimination of code paths */
static volatile int select_case = 0;

/* Initialize array with pattern */
void init_array(float *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = (float)(i % 100) * 0.5f;
    }
}

/* Test case 0: gang redundant (no partition modifier) */
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

/* Test case 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Explicit gang partitioning on dest */
            #pragma acc cache(dest[i:1]) gang
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test case 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Worker partitioned data clause */
            #pragma acc cache(dest[i:1]) worker
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) present(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Gang+worker partitioned */
            #pragma acc cache(dest[i:1]) gang worker
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test case 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Vector partitioned */
            #pragma acc cache(dest[i:1]) vector
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Gang+vector partitioned */
            #pragma acc cache(dest[i:1]) gang vector
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) present(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Worker+vector partitioned */
            #pragma acc cache(dest[i:1]) worker vector
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            /* Fully partitioned */
            #pragma acc cache(dest[i:1]) gang worker vector
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
    }
    
    *reduction = local_sum;
}

/* Wrapper that selects test case based on volatile variable */
void run_selected_test(int case_id, float *src, float *dest, int n, float *reduction) {
    switch (case_id) {
        case 0: test_gang_redundant(src, dest, n, reduction); break;
        case 1: test_gang_partitioned(src, dest, n, reduction); break;
        case 2: test_worker_partitioned(src, dest, n, reduction); break;
        case 3: test_gang_worker_partitioned(src, dest, n, reduction); break;
        case 4: test_vector_partitioned(src, dest, n, reduction); break;
        case 5: test_gang_vector_partitioned(src, dest, n, reduction); break;
        case 6: test_worker_vector_partitioned(src, dest, n, reduction); break;
        case 7: test_fully_partitioned(src, dest, n, reduction); break;
        default: *reduction = -1.0f; break;
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float reductions[NUM_CASES];
    float final_checksum = 0.0f;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(src, N);
    
    /* Run all test cases to ensure compiler processes each partition mode */
    for (int i = 0; i < NUM_CASES; i++) {
        /* Use volatile to force all paths to be considered */
        select_case = i;
        run_selected_test(select_case, src, dest, N, &reductions[i]);
        
        /* Compute partial checksum from destination array */
        float partial_sum = 0.0f;
        for (int j = 0; j < N; j++) {
            partial_sum += dest[j];
        }
        final_checksum += partial_sum + reductions[i];
        
        /* Print to prevent dead code elimination */
        printf("Test case %d: reduction = %f, partial = %f\n", 
               i, reductions[i], partial_sum);
    }
    
    /* Final checksum output ensures all computations have observable effect */
    printf("Final checksum: %f\n", final_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}
