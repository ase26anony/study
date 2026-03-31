/* 
 * This program is designed to trigger GCC's default_builtin_vectorized_function
 * hook, specifically the flag-setting block for vectorized built-in declarations.
 * It combines multiple techniques: math function vectorization, memory builtins,
 * architecture-specific intrinsics, visibility attributes, and OpenMP SIMD.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#include <cpuid.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to interact with declaration handling */
static inline float math_func_1(float x) __attribute__((always_inline));
static inline float math_func_2(float x) __attribute__((always_inline));
static void hidden_visibility_func(double *arr, int n) __attribute__((visibility("hidden"), used, nothrow));

/* Global arrays with alignment guarantees */
static float farr1[1024] ALIGN_32;
static float farr2[1024] ALIGN_32;
static double darr1[1024] ALIGN_64;
static double darr2[1024] ALIGN_64;
static char src_buf[4096] ALIGN_32;
static char dst_buf[4096] ALIGN_32;

/* Simple random initialization to prevent compile-time computation */
static void init_data(void) {
    srand(time(NULL));
    for (int i = 0; i < 1024; i++) {
        farr1[i] = (float)rand() / RAND_MAX * 6.28318530718f; /* 0 to 2π */
        farr2[i] = (float)rand() / RAND_MAX * 100.0f;
        darr1[i] = (double)rand() / RAND_MAX * 10.0;
        darr2[i] = (double)rand() / RAND_MAX * 5.0;
    }
    for (int i = 0; i < 4096; i++) {
        src_buf[i] = (char)(rand() % 256);
    }
}

/* 
 * Math-intensive function with OpenMP SIMD directive.
 * Triggers vectorization of sinf/cosf builtins.
 */
static void vectorized_math_loop(void) {
    #pragma omp simd
    for (int i = 0; i < 1024; i++) {
        farr2[i] = sinf(farr1[i]) + cosf(farr1[i]);
    }
}

/* 
 * Memory built-in in a loop - may trigger vectorized memcpy.
 * Also uses strlen which could be vectorized.
 */
static void memory_builtin_loop(void) {
    long total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < 1024; i++) {
        /* Create small copies using builtin memcpy */
        __builtin_memcpy(&dst_buf[i*4], &src_buf[i*4], 4);
        
        /* strlen on null-terminated substrings */
        src_buf[i*4 + 3] = '\0';
        total_len += __builtin_strlen(&src_buf[i*4]);
    }
    (void)total_len; /* Prevent unused warning */
}

/* 
 * Architecture-specific paths with fallback.
 * The compiler must analyze both vector and scalar paths.
 */
static void conditional_arch_specific(void) {
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 intrinsic path - may trigger builtin vectorization */
        for (int i = 0; i < 1024; i += 8) {
            __m256 vec = _mm256_load_ps(&farr1[i]);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(&farr2[i], result);
        }
    } else 
    #endif
    {
        /* Scalar fallback with sqrtf calls - should be vectorized */
        #pragma omp simd
        for (int i = 0; i < 1024; i++) {
            farr2[i] = sqrtf(farr1[i]);
        }
    }
}

/* 
 * Hidden visibility function with double precision math.
 * The visibility attribute aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN.
 */
static void hidden_visibility_func(double *arr, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        arr[i] = exp(arr[i]) + log(arr[i] + 1.0);
    }
}

/* 
 * Multiple small inline functions with different builtins.
 * Placed in conditional chain to ensure all are analyzed.
 */
static inline float math_func_1(float x) {
    return powf(x, 1.5f) + fabsf(x);
}

static inline float math_func_2(float x) {
    return expf(x) * 0.5f;
}

static void multi_builtin_selector(int mode) {
    switch (mode) {
        case 1:
            #pragma omp simd
            for (int i = 0; i < 1024; i++) {
                farr2[i] = math_func_1(farr1[i]);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < 1024; i++) {
                farr2[i] = math_func_2(farr1[i]);
            }
            break;
        default:
            /* Dead code path that still contains vectorizable calls */
            if (0) {
                #pragma GCC ivdep
                for (int i = 0; i < 1024; i++) {
                    farr2[i] = sinf(farr1[i]) * cosf(farr1[i]);
                }
            }
            break;
    }
}

/* 
 * Type-punning and mixed data types.
 * May trigger vectorization for type conversions and copies.
 */
static void mixed_type_operations(void) {
    union {
        float f[4];
        int i[4];
    } converter ALIGN_16;
    
    #pragma omp simd
    for (int i = 0; i < 1024; i += 4) {
        /* Use ilogb to get exponent */
        converter.f[0] = farr1[i];
        converter.i[0] = __builtin_ilogb(converter.f[0]);
        
        /* Type punning via memcpy */
        __builtin_memcpy(&converter.f[1], &farr1[i+1], sizeof(float));
        farr2[i] = converter.f[1];
    }
}

/* 
 * OpenMP parallel region with SIMD reduction.
 * Creates complex vectorization context.
 */
static double parallel_simd_reduction(void) {
    double sum = 0.0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1024; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 16; j++) {
            sum += sin(darr1[i]) * cos(darr2[i]);
        }
    }
    return sum;
}

/* Main function that sequences all test cases */
int main(void) {
    init_data();
    
    /* 1. Math-intensive vectorization */
    vectorized_math_loop();
    
    /* 2. Memory builtins */
    memory_builtin_loop();
    
    /* 3. Architecture-specific with fallback */
    conditional_arch_specific();
    
    /* 4. Hidden visibility function */
    hidden_visibility_func(darr1, 1024);
    
    /* 5. Multiple builtin selector */
    multi_builtin_selector(1);
    multi_builtin_selector(2);
    
    /* 6. Mixed type operations */
    mixed_type_operations();
    
    /* 7. OpenMP parallel SIMD reduction */
    double total = parallel_simd_reduction();
    
    /* Aggregate results to prevent dead code elimination */
    float checksum = 0.0f;
    #pragma omp simd reduction(+:checksum)
    for (int i = 0; i < 1024; i++) {
        checksum += farr2[i];
    }
    
    printf("Results: checksum = %f, total = %f\n", checksum, total);
    return 0;
}
