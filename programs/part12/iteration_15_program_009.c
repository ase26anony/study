/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */
/* Also try: gcc -O3 -mavx2 -mfma -ftree-vectorize vector_builtins.c -lm -o vector_builtins_avx2 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones - forces generation of multiple versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((visibility("hidden")))
    __attribute__((optimize("tree-vectorize")))
{
    /* Alignment hints for vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (float*)__builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
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
        
        /* Use powf built-in - has vectorized version with constant exponent */
        val = __builtin_powf(val, 2.5f);
        
        dst[i] = val;
        sum += val;
    }
    
    /* Second loop with different built-ins to increase coverage */
    for (int i = 0; i < n/2; i++) {
        /* Use logf built-in */
        dst[i] = __builtin_logf(dst[i] + 1.0f);
        
        /* Use cosf built-in */
        dst[i] = __builtin_cosf(dst[i]);
    }
    
    return sum;
}

/* Another function with explicit target attribute */
__attribute__((target("avx2,fma")))
static double compute_vector_double(double* restrict dst, const double* restrict src, int n)
    __attribute__((visibility("default")))
{
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (double*)__builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    /* Double precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        
        /* Double precision versions */
        val = __builtin_sqrt(val);
        val = __builtin_sin(val);
        val = __builtin_exp(val);
        val = __builtin_fabs(val);
        val = __builtin_pow(val, 1.5);
        
        /* Mix in ldexp for type conversion consideration */
        val = __builtin_ldexp(val, 2);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Function with mixed types to trigger different vectorization paths */
__attribute__((target("arch=core-avx2")))
static void mixed_precision_ops(float* farr, double* darr, int n)
    __attribute__((visibility("hidden")))
{
    farr = (float*)__builtin_assume_aligned(farr, 32);
    darr = (double*)__builtin_assume_aligned(darr, 32);
    
    /* Mix float and double operations */
    for (int i = 0; i < n; i++) {
        /* Float operations */
        float fval = farr[i];
        fval = __builtin_sqrtf(fval);
        fval = __builtin_sinf(fval);
        farr[i] = fval;
        
        /* Double operations */
        double dval = darr[i];
        dval = __builtin_sqrt(dval);
        dval = __builtin_cos(dval);
        darr[i] = dval;
    }
}

/* Main function that exercises all the vectorizable paths */
int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory for better vectorization */
    float* src_f = aligned_alloc(32, N * sizeof(float));
    float* dst_f = aligned_alloc(32, N * sizeof(float));
    double* src_d = aligned_alloc(32, N * sizeof(double));
    double* dst_d = aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values to avoid NaNs in sqrt */
    for (int i = 0; i < N; i++) {
        src_f[i] = 0.5f + 0.5f * sinf(i * 0.1f);
        src_d[i] = 0.5 + 0.5 * sin(i * 0.1);
    }
    
    printf("Testing vectorized built-in functions...\n");
    
    /* Call the target-cloned function - will generate multiple versions */
    float sum_f = compute_vector(dst_f, src_f, N);
    
    /* Call double precision version */
    double sum_d = compute_vector_double(dst_d, src_d, N);
    
    /* Call mixed precision version */
    mixed_precision_ops(dst_f, dst_d, N/2);
    
    /* Compute checksums to prevent dead code elimination */
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    
    for (int i = 0; i < N; i++) {
        checksum_f += dst_f[i];
        checksum_d += dst_d[i];
    }
    
    printf("Float checksum: %f (sum: %f)\n", checksum_f, sum_f);
    printf("Double checksum: %f (sum: %f)\n", checksum_d, sum_d);
    printf("Total: %f\n", checksum_f + checksum_d);
    
    /* Free allocated memory */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
