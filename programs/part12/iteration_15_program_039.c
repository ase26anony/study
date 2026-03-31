/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec -fno-inline -o vector_builtins vector_builtins.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones - forces generation of multiple versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((noinline))
    __attribute__((visibility("hidden")));

/* Another function with explicit target attribute for AVX2 */
__attribute__((target("avx2,fma")))
static float compute_avx2(float* dst, const float* src, int n)
    __attribute__((noinline));

/* Function for double precision operations */
__attribute__((target("avx2")))
static double compute_double(double* dst, const double* src, int n)
    __attribute__((noinline));

static float compute_vector(float* dst, const float* src, int n)
{
    /* Assume aligned arrays for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop with multiple vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float temp = __builtin_sqrtf(src[i]);
        temp = __builtin_sinf(temp);
        temp = __builtin_expf(temp);
        temp = __builtin_fabsf(temp);
        temp = __builtin_powf(temp, 0.5f);  /* sqrt via pow */
        
        dst[i] = temp;
        sum += temp;
    }
    
    /* Second loop with different built-ins */
    for (int i = 0; i < n; i += 2) {
        float temp = __builtin_cosf(src[i]);
        temp = __builtin_logf(temp + 1.0f);
        dst[i] += temp;
        sum += temp;
    }
    
    return sum;
}

__attribute__((target("avx2,fma")))
static float compute_avx2(float* dst, const float* src, int n)
{
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop designed for AVX2 vectorization (8 floats per vector) */
    for (int i = 0; i < n; i++) {
        /* Use built-ins that have vector equivalents */
        float temp = __builtin_sinf(src[i]);
        temp = __builtin_expf(temp * 0.5f);
        temp = __builtin_fabsf(temp);
        
        dst[i] = temp;
        sum += temp;
    }
    
    return sum;
}

__attribute__((target("avx2")))
static double compute_double(double* dst, const double* src, int n)
{
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (const double*)__builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    /* Double precision operations - will use different vectorized built-ins */
    for (int i = 0; i < n; i++) {
        double temp = __builtin_sqrt(src[i]);
        temp = __builtin_sin(temp);
        temp = __builtin_exp(temp);
        temp = __builtin_fabs(temp);
        
        dst[i] = temp;
        sum += temp;
    }
    
    return sum;
}

/* Function with mixed types and conversions */
__attribute__((target_clones("default", "avx2")))
static float mixed_types(float* dst, const float* src, int n)
    __attribute__((noinline));

static float mixed_types(float* dst, const float* src, int n)
{
    dst = (float*)__builtin_assume_aligned(dst, 16);
    src = (const float*)__builtin_assume_aligned(src, 16);
    
    float sum = 0.0f;
    
    /* Mix float and int operations */
    for (int i = 0; i < n; i++) {
        /* ldexp combines float and int */
        float temp = __builtin_ldexpf(src[i], i % 8);
        temp = __builtin_sinf(temp);
        
        /* Use copysign which has vector equivalent */
        temp = __builtin_copysignf(temp, src[i]);
        
        dst[i] = temp;
        sum += temp;
    }
    
    return sum;
}

int main(void)
{
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate with alignment for vectorization */
    float* src_f = (float*)aligned_alloc(32, N * sizeof(float));
    float* dst_f = (float*)aligned_alloc(32, N * sizeof(float));
    
    double* src_d = (double*)aligned_alloc(32, N * sizeof(double));
    double* dst_d = (double*)aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    double sum_d = 0.0;
    
    /* Call all vectorized functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        sum1 += compute_vector(dst_f, src_f, N);
        sum2 += compute_avx2(dst_f, src_f, N);
        sum3 += mixed_types(dst_f, src_f, N);
        sum_d += compute_double(dst_d, src_d, N);
        
        /* Modify source slightly each iteration */
        for (int i = 0; i < N; i++) {
            src_f[i] += 0.01f;
            src_d[i] += 0.01;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_f += dst_f[i];
        checksum_d += dst_d[i];
    }
    
    printf("Float checksum: %f (sums: %f, %f, %f)\n", 
           checksum_f, sum1, sum2, sum3);
    printf("Double checksum: %lf (sum: %lf)\n", checksum_d, sum_d);
    
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
