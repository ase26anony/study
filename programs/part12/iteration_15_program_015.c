/* Vectorized built-in test for GCC targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force generation of vectorized built-in declarations */
#ifdef __GNUC__

/* Function with target clones to generate multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((noinline, visibility("hidden")));

/* Hidden visibility to interact with DECL_VISIBILITY logic */
__attribute__((visibility("hidden")))
static float compute_vector(float* dst, const float* src, int n)
{
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    src = __builtin_assume_aligned(src, 32);
    dst = __builtin_assume_aligned(dst, 32);
    
    /* Loop with vectorizable math built-ins */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(x);           /* sqrtf -> __builtin_sqrtf */
        float z = __builtin_sinf(y);            /* sinf -> __builtin_sinf */
        float w = __builtin_expf(z);            /* expf -> __builtin_expf */
        float v = __builtin_fabsf(w);           /* fabsf -> __builtin_fabsf */
        dst[i] = v;
        sum += v;
    }
    
    /* Second loop with powf for additional vectorization opportunity */
    for (int i = 0; i < n; i += 2) {
        dst[i] = __builtin_powf(dst[i], 1.5f);  /* powf -> __builtin_powf */
    }
    
    return sum;
}

/* AVX2-specific version with mixed precision */
__attribute__((target("avx2,fma"), noinline, visibility("hidden")))
static double compute_vector_double(double* dst, const double* src, int n)
{
    double sum = 0.0;
    
    src = __builtin_assume_aligned(src, 32);
    dst = __builtin_assume_aligned(dst, 32);
    
    /* Double precision vectorized built-ins */
    for (int i = 0; i < n; i++) {
        double x = src[i];
        double y = __builtin_sqrt(x);           /* sqrt -> __builtin_sqrt */
        double z = __builtin_sin(y);            /* sin -> __builtin_sin */
        double w = __builtin_cos(z);            /* cos -> __builtin_cos */
        dst[i] = w;
        sum += w;
    }
    
    /* Mix with float operations using conversion */
    for (int i = 0; i < n; i++) {
        float f = (float)dst[i];
        f = __builtin_logf(fabsf(f) + 1.0f);    /* logf -> __builtin_logf */
        dst[i] = (double)f;
    }
    
    return sum;
}

/* SSE4.2 version with different built-in combination */
__attribute__((target("sse4.2"), noinline))
static float compute_vector_sse(float* dst, const float* src, int n)
{
    float sum = 0.0f;
    
    src = __builtin_assume_aligned(src, 16);
    dst = __builtin_assume_aligned(dst, 16);
    
    for (int i = 0; i < n; i++) {
        /* Different built-in combination */
        float x = src[i];
        float y = __builtin_cosf(x);            /* cosf -> __builtin_cosf */
        float z = __builtin_tanf(y);            /* tanf -> __builtin_tanf */
        float w = __builtin_log10f(fabsf(z) + 1.0f); /* log10f -> __builtin_log10f */
        dst[i] = w;
        sum += w;
    }
    
    return sum;
}

#endif /* __GNUC__ */

int main(void)
{
#ifdef __GNUC__
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src_f = aligned_alloc(32, N * sizeof(float));
    float* dst_f = aligned_alloc(32, N * sizeof(float));
    double* src_d = aligned_alloc(32, N * sizeof(double));
    double* dst_d = aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    /* Call target-cloned function (multiple versions generated) */
    float sum_f = compute_vector(dst_f, src_f, N);
    
    /* Call architecture-specific versions */
    double sum_d = compute_vector_double(dst_d, src_d, N);
    float sum_sse = compute_vector_sse(dst_f, src_f, N);
    
    /* Compute checksum to prevent optimization */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst_f[i] + (float)dst_d[i];
    }
    
    printf("Results: sum_f=%.6f, sum_d=%.6f, sum_sse=%.6f, checksum=%.6f\n",
           sum_f, (float)sum_d, sum_sse, checksum);
    
    /* Cleanup */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
#else
    printf("GCC built-ins not available\n");
    return 0;
#endif
}
