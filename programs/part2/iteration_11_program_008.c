/* test_omp_acc_partition_codes.c
 * 
 * This program systematically exercises GCC's OpenACC data partition
 * code mapping logic (lines 335-343 of omp-oacc-neuter-broadcast.cc).
 * It creates OpenACC regions with all 8 partition modes (0-7) to ensure
 * the compiler generates and potentially queries the partition codes
 * during compilation, especially when using -fdump-tree-omplower.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define NUM_TESTS 8

/* Use volatile to prevent compile-time elimination of test cases */
static volatile int force_test = 1;

/* Test 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            *sum += src[i];
        }
    }
}

/* Test 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) copy gang(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            *sum += src[i] * 2;
        }
    }
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) copy worker(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            *sum += src[i] * 3;
        }
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) copy gang worker(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            *sum += src[i] * 4;
        }
    }
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) copy vector(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            *sum += src[i] * 5;
        }
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) copy gang vector(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            *sum += src[i] * 6;
        }
    }
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) copy worker vector(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            *sum += src[i] * 7;
        }
    }
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) copy gang worker vector(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            *sum += src[i] * 8;
        }
    }
}

/* Additional test with present clause to trigger different code paths */
void test_with_present(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc data copyin(src[0:n]) create(dest[0:n])
    {
        #pragma acc parallel present(src[0:n], dest[0:n]) \
                    copy gang worker vector(dest[0:n]) reduction(+:*sum)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < n; i++) {
                dest[i] = src[i] * 10.0f;
                *sum += src[i] * 9;
            }
        }
    }
}

/* Test with create clause (alternative to copy) */
void test_with_create(float *src, float *dest, int n, float *sum) {
    if (!force_test) return;
    
    #pragma acc parallel copy(src[0:n]) create gang worker(dest[0:n]) reduction(+:*sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 11.0f;
            *sum += src[i] * 10;
        }
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[NUM_TESTS + 2] = {0};  /* +2 for extra tests */
    float final_checksum = 0.0f;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (i % 100) * 0.1f;
    }
    
    printf("Testing OpenACC partition modes (0-7)...\n");
    
    /* Execute all test cases covering partition codes 0-7 */
    test_gang_redundant(src, dest, N, &sums[0]);
    test_gang_partitioned(src, dest, N, &sums[1]);
    test_worker_partitioned(src, dest, N, &sums[2]);
    test_gang_worker_partitioned(src, dest, N, &sums[3]);
    test_vector_partitioned(src, dest, N, &sums[4]);
    test_gang_vector_partitioned(src, dest, N, &sums[5]);
    test_worker_vector_partitioned(src, dest, N, &sums[6]);
    test_fully_partitioned(src, dest, N, &sums[7]);
    
    /* Extra tests to ensure broader coverage */
    test_with_present(src, dest, N, &sums[8]);
    test_with_create(src, dest, N, &sums[9]);
    
    /* Compute final checksum from all results */
    for (int i = 0; i < NUM_TESTS + 2; i++) {
        final_checksum += sums[i];
    }
    
    /* Add destination array values to checksum */
    for (int i = 0; i < N; i++) {
        final_checksum += dest[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Partition mode tests completed.\n");
    
    /* Compilation note */
    printf("\nCompile with: gcc -O2 -fopenacc -fdump-tree-omplower ");
    printf("-fdump-tree-optimized -foffload=nvptx-none ");
    printf("-o test_partition test.c\n");
    
    free(src);
    free(dest);
    
    return 0;
}
