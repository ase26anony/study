/* Vectorized built-in test for GCC target hooks coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones - forces multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")))
    __attribute__((optimize("O3", "tree-vectorize")))
{
    /* Alignment hints for vectorization */
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop 1: Multiple vectorizable math built-ins on float */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable built-ins - all have vector equivalents */
        float val = src[i];
        val = __builtin_sqrtf(val + 1.0f);          /* sqrtf */
        val = __builtin_sinf(val);                  /* sinf */
        val = __builtin_expf(val * 0.5f);           /* expf */
        val = __builtin_fabsf(val);                 /* fabsf */
        val = __builtin_logf(val + 1.0f);           /* logf */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* AVX2-specific version with explicit target */
__attribute__((target("avx2,fma")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n)
    __attribute__((visibility("hidden")))
{
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    /* Loop 2: Double precision vectorizable built-ins */
    for (int i = 0; i < n; i += 2) {  /* Step 2 helps vectorization */
        double val1 = src[i];
        double val2 = (i+1 < n) ? src[i+1] : 0.0;
        
        /* Mixed built-ins that should vectorize */
        val1 = __builtin_sqrt(__builtin_fabs(val1) + 1.0);
        val2 = __builtin_sqrt(__builtin_fabs(val2) + 1.0);
        
        val1 = __builtin_sin(val1);
        val2 = __builtin_sin(val2);
        
        /* Power function - often has vectorized version */
        val1 = __builtin_pow(val1, 1.5);
        val2 = __builtin_pow(val2, 1.5);
        
        dst[i] = val1;
        if (i+1 < n) dst[i+1] = val2;
        
        sum += val1 + val2;
    }
    
    return sum;
}

/* SSE4.2 specific version */
__attribute__((target("sse4.2")))
static void compute_mixed(float* restrict dst, const float* restrict src, int n)
{
    /* Type conversion built-ins that may vectorize */
    for (int i = 0; i < n; i++) {
        /* ldexp may have vectorized version on some targets */
        dst[i] = __builtin_ldexpf(src[i], 2);
    }
}

/* Main test driver */
int main(void) {
    const int N = 1024;
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst = aligned_alloc(32, N * sizeof(float));
    double* src_d = aligned_alloc(32, N * sizeof(double));
    double* dst_d = aligned_alloc(32, N * sizeof(double));
    
    if (!src || !dst || !src_d || !dst_d) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    /* Call target-cloned function - will generate multiple versions */
    float sum1 = compute_vector(dst, src, N);
    
    /* Call AVX2-specific double version */
    double sum2 = compute_vector_double(dst_d, src_d, N);
    
    /* Call SSE4.2 version */
    compute_mixed(dst, src, N/2);
    
    /* Additional loop with powf - another vectorizable built-in */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = __builtin_powf(dst[i], 1.1f);
    }
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    double dchecksum = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
        dchecksum += dst_d[i];
    }
    
    printf("Float checksum: %f (from compute_vector: %f)\n", checksum, sum1);
    printf("Double checksum: %f (from compute_vector_double: %f)\n", dchecksum, sum2);
    printf("Total: %f\n", checksum + (float)dchecksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(src_d);
    free(dst_d);
    
    return 0;
}
