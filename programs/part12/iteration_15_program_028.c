/* Compile with: gcc -O3 -march=native -ftree-vectorize -fopt-info-vec vector_builtins.c -lm -o vector_builtins */
/* Also try: gcc -O3 -mavx2 -mfma -ftree-vectorize vector_builtins.c -lm -o vector_builtins_avx2 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force hidden visibility to interact with DECL_VISIBILITY logic */
__attribute__((visibility("hidden")))
float compute_basic(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Assume aligned arrays for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    /* Loop with vectorizable built-in calls */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float val = src[i];
        val = __builtin_sqrtf(val);           /* sqrtf vectorization */
        val = __builtin_sinf(val);            /* sinf vectorization */
        val = __builtin_expf(val);            /* expf vectorization */
        val = __builtin_fabsf(val);           /* fabsf vectorization */
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Function with target clones - forces generation of multiple versions */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
float compute_vector(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    /* Alignment hints */
    dst = (float*)__builtin_assume_aligned(dst, 64);
    src = (const float*)__builtin_assume_aligned(src, 64);
    
    /* First loop: float operations with various built-ins */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        /* Multiple vectorizable built-in calls */
        val = __builtin_sinf(val);            /* Trig function */
        val = __builtin_cosf(val);            /* Another trig */
        val = __builtin_logf(val + 1.0f);     /* Log function */
        val = __builtin_powf(val, 0.5f);      /* Power function */
        
        dst[i] = val;
        sum += val;
    }
    
    /* Second loop: different built-in combinations */
    for (int i = 0; i < n; i += 2) {
        float val1 = dst[i];
        float val2 = dst[i + 1];
        
        val1 = __builtin_floorf(val1);        /* Floor function */
        val2 = __builtin_ceilf(val2);         /* Ceil function */
        
        dst[i] = val1;
        dst[i + 1] = val2;
        sum += val1 + val2;
    }
    
    return sum;
}

/* AVX2-specific version using attribute target */
__attribute__((target("avx2,fma")))
float compute_avx2(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    /* Loop designed for AVX2 vectorization (8 floats per vector) */
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        /* Complex chain of built-ins */
        val = __builtin_sqrtf(val);
        val = __builtin_sinf(val * 0.5f);
        val = __builtin_expf(val);
        val = __builtin_fabsf(val);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Mixed precision function with double operations */
__attribute__((target("avx2")))
double compute_mixed(double* d_dst, const double* d_src, float* f_dst, const float* f_src, int n) {
    double sum = 0.0;
    
    d_dst = (double*)__builtin_assume_aligned(d_dst, 32);
    d_src = (const double*)__builtin_assume_aligned(d_src, 32);
    f_dst = (float*)__builtin_assume_aligned(f_dst, 32);
    f_src = (const float*)__builtin_assume_aligned(f_src, 32);
    
    /* Double precision built-ins */
    for (int i = 0; i < n; i++) {
        double dval = d_src[i];
        dval = __builtin_sin(dval);           /* Double precision sin */
        dval = __builtin_sqrt(dval);          /* Double precision sqrt */
        d_dst[i] = dval;
        sum += dval;
    }
    
    /* Float precision built-ins */
    for (int i = 0; i < n; i++) {
        float fval = f_src[i];
        fval = __builtin_sinf(fval);          /* Float precision sin */
        fval = __builtin_sqrtf(fval);         /* Float precision sqrt */
        f_dst[i] = fval;
        sum += fval;
    }
    
    return sum;
}

/* Function using ldexp for type conversion patterns */
__attribute__((target("sse4.2")))
float compute_with_conversions(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        float val = src[i];
        
        /* Use ldexp which may have vectorized versions */
        int exp = i % 10;
        val = __builtin_ldexpf(val, exp);     /* ldexp vectorization */
        
        /* More math built-ins */
        val = __builtin_tanf(val);
        val = __builtin_atanf(val);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

int main() {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src = aligned_alloc(64, N * sizeof(float));
    float* dst1 = aligned_alloc(64, N * sizeof(float));
    float* dst2 = aligned_alloc(64, N * sizeof(float));
    float* dst3 = aligned_alloc(64, N * sizeof(float));
    
    double* d_src = aligned_alloc(64, N * sizeof(double));
    double* d_dst = aligned_alloc(64, N * sizeof(double));
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = sinf(i * 0.1f);
        d_src[i] = sin(i * 0.1);
    }
    
    /* Call all vectorized functions */
    float sum1 = compute_basic(dst1, src, N);
    float sum2 = compute_vector(dst2, src, N);
    float sum3 = compute_avx2(dst3, src, N);
    double sum4 = compute_mixed(d_dst, d_src, dst1, src, N);
    float sum5 = compute_with_conversions(dst2, src, N);
    
    /* Compute checksums to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst1[i] + dst2[i] + dst3[i] + (float)d_dst[i];
    }
    
    printf("Results:\n");
    printf("  compute_basic sum: %f\n", sum1);
    printf("  compute_vector sum: %f\n", sum2);
    printf("  compute_avx2 sum: %f\n", sum3);
    printf("  compute_mixed sum: %f\n", sum4);
    printf("  compute_with_conversions sum: %f\n", sum5);
    printf("  Final checksum: %f\n", checksum);
    
    /* Free memory */
    free(src);
    free(dst1);
    free(dst2);
    free(dst3);
    free(d_src);
    free(d_dst);
    
    return 0;
}
