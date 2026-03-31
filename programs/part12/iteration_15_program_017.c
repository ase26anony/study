/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force generation of vectorized built-ins */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) {
    /* Alignment hints to help vectorization */
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop with vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float val = src[i];
        val = __builtin_sqrtf(val);           /* sqrtf has vector equivalent */
        val = __builtin_sinf(val);            /* sinf has vector equivalent */
        val = __builtin_expf(val);            /* expf has vector equivalent */
        val = __builtin_fabsf(val);           /* fabsf has vector equivalent */
        val = __builtin_powf(val, 1.5f);      /* powf has vector equivalent */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Another function with explicit AVX2 target */
__attribute__((target("avx2")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n) {
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    /* Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        val = __builtin_sqrt(val);            /* sqrt has vector equivalent */
        val = __builtin_sin(val);             /* sin has vector equivalent */
        val = __builtin_exp(val);             /* exp has vector equivalent */
        val = __builtin_fabs(val);            /* fabs has vector equivalent */
        val = __builtin_pow(val, 1.5);        /* pow has vector equivalent */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Function with mixed precision operations */
__attribute__((target_clones("avx2,fma", "sse4.2", "default")))
static float compute_mixed(float* restrict dst, const float* restrict src, int n) {
    dst = __builtin_assume_aligned(dst, 32);
    src = __builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Mixed operations that may trigger different vectorized built-ins */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        /* Use ldexp which has vector equivalent */
        int exp = i % 10;
        val = __builtin_ldexpf(val, exp);
        
        /* Trigonometric functions */
        val = __builtin_cosf(val);
        val = __builtin_tanf(val);
        
        /* Logarithmic function */
        val = __builtin_logf(__builtin_fabsf(val) + 1.0f);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Main function with hidden visibility to interact with DECL_VISIBILITY */
__attribute__((visibility("hidden")))
int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst = aligned_alloc(32, N * sizeof(float));
    double* src_double = aligned_alloc(32, N * sizeof(double));
    double* dst_double = aligned_alloc(32, N * sizeof(double));
    
    if (!src || !dst || !src_double || !dst_double) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with mathematical values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);  /* Use sinf to avoid built-in optimization */
        src_double[i] = sin(i * 0.1);
    }
    
    /* Call target-cloned functions to trigger vectorized built-in generation */
    float sum1 = compute_vector(dst, src, N);
    double sum2 = compute_vector_double(dst_double, src_double, N);
    float sum3 = compute_mixed(dst, src, N);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst[i] + dst_double[i];
    }
    
    printf("Results:\n");
    printf("  Float sum: %f\n", sum1);
    printf("  Double sum: %f\n", sum2);
    printf("  Mixed sum: %f\n", sum3);
    printf("  Checksum: %f\n", checksum);
    
    /* Free memory */
    free(src);
    free(dst);
    free(src_double);
    free(dst_double);
    
    return 0;
}
