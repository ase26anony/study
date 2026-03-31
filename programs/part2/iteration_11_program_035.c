/* test-omp-oacc-partition-codes.c
 * 
 * This program systematically exercises GCC's OpenACC data partition
 * code mapping logic (lines 335-343 in omp-oacc-neuter-broadcast.cc).
 * Each test function uses a different data partition clause combination
 * to trigger the internal mapping of partition codes 0-7 to their
 * string representations.
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -foffload=nvptx-none
 *               -fdump-tree-optimized test.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define NUM_TESTS 8

/* Use volatile to prevent compile-time elimination of partition logic */
static volatile int force_partition_mode = 0;

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 0;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(sum[0:1]) \
                         present_or_copyin(src[0:n]) /* gang redundant */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 1;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         create gang(dest[0:n]) /* gang partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 2;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy worker(dest[0:n]) /* worker partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 3;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy gang worker(dest[0:n]) /* gang+worker partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 4;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy vector(dest[0:n]) /* vector partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 5;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy gang vector(dest[0:n]) /* gang+vector partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 6;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy worker vector(dest[0:n]) /* worker+vector partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    force_partition_mode = 7;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy gang worker vector(dest[0:n]) /* fully partitioned */
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Array of test function pointers */
typedef void (*test_func_t)(float*, float*, int, float*);
test_func_t test_functions[NUM_TESTS] = {
    test_gang_redundant,
    test_gang_partitioned,
    test_worker_partitioned,
    test_gang_worker_partitioned,
    test_vector_partitioned,
    test_gang_vector_partitioned,
    test_worker_vector_partitioned,
    test_fully_partitioned
};

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[NUM_TESTS] = {0};
    float final_checksum = 0.0f;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (i % 100) * 0.1f;
    }
    
    printf("Running OpenACC partition code tests...\n");
    
    /* Execute all test functions */
    for (int test = 0; test < NUM_TESTS; test++) {
        /* Clear destination for each test */
        memset(dest, 0, N * sizeof(float));
        
        /* Execute test with current partition mode */
        test_functions[test](src, dest, N, &sums[test]);
        
        /* Compute checksum from destination array */
        float dest_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            dest_sum += dest[i];
        }
        
        final_checksum += sums[test] + dest_sum;
        
        printf("Test %d completed: sum = %f, dest_sum = %f\n", 
               test, sums[test], dest_sum);
    }
    
    printf("\nFinal checksum: %f\n", final_checksum);
    
    /* Force compiler to consider all partition modes by using volatile */
    printf("Forced partition mode values (prevents dead code elimination):\n");
    for (int i = 0; i < NUM_TESTS; i++) {
        printf(" %d", force_partition_mode);
    }
    printf("\n");
    
    free(src);
    free(dest);
    
    return 0;
}
