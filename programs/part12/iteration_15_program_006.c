/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force generation of vectorized built-ins */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")))
    __attribute__((optimize("tree-vectorize")))
{
    /* Alignment hints for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (float*)__builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop with multiple vectorizable built-in calls */
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(__builtin_fabsf(x));
        float z = __builtin_sinf(__builtin_expf(y));
        float w = __builtin_cosf(__builtin_logf(z + 1.0f));
        dst[i] = __builtin_powf(w, 0.5f);
        sum += dst[i];
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
    
    /* Double precision vectorizable built-ins */
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        double x = src[i];
        double y = __builtin_sqrt(__builtin_fabs(x));
        double z = __builtin_sin(__builtin_exp(y));
        double w = __builtin_cos(__builtin_log(z + 1.0));
        dst[i] = __builtin_pow(w, 0.5);
        sum += dst[i];
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
    
    /* Mixed operations including type conversions */
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        float x = src[i];
        /* Use ldexp which may have vectorized version */
        float y = __builtin_ldexpf(x, 2);
        float z = __builtin_sinf(__builtin_sqrtf(y));
        /* Use floor/ceil which may vectorize */
        float w = __builtin_floorf(z * 10.0f) * 0.1f;
        dst[i] = __builtin_truncf(w * 100.0f) * 0.01f;
        sum += dst[i];
    }
    
    return sum;
}

/* Function that will be cloned for multiple targets */
__attribute__((target_clones("avx2", "sse4.2", "default")))
static float process_data(float* restrict out, const float* restrict in, int n)
    __attribute__((visibility("hidden")))
{
    float temp[1024] __attribute__((aligned(32)));
    
    /* Multiple stages with different vectorizable operations */
    float sum1 = compute_vector(temp, in, n);
    
    float sum2 = compute_vector_mixed(out, temp, n);
    
    return sum1 + sum2;
}

int main(void) {
    const int N = 1024;
    
    /* Allocate aligned memory */
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
    
    /* Call target-cloned functions to trigger vectorized built-in generation */
    float sum_float = process_data(dst, src, N);
    
    double sum_double = compute_vector_double(dst_double, src_double, N);
    
    /* Compute checksums to prevent dead code elimination */
    float checksum_float = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_float += dst[i];
        checksum_double += dst_double[i];
    }
    
    printf("Float checksum: %f (process_data returned: %f)\n", checksum_float, sum_float);
    printf("Double checksum: %f (compute_vector_double returned: %f)\n", 
           checksum_double, sum_double);
    
    /* Free allocated memory */
    free(src);
    free(dst);
    free(src_double);
    free(dst_double);
    
    return 0;
}
