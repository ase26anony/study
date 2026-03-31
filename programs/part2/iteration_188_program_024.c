/* 
 * This program is designed to trigger GCC's vectorization of built-in functions,
 * specifically targeting the flag-setting block in default_builtin_vectorized_function
 * in targhooks.cc (lines 981-990).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Architecture-specific headers */
#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#include <cpuid.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* OpenMP for SIMD directives */
#include <omp.h>

/* Alignment hints for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Helper function to generate pseudo-random data (prevents compile-time computation) */
static float random_float(void) {
    static unsigned int seed = 123456789;
    seed = seed * 1103515245 + 12345;
    return (float)(seed % 1000) / 1000.0f;
}

/* Function with hidden visibility attribute - aligns with DECL_VISIBILITY setting */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(float *restrict out, const float *restrict in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Vectorizable built-in calls */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with always_inline attribute containing multiple built-ins */
static inline __attribute__((always_inline)) 
void inline_math_pow(float *restrict out, const float *restrict in, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        out[i] = powf(in[i], 2.5f) + expf(in[i]) + logf(fabsf(in[i]) + 1.0f);
    }
}

/* Function using __builtin_memcpy in a vectorizable context */
__attribute__((used))
static void builtin_memcpy_loop(char *restrict dst, const char *restrict src, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i += 64) {
        /* Vectorizable built-in memory operation */
        __builtin_memcpy(dst + i, src + i, 64);
    }
}

/* Function using __builtin_strlen in a loop - may trigger vectorized strlen */
__attribute__((used))
static int builtin_strlen_sum(const char *strings[], int count) {
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < count; ++i) {
        total += __builtin_strlen(strings[i]);
    }
    return total;
}

/* OpenMP declare simd function - creates SIMD variants */
#pragma omp declare simd
float simd_math_function(float x) {
    return sinf(x) * cosf(x) + sqrtf(x);
}

/* Function with architecture-specific intrinsics and fallback */
__attribute__((noinline))
void architecture_specific_math(float *restrict out, const float *restrict in, int n) {
    /* Conditional chain ensuring compiler analyzes both paths */
    if (__builtin_cpu_supports("avx2")) {
#ifdef __x86_64__
        /* AVX2 intrinsic path */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(in + i);
            __m256 result = _mm256_add_ps(_mm256_sin_ps(vec), _mm256_cos_ps(vec));
            _mm256_store_ps(out + i, result);
        }
#else
        /* Fallback to scalar built-ins */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            out[i] = sinf(in[i]) + cosf(in[i]);
        }
#endif
    } else {
        /* Scalar path with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(in[i]);
        }
    }
}

/* Dead code path that still contains vectorizable built-in calls */
static void dead_code_path(float *out, const float *in, int n) {
    if (0) {  /* Dead code, but compiler still parses it */
        #pragma omp simd
        for (int i = 0; i < n; ++i) {
            out[i] = tanf(in[i]) + asinf(in[i] * 0.5f) + expf(in[i]);
        }
    }
}

/* Mixed data types and type-punning */
__attribute__((used))
static void mixed_type_operations(double *dbl_out, float *flt_out, 
                                  const double *dbl_in, const float *flt_in, int n) {
    union {
        __m128 vec;
        float arr[4];
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Mixed built-in calls with different types */
        dbl_out[i] = __builtin_sqrt(dbl_in[i]) + __builtin_log(dbl_in[i]);
        flt_out[i] = __builtin_sqrtf(flt_in[i]) + __builtin_logf(flt_in[i]);
        
        /* Type punning that might trigger built-in vectorization */
        converter.arr[i % 4] = flt_in[i];
    }
}

/* Main test function with multiple vectorization opportunities */
int main(void) {
    const int N = 1024;
    const int N_ALIGNED = (N + 63) & ~63;  /* Multiple of 64 for alignment */
    
    /* Aligned arrays with initialization */
    float ALIGN_32 array_a[N_ALIGNED];
    float ALIGN_32 array_b[N_ALIGNED];
    float ALIGN_32 array_c[N_ALIGNED];
    double ALIGN_64 array_d[N_ALIGNED];
    char ALIGN_32 buffer1[N_ALIGNED];
    char ALIGN_32 buffer2[N_ALIGNED];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N_ALIGNED; ++i) {
        array_a[i] = random_float() + 0.5f;  /* Ensure positive for sqrt */
        array_d[i] = (double)array_a[i];
        buffer1[i] = (char)(i % 256);
    }
    
    /* String array for strlen testing */
    const char *strings[] = {"test1", "test22", "test333", "test4444"};
    
    float total = 0.0f;
    
    /* 1. Math-intensive function with OpenMP SIMD */
    #pragma omp parallel for simd reduction(+:total)
    for (int i = 0; i < N; ++i) {
        array_b[i] = sinf(array_a[i]) + cosf(array_a[i]);
        total += array_b[i];
    }
    
    /* 2. Hidden visibility helper */
    hidden_visibility_math(array_c, array_a, N);
    
    /* 3. Inline function with multiple built-ins */
    inline_math_pow(array_b, array_a, N);
    
    /* 4. Built-in memcpy in loop */
    builtin_memcpy_loop(buffer2, buffer1, N_ALIGNED);
    
    /* 5. Built-in strlen reduction */
    int str_total = builtin_strlen_sum(strings, 4);
    
    /* 6. Architecture-specific math with fallback */
    architecture_specific_math(array_c, array_a, N);
    
    /* 7. Mixed type operations */
    mixed_type_operations(array_d, array_b, array_d, array_a, N);
    
    /* 8. OpenMP declare simd function usage */
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        array_c[i] += simd_math_function(array_a[i]);
    }
    
    /* 9. Dead code path (still parsed) */
    dead_code_path(array_b, array_a, N);
    
    /* Aggregate results to prevent dead code elimination */
    float final_sum = 0.0f;
    #pragma omp simd reduction(+:final_sum)
    for (int i = 0; i < N; ++i) {
        final_sum += array_b[i] + array_c[i] + (float)array_d[i];
    }
    
    printf("Results: total=%.2f, str_total=%d, final_sum=%.2f\n", 
           total, str_total, final_sum);
    
    return 0;
}
