/* test_openacc_partitions.c
 * 
 * This program systematically exercises OpenACC data partition modes
 * to trigger coverage of the partition code string mapping function
 * in GCC's omp-oacc-neuter-broadcast.cc (lines 335-343).
 *
 * Compile with: gcc -O2 -fopenacc -fdump-tree-omplower -fdump-tree-optimized test_openacc_partitions.c -o test_program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#include <openacc.h>
#endif

#define N 1024
#define CHUNK_SIZE 128

/* Helper to prevent aggressive optimization */
static volatile int use_partition_mode = 0;

/* Test function for partition code 0: gang redundant */
void test_gang_redundant(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copy(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         present_or_copyin(src[0:n]) /* gang redundant by default */
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 2.0f;
            local_sum += src[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for partition code 1: gang partitioned */
void test_gang_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         create gang(dest[0:n])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 3.0f;
            local_sum += src[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for partition code 2: worker partitioned */
void test_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy worker(dest[0:n])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 4.0f;
            local_sum += src[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for partition code 3: gang+worker partitioned */
void test_gang_worker_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy gang worker(dest[0:n])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 5.0f;
            local_sum += src[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for partition code 4: vector partitioned */
void test_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy vector(dest[0:n])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 6.0f;
            local_sum += src[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for partition code 5: gang+vector partitioned */
void test_gang_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy gang vector(dest[0:n])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 7.0f;
            local_sum += src[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for partition code 6: worker+vector partitioned */
void test_worker_vector_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy worker vector(dest[0:n])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 8.0f;
            local_sum += src[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
    }
}

/* Test function for partition code 7: fully partitioned */
void test_fully_partitioned(const float* src, float* dest, int n, float* sum) {
    #pragma acc parallel copyin(src[0:n]) copy(dest[0:n]) copy(sum[0:1]) \
                         copy gang worker vector(dest[0:n])
    {
        float local_sum = 0.0f;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            dest[i] = src[i] * 9.0f;
            local_sum += src[i];
        }
        #pragma acc atomic update
        sum[0] += local_sum;
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
        default: break;
    }
}

int main() {
    float *src = (float*)malloc(N * sizeof(float));
    float *dest = (float*)malloc(N * sizeof(float));
    float sum = 0.0f;
    
    if (!src || !dest) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize source array with patterned data */
    for (int i = 0; i < N; i++) {
        src[i] = (float)(i % 100) * 0.1f;
    }
    
    /* Clear destination and sum */
    memset(dest, 0, N * sizeof(float));
    sum = 0.0f;
    
    /* Test all partition modes systematically */
    for (int mode = 0; mode < 8; mode++) {
        /* Use volatile to prevent compile-time elimination */
        use_partition_mode = mode;
        
        /* Create a fresh destination buffer for each mode */
        float *mode_dest = (float*)malloc(N * sizeof(float));
        float mode_sum = 0.0f;
        
        if (!mode_dest) {
            fprintf(stderr, "Memory allocation failed for mode %d\n", mode);
            continue;
        }
        
        memset(mode_dest, 0, N * sizeof(float));
        
        /* Call wrapper with specific partition mode */
        test_partition_wrapper(use_partition_mode, src, mode_dest, N, &mode_sum);
        
        /* Accumulate results to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            sum += mode_dest[i];
        }
        sum += mode_sum;
        
        /* Print diagnostic info */
        printf("Mode %d: dest[0]=%.2f, sum=%.2f\n", 
               mode, mode_dest[0], mode_sum);
        
        free(mode_dest);
    }
    
    /* Final checksum output */
    printf("Final checksum: %.6f\n", sum);
    
    /* Additional test with kernels construct for broader coverage */
    {
        float ksum = 0.0f;
        float kdest[N];
        
        #pragma acc kernels copyin(src[0:N]) copyout(kdest[0:N]) copy(ksum) \
                            create gang worker vector(kdest[0:N])
        {
            #pragma acc loop gang worker vector reduction(+:ksum)
            for (int i = 0; i < N; i++) {
                kdest[i] = src[i] * 10.0f;
                ksum += src[i];
            }
        }
        
        printf("Kernels checksum: %.6f\n", ksum);
        sum += ksum;
    }
    
    /* Test with nested data regions */
    {
        float ndest[N];
        float nsum = 0.0f;
        
        #pragma acc data copyin(src[0:N]) copyout(ndest[0:N]) copy(nsum)
        {
            #pragma acc parallel present(src[0:N], ndest[0:N], nsum) \
                                 copy gang worker(ndest[0:N])
            {
                #pragma acc loop gang worker reduction(+:nsum)
                for (int i = 0; i < N; i++) {
                    ndest[i] = src[i] * 11.0f;
                    nsum += src[i];
                }
            }
        }
        
        printf("Nested data checksum: %.6f\n", nsum);
        sum += nsum;
    }
    
    free(src);
    free(dest);
    
    printf("Total checksum: %.6f\n", sum);
    
    return 0;
}
