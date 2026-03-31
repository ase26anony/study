/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force generation of multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")))
    __attribute__((optimize("tree-vectorize")))
{
    /* Alignment hints for better vectorization */
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop with multiple vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(__builtin_fabsf(x));
        float z = __builtin_sinf(y);
        float w = __builtin_expf(z * 0.1f);
        dst[i] = w;
        sum += w;
    }
    
    return sum;
}

/* Another function with explicit AVX2 target */
__attribute__((target("avx2,fma")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n)
    __attribute__((visibility("default")))
{
    dst = __builtin_assume_aligned(dst, 64);
    src = __builtin_assume_aligned(src, 64);
    
    double sum = 0.0;
    
    /* Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double x = src[i];
        double y = __builtin_sqrt(__builtin_fabs(x));
        double z = __builtin_sin(y);
        double w = __builtin_pow(z, 1.5);
        dst[i] = w;
        sum += w;
    }
    
    return sum;
}

/* Function with mixed types and conversions */
__attribute__((target("avx512f")))
static float compute_mixed(float* restrict dst, const float* restrict src, int n)
{
    dst = __builtin_assume_aligned(dst, 64);
    src = __builtin_assume_aligned(src, 64);
    
    float sum = 0.0f;
    
    /* Mix float and int operations with ldexp */
    for (int i = 0; i < n; i++) {
        float x = src[i];
        float y = __builtin_sqrtf(x);
        
        /* Use ldexp which may have vectorized version */
        float z = __builtin_ldexpf(y, i & 3);  /* Scale by 0-3 */
        
        /* Trigonometric function */
        float w = __builtin_cosf(z);
        
        dst[i] = w;
        sum += w;
    }
    
    return sum;
}

/* Main function that exercises all variants */
int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src_f = aligned_alloc(64, N * sizeof(float));
    float* dst_f = aligned_alloc(64, N * sizeof(float));
    double* src_d = aligned_alloc(64, N * sizeof(double));
    double* dst_d = aligned_alloc(64, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    /* Call the target-cloned function (multiple versions will be generated) */
    float sum1 = compute_vector(dst_f, src_f, N);
    
    /* Call double precision version */
    double sum2 = compute_vector_double(dst_d, src_d, N);
    
    /* Call mixed version */
    float sum3 = compute_mixed(dst_f, src_f, N);
    
    /* Additional loop in main to ensure vectorization happens */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Use more built-ins in main's loop */
        checksum += __builtin_sinf(dst_f[i]) + __builtin_logf(fabsf(dst_f[i]) + 1.0f);
    }
    
    printf("Results:\n");
    printf("  Float sum: %f\n", sum1);
    printf("  Double sum: %f\n", sum2);
    printf("  Mixed sum: %f\n", sum3);
    printf("  Checksum: %f\n", checksum);
    
    /* Prevent dead code elimination */
    volatile float sink = sum1 + sum2 + sum3 + checksum;
    (void)sink;
    
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
