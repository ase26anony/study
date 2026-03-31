/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec -fno-inline -fdump-tree-vect -o vector_builtins vector_builtins.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SIZE 1024
#define ALIGNMENT 64

/* Function with target clones to force generation of multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")));

/* AVX2-specific version */
__attribute__((target("avx2,fma")))
static float compute_vector_avx2(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, ALIGNMENT);
    src = (float*)__builtin_assume_aligned(src, ALIGNMENT);
    
    /* Loop with multiple vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(__builtin_fabsf(x));
        float z = __builtin_sinf(y);
        float w = __builtin_expf(z * 0.5f);
        dst[i] = __builtin_powf(w, 1.5f);
        sum += dst[i];
    }
    
    /* Second loop with different operations */
    for (int i = 0; i < n/2; i++) {
        dst[i] = __builtin_cosf(__builtin_logf(src[i] + 1.0f));
    }
    
    return sum;
}

/* SSE4.2-specific version */
__attribute__((target("sse4.2")))
static float compute_vector_sse(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    dst = (float*)__builtin_assume_aligned(dst, 16);
    src = (float*)__builtin_assume_aligned(src, 16);
    
    for (int i = 0; i < n; i++) {
        /* Different combination of built-ins */
        float x = __builtin_fabsf(src[i]);
        float y = __builtin_sinf(__builtin_sqrtf(x));
        dst[i] = __builtin_expf(y * 0.3f);
        sum += dst[i];
    }
    
    return sum;
}

/* Default version */
static float compute_vector_default(float* restrict dst, const float* restrict src, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        float x = src[i];
        float y = __builtin_sqrtf(x * x);  /* Equivalent to fabs */
        dst[i] = __builtin_sinf(y);
        sum += dst[i];
    }
    
    return sum;
}

/* Main target-cloned function */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) {
    /* This will be cloned for each target */
    #ifdef __AVX512F__
    return compute_vector_avx2(dst, src, n);
    #elif defined(__AVX2__)
    return compute_vector_avx2(dst, src, n);
    #elif defined(__SSE4_2__)
    return compute_vector_sse(dst, src, n);
    #else
    return compute_vector_default(dst, src, n);
    #endif
}

/* Double precision version for architectures with double vector support */
__attribute__((target("avx2")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n) {
    double sum = 0.0;
    
    dst = (double*)__builtin_assume_aligned(dst, ALIGNMENT);
    src = (double*)__builtin_assume_aligned(src, ALIGNMENT);
    
    for (int i = 0; i < n; i++) {
        /* Double precision built-ins */
        double x = __builtin_sqrt(__builtin_fabs(src[i]));
        double y = __builtin_sin(x);
        double z = __builtin_exp(y);
        dst[i] = __builtin_pow(z, 1.5);
        sum += dst[i];
    }
    
    /* Mixed precision operations */
    for (int i = 0; i < n/4; i++) {
        /* ldexp is vectorizable on some architectures */
        dst[i] = __builtin_ldexp(src[i], 2);
    }
    
    return sum;
}

int main() {
    /* Aligned memory allocation */
    float* src_f = (float*)aligned_alloc(ALIGNMENT, SIZE * sizeof(float));
    float* dst_f = (float*)aligned_alloc(ALIGNMENT, SIZE * sizeof(float));
    
    double* src_d = (double*)aligned_alloc(ALIGNMENT, SIZE/2 * sizeof(double));
    double* dst_d = (double*)aligned_alloc(ALIGNMENT, SIZE/2 * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with trigonometric values */
    for (int i = 0; i < SIZE; i++) {
        src_f[i] = sinf(i * 0.1f);
    }
    
    for (int i = 0; i < SIZE/2; i++) {
        src_d[i] = sin(i * 0.1);
    }
    
    /* Call the target-cloned function multiple times */
    float sum_f = 0.0f;
    for (int iter = 0; iter < 10; iter++) {
        sum_f += compute_vector(dst_f, src_f, SIZE);
        
        /* Modify input slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            src_f[i] += 0.01f;
        }
    }
    
    /* Call double precision version if AVX2 is available */
    #ifdef __AVX2__
    double sum_d = compute_vector_double(dst_d, src_d, SIZE/2);
    printf("Double sum: %f\n", sum_d);
    #endif
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        checksum += dst_f[i];
    }
    
    printf("Float checksum: %f\n", checksum);
    printf("Accumulated sum: %f\n", sum_f);
    
    /* Cleanup */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
