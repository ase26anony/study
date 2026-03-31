/* Vectorized built-in test for GCC target hooks coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force visibility settings that might interact with DECL_VISIBILITY */
__attribute__((visibility("default")))
float compute_scalar(float x) {
    /* Mix of vectorizable math built-ins */
    float a = __builtin_sinf(x);
    float b = __builtin_cosf(x);
    float c = __builtin_expf(x * 0.1f);
    float d = __builtin_logf(fabsf(x) + 1.0f);
    float e = __builtin_sqrtf(fabsf(x) + 0.5f);
    float f = __builtin_powf(x + 1.0f, 1.5f);
    
    return a + b + c + d + e + f;
}

/* Function with target clones - forces multiple vectorized versions */
__attribute__((target_clones("default", "sse4.2", "avx2", "avx512f")))
float compute_vector(float* __restrict dst, const float* __restrict src, int n) {
    float sum = 0.0f;
    
    /* Alignment hint for vectorizer */
    const float* __restrict aligned_src = __builtin_assume_aligned(src, 32);
    float* __restrict aligned_dst = __builtin_assume_aligned(dst, 32);
    
    /* Loop designed for auto-vectorization with built-ins */
    for (int i = 0; i < n; i++) {
        float x = aligned_src[i];
        
        /* Sequence of vectorizable built-in calls */
        float a = __builtin_sinf(x);
        float b = __builtin_cosf(x);
        float c = __builtin_expf(x * 0.1f);
        float d = __builtin_logf(__builtin_fabsf(x) + 1.0f);
        float e = __builtin_sqrtf(__builtin_fabsf(x) + 0.5f);
        
        /* Combined operation - sqrt(sin^2 + cos^2) should be ~1 */
        float f = __builtin_sqrtf(a * a + b * b);
        
        /* Use pow with constant exponent */
        float g = __builtin_powf(fabsf(x) + 1.0f, 1.5f);
        
        aligned_dst[i] = a + b + c + d + e + f + g;
        sum += aligned_dst[i];
    }
    
    return sum;
}

/* AVX2-specific version with explicit target attribute */
__attribute__((target("avx2")))
float compute_vector_avx2(float* __restrict dst, const float* __restrict src, int n) {
    float sum = 0.0f;
    
    /* Different alignment for AVX2 */
    const float* __restrict aligned_src = __builtin_assume_aligned(src, 32);
    float* __restrict aligned_dst = __builtin_assume_aligned(dst, 32);
    
    for (int i = 0; i < n; i += 8) {  /* 8 floats per AVX2 register */
        for (int j = 0; j < 8 && (i + j) < n; j++) {
            float x = aligned_src[i + j];
            
            /* Different mix of built-ins */
            float a = __builtin_sinf(x);
            float b = __builtin_cosf(x);
            float c = __builtin_expf(__builtin_fabsf(x) * 0.2f);
            float d = __builtin_sqrtf(__builtin_fabsf(x) + 1.0f);
            
            /* Use ldexp for type conversion interest */
            float e = __builtin_ldexpf(__builtin_fabsf(x), -1);
            
            aligned_dst[i + j] = a + b + c + d + e;
            sum += aligned_dst[i + j];
        }
    }
    
    return sum;
}

/* Double precision version for architectures with DP vector support */
__attribute__((target("avx2")))
double compute_vector_double(double* __restrict dst, const double* __restrict src, int n) {
    double sum = 0.0;
    
    const double* __restrict aligned_src = __builtin_assume_aligned(src, 32);
    double* __restrict aligned_dst = __builtin_assume_aligned(dst, 32);
    
    for (int i = 0; i < n; i++) {
        double x = aligned_src[i];
        
        /* Double precision built-ins */
        double a = __builtin_sin(x);
        double b = __builtin_cos(x);
        double c = __builtin_exp(x * 0.1);
        double d = __builtin_log(__builtin_fabs(x) + 1.0);
        double e = __builtin_sqrt(__builtin_fabs(x) + 0.5);
        double f = __builtin_pow(__builtin_fabs(x) + 1.0, 1.5);
        
        aligned_dst[i] = a + b + c + d + e + f;
        sum += aligned_dst[i];
    }
    
    return sum;
}

/* Mixed types to trigger different vectorization patterns */
__attribute__((target("sse4.2")))
float compute_mixed(float* __restrict dst, const float* __restrict src, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        float x = src[i];
        
        /* Mix of regular and built-in calls */
        float a = sinf(x);           /* libm call */
        float b = __builtin_cosf(x); /* built-in call */
        float c = __builtin_expf(x * 0.3f);
        float d = sqrtf(fabsf(x));   /* libm call */
        float e = __builtin_sqrtf(__builtin_fabsf(x) + 1.0f);
        
        dst[i] = a + b + c + d + e;
        sum += dst[i];
    }
    
    return sum;
}

int main() {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate with alignment for vectorization */
    float* src = aligned_alloc(32, N * sizeof(float));
    float* dst = aligned_alloc(32, N * sizeof(float));
    double* src_d = aligned_alloc(32, N * sizeof(double));
    double* dst_d = aligned_alloc(32, N * sizeof(double));
    
    if (!src || !dst || !src_d || !dst_d) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src[i] = 0.01f * i;
        src_d[i] = 0.01 * i;
    }
    
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    double sum_d = 0.0;
    
    /* Call different vectorized versions */
    sum1 = compute_vector(dst, src, N);
    sum2 = compute_vector_avx2(dst, src, N);
    sum3 = compute_mixed(dst, src, N);
    sum_d = compute_vector_double(dst_d, src_d, N);
    
    /* Also call scalar version to ensure all paths are compiled */
    for (int i = 0; i < N; i++) {
        dst[i] += compute_scalar(src[i]);
    }
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += dst[i];
    }
    checksum += sum1 + sum2 + sum3 + (float)sum_d;
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(src_d);
    free(dst_d);
    
    return 0;
}
