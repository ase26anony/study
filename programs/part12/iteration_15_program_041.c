#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force vectorization by using target-specific attributes */
#ifdef __AVX512F__
#define TARGET_ATTR __attribute__((target("avx512f")))
#elif defined(__AVX2__)
#define TARGET_ATTR __attribute__((target("avx2")))
#elif defined(__SSE4_2__)
#define TARGET_ATTR __attribute__((target("sse4.2")))
#else
#define TARGET_ATTR
#endif

/* Function with multiple target clones to increase chances of hitting the hook */
__attribute__((target_clones("default", "sse4.2", "avx2", "avx512f")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((noinline, visibility("hidden")));

/* Hidden visibility to interact with DECL_VISIBILITY logic */
__attribute__((visibility("hidden")))
static float compute_vector(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Provide alignment hints for better vectorization */
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    /* Loop 1: Multiple vectorizable built-ins on float data */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float val = src[i];
        val = __builtin_sqrtf(val);           /* sqrtf has vector equivalent */
        val = __builtin_sinf(val);            /* sinf has vector equivalent */
        val = __builtin_expf(val);            /* expf has vector equivalent */
        val = __builtin_fabsf(val);           /* fabsf has vector equivalent */
        dst[i] = val;
        sum += val;
    }
    
    /* Loop 2: Different built-in combinations */
    for (int i = 0; i < n; i += 2) {
        if (i + 1 < n) {
            /* Use powf which has vector equivalent */
            float val1 = __builtin_powf(src[i], 1.5f);
            float val2 = __builtin_powf(src[i+1], 2.0f);
            dst[i] = val1;
            dst[i+1] = val2;
            sum += val1 + val2;
        }
    }
    
    return sum;
}

/* Separate function for double precision to trigger different vectorization */
__attribute__((target_clones("default", "avx2", "avx512f")))
static double compute_vector_double(double* dst, const double* src, int n)
    __attribute__((noinline));

static double compute_vector_double(double* dst, const double* src, int n) {
    double sum = 0.0;
    
    src = (const double*)__builtin_assume_aligned(src, 32);
    dst = (double*)__builtin_assume_aligned(dst, 32);
    
    /* Double precision built-ins */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        val = __builtin_sqrt(val);            /* sqrt has vector equivalent */
        val = __builtin_sin(val);             /* sin has vector equivalent */
        val = __builtin_exp(val);             /* exp has vector equivalent */
        dst[i] = val;
        sum += val;
    }
    
    /* Mix with ldexp for type conversion built-in */
    for (int i = 0; i < n; i++) {
        int exp = i % 10;
        dst[i] = __builtin_ldexp(dst[i], exp); /* ldexp has vector equivalent */
        sum += dst[i];
    }
    
    return sum;
}

/* Another variant with explicit AVX target */
__attribute__((target("avx2")))
static float compute_vector_avx2(float* dst, const float* src, int n)
    __attribute__((noinline));

static float compute_vector_avx2(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Very simple loop that's highly vectorizable */
    for (int i = 0; i < n; i++) {
        /* Simple chain of built-ins */
        float val = __builtin_sqrtf(__builtin_fabsf(src[i]));
        val = __builtin_sinf(val);
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    float* src_f = (float*)aligned_alloc(32, N * sizeof(float));
    float* dst_f = (float*)aligned_alloc(32, N * sizeof(float));
    double* src_d = (double*)aligned_alloc(32, N * sizeof(double));
    double* dst_d = (double*)aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with mathematical values to avoid NaNs/Infs */
    for (int i = 0; i < N; i++) {
        src_f[i] = 0.1f + 0.01f * (i % 100);  /* Range: 0.1 to 1.09 */
        src_d[i] = 0.1 + 0.01 * (i % 100);
    }
    
    /* Call all vectorized functions to trigger compilation of different versions */
    float sum1 = compute_vector(dst_f, src_f, N);
    float sum2 = compute_vector_avx2(dst_f, src_f, N);
    double sum3 = compute_vector_double(dst_d, src_d, N);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst_f[i] + (float)dst_d[i];
    }
    
    printf("Results: sum1=%.6f, sum2=%.6f, sum3=%.6f, checksum=%.6f\n", 
           sum1, sum2, (float)sum3, checksum);
    
    /* Cleanup */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
