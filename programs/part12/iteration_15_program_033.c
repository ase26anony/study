/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((noinline, visibility("hidden")));

/* Hidden visibility to interact with DECL_VISIBILITY logic */
__attribute__((visibility("hidden")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) {
    /* Assume aligned data for better vectorization */
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    float sum = 0.0f;
    
    /* Loop with vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Sequence of vectorizable math built-ins */
        float x = src[i];
        
        /* sqrt -> potentially vectorized as __builtin_sqrtf */
        float s = __builtin_sqrtf(x);
        
        /* sin -> potentially vectorized as __builtin_sinf */
        float t = __builtin_sinf(s);
        
        /* exp -> potentially vectorized as __builtin_expf */
        float u = __builtin_expf(t);
        
        /* fabs -> potentially vectorized as __builtin_fabsf */
        float v = __builtin_fabsf(u);
        
        /* pow -> potentially vectorized as __builtin_powf */
        dst[i] = __builtin_powf(v, 0.5f);
        
        sum += dst[i];
    }
    
    return sum;
}

/* AVX2-specific version with explicit target attribute */
__attribute__((target("avx2"), noinline, visibility("hidden")))
static float compute_vector_avx2(float* restrict dst, const float* restrict src, int n) {
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    float sum = 0.0f;
    
    /* Different built-in combination for AVX2 */
    for (int i = 0; i < n; i++) {
        float x = src[i];
        
        /* Use log built-in */
        float l = __builtin_logf(x + 1.0f);
        
        /* Use sin built-in */
        float s = __builtin_sinf(l);
        
        /* Use exp built-in */
        dst[i] = __builtin_expf(s);
        
        sum += dst[i];
    }
    
    return sum;
}

/* SSE4.2-specific version with double precision */
__attribute__((target("sse4.2"), noinline))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n) {
    src = (const double*)__builtin_assume_aligned(src, 16);
    dst = (double*)__builtin_assume_aligned(dst, 16);
    
    double sum = 0.0;
    
    /* Double precision built-ins */
    for (int i = 0; i < n; i++) {
        double x = src[i];
        
        /* sqrt on double */
        double s = __builtin_sqrt(x);
        
        /* sin on double */
        double t = __builtin_sin(s);
        
        /* exp on double */
        dst[i] = __builtin_exp(t);
        
        sum += dst[i];
    }
    
    return sum;
}

/* Function with mixed precision operations */
__attribute__((target("avx2,fma"), noinline))
static float compute_mixed(float* restrict dst, const float* restrict src, int n) {
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    float sum = 0.0f;
    
    /* Mix int and float operations */
    for (int i = 0; i < n; i++) {
        float x = src[i];
        
        /* Use ldexp built-in which handles int/float conversion */
        float scaled = __builtin_ldexpf(x, i % 10);
        
        /* sqrt */
        float s = __builtin_sqrtf(scaled);
        
        /* sin */
        dst[i] = __builtin_sinf(s);
        
        sum += dst[i];
    }
    
    return sum;
}

int main() {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src_f = (float*)aligned_alloc(32, N * sizeof(float));
    float* dst_f = (float*)aligned_alloc(32, N * sizeof(float));
    
    double* src_d = (double*)aligned_alloc(32, N * sizeof(double));
    double* dst_d = (double*)aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values to avoid NaNs in sqrt */
    for (int i = 0; i < N; i++) {
        src_f[i] = 0.5f + 0.5f * sinf(i * 0.1f);
        src_d[i] = 0.5 + 0.5 * sin(i * 0.1);
    }
    
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    double sum4 = 0.0;
    
    /* Call target-cloned function (multiple versions generated) */
    sum1 = compute_vector(dst_f, src_f, N);
    
    /* Call AVX2-specific version */
    sum2 = compute_vector_avx2(dst_f, src_f, N);
    
    /* Call mixed precision version */
    sum3 = compute_mixed(dst_f, src_f, N);
    
    /* Call double precision version */
    sum4 = compute_vector_double(dst_d, src_d, N);
    
    /* Compute checksums to prevent dead code elimination */
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_f += dst_f[i];
        checksum_d += dst_d[i];
    }
    
    printf("Results:\n");
    printf("  compute_vector sum: %f\n", sum1);
    printf("  AVX2 version sum: %f\n", sum2);
    printf("  mixed version sum: %f\n", sum3);
    printf("  double version sum: %lf\n", sum4);
    printf("  Float checksum: %f\n", checksum_f);
    printf("  Double checksum: %lf\n", checksum_d);
    
    /* Free memory */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
