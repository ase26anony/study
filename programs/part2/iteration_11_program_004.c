/* test_omp_acc_partition_codes.c
 * 
 * This program systematically exercises OpenACC data partition codes 0-7
 * to trigger the string lookup logic in GCC's omp-oacc-neuter-broadcast.cc
 * 
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test_partition test_omp_acc_partition_codes.c
 * 
 * The -fdump-tree-omplower flag is crucial to trigger the compiler's internal
 * OpenACC processing that generates and queries partition codes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define GANG_SIZE 32
#define WORKER_SIZE 4
#define VECTOR_SIZE 32

/* Helper to initialize arrays with patterned data */
void init_array(float *arr, int size, float base) {
    for (int i = 0; i < size; i++) {
        arr[i] = base + i * 0.1f;
    }
}

/* Test 0: gang redundant (no explicit partition modifier) */
void test_gang_redundant(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(local_sum) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
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
    
    #pragma acc parallel copy gang(src[0:n], dest[0:n]) copyout(local_sum) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
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
    
    #pragma acc parallel copy worker(src[0:n], dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
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
    
    #pragma acc parallel copy gang worker(src[0:n], dest[0:n]) copyout(local_sum) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
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
    
    #pragma acc parallel copy vector(src[0:n], dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(1) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 0.166667f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy gang vector(src[0:n], dest[0:n]) copyout(local_sum) \
                         num_gangs(GANG_SIZE) num_workers(1) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 0.142857f;
        }
    }
    
    *reduction = local_sum;
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy worker vector(src[0:n], dest[0:n]) copyout(local_sum) \
                         num_gangs(1) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
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
    
    #pragma acc parallel copy gang worker vector(src[0:n], dest[0:n]) copyout(local_sum) \
                         num_gangs(GANG_SIZE) num_workers(WORKER_SIZE) vector_length(VECTOR_SIZE)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 0.111111f;
        }
    }
    
    *reduction = local_sum;
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float reductions[8] = {0};
    float final_checksum = 0.0f;
    
    /* Initialize source array with patterned data */
    init_array(src, N, 1.0f);
    
    /* Use volatile to prevent compile-time elimination of test calls */
    volatile int run_all_tests = 1;
    
    if (run_all_tests) {
        /* Test all 8 partition codes systematically */
        test_gang_redundant(src, dest, N, &reductions[0]);
        test_gang_partitioned(src, dest, N, &reductions[1]);
        test_worker_partitioned(src, dest, N, &reductions[2]);
        test_gang_worker_partitioned(src, dest, N, &reductions[3]);
        test_vector_partitioned(src, dest, N, &reductions[4]);
        test_gang_vector_partitioned(src, dest, N, &reductions[5]);
        test_worker_vector_partitioned(src, dest, N, &reductions[6]);
        test_fully_partitioned(src, dest, N, &reductions[7]);
    }
    
    /* Compute final checksum from all reductions */
    for (int i = 0; i < 8; i++) {
        final_checksum += reductions[i];
    }
    
    /* Also add some values from dest array to ensure data movement occurred */
    for (int i = 0; i < N; i += 64) {
        final_checksum += dest[i];
    }
    
    /* Print checksum to ensure code has observable side effects */
    printf("Final checksum: %f\n", final_checksum);
    
    /* Cleanup */
    free(src);
    free(dest);
    
    return 0;
}
