/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */
/* Alternative: gcc -O3 -mavx2 -mfma -ftree-vectorize vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
__attribute__((visibility("hidden")))
static float compute_vector(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    /* Loop with multiple vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(x);          /* sqrtf has vector equivalent */
        float z = __builtin_sinf(y);           /* sinf has vector equivalent */
        float w = __builtin_expf(z);           /* expf has vector equivalent */
        float v = __builtin_fabsf(w);          /* fabsf has vector equivalent */
        
        /* Use powf which also has vector equivalent */
        dst[i] = __builtin_powf(v, 1.5f);
        sum += dst[i];
    }
    
    return sum;
}

/* AVX2-specific version with explicit target attribute */
__attribute__((target("avx2,fma")))
static double compute_vector_double(double* dst, const double* src, int n) {
    double sum = 0.0;
    
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (const double*)__builtin_assume_aligned(src, 32);
    
    for (int i = 0; i < n; i++) {
        /* Double precision vectorizable built-ins */
        double x = src[i];
        double y = __builtin_sqrt(x);          /* sqrt has vector equivalent */
        double z = __builtin_sin(y);           /* sin has vector equivalent */
        double w = __builtin_cos(z);           /* cos has vector equivalent */
        
        /* Mix with conversion-like operations */
        dst[i] = __builtin_pow(w, 2.0) + __builtin_fabs(z);
        sum += dst[i];
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2")))
static float compute_vector_mixed(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Unaligned version to test different paths */
    for (int i = 0; i < n; i++) {
        /* Complex expression with multiple vectorizable built-ins */
        float x = src[i];
        float a = __builtin_sqrtf(__builtin_fabsf(x));
        float b = __builtin_sinf(a);
        float c = __builtin_cosf(b);
        float d = __builtin_expf(c);
        
        /* Use ldexp which may have vector equivalent */
        dst[i] = __builtin_ldexpf(d, 2);
        sum += dst[i];
    }
    
    return sum;
}

/* Function with ifunc resolver for runtime dispatch */
#ifdef __linux__
static void* resolve_compute(void) {
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx512f")) return compute_vector;
    if (__builtin_cpu_supports("avx2")) return compute_vector;
    return compute_vector_mixed;
}

__attribute__((target_clones("avx2", "avx512f", "sse4.2", "default")))
float compute_multiversion(float* dst, const float* src, int n);
#endif

int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst = aligned_alloc(32, N * sizeof(float));
    double* src_double = aligned_alloc(32, N * sizeof(double));
    double* dst_double = aligned_alloc(32, N * sizeof(double));
    
    if (!src || !dst || !src_double || !dst_double) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);
        src_double[i] = sin(i * 0.1);
    }
    
    float sum1 = 0.0f, sum2 = 0.0f;
    double sum3 = 0.0;
    
    /* Call all vectorized versions to ensure compilation */
    sum1 = compute_vector(dst, src, N);
    sum2 = compute_vector_mixed(dst, src, N);
    sum3 = compute_vector_double(dst_double, src_double, N);
    
    /* Also call through ifunc if available */
    #ifdef __linux__
    float sum4 = compute_multiversion(dst, src, N);
    printf("Multiversion sum: %.6f\n", sum4);
    #endif
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    
    printf("Float checksum: %.6f (sum1: %.6f, sum2: %.6f)\n", 
           checksum, sum1, sum2);
    printf("Double sum: %.6f\n", sum3);
    
    free(src);
    free(dst);
    free(src_double);
    free(dst_double);
    
    return 0;
}
