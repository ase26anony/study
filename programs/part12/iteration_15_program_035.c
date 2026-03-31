/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones to force generation of multiple vectorized versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) 
    __attribute__((noinline, visibility("hidden")));

/* Another function with explicit target attribute for AVX2 */
__attribute__((target("avx2,fma")))
static float compute_avx2(float* restrict dst, const float* restrict src, int n) 
    __attribute__((noinline));

/* Function for double precision operations */
__attribute__((target_clones("default", "avx2", "avx512f")))
static double compute_double_vector(double* restrict dst, const double* restrict src, int n) 
    __attribute__((noinline));

/* Main vector computation function with multiple target clones */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* restrict dst, const float* restrict src, int n) {
    /* Assume aligned pointers for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (float*)__builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop with multiple vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float x = src[i];
        
        /* Use sqrtf built-in - has vectorized version */
        float sqrt_val = __builtin_sqrtf(x);
        
        /* Use sinf built-in - has vectorized version */
        float sin_val = __builtin_sinf(sqrt_val);
        
        /* Use expf built-in - has vectorized version */
        float exp_val = __builtin_expf(sin_val);
        
        /* Use fabsf built-in - has vectorized version */
        float abs_val = __builtin_fabsf(exp_val);
        
        /* Use powf built-in - has vectorized version */
        float pow_val = __builtin_powf(abs_val, 0.5f);
        
        /* Store result */
        dst[i] = pow_val;
        
        /* Accumulate to prevent dead code elimination */
        sum += pow_val;
    }
    
    /* Second loop with different built-in combination */
    for (int i = 0; i < n/2; i++) {
        /* Use logf built-in */
        float log_val = __builtin_logf(dst[i] + 1.0f);
        
        /* Use cosf built-in */
        float cos_val = __builtin_cosf(log_val);
        
        /* Use fmaf built-in if available */
        #ifdef __FP_FAST_FMAF
        dst[i] = __builtin_fmaf(cos_val, 2.0f, dst[i]);
        #else
        dst[i] = cos_val * 2.0f + dst[i];
        #endif
        
        sum += dst[i];
    }
    
    return sum;
}

/* AVX2-specific implementation */
__attribute__((target("avx2,fma")))
static float compute_avx2(float* restrict dst, const float* restrict src, int n) {
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (float*)__builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop designed for AVX2 vectorization (8 floats per vector) */
    for (int i = 0; i < (n & ~7); i++) {
        /* Complex expression with multiple vectorizable built-ins */
        float x = src[i];
        
        /* sqrt -> sin -> exp chain */
        float val = __builtin_expf(__builtin_sinf(__builtin_sqrtf(x)));
        
        /* Add some fabs and pow */
        val = __builtin_powf(__builtin_fabsf(val), 0.333f);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Double precision version */
__attribute__((target_clones("default", "avx2", "avx512f")))
static double compute_double_vector(double* restrict dst, const double* restrict src, int n) {
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (double*)__builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    /* Double precision built-ins */
    for (int i = 0; i < n; i++) {
        double x = src[i];
        
        /* Double precision built-in chain */
        double val = __builtin_sqrt(x);
        val = __builtin_sin(val);
        val = __builtin_exp(val);
        val = __builtin_fabs(val);
        val = __builtin_pow(val, 0.5);
        
        /* Mix with ldexp for type conversion interest */
        val = __builtin_ldexp(val, 1);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Function with mixed types */
__attribute__((target("avx2")))
static float mixed_types_computation(float* restrict out, const float* restrict in, int n) {
    out = (float*)__builtin_assume_aligned(out, 32);
    in = (float*)__builtin_assume_aligned(in, 32);
    
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Mix float and int operations */
        float x = in[i];
        
        /* Use built-in that might convert between types */
        float val = __builtin_sqrtf(x);
        
        /* Use trunc/floor/ceil built-ins */
        val = __builtin_truncf(val * 10.0f) / 10.0f;
        
        /* Use rint/round built-ins */
        val = __builtin_rintf(val * 100.0f) / 100.0f;
        
        out[i] = val;
        sum += val;
    }
    
    return sum;
}

int main(void) {
    const int N = 1024;
    const int ALIGN = 32;
    
    /* Allocate aligned memory */
    float* src_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* dst_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    double* src_d = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* dst_d = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    float checksum = 0.0f;
    double dchecksum = 0.0;
    
    /* Call the target-cloned functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        /* Call the main vector function */
        checksum += compute_vector(dst_f, src_f, N);
        
        /* Call AVX2-specific version */
        checksum += compute_avx2(dst_f, src_f, N);
        
        /* Call double precision version */
        dchecksum += compute_double_vector(dst_d, src_d, N);
        
        /* Call mixed types function */
        checksum += mixed_types_computation(dst_f, src_f, N);
        
        /* Modify source slightly each iteration */
        for (int i = 0; i < N; i++) {
            src_f[i] += 0.01f;
            src_d[i] += 0.01;
        }
    }
    
    /* Compute final checksum from results */
    float final_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_checksum += dst_f[i];
    }
    
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %f\n", dchecksum);
    printf("Final array checksum: %f\n", final_checksum);
    
    /* Clean up */
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
