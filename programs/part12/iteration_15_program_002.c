/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")));

/* Hidden visibility to interact with DECL_VISIBILITY logic */
__attribute__((visibility("hidden")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    /* Loop with vectorizable math built-ins */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable built-ins */
        float val = src[i];
        
        /* Use multiple GCC math built-ins with vector equivalents */
        float sqrt_val = __builtin_sqrtf(val);
        float sin_val = __builtin_sinf(sqrt_val);
        float exp_val = __builtin_expf(sin_val);
        float fabs_val = __builtin_fabsf(exp_val);
        
        /* Power function with vector equivalent */
        float pow_val = __builtin_powf(fabs_val, 1.5f);
        
        dst[i] = pow_val;
        sum += pow_val;
    }
    
    return sum;
}

/* AVX2-specific version with explicit target attribute */
__attribute__((target("avx2")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n) {
    double sum = 0.0;
    
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (const double*)__builtin_assume_aligned(src, 32);
    
    /* Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        
        /* Double precision vector built-ins */
        double sqrt_val = __builtin_sqrt(val);
        double sin_val = __builtin_sin(sqrt_val);
        double exp_val = __builtin_exp(sin_val);
        double fabs_val = __builtin_fabs(exp_val);
        
        /* Mix with conversion built-in */
        double ldexp_val = __builtin_ldexp(fabs_val, 1);
        
        dst[i] = ldexp_val;
        sum += ldexp_val;
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2")))
static float compute_vector_mixed(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    dst = (float*)__builtin_assume_aligned(dst, 16);
    src = (const float*)__builtin_assume_aligned(src, 16);
    
    /* Mixed operations to trigger different vectorization paths */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        /* Trigonometric functions with vector equivalents */
        float cos_val = __builtin_cosf(val);
        float tan_val = __builtin_tanf(cos_val);
        float log_val = __builtin_logf(tan_val + 1.0f);
        
        dst[i] = log_val;
        sum += log_val;
    }
    
    return sum;
}

/* Function with multiple vectorizable built-in patterns */
__attribute__((target_clones("avx2,fma", "sse4.2", "default")))
static void process_arrays(float* restrict out1, float* restrict out2, 
                          const float* restrict in, int n) {
    /* Two independent loops with different built-ins */
    for (int i = 0; i < n; i++) {
        /* First output: sqrt -> sin -> exp chain */
        float val = in[i];
        out1[i] = __builtin_expf(__builtin_sinf(__builtin_sqrtf(val)));
    }
    
    for (int i = 0; i < n; i++) {
        /* Second output: different chain to trigger different vectorization */
        float val = in[i];
        out2[i] = __builtin_powf(__builtin_fabsf(val), 0.5f);
    }
}

int main() {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst1 = aligned_alloc(32, N * sizeof(float));
    float* dst2 = aligned_alloc(32, N * sizeof(float));
    double* src_double = aligned_alloc(32, N * sizeof(double));
    double* dst_double = aligned_alloc(32, N * sizeof(double));
    
    if (!src || !dst1 || !dst2 || !src_double || !dst_double) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with mathematical values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);  /* Range that avoids domain errors */
        src_double[i] = sin(i * 0.1);
    }
    
    /* Call target-cloned functions to trigger vectorized built-in declarations */
    float sum1 = compute_vector(dst1, src, N);
    double sum2 = compute_vector_double(dst_double, src_double, N);
    float sum3 = compute_vector_mixed(dst2, src, N);
    
    /* Process arrays with multiversioned function */
    process_arrays(dst1, dst2, src, N);
    
    /* Compute checksums to prevent dead code elimination */
    float checksum = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum += dst1[i] + dst2[i];
        checksum_double += dst_double[i];
    }
    
    /* Use results to prevent optimization */
    printf("Checksums: float=%f double=%lf\n", checksum + sum1 + sum3, checksum_double + sum2);
    printf("Sample values: dst1[0]=%f, dst_double[0]=%lf\n", dst1[0], dst_double[0]);
    
    /* Free memory */
    free(src);
    free(dst1);
    free(dst2);
    free(src_double);
    free(dst_double);
    
    return 0;
}
