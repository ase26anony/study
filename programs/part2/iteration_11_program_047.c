/* test_openacc_partitions.c
 * Designed to trigger all data partition mapping codes in GCC's OpenACC compiler
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64

/* Function prototypes for different partition schemes */
void test_gang_redundant(float *src, float *dest, int n, float *sum);
void test_gang_partitioned(float *src, float *dest, int n, float *sum);
void test_worker_partitioned(float *src, float *dest, int n, float *sum);
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum);
void test_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_fully_partitioned(float *src, float *dest, int n, float *sum);

/* Use volatile to prevent compile-time elimination */
volatile int use_all_partitions = 1;

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float total_sum = 0.0f;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Call each test function based on volatile condition */
    if (use_all_partitions) {
        float sum0 = 0.0f;
        test_gang_redundant(src, dest, N, &sum0);
        total_sum += sum0;
        
        float sum1 = 0.0f;
        test_gang_partitioned(src, dest, N, &sum1);
        total_sum += sum1;
        
        float sum2 = 0.0f;
        test_worker_partitioned(src, dest, N, &sum2);
        total_sum += sum2;
        
        float sum3 = 0.0f;
        test_gang_worker_partitioned(src, dest, N, &sum3);
        total_sum += sum3;
        
        float sum4 = 0.0f;
        test_vector_partitioned(src, dest, N, &sum4);
        total_sum += sum4;
        
        float sum5 = 0.0f;
        test_gang_vector_partitioned(src, dest, N, &sum5);
        total_sum += sum5;
        
        float sum6 = 0.0f;
        test_worker_vector_partitioned(src, dest, N, &sum6);
        total_sum += sum6;
        
        float sum7 = 0.0f;
        test_fully_partitioned(src, dest, N, &sum7);
        total_sum += sum7;
    }
    
    /* Compute final checksum to ensure code isn't dead-stripped */
    float final_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_checksum += dest[i];
    }
    final_checksum += total_sum;
    
    printf("Final checksum: %f\n", final_checksum);
    
    free(src);
    free(dest);
    
    return 0;
}

/* Case 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
    }
    
    *sum = local_sum;
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                dest[i] = src[i] * (j + 1);
            }
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M/2; j++) {
                dest[i] += src[i] * j;
            }
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < M/4; j++) {
                dest[i] += src[i] * (j + 2);
            }
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}

/* Case 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < M/8; j++) {
                dest[i] += src[i] * (j + 3);
            }
            local_sum += dest[i];
        }
    }
    
    *sum = local_sum;
}
