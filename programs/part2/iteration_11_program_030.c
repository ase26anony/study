/* test-omp-oacc-partition-codes.c
 * 
 * This program systematically exercises OpenACC data partition codes 0-7
 * to trigger coverage of the string mapping function in GCC's
 * omp-oacc-neuter-broadcast.cc (lines 335-343).
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -o test test.c
 * Additional flags for offload: -foffload=nvptx-none
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define CHUNK_SIZE 128

/* Helper to initialize data */
void init_data(float *arr, int size, float base) {
    for (int i = 0; i < size; i++) {
        arr[i] = base + (i % 100) * 0.1f;
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
    
    #pragma acc parallel copy(src[0:n]) create(gang: dest[0:n]) copyout(local_sum)
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
    
    #pragma acc parallel copy(src[0:n]) create(worker: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
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
    
    #pragma acc parallel copy(src[0:n]) create(gang worker: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i += CHUNK_SIZE) {
            #pragma acc loop vector
            for (int j = 0; j < CHUNK_SIZE && (i + j) < n; j++) {
                dest[i + j] = src[i + j] * 5.0f;
                local_sum += src[i + j] * 0.2f;
            }
        }
    }
    
    *reduction = local_sum;
}

/* Test 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *reduction) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(vector: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
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
    
    #pragma acc parallel copy(src[0:n]) create(gang vector: dest[0:n]) copyout(local_sum)
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
    
    #pragma acc parallel copy(src[0:n]) create(worker vector: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
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
    
    #pragma acc parallel copy(src[0:n]) create(gang worker vector: dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 0.111f;
        }
    }
    
    *reduction = local_sum;
}

/* Main driver that calls all test functions */
int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float reductions[8] = {0};
    float checksum = 0.0f;
    
    /* Use volatile to prevent compile-time elimination */
    volatile int run_all = 1;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_data(src, N, 1.0f);
    
    printf("Testing OpenACC partition codes 0-7...\n");
    
    /* Call each test function based on volatile condition */
    if (run_all) {
        test_gang_redundant(src, dest, N, &reductions[0]);
        checksum += reductions[0];
        
        test_gang_partitioned(src, dest, N, &reductions[1]);
        checksum += reductions[1];
        
        test_worker_partitioned(src, dest, N, &reductions[2]);
        checksum += reductions[2];
        
        test_gang_worker_partitioned(src, dest, N, &reductions[3]);
        checksum += reductions[3];
        
        test_vector_partitioned(src, dest, N, &reductions[4]);
        checksum += reductions[4];
        
        test_gang_vector_partitioned(src, dest, N, &reductions[5]);
        checksum += reductions[5];
        
        test_worker_vector_partitioned(src, dest, N, &reductions[6]);
        checksum += reductions[6];
        
        test_fully_partitioned(src, dest, N, &reductions[7]);
        checksum += reductions[7];
    }
    
    /* Compute final checksum from destination arrays */
    for (int i = 0; i < N; i++) {
        checksum += dest[i] * 0.001f;
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Partition code tests completed.\n");
    
    /* Cleanup */
    free(src);
    free(dest);
    
    return 0;
}
