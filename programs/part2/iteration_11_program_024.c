/* Test program to cover partition code string mapping in GCC's OpenACC neuter/broadcast */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define CHUNK 128

/* Helper to prevent optimization */
static volatile int use_partition_mode = 0;

/* Test 0: gang redundant (no partition modifier) */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n], dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test 1: gang partitioned */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) create gang(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test 2: worker partitioned */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) create worker(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test 3: gang+worker partitioned */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) create gang worker(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test 4: vector partitioned */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) create vector(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test 5: gang+vector partitioned */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) create gang vector(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test 6: worker+vector partitioned */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) create worker vector(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Test 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) create gang worker vector(dest[0:n]) copyout(sum[0:1])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += dest[i];
        }
        sum[0] = local_sum;
    }
}

/* Wrapper that selects partition mode based on volatile variable */
void test_partition_wrapper(int mode, const float* src, float* dest, int n, float* sum) {
    switch (mode) {
        case 0: test_gang_redundant(src, dest, n, sum); break;
        case 1: test_gang_partitioned(src, dest, n, sum); break;
        case 2: test_worker_partitioned(src, dest, n, sum); break;
        case 3: test_gang_worker_partitioned(src, dest, n, sum); break;
        case 4: test_vector_partitioned(src, dest, n, sum); break;
        case 5: test_gang_vector_partitioned(src, dest, n, sum); break;
        case 6: test_worker_vector_partitioned(src, dest, n, sum); break;
        case 7: test_fully_partitioned(src, dest, n, sum); break;
        default: sum[0] = -1.0f; break;
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sums[8] = {0};
    float total_sum = 0.0f;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < N; i++) {
        src[i] = (i % 13) * 0.5f;
    }
    
    printf("Testing OpenACC partition modes 0-7...\n");
    
    /* Test all 8 partition modes */
    for (int mode = 0; mode < 8; mode++) {
        /* Use volatile to prevent compile-time elimination */
        use_partition_mode = mode;
        
        /* Clear destination */
        memset(dest, 0, N * sizeof(float));
        
        /* Test with current mode */
        test_partition_wrapper(use_partition_mode, src, dest, N, &sums[mode]);
        
        /* Verify computation on host side */
        float host_sum = 0.0f;
        float factor = 2.0f + mode; /* Matches factors used in test functions */
        for (int i = 0; i < N; i++) {
            host_sum += src[i] * factor;
        }
        
        /* Accumulate to final checksum */
        total_sum += sums[mode];
        
        #ifdef _OPENACC
        printf("Mode %d: device sum = %.2f, host sum = %.2f\n", 
               mode, sums[mode], host_sum);
        #else
        printf("Mode %d: (OpenACC not enabled)\n", mode);
        #endif
    }
    
    /* Final checksum output */
    printf("\nTotal checksum: %.2f\n", total_sum);
    
    /* Cleanup */
    free(src);
    free(dest);
    
    return 0;
}
