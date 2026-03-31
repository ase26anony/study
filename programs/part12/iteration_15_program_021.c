/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec -fno-inline -o vector_builtins vector_builtins.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")))
    __attribute__((optimize("tree-vectorize")))
{
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    /* Loop with multiple vectorizable built-ins */
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float val = src[i];
        
        /* Use built-ins that have vector equivalents */
        val = __builtin_sqrtf(val);           /* sqrtf */
        val = __builtin_sinf(val);            /* sinf */
        val = __builtin_expf(val);            /* expf */
        val = __builtin_fabsf(val);           /* fabsf */
        val = __builtin_logf(val + 1.0f);     /* logf */
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Another function with different target attribute for double precision */
__attribute__((target("avx2,fma")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n)
    __attribute__((visibility("hidden")))
{
    double sum = 0.0;
    
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (const double*)__builtin_assume_aligned(src, 32);
    
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        double val = src[i];
        
        /* Double precision built-ins */
        val = __builtin_sqrt(val);            /* sqrt */
        val = __builtin_sin(val);             /* sin */
        val = __builtin_cos(val);             /* cos */
        val = __builtin_pow(val, 1.5);        /* pow */
        val = __builtin_fabs(val);            /* fabs */
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Function mixing int and float operations */
__attribute__((target_clones("avx2", "sse4.2")))
static void mixed_types(float* restrict dst, const float* restrict src, int n)
    __attribute__((visibility("default")))
{
    dst = (float*)__builtin_assume_aligned(dst, 16);
    src = (const float*)__builtin_assume_aligned(src, 16);
    
    for (int i = 0; i < n; i++) {
        /* Use ldexp which has vector versions */
        int exp = i % 10;
        dst[i] = __builtin_ldexpf(src[i], exp);
    }
}

/* Main test function */
int main(void) {
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
    
    /* Call the target-cloned function (multiple versions will be generated) */
    float sum_f = compute_vector(dst_f, src_f, N);
    
    /* Call double precision version */
    double sum_d = compute_vector_double(dst_d, src_d, N);
    
    /* Call mixed types function */
    mixed_types(dst_f, src_f, N);
    
    /* Compute checksums to prevent dead code elimination */
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_f += dst_f[i];
        checksum_d += dst_d[i];
    }
    
    printf("Float checksum: %f (sum: %f)\n", checksum_f, sum_f);
    printf("Double checksum: %f (sum: %f)\n", checksum_d, sum_d);
    
    /* Cleanup */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
