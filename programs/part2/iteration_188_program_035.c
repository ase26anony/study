/* 
 * Test program designed to trigger GCC's default_builtin_vectorized_function
 * and specifically execute the flag-setting block in targhooks.cc (lines 981-990)
 * by creating patterns that cause the vectorizer to request vectorized built-ins.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Architecture-specific intrinsics */
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

/* Compiler attributes to influence declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Array sizes - known at compile time for vectorization */
#define SIZE 1024
#define ITER 100

/* Aligned arrays */
static float arr_f1[SIZE] ALIGN_32;
static float arr_f2[SIZE] ALIGN_32;
static double arr_d1[SIZE] ALIGN_64;
static double arr_d2[SIZE] ALIGN_64;
static int arr_i1[SIZE] ALIGN_32;
static char src_str[SIZE] ALIGN_32;
static char dst_str[SIZE] ALIGN_32;

/* Simple random init to prevent compile-time computation */
static void init_data(void) {
    for (int i = 0; i < SIZE; i++) {
        arr_f1[i] = (float)(i % 100) * 0.1f;
        arr_f2[i] = (float)(i % 50) * 0.2f;
        arr_d1[i] = (double)(i % 200) * 0.05;
        arr_d2[i] = (double)(i % 150) * 0.1;
        arr_i1[i] = i;
        src_str[i] = 'A' + (i % 26);
        dst_str[i] = 0;
    }
    src_str[SIZE-1] = '\0';
}

/* Function 1: Math-intensive with OpenMP SIMD pragma */
USED_FUNC NOTHROW_FUNC static void math_intensive_sincos(void) {
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        /* Trig functions that should trigger builtin vectorization */
        arr_f1[i] = sinf(arr_f1[i]) + cosf(arr_f2[i]);
    }
}

/* Function 2: Memory operations with builtin memcpy in loop */
static void memory_operations(void) {
    for (int i = 0; i < SIZE; i += 64) {
        /* Builtin memcpy that could be vectorized */
        __builtin_memcpy(&dst_str[i], &src_str[i], 64);
    }
}

/* Function 3: Conditional path with CPU feature detection */
ALWAYS_INLINE static inline void conditional_vector_path(int path) {
    float temp[SIZE] ALIGN_32;
    
    switch(path) {
        case 0:
            #pragma GCC ivdep
            for (int i = 0; i < SIZE; i++) {
                temp[i] = sqrtf(arr_f1[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < SIZE; i++) {
                temp[i] = powf(arr_f1[i], arr_f2[i]);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < SIZE; i++) {
                temp[i] = fabsf(arr_f1[i] - arr_f2[i]);
            }
            break;
        default:
            /* Dead code path but still processed */
            if (0) {
                #pragma omp simd
                for (int i = 0; i < SIZE; i++) {
                    temp[i] = expf(arr_f1[i]) * logf(arr_f2[i] + 1.0f);
                }
            }
            break;
    }
    
    /* Use temp to prevent elimination */
    arr_f2[0] += temp[0];
}

/* Function 4: Hidden visibility helper with double precision math */
HIDDEN_VIS static void hidden_visibility_helper(void) {
    double sum = 0.0;
    
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        /* exp and log calls that should trigger builtin vectorization */
        sum += exp(arr_d1[i]) * log(arr_d2[i] + 1.0);
    }
    
    arr_d1[0] = sum / SIZE;
}

/* Function 5: Mixed types and type punning */
static void mixed_type_operations(void) {
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        /* Type punning and builtin usage */
        converter.f = arr_f1[i];
        arr_i1[i] = __builtin_ilogb(converter.f);
    }
}

/* Function 6: Architecture-specific intrinsics with fallback */
static void arch_specific_vectorization(void) {
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 path - compiler may still consider builtin alternatives */
        for (int i = 0; i < SIZE; i += 8) {
            __m256 vec = _mm256_load_ps(&arr_f1[i]);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(&arr_f2[i], result);
        }
    } else 
    #endif
    {
        /* Fallback scalar path with builtin sqrt */
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            arr_f2[i] = sqrtf(arr_f1[i]);
        }
    }
}

/* Function 7: OpenMP declare simd function */
#pragma omp declare simd
static float simd_function(float x, float y) {
    /* Builtin calls inside SIMD function */
    return sinf(x) * cosf(y) + sqrtf(x * y);
}

/* Function 8: Complex reduction with nested pragmas */
static void complex_reduction(void) {
    float sum = 0.0f;
    
    #pragma omp parallel for
    for (int i = 0; i < ITER; i++) {
        float local_sum = 0.0f;
        
        #pragma omp simd reduction(+:local_sum)
        for (int j = 0; j < SIZE; j++) {
            local_sum += simd_function(arr_f1[j], arr_f2[j]);
        }
        
        #pragma omp atomic
        sum += local_sum;
    }
    
    arr_f1[0] = sum / (ITER * SIZE);
}

/* Main function that sequences all tests */
int main(void) {
    init_data();
    
    printf("Starting vectorization tests...\n");
    
    /* Execute all vectorization patterns */
    for (int iter = 0; iter < 10; iter++) {
        math_intensive_sincos();
        memory_operations();
        conditional_vector_path(iter % 3);
        hidden_visibility_helper();
        mixed_type_operations();
        arch_specific_vectorization();
        complex_reduction();
        
        /* Modify data slightly each iteration */
        arr_f1[iter % SIZE] += 0.1f;
        arr_d1[iter % SIZE] += 0.01;
    }
    
    /* Print results to prevent elimination */
    printf("Results: f1[0]=%f, d1[0]=%f, i1[0]=%d\n", 
           arr_f1[0], arr_d1[0], arr_i1[0]);
    printf("String length via builtin: %zu\n", __builtin_strlen(src_str));
    
    return 0;
}
