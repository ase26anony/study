/* Test program to cover partition code string mapping in GCC's OpenACC neuter/broadcast */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define CHUNK_SIZE 128

/* Function prototypes for each partition test */
void test_gang_redundant(const float* src, float* dest, int n, float* sum);
void test_gang_partitioned(const float* src, float* dest, int n, float* sum);
void test_worker_partitioned(const float* src, float* dest, int n, float* sum);
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum);
void test_vector_partitioned(const float* src, float* dest, int n, float* sum);
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum);
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum);
void test_fully_partitioned(const float* src, float* dest, int n, float* sum);

/* Volatile variable to prevent compile-time elimination */
volatile int use_partition_mode = 0;

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += dest[i];
        }
        
        #pragma acc single
        {
            *sum = local_sum;
        }
    }
}

/* Test 1: gang partitioned */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(gang: dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += dest[i];
        }
        
        #pragma acc single
        {
            *sum = local_sum;
        }
    }
}

/* Test 2: worker partitioned */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(worker: dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += dest[i];
        }
        
        *sum = local_sum;
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(gang worker: dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += dest[i];
        }
        
        *sum = local_sum;
    }
}

/* Test 4: vector partitioned */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(vector: dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += dest[i];
        }
        
        *sum = local_sum;
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(gang vector: dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += dest[i];
        }
        
        *sum = local_sum;
    }
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(worker vector: dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += dest[i];
        }
        
        *sum = local_sum;
    }
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    *sum = 0.0f;
    
    #pragma acc parallel copy(src[0:n]) create(gang worker vector: dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += dest[i];
        }
        
        *sum = local_sum;
    }
}

/* Main function that runs all partition tests */
int main() {
    float *src, *dest;
    float sums[8] = {0};
    float total_checksum = 0.0f;
    
    /* Allocate and initialize data */
    src = (float*)malloc(N * sizeof(float));
    dest = (float*)malloc(N * sizeof(float));
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    printf("Starting OpenACC partition tests...\n");
    
    /* Run all partition tests based on volatile variable to prevent optimization */
    if (use_partition_mode == 0 || use_partition_mode == 1) {
        test_gang_redundant(src, dest, N, &sums[0]);
        printf("Test 0 (gang redundant) sum: %f\n", sums[0]);
        total_checksum += sums[0];
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 2) {
        test_gang_partitioned(src, dest, N, &sums[1]);
        printf("Test 1 (gang partitioned) sum: %f\n", sums[1]);
        total_checksum += sums[1];
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 3) {
        test_worker_partitioned(src, dest, N, &sums[2]);
        printf("Test 2 (worker partitioned) sum: %f\n", sums[2]);
        total_checksum += sums[2];
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 4) {
        test_gang_worker_partitioned(src, dest, N, &sums[3]);
        printf("Test 3 (gang+worker partitioned) sum: %f\n", sums[3]);
        total_checksum += sums[3];
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 5) {
        test_vector_partitioned(src, dest, N, &sums[4]);
        printf("Test 4 (vector partitioned) sum: %f\n", sums[4]);
        total_checksum += sums[4];
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 6) {
        test_gang_vector_partitioned(src, dest, N, &sums[5]);
        printf("Test 5 (gang+vector partitioned) sum: %f\n", sums[5]);
        total_checksum += sums[5];
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 7) {
        test_worker_vector_partitioned(src, dest, N, &sums[6]);
        printf("Test 6 (worker+vector partitioned) sum: %f\n", sums[6]);
        total_checksum += sums[6];
    }
    
    if (use_partition_mode == 0 || use_partition_mode == 8) {
        test_fully_partitioned(src, dest, N, &sums[7]);
        printf("Test 7 (fully partitioned) sum: %f\n", sums[7]);
        total_checksum += sums[7];
    }
    
    /* Final checksum to ensure all computations are observable */
    printf("\nTotal checksum: %f\n", total_checksum);
    
    /* Cleanup */
    free(src);
    free(dest);
    
    return 0;
}
