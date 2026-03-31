/* test_openacc_partitions.c - Coverage for omp-oacc-neuter-broadcast.cc partition codes */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define M 32

/* Function prototypes for each partition scheme */
void test_gang_redundant(float *src, float *dest, int n, float *sum);
void test_gang_partitioned(float *src, float *dest, int n, float *sum);
void test_worker_partitioned(float *src, float *dest, int n, float *sum);
void test_gang_worker_partitioned(float *src, float *dest, int n, float *sum);
void test_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_gang_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_worker_vector_partitioned(float *src, float *dest, int n, float *sum);
void test_fully_partitioned(float *src, float *dest, int n, float *sum);

/* Use volatile to prevent compile-time elimination */
volatile int use_partition = 0;

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float total_sum = 0.0f;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Test each partition scheme */
    printf("Testing OpenACC partition schemes...\n");
    
    /* Case 0: gang redundant */
    use_partition = 0;
    test_gang_redundant(src, dest, N, &total_sum);
    
    /* Case 1: gang partitioned */
    use_partition = 1;
    test_gang_partitioned(src, dest, N, &total_sum);
    
    /* Case 2: worker partitioned */
    use_partition = 2;
    test_worker_partitioned(src, dest, N, &total_sum);
    
    /* Case 3: gang+worker partitioned */
    use_partition = 3;
    test_gang_worker_partitioned(src, dest, N, &total_sum);
    
    /* Case 4: vector partitioned */
    use_partition = 4;
    test_vector_partitioned(src, dest, N, &total_sum);
    
    /* Case 5: gang+vector partitioned */
    use_partition = 5;
    test_gang_vector_partitioned(src, dest, N, &total_sum);
    
    /* Case 6: worker+vector partitioned */
    use_partition = 6;
    test_worker_vector_partitioned(src, dest, N, &total_sum);
    
    /* Case 7: fully partitioned */
    use_partition = 7;
    test_fully_partitioned(src, dest, N, &total_sum);
    
    /* Compute final checksum */
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

/* Case 0: gang redundant */
void test_gang_redundant(float *src, float *dest, int n, float *total_sum) {
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
    
    *total_sum += local_sum;
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *src, float *dest, int n, float *total_sum) {
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
    
    *total_sum += local_sum;
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *src, float *dest, int n, float *total_sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(local_sum) \
        reduction(+:local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            if (i % 2 == 0) {
                dest[i] = src[i] * 4.0f;
            }
            local_sum += src[i] * 0.25f;
        }
    }
    
    *total_sum += local_sum;
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *src, float *dest, int n, float *total_sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(local_sum) \
        reduction(+:local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * (float)(i % 10);
            local_sum += src[i];
        }
    }
    
    *total_sum += local_sum;
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *src, float *dest, int n, float *total_sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(local_sum) \
        reduction(+:local_sum)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * src[i];
            local_sum += src[i] * 0.1f;
        }
    }
    
    *total_sum += local_sum;
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *src, float *dest, int n, float *total_sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(local_sum) \
        reduction(+:local_sum)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            if (i < n/2) {
                dest[i] = src[i] * 5.0f;
            } else {
                dest[i] = src[i] * 6.0f;
            }
            local_sum += src[i] * 0.75f;
        }
    }
    
    *total_sum += local_sum;
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *src, float *dest, int n, float *total_sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(local_sum) \
        reduction(+:local_sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] + (float)(i % 20);
            local_sum += src[i] * 0.33f;
        }
    }
    
    *total_sum += local_sum;
}

/* Case 7: fully partitioned */
void test_fully_partitioned(float *src, float *dest, int n, float *total_sum) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(local_sum) \
        reduction(+:local_sum)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            float temp = src[i];
            for (int j = 0; j < 4; j++) {
                temp *= 1.1f;
            }
            dest[i] = temp;
            local_sum += src[i] * 0.9f;
        }
    }
    
    *total_sum += local_sum;
}
