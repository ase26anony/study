/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */
/* Alternative: gcc -O3 -mavx2 -mfma -ftree-vectorize vector_builtins.c -lm -o vector_builtins */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Function with target clones - forces generation of multiple versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((noinline))
    __attribute__((visibility("hidden")));  /* Interact with DECL_VISIBILITY */

/* Another function with explicit target attribute */
__attribute__((target("avx2,fma")))
static float compute_vector_avx2(float* dst, const float* src, int n)
    __attribute__((noinline));

/* Double precision version for architectures with DP vector support */
__attribute__((target("avx2")))
static double compute_vector_double(double* dst, const double* src, int n)
    __attribute__((noinline));

/* Main vector computation with multiple built-ins */
__attribute__((target_clones("default", "avx2", "avx512f")))
static float compute_vector(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned data for better vectorization */
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    /* Loop with multiple vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float val = src[i];
        
        /* Use __builtin_sqrtf which has vectorized versions */
        val = __builtin_sqrtf(val);
        
        /* Use __builtin_sinf - many targets have vectorized versions */
        val = __builtin_sinf(val);
        
        /* Use __builtin_expf - vectorized in some math libraries */
        val = __builtin_expf(val);
        
        /* Use __builtin_fabsf - definitely vectorizable */
        val = __builtin_fabsf(val);
        
        /* Use __builtin_powf with constant exponent */
        val = __builtin_powf(val, 2.0f);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* AVX2-specific implementation */
__attribute__((target("avx2,fma")))
static float compute_vector_avx2(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    src = (const float*)__builtin_assume_aligned(src, 32);
    dst = (float*)__builtin_assume_aligned(dst, 32);
    
    /* Different built-in combination for this target */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        /* Use __builtin_logf */
        val = __builtin_logf(val + 1.0f);
        
        /* Use __builtin_cosf */
        val = __builtin_cosf(val);
        
        /* Use __builtin_ldexpf for type conversion interest */
        val = __builtin_ldexpf(val, 1);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Double precision version */
__attribute__((target("avx2")))
static double compute_vector_double(double* dst, const double* src, int n) {
    double sum = 0.0;
    
    src = (const double*)__builtin_assume_aligned(src, 32);
    dst = (double*)__builtin_assume_aligned(dst, 32);
    
    /* Double precision built-ins */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        
        /* Scalar double built-ins that have vector equivalents */
        val = __builtin_sqrt(val);
        val = __builtin_sin(val);
        val = __builtin_exp(val);
        val = __builtin_fabs(val);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Another function with mixed operations */
__attribute__((target_clones("default", "avx2")))
static void mixed_vector_ops(float* out, const float* in1, const float* in2, int n) {
    in1 = (const float*)__builtin_assume_aligned(in1, 32);
    in2 = (const float*)__builtin_assume_aligned(in2, 32);
    out = (float*)__builtin_assume_aligned(out, 32);
    
    for (int i = 0; i < n; i++) {
        /* Mix different built-ins */
        float a = __builtin_sqrtf(in1[i]);
        float b = __builtin_sinf(in2[i]);
        
        /* Use __builtin_powf with non-constant exponent */
        out[i] = __builtin_powf(a, b);
    }
}

int main() {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src_f = aligned_alloc(32, N * sizeof(float));
    float* dst_f = aligned_alloc(32, N * sizeof(float));
    float* dst_f2 = aligned_alloc(32, N * sizeof(float));
    
    double* src_d = aligned_alloc(32, N * sizeof(double));
    double* dst_d = aligned_alloc(32, N * sizeof(double));
    
    if (!src_f || !dst_f || !dst_f2 || !src_d || !dst_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with mathematical values */
    for (int i = 0; i < N; i++) {
        src_f[i] = 0.1f * i + 0.5f;  /* Range that avoids domain errors */
        src_d[i] = 0.1 * i + 0.5;
    }
    
    /* Call target-cloned functions to trigger vector built-in generation */
    float sum1 = compute_vector(dst_f, src_f, N);
    float sum2 = compute_vector_avx2(dst_f2, src_f, N);
    double sum3 = compute_vector_double(dst_d, src_d, N);
    
    /* Call mixed operations function */
    mixed_vector_ops(dst_f, src_f, dst_f2, N);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst_f[i] + dst_f2[i] + (float)dst_d[i];
    }
    
    printf("Results: sum1=%.6f, sum2=%.6f, sum3=%.6f, checksum=%.6f\n", 
           sum1, sum2, (float)sum3, checksum);
    
    /* Clean up */
    free(src_f);
    free(dst_f);
    free(dst_f2);
    free(src_d);
    free(dst_d);
    
    return 0;
}
