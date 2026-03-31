/* 
 * Comprehensive test program to trigger GCC's default_builtin_vectorized_function
 * and cover the flag-setting block in targhooks.cc (lines 981-990)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

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

/* Function attributes to influence visibility and linkage */
static inline float test_sin_cos(float x) __attribute__((always_inline));
static void math_intensive(float *arr, int n) __attribute__((visibility("hidden")));
static void memory_operations(void) __attribute__((used, nothrow));

/* Global aligned arrays */
static float arr1[1024] ALIGN_32;
static float arr2[1024] ALIGN_32;
static double darr1[1024] ALIGN_64;
static double darr2[1024] ALIGN_64;
static int iarr1[1024] ALIGN_32;

/* Simple random generator to prevent compile-time computation */
static float simple_rand(int seed) {
    static unsigned int state = 0;
    if (seed) state = seed;
    state = state * 1103515245 + 12345;
    return (float)(state % 1000) / 1000.0f;
}

/* 
 * Function 1: Math-intensive with explicit SIMD pragmas
 * Triggers vectorization of sinf/cosf builtins
 */
__attribute__((visibility("hidden")))
static void math_intensive(float *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple built-in math functions in vectorizable loop */
        arr[i] = sinf(arr[i]) * cosf(arr[i]) + sqrtf(fabsf(arr[i]));
    }
}

/* 
 * Function 2: Memory operations with builtin memcpy
 * Triggers vectorization of memory builtins
 */
__attribute__((used, nothrow))
static void memory_operations(void) {
    char src[256] ALIGN_32;
    char dst[256] ALIGN_32;
    
    /* Initialize with pattern */
    for (int i = 0; i < 256; i++) {
        src[i] = (char)(i % 128);
    }
    
    /* Vectorizable loop with __builtin_memcpy */
    #pragma GCC ivdep
    for (int i = 0; i < 16; i++) {
        __builtin_memcpy(dst + i*16, src + i*16, 16);
    }
    
    /* strlen in loop - another builtin candidate */
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < 16; i++) {
        total += (int)__builtin_strlen(src + i*16);
    }
}

/*
 * Function 3: Architecture-specific intrinsics with fallback
 * Triggers vectorization analysis for both paths
 */
static void conditional_vectorization(float *in, float *out, int n) {
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path - compiler may create vectorized builtin alternatives */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(in + i);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(out + i, result);
        }
    } else 
    #endif
    {
        /* Fallback scalar path with sqrtf calls - should trigger vectorization */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = sqrtf(in[i]);
        }
    }
}

/*
 * Function 4: Multiple small inline functions with different builtins
 * Presented in conditional chain for analysis of all paths
 */
static inline float func_pow(float x) __attribute__((always_inline));
static inline float func_exp(float x) __attribute__((always_inline));
static inline float func_log(float x) __attribute__((always_inline));

static inline float func_pow(float x) {
    return powf(x, 2.5f);
}

static inline float func_exp(float x) {
    return expf(x * 0.5f);
}

static inline float func_log(float x) {
    return logf(x + 1.0f);
}

static void multi_builtin_selector(int mode, float *arr, int n) {
    /* Switch ensures compiler analyzes all paths */
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = func_pow(arr[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = func_exp(arr[i]);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = func_log(arr[i]);
            }
            break;
        default:
            /* Dead code path but still analyzed */
            if (0) {
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    arr[i] = sinf(arr[i]) * cosf(arr[i]);
                }
            }
            break;
    }
}

/*
 * Function 5: Mixed data types and type-punning
 * Uses unions and different builtins
 */
static void mixed_data_types(void) {
    union {
        float f[4];
        int i[4];
    } converter ALIGN_32;
    
    double sum = 0.0;
    
    /* Loop with double builtins */
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 256; i++) {
        sum += __builtin_sqrt(darr1[i]) + __builtin_exp(darr2[i]);
    }
    
    /* Integer builtin */
    #pragma GCC ivdep
    for (int i = 0; i < 256; i++) {
        iarr1[i] = __builtin_ilogb(darr1[i]);
    }
    
    /* Type punning with memcpy */
    for (int i = 0; i < 4; i++) {
        converter.f[i] = (float)i;
    }
    __builtin_memcpy(arr1, converter.f, sizeof(float) * 4);
}

/*
 * Function 6: OpenMP SIMD declared function
 * Creates SIMD variants of functions with builtins
 */
#pragma omp declare simd uniform(n) linear(arr:1)
static void simd_declared_function(float *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = sinf(arr[i]) + cosf(arr[i]);
    }
}

/* Main execution flow */
int main(void) {
    const int n = 1024;
    float result = 0.0f;
    
    /* Initialize arrays with random data */
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        arr1[i] = simple_rand(rand());
        arr2[i] = simple_rand(rand());
        darr1[i] = (double)arr1[i];
        darr2[i] = (double)arr2[i];
    }
    
    printf("Starting vectorization tests...\n");
    
    /* Test 1: Math-intensive function */
    math_intensive(arr1, n);
    
    /* Test 2: Memory operations */
    memory_operations();
    
    /* Test 3: Conditional vectorization */
    conditional_vectorization(arr1, arr2, n);
    
    /* Test 4: Multiple builtin selector */
    for (int mode = 0; mode < 3; mode++) {
        multi_builtin_selector(mode, arr1, n);
    }
    
    /* Test 5: Mixed data types */
    mixed_data_types();
    
    /* Test 6: OpenMP SIMD declared function */
    #pragma omp parallel for simd reduction(+:result)
    for (int i = 0; i < n; i += 64) {
        simd_declared_function(arr1 + i, 64);
        result += arr1[i];
    }
    
    /* Additional complex OpenMP context */
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:result)
        for (int i = 0; i < n; i++) {
            arr2[i] = sqrtf(arr1[i] * arr1[i] + arr2[i] * arr2[i]);
            result += arr2[i];
        }
    }
    
    /* Prevent dead code elimination */
    printf("Final result: %f\n", result);
    
    /* Use results to prevent optimization */
    volatile float sink = result;
    (void)sink;
    
    return 0;
}
