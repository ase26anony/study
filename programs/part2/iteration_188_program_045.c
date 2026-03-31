/* 
 * Test program designed to trigger GCC's default_builtin_vectorized_function
 * and specifically execute the flag-setting block in targhooks.cc lines 981-990.
 * Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -ffast-math -fopt-info-vec-all
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <omp.h>

/* Architecture-specific includes */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
static void math_intensive(float *result, const float *input, int n) 
    __attribute__((visibility("hidden"), used, nothrow));

static void memory_copy_operations(char *dest, const char *src, int n) 
    __attribute__((visibility("hidden")));

static inline void conditional_vector_path(double *arr, int n) 
    __attribute__((always_inline));

static void hidden_visibility_helper(double *arr, int n) 
    __attribute__((visibility("hidden"), used));

/* Simple random generator to prevent compile-time computation */
static float simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff;
}

/* 
 * Math-intensive function with explicit SIMD pragma
 * Triggers vectorization of sinf/cosf builtins
 */
static void math_intensive(float *result, const float *input, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple builtin calls to increase vectorization opportunities */
        result[i] = sinf(input[i]) * cosf(input[i]) + sqrtf(fabsf(input[i]));
    }
}

/* 
 * Memory operations with __builtin_memcpy in vectorizable loop
 */
static void memory_copy_operations(char *dest, const char *src, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i += 64) {
        /* Vectorized memcpy */
        __builtin_memcpy(dest + i, src + i, 64);
    }
}

/* 
 * OpenMP declare simd function with builtin calls
 */
#pragma omp declare simd
static float simd_pow_wrapper(float a, float b) {
    return powf(a, b);
}

/* 
 * Conditional architecture-specific paths
 * Both paths should be analyzed by vectorizer
 */
static inline void conditional_vector_path(double *arr, int n) {
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path - compiler may still analyze scalar fallback */
        for (int i = 0; i < n; i += 4) {
            __m256d vec = _mm256_load_pd(arr + i);
            __m256d result = _mm256_sqrt_pd(vec);
            _mm256_store_pd(arr + i, result);
        }
    } else {
        /* Scalar fallback with builtin calls */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            arr[i] = __builtin_sqrt(arr[i]);
        }
    }
}

/* 
 * Hidden visibility helper with multiple builtin types
 */
static void hidden_visibility_helper(double *arr, int n) {
    /* Type punning through union */
    union {
        double d;
        float f[2];
    } converter;
    
    #pragma omp simd reduction(+:converter.d)
    for (int i = 0; i < n; i++) {
        /* Mixed builtin calls */
        arr[i] = exp(arr[i]) + log(fabs(arr[i]) + 1.0);
        
        /* Use ilogb builtin */
        int exp = __builtin_ilogb(arr[i]);
        arr[i] += exp * 0.001;
    }
}

/* 
 * Dead code path that still gets analyzed
 * Contains vectorizable builtin calls
 */
static void dead_code_path(float *arr, int n) {
    if (0) {  /* Dead code, but declarations may still be processed */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            arr[i] = tanhf(arr[i]) * asinf(arr[i] * 0.5f);
        }
    }
}

/* 
 * Multiple small functions with different builtins
 * Called via switch to ensure all are analyzed
 */
static inline void func_with_pow(float *arr, int n) __attribute__((always_inline));
static inline void func_with_exp(float *arr, int n) __attribute__((always_inline));
static inline void func_with_log(float *arr, int n) __attribute__((always_inline));

static inline void func_with_pow(float *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = powf(arr[i], 1.5f);
    }
}

static inline void func_with_exp(float *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = expf(arr[i]);
    }
}

static inline void func_with_log(float *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = logf(fabsf(arr[i]) + 1.0f);
    }
}

static void call_via_switch(float *arr, int n, int choice) {
    switch (choice) {
        case 0: func_with_pow(arr, n); break;
        case 1: func_with_exp(arr, n); break;
        case 2: func_with_log(arr, n); break;
        default: break;
    }
}

/* Main test driver */
int main() {
    const int N = 1024;
    const int M = 4096;
    
    /* Aligned arrays as recommended */
    float float_array[N] ALIGN_32;
    double double_array[N] ALIGN_64;
    char char_src[M] ALIGN_32;
    char char_dest[M] ALIGN_32;
    
    /* Initialize with pattern data */
    for (int i = 0; i < N; i++) {
        float_array[i] = simple_rand(i) * 2.0f - 1.0f;
        double_array[i] = simple_rand(i + N) * 2.0 - 1.0;
    }
    
    for (int i = 0; i < M; i++) {
        char_src[i] = (char)(i % 256);
    }
    
    float result_array[N] ALIGN_32;
    double sum = 0.0;
    
    /* Test 1: Math-intensive with OpenMP SIMD */
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < N; i++) {
        result_array[i] = sinf(float_array[i]) + cosf(float_array[i]);
        sum += result_array[i];
    }
    
    /* Test 2: Memory copy operations */
    memory_copy_operations(char_dest, char_src, M);
    
    /* Test 3: Conditional vector path */
    conditional_vector_path(double_array, N);
    
    /* Test 4: Hidden visibility helper */
    hidden_visibility_helper(double_array, N);
    
    /* Test 5: Call via switch (all paths analyzed) */
    for (int i = 0; i < 3; i++) {
        call_via_switch(float_array, N, i);
    }
    
    /* Test 6: strlen in vectorizable context */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < 100; i++) {
        char test_str[100];
        snprintf(test_str, sizeof(test_str), "test%d", i);
        total_len += __builtin_strlen(test_str);
    }
    
    /* Test 7: Mixed OpenMP parallel and SIMD */
    #pragma omp parallel for
    for (int i = 0; i < N/4; i++) {
        #pragma omp simd
        for (int j = 0; j < 4; j++) {
            int idx = i*4 + j;
            result_array[idx] = simd_pow_wrapper(result_array[idx], 0.5f);
        }
    }
    
    /* Dead code path (still processed) */
    dead_code_path(float_array, N);
    
    /* Aggregate results to prevent elimination */
    double final_sum = sum;
    for (int i = 0; i < N; i++) {
        final_sum += float_array[i] + double_array[i];
    }
    for (int i = 0; i < M; i++) {
        final_sum += char_dest[i];
    }
    
    printf("Result: %f\n", final_sum);
    printf("Total string length: %d\n", total_len);
    
    return 0;
}
