/* test_omp_acc_partitions.c
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define VOLATILE_MODE volatile int

/* Function prototypes for different partition schemes */
void test_gang_redundant(float *src, float *dest, int n, float *sum);
void test_gang_partitioned(float *src, float *dest, int n, float *sum);
void test_worker_partitioned(float *src, float *dest, int n, float *sum);
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum);
void test_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_fully_partitioned(float *src, float *dest, int n, float *sum);

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float total_sum = 0.0f;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Use volatile to prevent compile-time elimination */
    VOLATILE_MODE mode = 0;
    
    /* Test all partition modes */
    if (mode == 0) test_gang_redundant(src, dest, N, &total_sum);
    if (mode <= 1) test_gang_partitioned(src, dest, N, &total_sum);
    if (mode <= 2) test_worker_partitioned(src, dest, N, &total_sum);
    if (mode <= 3) test_gang_worker_partitioned(src, dest, N, &total_sum);
    if (mode <= 4) test_vector_partitioned(src, dest, N, &total_sum);
    if (mode <= 5) test_gang_vector_partitioned(src, dest, N, &total_sum);
    if (mode <= 6) test_worker_vector_partitioned(src, dest, N, &total_sum);
    if (mode <= 7) test_fully_partitioned(src, dest, N, &total_sum);
    
    /* Compute checksum to ensure code isn't dead-stripped */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dest[i];
    }
    checksum += total_sum;
    
    printf("Final checksum: %f\n", checksum);
    
    free(src);
    free(dest);
    
    return 0;
}

/* Code 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(local_sum) \
                         reduction(+:local_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
    }
    
    *sum += local_sum;
}

/* Code 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copy(local_sum) \
                         reduction(+:local_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i] * 0.5f;
        }
    }
    
    *sum += local_sum;
}

/* Code 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n]) copy(local_sum) \
                         reduction(+:local_sum)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i] * 0.25f;
        }
    }
    
    *sum += local_sum;
}

/* Code 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel present(src[0:n]) create(dest[0:n]) copy(local_sum) \
                         reduction(+:local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i] * 0.2f;
        }
    }
    
    *sum += local_sum;
}

/* Code 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(local_sum) \
                         reduction(+:local_sum)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i] * 0.1667f;
        }
    }
    
    *sum += local_sum;
}

/* Code 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copyout(dest[0:n]) copy(local_sum) \
                         reduction(+:local_sum)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i] * 0.1429f;
        }
    }
    
    *sum += local_sum;
}

/* Code 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copy(local_sum) \
                         reduction(+:local_sum)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i] * 0.125f;
        }
    }
    
    *sum += local_sum;
}

/* Code 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(dest[0:n]) copy(local_sum) \
                         reduction(+:local_sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i] * 0.1111f;
        }
    }
    
    *sum += local_sum;
}
