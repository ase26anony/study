#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((noinline, visibility("hidden")));

/* Hidden visibility to interact with DECL_VISIBILITY logic */
__attribute__((visibility("hidden")))
static float compute_vector(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned for better vectorization */
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    /* Loop 1: Multiple vectorizable float built-ins */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(x);          /* sqrtf vectorization */
        float z = __builtin_sinf(y);           /* sinf vectorization */
        float w = __builtin_expf(z);           /* expf vectorization */
        float v = __builtin_fabsf(w);          /* fabsf vectorization */
        dst[i] = v;
        sum += v;
    }
    
    /* Loop 2: Different pattern to trigger more vectorization */
    for (int i = 0; i < n/2; i++) {
        /* Use powf which may have vectorized version */
        float x = src[i];
        float y = __builtin_powf(x, 2.5f);     /* powf vectorization */
        float z = __builtin_logf(y + 1.0f);    /* logf vectorization */
        dst[i] = z;
        sum += z;
    }
    
    return sum;
}

/* AVX2-specific version with explicit target attribute */
__attribute__((target("avx2"), noinline, visibility("hidden")))
static double compute_vector_double(double* dst, const double* src, int n) {
    double sum = 0.0;
    
    src = (const double*)__builtin_assume_aligned(src, 32);
    dst = (double*)__builtin_assume_aligned(dst, 32);
    
    /* Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double x = src[i];
        double y = __builtin_sqrt(x);          /* sqrt vectorization */
        double z = __builtin_sin(y);           /* sin vectorization */
        double w = __builtin_cos(z);           /* cos vectorization */
        dst[i] = w;
        sum += w;
    }
    
    /* Mixed operations with ldexp */
    for (int i = 0; i < n; i++) {
        double x = src[i];
        double y = __builtin_exp(x);           /* exp vectorization */
        double z = __builtin_ldexp(y, 2);      /* ldexp vectorization */
        dst[i] = z;
        sum += z;
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2"), noinline))
static float compute_vector_sse(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Different built-in combination */
    for (int i = 0; i < n; i++) {
        float x = src[i];
        float y = __builtin_floorf(x);         /* floorf vectorization */
        float z = __builtin_ceilf(y);          /* ceilf vectorization */
        float w = __builtin_truncf(z);         /* truncf vectorization */
        dst[i] = w;
        sum += w;
    }
    
    return sum;
}

int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst = aligned_alloc(32, N * sizeof(float));
    double* src_d = aligned_alloc(32, N * sizeof(double));
    double* dst_d = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    double sum_d = 0.0;
    
    /* Call all vectorized versions */
    sum1 = compute_vector(dst, src, N);
    sum2 = compute_vector_sse(dst, src, N);
    sum_d = compute_vector_double(dst_d, src_d, N);
    
    /* Additional loop in main to trigger more vectorization */
    for (int i = 0; i < N; i++) {
        /* Use built-ins directly in main */
        float x = src[i];
        float y = __builtin_tanf(x);           /* tanf vectorization */
        float z = __builtin_asinf(y * 0.1f);   /* asinf vectorization */
        dst[i] = z;
        sum3 += z;
    }
    
    /* Compute checksum */
    float checksum = sum1 + sum2 + sum3 + (float)sum_d;
    
    /* Prevent dead code elimination */
    volatile float result = checksum;
    printf("Result: %f\n", result);
    
    free(src);
    free(dst);
    free(src_d);
    free(dst_d);
    
    return 0;
}
