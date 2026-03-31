/* test_omp_acc_partition_codes.c
 * 
 * This program systematically exercises GCC's OpenACC data partition
 * mapping logic to cover the string lookup cases in omp-oacc-neuter-broadcast.cc
 * lines 335-343.
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partition test_omp_acc_partition_codes.c
 * Additional flags for debugging: -fdump-tree-all -da
 * For NVIDIA offload: gcc -O2 -fopenacc -fdump-tree-omplower -foffload=nvptx-none -o test_partition_nv test_omp_acc_partition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define NUM_CASES 8

/* Use volatile to prevent compile-time elimination of code paths */
static volatile int force_runtime = 1;

/* Test case 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum_out) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum) \
        num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            sum += src[i];
        }
    }
    
    *sum_out = sum;
}

/* Test case 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum_out) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(sum) \
        num_gangs(4) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            sum += src[i] * 2.0f;
        }
    }
    
    *sum_out = sum;
}

/* Test case 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum_out) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum) \
        num_gangs(2) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            sum += src[i] * 3.0f;
        }
    }
    
    *sum_out = sum;
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum_out) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum) \
        num_gangs(4) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            sum += src[i] * 4.0f;
        }
    }
    
    *sum_out = sum;
}

/* Test case 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum_out) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum) \
        num_gangs(1) num_workers(1) vector_length(64)
    {
        #pragma acc loop vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            sum += src[i] * 5.0f;
        }
    }
    
    *sum_out = sum;
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum_out) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum) \
        num_gangs(4) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            sum += src[i] * 6.0f;
        }
    }
    
    *sum_out = sum;
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum_out) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum) \
        num_gangs(1) num_workers(4) vector_length(64)
    {
        #pragma acc loop worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            sum += src[i] * 7.0f;
        }
    }
    
    *sum_out = sum;
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum_out) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(sum) \
        num_gangs(4) num_workers(4) vector_length(64)
    {
        #pragma acc loop gang worker vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            sum += src[i] * 8.0f;
        }
    }
    
    *sum_out = sum;
}

/* Function pointer type for test cases */
typedef void (*test_func_t)(float*, float*, int, float*);

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[NUM_CASES];
    float final_checksum = 0.0f;
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (i % 100) * 0.1f;
    }
    
    /* Array of test functions for each partition code */
    test_func_t test_functions[NUM_CASES] = {
        test_gang_redundant,
        test_gang_partitioned,
        test_worker_partitioned,
        test_gang_worker_partitioned,
        test_vector_partitioned,
        test_gang_vector_partitioned,
        test_worker_vector_partitioned,
        test_fully_partitioned
    };
    
    /* Execute all test cases */
    for (int case_id = 0; case_id < NUM_CASES; case_id++) {
        /* Clear destination array */
        memset(dest, 0, N * sizeof(float));
        
        /* Use volatile to force runtime execution */
        if (force_runtime) {
            test_functions[case_id](src, dest, N, &sums[case_id]);
        }
        
        /* Compute checksum from destination array */
        float dest_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            dest_sum += dest[i];
        }
        
        final_checksum += sums[case_id] + dest_sum;
        
        /* Print diagnostic info (prevents dead code elimination) */
        printf("Case %d: sum_out = %f, dest_sum = %f\n", 
               case_id, sums[case_id], dest_sum);
    }
    
    /* Final checksum output ensures all computations have observable effect */
    printf("Final checksum: %f\n", final_checksum);
    
    /* Cleanup */
    free(src);
    free(dest);
    
    return 0;
}
