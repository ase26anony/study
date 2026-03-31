/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force generation of vectorized built-ins */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    src = __builtin_assume_aligned(src, 32);
    dst = __builtin_assume_aligned(dst, 32);
    
    /* Loop with multiple vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float val = src[i];
        
        /* Use sqrtf built-in - has vectorized version */
        val = __builtin_sqrtf(val);
        
        /* Use sinf built-in - has vectorized version */
        val = __builtin_sinf(val);
        
        /* Use expf built-in - has vectorized version */
        val = __builtin_expf(val);
        
        /* Use fabsf built-in - has vectorized version */
        val = __builtin_fabsf(val);
        
        /* Use powf built-in - has vectorized version */
        val = __builtin_powf(val, 1.5f);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* AVX2-specific version with explicit target attribute */
__attribute__((target("avx2,fma"), noinline, visibility("hidden")))
static double compute_vector_double(double* dst, const double* src, int n) {
    double sum = 0.0;
    
    src = __builtin_assume_aligned(src, 32);
    dst = __builtin_assume_aligned(dst, 32);
    
    /* Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        
        /* Double precision versions of built-ins */
        val = __builtin_sqrt(val);
        val = __builtin_sin(val);
        val = __builtin_exp(val);
        val = __builtin_fabs(val);
        val = __builtin_pow(val, 1.5);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2"), noinline))
static float compute_vector_mixed(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    src = __builtin_assume_aligned(src, 16);
    dst = __builtin_assume_aligned(dst, 16);
    
    /* Mix of operations including type conversions */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        /* Use ldexp built-in which may have vectorized version */
        int exp = i % 10;
        val = __builtin_ldexpf(val, exp);
        
        /* More math built-ins */
        val = __builtin_cosf(val);
        val = __builtin_logf(val + 1.0f);
        val = __builtin_floorf(val * 10.0f) / 10.0f;
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Function with multiple loops and conditions to encourage vectorization */
__attribute__((target_clones("avx2", "sse4.2", "default")))
static float compute_complex(float* dst, const float* src, int n) {
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* First loop - simple operations */
    for (int i = 0; i < n; i += 2) {
        float val = src[i];
        val = __builtin_sqrtf(val);
        val = __builtin_sinf(val);
        dst[i] = val;
        sum1 += val;
    }
    
    /* Second loop - different operations */
    for (int i = 1; i < n; i += 2) {
        float val = src[i];
        val = __builtin_expf(val);
        val = __builtin_fabsf(val);
        dst[i] = val;
        sum2 += val;
    }
    
    return sum1 + sum2;
}

int main(void) {
    const int N = 1024;
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst1 = aligned_alloc(32, N * sizeof(float));
    float* dst2 = aligned_alloc(32, N * sizeof(float));
    double* src_double = aligned_alloc(32, N * sizeof(double));
    double* dst_double = aligned_alloc(32, N * sizeof(double));
    
    if (!src || !dst1 || !dst2 || !src_double || !dst_double) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f) + 1.0f;  /* Ensure positive values for sqrt */
        src_double[i] = sin(i * 0.1) + 1.0;
    }
    
    /* Call target-cloned functions to trigger vectorized built-in generation */
    float sum1 = compute_vector(dst1, src, N);
    float sum2 = compute_vector_mixed(dst2, src, N);
    float sum3 = compute_complex(dst1, src, N);
    double sum_double = compute_vector_double(dst_double, src_double, N);
    
    /* Compute checksums to prevent dead code elimination */
    float checksum_float = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_float += dst1[i] + dst2[i];
        checksum_double += dst_double[i];
    }
    
    printf("Float checksum: %f (sum1=%f, sum2=%f, sum3=%f)\n", 
           checksum_float, sum1, sum2, sum3);
    printf("Double checksum: %lf (sum=%lf)\n", checksum_double, sum_double);
    
    /* Free allocated memory */
    free(src);
    free(dst1);
    free(dst2);
    free(src_double);
    free(dst_double);
    
    return 0;
}
