/* Vectorized built-in test for GCC target hooks coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones - forces multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")))
    __attribute__((optimize("O3")))
{
    /* Alignment hints for vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (float*)__builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop 1: Multiple vectorizable math built-ins on float */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable built-ins - each may get vectorized */
        float val = src[i];
        val = __builtin_sqrtf(val);          /* sqrtf vectorization */
        val = __builtin_sinf(val);           /* sinf vectorization */
        val = __builtin_expf(val);           /* expf vectorization */
        val = __builtin_fabsf(val);          /* fabsf vectorization */
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
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (double*)__builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    /* Loop 2: Double precision vectorizable built-ins */
    for (int i = 0; i < n; i += 2) {  /* Step 2 helps vectorization */
        double val = src[i];
        val = __builtin_sqrt(val);     /* sqrt vectorization */
        val = __builtin_sin(val);      /* sin vectorization */
        val = __builtin_cos(val);      /* cos vectorization */
        val = __builtin_pow(val, 2.0); /* pow vectorization */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2")))
static float compute_vector_mixed(float* restrict dst, const float* restrict src, int n)
{
    dst = (float*)__builtin_assume_aligned(dst, 16);
    src = (float*)__builtin_assume_aligned(src, 16);
    
    float sum = 0.0f;
    
    /* Loop 3: Mixed operations including type conversions */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        /* Mix of math and conversion built-ins */
        val = __builtin_logf(val + 1.0f);      /* logf vectorization */
        val = __builtin_floorf(val);           /* floorf vectorization */
        val = __builtin_ldexpf(val, 2);        /* ldexpf vectorization */
        val = __builtin_truncf(val);           /* truncf vectorization */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate with alignment for vectorization */
    float* src_f = (float*)aligned_alloc(32, N * sizeof(float));
    float* dst_f = (float*)aligned_alloc(32, N * sizeof(float));
    double* src_d = (double*)aligned_alloc(32, N * sizeof(double));
    double* dst_d = (double*)aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with mathematical values */
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f) + 1.0f;  /* Ensure positive for sqrt */
        src_d[i] = sin(i * 0.1) + 1.0;
    }
    
    float sum1 = 0.0f, sum2 = 0.0f;
    double sum3 = 0.0;
    
    /* Call all vectorized versions */
    sum1 = compute_vector(dst_f, src_f, N);
    sum2 = compute_vector_mixed(dst_f, src_f, N);
    sum3 = compute_vector_double(dst_d, src_d, N);
    
    /* Compute checksum to prevent optimization */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst_f[i];
        if (i % 4 == 0) checksum += (float)dst_d[i];
    }
    
    checksum += sum1 + sum2 + (float)sum3;
    
    printf("Checksum: %f\n", checksum);
    printf("Results: sum1=%f, sum2=%f, sum3=%f\n", sum1, sum2, sum3);
    
    /* Cleanup */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
