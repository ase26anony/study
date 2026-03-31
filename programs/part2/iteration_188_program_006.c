/* 
 * This program is designed to trigger GCC's default_builtin_vectorized_function
 * to create vectorized built-in function declarations with specific flags set.
 * The goal is to cover the flag-setting block in targhooks.cc lines 981-990.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Architecture-specific intrinsics */
#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints for arrays */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence visibility and linkage */
static void math_intensive(float *result, const float *input, int n) 
    __attribute__((visibility("hidden"), used, nothrow));

static inline void memory_ops(char *dst, const char *src, int n) 
    __attribute__((always_inline, used));

static void conditional_vectorization(double *arr, int n) 
    __attribute__((used));

/* Hidden visibility helper - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_helper(double *result, const double *input, int n) {
    /* This function's hidden visibility may interact with vectorization */
    #pragma omp simd aligned(result, input: 32)
    for (int i = 0; i < n; i++) {
        /* Vectorizable built-in calls */
        result[i] = exp(input[i]) + log(fabs(input[i]) + 1.0);
    }
}

/* Math-intensive function with OpenMP SIMD directive */
static void math_intensive(float *result, const float *input, int n) {
    /* Force vectorization of sinf/cosf built-ins */
    #pragma omp simd reduction(+:result[:n]) aligned(result, input: 32)
    for (int i = 0; i < n; i++) {
        result[i] = sinf(input[i]) * cosf(input[i]) + sqrtf(input[i] + 1.0f);
    }
}

/* Memory operations using __builtin_memcpy in vectorizable context */
static inline void memory_ops(char *dst, const char *src, int n) {
    /* Small blocks to encourage vectorized memcpy */
    #pragma GCC ivdep
    for (int i = 0; i < n; i += 64) {
        int chunk = (n - i) < 64 ? (n - i) : 64;
        __builtin_memcpy(dst + i, src + i, chunk);
    }
}

/* Conditional vectorization with CPU feature detection */
static void conditional_vectorization(double *arr, int n) {
    /* Dead code path that still gets analyzed */
    if (0) {
        /* This path should still be analyzed by the vectorizer */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            arr[i] = __builtin_sqrt(arr[i]) + __builtin_exp(arr[i]);
        }
    }
    
    /* Real path with CPU feature detection */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* Use AVX intrinsics - may trigger built-in vectorization */
        #pragma omp simd aligned(arr: 32)
        for (int i = 0; i < n; i += 4) {
            __m256d vec = _mm256_load_pd(arr + i);
            __m256d result = _mm256_sqrt_pd(vec);
            _mm256_store_pd(arr + i, result);
        }
    } else 
    #endif
    {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            arr[i] = sqrt(arr[i]) + pow(arr[i], 1.5);
        }
    }
}

/* Multiple small functions with different built-ins */
static inline void func_with_pow(float *arr, int n) 
    __attribute__((always_inline, used)) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = powf(arr[i], 2.0f);
    }
}

static inline void func_with_exp(float *arr, int n) 
    __attribute__((always_inline, used)) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = expf(arr[i]);
    }
}

static inline void func_with_fabs(float *arr, int n) 
    __attribute__((always_inline, used)) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = fabsf(arr[i]);
    }
}

/* Function with switch statement to analyze multiple paths */
static void multi_path_vectorization(int path, float *arr, int n) {
    switch (path) {
        case 0:
            func_with_pow(arr, n);
            break;
        case 1:
            func_with_exp(arr, n);
            break;
        case 2:
            func_with_fabs(arr, n);
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = sinf(arr[i]) * cosf(arr[i]);
            }
    }
}

/* Mixed data types and type punning */
static void mixed_types_operations(void) {
    union {
        float f[8] ALIGN_32;
        int i[8];
    } data;
    
    /* Initialize with pattern */
    for (int j = 0; j < 8; j++) {
        data.f[j] = (float)j;
    }
    
    /* Type punning and built-in usage */
    #pragma omp simd
    for (int j = 0; j < 8; j++) {
        data.i[j] = __builtin_ilogb(data.f[j]);
    }
}

/* OpenMP parallel region with SIMD reduction */
static double omp_simd_reduction(const double *arr, int n) {
    double sum = 0.0;
    
    #pragma omp parallel for simd reduction(+:sum) aligned(arr: 32)
    for (int i = 0; i < n; i++) {
        sum += exp(arr[i]) * log(fabs(arr[i]) + 1.0);
    }
    
    return sum;
}

/* Simple random number generator to prevent compile-time computation */
static float simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7FFFFFFF) / 2147483647.0f;
}

int main(void) {
    const int N = 1024;
    const int M = 512;
    
    /* Aligned arrays as recommended */
    float float_array[N] ALIGN_32;
    double double_array[N] ALIGN_64;
    char char_array[M] ALIGN_32;
    char char_array2[M] ALIGN_32;
    
    /* Initialize with pattern data */
    for (int i = 0; i < N; i++) {
        float_array[i] = simple_rand(i) * 4.0f - 2.0f; /* Range: -2 to 2 */
        double_array[i] = (double)float_array[i];
    }
    
    for (int i = 0; i < M; i++) {
        char_array[i] = (char)(i % 256);
    }
    
    float result_array[N] ALIGN_32;
    double sum = 0.0;
    
    /* 1. Math-intensive function with sinf/cosf */
    math_intensive(result_array, float_array, N);
    sum += result_array[N-1];
    
    /* 2. Memory operations */
    memory_ops(char_array2, char_array, M);
    sum += char_array2[M-1];
    
    /* 3. Conditional vectorization */
    conditional_vectorization(double_array, N);
    sum += double_array[N-1];
    
    /* 4. Hidden visibility helper */
    hidden_visibility_helper(double_array, double_array, N);
    sum += double_array[N-1];
    
    /* 5. Multiple path vectorization - analyze all paths */
    for (int path = 0; path < 4; path++) {
        multi_path_vectorization(path, float_array, N);
        sum += float_array[N-1];
    }
    
    /* 6. Mixed type operations */
    mixed_types_operations();
    
    /* 7. OpenMP SIMD reduction */
    sum += omp_simd_reduction(double_array, N);
    
    /* 8. strlen in vectorizable context */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < M/64; i++) {
        total_len += __builtin_strlen(char_array + i*64);
    }
    sum += total_len;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", sum);
    
    return 0;
}
