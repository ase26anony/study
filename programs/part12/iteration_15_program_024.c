#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force vectorization by using target-specific attributes */
#ifdef __AVX512F__
#define VEC_TARGET "avx512f"
#elif defined(__AVX2__)
#define VEC_TARGET "avx2"
#elif defined(__SSE4_2__)
#define VEC_TARGET "sse4.2"
#else
#define VEC_TARGET "default"
#endif

/* Function with multiple target clones to increase chance of hitting the hook */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
static float compute_vector(float* dst, const float* src, int n) 
    __attribute__((noinline, visibility("hidden")));

/* Separate function for double precision */
__attribute__((target_clones("default", "avx2", "avx512f")))
static double compute_vector_double(double* dst, const double* src, int n) 
    __attribute__((noinline, visibility("hidden")));

/* Function with explicit target attribute for AVX2 */
__attribute__((target("avx2,fma"), noinline, visibility("hidden")))
static float compute_avx2(float* dst, const float* src, int n) {
    /* Assume aligned for better vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 32);
    src = (const float*)__builtin_assume_aligned(src, 32);
    
    float sum = 0.0f;
    
    /* Loop with multiple vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        /* Chain of vectorizable math built-ins */
        float x = src[i];
        float y = __builtin_sqrtf(__builtin_fabsf(x));
        float z = __builtin_sinf(y);
        float w = __builtin_expf(z * 0.1f);
        dst[i] = __builtin_powf(w, 0.5f);
        sum += dst[i];
    }
    
    return sum;
}

/* Function with explicit target attribute for AVX512 */
__attribute__((target("avx512f"), noinline, visibility("hidden")))
static float compute_avx512(float* dst, const float* src, int n) {
    dst = (float*)__builtin_assume_aligned(dst, 64);
    src = (const float*)__builtin_assume_aligned(src, 64);
    
    float sum = 0.0f;
    
    /* Different loop structure to trigger different vectorization paths */
    for (int i = 0; i < n; i += 2) {
        /* Use multiple vectorizable operations */
        float x1 = __builtin_sinf(src[i]);
        float x2 = __builtin_sinf(src[i + 1]);
        
        float y1 = __builtin_expf(x1);
        float y2 = __builtin_expf(x2);
        
        dst[i] = __builtin_sqrtf(y1);
        dst[i + 1] = __builtin_sqrtf(y2);
        
        sum += dst[i] + dst[i + 1];
    }
    
    return sum;
}

/* Target-cloned function implementation */
static float compute_vector(float* dst, const float* src, int n) {
    /* Alignment hints for vectorization */
    dst = (float*)__builtin_assume_aligned(dst, 16);
    src = (const float*)__builtin_assume_aligned(src, 16);
    
    float sum = 0.0f;
    
    /* Main vectorizable loop with multiple built-ins */
    for (int i = 0; i < n; i++) {
        /* Complex expression using vectorizable built-ins */
        float val = src[i];
        
        /* Sequence of math built-ins that have vector equivalents */
        val = __builtin_fabsf(val);
        val = __builtin_sqrtf(val + 1.0f);
        val = __builtin_sinf(val);
        val = __builtin_expf(val * 0.5f);
        val = __builtin_logf(val + 1.0f);
        
        dst[i] = val;
        sum += val;
    }
    
    /* Second loop with different built-ins */
    for (int i = 0; i < n; i += 4) {
        /* Process multiple elements to encourage vectorization */
        for (int j = 0; j < 4 && (i + j) < n; j++) {
            float x = dst[i + j];
            x = __builtin_cosf(x);
            x = __builtin_powf(x, 2.0f);
            dst[i + j] = x;
        }
    }
    
    return sum;
}

/* Double precision version */
static double compute_vector_double(double* dst, const double* src, int n) {
    dst = (double*)__builtin_assume_aligned(dst, 32);
    src = (const double*)__builtin_assume_aligned(src, 32);
    
    double sum = 0.0;
    
    /* Loop with double-precision vectorizable built-ins */
    for (int i = 0; i < n; i++) {
        double val = src[i];
        
        /* Use double-precision versions */
        val = __builtin_fabs(val);
        val = __builtin_sqrt(val);
        val = __builtin_sin(val);
        val = __builtin_exp(val * 0.25);
        val = __builtin_log(val + 1.0);
        
        dst[i] = val;
        sum += val;
    }
    
    return sum;
}

/* Function using ldexp for type conversion operations */
__attribute__((target("avx2"), noinline, visibility("hidden")))
static float compute_with_conversions(float* dst, const float* src, int n) {
    float sum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Mix float and int operations with ldexp */
        float x = src[i];
        int exp = (int)(x * 10) % 10;
        
        /* Use ldexp which may have vectorized version */
        float y = __builtin_ldexpf(x, exp);
        y = __builtin_sqrtf(y);
        
        dst[i] = y;
        sum += y;
    }
    
    return sum;
}

int main(void) {
    const int N = 1024;  /* Divisible by common vector widths */
    
    /* Allocate aligned memory */
    float* src_f = (float*)aligned_alloc(64, N * sizeof(float));
    float* dst_f = (float*)aligned_alloc(64, N * sizeof(float));
    
    double* src_d = (double*)aligned_alloc(64, N * sizeof(double));
    double* dst_d = (double*)aligned_alloc(64, N * sizeof(double));
    
    /* Initialize with trigonometric values */
    for (int i = 0; i < N; i++) {
        src_f[i] = sinf(i * 0.1f);
        src_d[i] = sin(i * 0.1);
    }
    
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    double sum_d = 0.0;
    
    /* Call target-cloned functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        sum1 += compute_vector(dst_f, src_f, N);
        sum2 += compute_avx2(dst_f, src_f, N);
        sum3 += compute_avx512(dst_f, src_f, N);
        sum_d += compute_vector_double(dst_d, src_d, N);
        
        /* Also call conversion function */
        sum1 += compute_with_conversions(dst_f, src_f, N);
        
        /* Modify source slightly each iteration */
        for (int i = 0; i < N; i++) {
            src_f[i] += 0.01f;
            src_d[i] += 0.01;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    float checksum_f = sum1 + sum2 + sum3;
    double checksum_d = sum_d;
    
    /* Use results to prevent optimization */
    printf("Float checksum: %f\n", checksum_f);
    printf("Double checksum: %f\n", checksum_d);
    
    /* Additional verification */
    float final_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        final_sum += dst_f[i];
    }
    printf("Final array sum: %f\n", final_sum);
    
    free(src_f);
    free(dst_f);
    free(src_d);
    free(dst_d);
    
    return 0;
}
