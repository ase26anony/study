/* test_omp_acc_partition_codes.c
 * 
 * This program systematically exercises GCC's OpenACC data partition
 * code mapping logic (covering codes 0-7) by generating OpenACC
 * parallel regions with explicit data clause modifiers.
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partition test_omp_acc_partition_codes.c
 * 
 * The -fdump-tree-omplower flag is critical to trigger the compiler's
 * internal partition code generation and string lookup logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define N 1024
#define NUM_TESTS 8

/* Use volatile to prevent compile-time elimination of test selection */
static volatile int test_selector = 0;

/* Initialize array with pattern */
void init_array(float *arr, int n, float base) {
    for (int i = 0; i < n; i++) {
        arr[i] = base + i * 0.1f;
    }
}

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(local_sum)
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
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 0.5f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i] * 0.25f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i] * 0.2f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 0.166f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 0.142f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i] * 0.125f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 0.111f;
        }
    }
    
    *reduction = local_sum;
}

/* Wrapper that selects test based on volatile variable */
void run_partition_test(int test_id, float *src, float *dest, int n, float *reduction) {
    switch (test_id) {
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
    float reductions[NUM_TESTS];
    float final_checksum = 0.0f;
    
    /* Initialize source array */
    init_array(src, N, 1.0f);
    
    printf("Testing OpenACC data partition codes 0-7...\n");
    
    /* Run all 8 partition tests */
    for (int test_id = 0; test_id < NUM_TESTS; test_id++) {
        /* Reset destination array */
        for (int i = 0; i < N; i++) dest[i] = 0.0f;
        
        /* Use volatile to force runtime selection */
        test_selector = test_id;
        
        /* Run the specific partition test */
        run_partition_test(test_selector, src, dest, N, &reductions[test_id]);
        
        /* Compute checksum from destination array */
        float dest_sum = 0.0f;
        for (int i = 0; i < N; i++) {
            dest_sum += dest[i];
        }
        
        /* Accumulate to final checksum */
        final_checksum += reductions[test_id] + dest_sum;
        
        printf("  Test %d: reduction = %f, dest_sum = %f\n", 
               test_id, reductions[test_id], dest_sum);
    }
    
    /* Print final checksum to ensure side effects are observable */
    printf("\nFinal checksum: %f\n", final_checksum);
    
    /* Cleanup */
    free(src);
    free(dest);
    
    return 0;
}
