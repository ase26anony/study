/* test_omp_acc_partition_codes.c
 * 
 * This program systematically exercises OpenACC data partition codes 0-7
 * to trigger the string lookup logic in GCC's omp-oacc-neuter-broadcast.cc
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partition test_omp_acc_partition_codes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define VOLATILE_SELECTOR

/* Helper to prevent dead code elimination */
static volatile int force_runtime = 1;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(1) vector_length(32)
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
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(4) num_workers(1) vector_length(32)
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
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(4) vector_length(32)
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
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(2) num_workers(2) vector_length(32)
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
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(1) vector_length(64)
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
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(2) num_workers(1) vector_length(32)
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
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(2) vector_length(32)
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
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum) \
                         num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 8.0f;
        }
    }
    
    *sum = local_sum;
}

/* Additional tests with explicit data clauses for different partition modes */
void test_explicit_data_clauses() {
    int n = 512;
    float *a = (float*)malloc(n * sizeof(float));
    float *b = (float*)malloc(n * sizeof(float));
    float *c = (float*)malloc(n * sizeof(float));
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
    
    /* Test 1: gang partitioned data clause */
    #pragma acc data copyin(a[0:n]) copyout(b[0:n]) create(c[0:n])
    {
        #pragma acc parallel present(a, b, c) num_gangs(4)
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                c[i] = a[i] + b[i];
            }
        }
    }
    
    /* Test 2: worker partitioned data clause */
    #pragma acc data copyin(a[0:n]) copyout(b[0:n])
    {
        #pragma acc parallel present(a, b) num_workers(4)
        {
            #pragma acc loop worker
            for (int i = 0; i < n; i++) {
                b[i] = a[i] * 1.5f;
            }
        }
    }
    
    /* Test 3: vector partitioned data clause */
    #pragma acc data copyin(a[0:n]) copyout(b[0:n])
    {
        #pragma acc parallel present(a, b) vector_length(64)
        {
            #pragma acc loop vector
            for (int i = 0; i < n; i++) {
                b[i] = a[i] / 2.0f;
            }
        }
    }
    
    free(a);
    free(b);
    free(c);
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    float final_checksum = 0.0f;
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)((i % 100) + 1) * 0.5f;
    }
    
    /* Use volatile to prevent compile-time elimination */
    #ifdef VOLATILE_SELECTOR
    volatile int selector = force_runtime;
    #else
    int selector = 1;
    #endif
    
    /* Execute all test functions to cover partition codes 0-7 */
    if (selector > 0) {
        test_gang_redundant(src, dest, N, &sums[0]);
        test_gang_partitioned(src, dest, N, &sums[1]);
        test_worker_partitioned(src, dest, N, &sums[2]);
        test_gang_worker_partitioned(src, dest, N, &sums[3]);
        test_vector_partitioned(src, dest, N, &sums[4]);
        test_gang_vector_partitioned(src, dest, N, &sums[5]);
        test_worker_vector_partitioned(src, dest, N, &sums[6]);
        test_fully_partitioned(src, dest, N, &sums[7]);
        
        /* Additional tests with explicit data clauses */
        test_explicit_data_clauses();
    }
    
    /* Compute final checksum to ensure all computations are observable */
    for (int i = 0; i < 8; i++) {
        final_checksum += sums[i];
    }
    
    /* Also checksum the destination array from the last test */
    float dest_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        dest_sum += dest[i];
    }
    final_checksum += dest_sum;
    
    printf("Final checksum: %f\n", final_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}
