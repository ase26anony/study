/* 
 * Program to trigger vectorization of built-in functions and reach
 * the flag-setting block in default_builtin_vectorized_function
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Architecture-specific headers */
#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#include <cpuid.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment attributes for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Simple random generator to prevent compile-time computation */
static inline float simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff;
}

/* Function with hidden visibility attribute */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_helper(double *arr, int n, double *result) {
    double sum = 0.0;
    
    #pragma omp simd reduction(+:sum) aligned(arr:64)
    for (int i = 0; i < n; i++) {
        /* Vectorization of exp and log built-ins */
        sum += exp(arr[i]) * log(arr[i] + 1.0);
    }
    
    *result = sum;
}

/* Static function with math built-ins */
__attribute__((used, nothrow))
static float math_intensive(float *arr, int n) {
    float sum = 0.0f;
    
    /* Explicit SIMD directive for vectorization */
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        /* These should trigger vectorized built-in versions */
        sum += sinf(arr[i]) * cosf(arr[i]) + sqrtf(fabsf(arr[i]));
    }
    
    return sum;
}

/* Function with memory built-ins */
__attribute__((always_inline))
static inline void memory_operations(char *dst, const char *src, int n) {
    /* Loop with __builtin_memcpy - may trigger vectorization */
    for (int i = 0; i < n; i += 64) {
        int size = (n - i) > 64 ? 64 : (n - i);
        __builtin_memcpy(dst + i, src + i, size);
    }
}

/* Function using architecture-specific intrinsics */
__attribute__((target_clones("avx2", "default")))
static double vector_intrinsics(float *arr, int n) {
    double sum = 0.0;
    
    /* Conditional path for AVX2 */
    if (__builtin_cpu_supports("avx2")) {
        #ifdef __x86_64__
        __m256 sum_vec = _mm256_setzero_ps();
        for (int i = 0; i < n; i += 8) {
            __m256 data = _mm256_load_ps(&arr[i]);
            /* Use approximate reciprocal sqrt - may use builtin */
            __m256 rsqrt = _mm256_rsqrt_ps(data);
            sum_vec = _mm256_add_ps(sum_vec, rsqrt);
        }
        /* Horizontal add */
        float temp[8];
        _mm256_store_ps(temp, sum_vec);
        for (int i = 0; i < 8; i++) sum += temp[i];
        #endif
    } else {
        /* Fallback scalar path with built-in calls */
        #pragma GCC ivdep
        for (int i = 0; i < n; i++) {
            sum += sqrt(arr[i]) + pow(arr[i], 1.5);
        }
    }
    
    return sum;
}

/* Multiple small functions with different built-ins */
__attribute__((always_inline))
static inline float func_with_pow(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        sum += powf(arr[i], 2.0f);
    }
    return sum;
}

__attribute__((always_inline))
static inline float func_with_exp(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        sum += expf(arr[i]);
    }
    return sum;
}

__attribute__((always_inline))
static inline float func_with_log(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        sum += logf(arr[i] + 1.0f);
    }
    return sum;
}

/* Function with complex control flow */
static float complex_control_flow(int mode, float *arr, int n) {
    float result = 0.0f;
    
    /* Switch to ensure all paths are analyzed */
    switch (mode) {
        case 0:
            result = func_with_pow(arr, n);
            break;
        case 1:
            result = func_with_exp(arr, n);
            break;
        case 2:
            result = func_with_log(arr, n);
            break;
        default:
            /* Dead code path that still contains vectorizable calls */
            if (0) {  /* Always false, but compiler still parses */
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    result += sinf(arr[i]) * cosf(arr[i]);
                }
            }
            break;
    }
    
    return result;
}

/* Function with mixed data types */
static double mixed_data_types(int n) {
    /* Aligned arrays of different types */
    float float_arr[n] ALIGN_32;
    double double_arr[n] ALIGN_64;
    int int_arr[n] ALIGN_32;
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        float_arr[i] = simple_rand(i);
        double_arr[i] = (double)simple_rand(i + 1000);
        int_arr[i] = i;
    }
    
    double sum = 0.0;
    
    /* Loop with type conversions and built-ins */
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Mixed built-in calls */
        sum += __builtin_sqrt(double_arr[i]) + 
               __builtin_ilogb(float_arr[i]) +
               __builtin_sqrtf(float_arr[i]);
    }
    
    return sum;
}

/* Function using union for type punning */
static void type_punning_operations(float *arr, int n) {
    union {
        float f[4];
        __m128 v;
    } converter ALIGN_16;
    
    #pragma omp simd
    for (int i = 0; i < n; i += 4) {
        /* Copy using memcpy - may trigger builtin vectorization */
        __builtin_memcpy(converter.f, &arr[i], sizeof(float) * 4);
        
        /* Process vector */
        for (int j = 0; j < 4 && (i + j) < n; j++) {
            converter.f[j] = sinf(converter.f[j]);
        }
        
        __builtin_memcpy(&arr[i], converter.f, sizeof(float) * 4);
    }
}

/* OpenMP SIMD declared function */
#pragma omp declare simd uniform(n) linear(i:1)
static float simd_declared_func(float x, int n) {
    float result = x;
    for (int i = 0; i < n; i++) {
        result = sinf(result) + cosf(result);
    }
    return result;
}

/* Main test driver */
int main(int argc, char **argv) {
    const int N = 1024;
    float float_array[N] ALIGN_32;
    double double_array[N] ALIGN_64;
    char char_array[N] ALIGN_32;
    char char_array2[N] ALIGN_32;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        float_array[i] = simple_rand(i);
        double_array[i] = (double)simple_rand(i + 1000);
        char_array[i] = 'A' + (i % 26);
    }
    
    float total = 0.0f;
    
    /* Test 1: Math-intensive function */
    total += math_intensive(float_array, N);
    
    /* Test 2: Memory operations */
    memory_operations(char_array2, char_array, N);
    
    /* Test 3: Check strlen in vectorizable context */
    int len_sum = 0;
    #pragma omp simd reduction(+:len_sum)
    for (int i = 0; i < N/64; i++) {
        len_sum += __builtin_strlen(&char_array[i * 64]);
    }
    total += (float)len_sum;
    
    /* Test 4: Architecture-specific intrinsics */
    total += (float)vector_intrinsics(float_array, N);
    
    /* Test 5: Hidden visibility helper */
    double hidden_result;
    hidden_visibility_helper(double_array, N, &hidden_result);
    total += (float)hidden_result;
    
    /* Test 6: Complex control flow */
    for (int mode = 0; mode < 4; mode++) {
        total += complex_control_flow(mode, float_array, N);
    }
    
    /* Test 7: Mixed data types */
    total += (float)mixed_data_types(N);
    
    /* Test 8: Type punning */
    type_punning_operations(float_array, N);
    
    /* Test 9: OpenMP SIMD declared function */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        float_array[i] = simd_declared_func(float_array[i], 4);
    }
    
    /* Aggregate results to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    return 0;
}
