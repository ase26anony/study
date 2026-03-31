/* 
 * Comprehensive test program to trigger GCC's default_builtin_vectorized_function
 * and specifically cover the flag-setting block in targhooks.cc (lines 981-990)
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

/* Alignment attributes for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Array sizes - known at compile time for vectorization */
#define SIZE 1024
#define SMALL_SIZE 128

/* Global aligned arrays */
static float arr_f1[SIZE] ALIGN_32;
static float arr_f2[SIZE] ALIGN_32;
static double arr_d1[SIZE] ALIGN_64;
static double arr_d2[SIZE] ALIGN_64;
static int arr_i1[SIZE] ALIGN_32;
static char str_buf[SMALL_SIZE * 8] ALIGN_32;

/* Simple random initialization to prevent compile-time computation */
static void init_data(void) {
    for (int i = 0; i < SIZE; i++) {
        arr_f1[i] = (float)(i * 0.1 + 0.001 * (i % 7));
        arr_f2[i] = (float)(i * 0.05 + 0.002 * (i % 5));
        arr_d1[i] = (double)(i * 0.01 + 0.003 * (i % 11));
        arr_d2[i] = (double)(i * 0.02 + 0.004 * (i % 13));
        arr_i1[i] = i * 3 + (i % 17);
    }
    
    for (int i = 0; i < SMALL_SIZE * 8 - 1; i++) {
        str_buf[i] = 'A' + (i % 26);
    }
    str_buf[SMALL_SIZE * 8 - 1] = '\0';
}

/* 
 * Function 1: Math-intensive with explicit SIMD pragma
 * Triggers vectorization of sinf/cosf builtins
 */
USED_FUNC NOTHROW_FUNC static void math_intensive_vectorized(float* out, const float* in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Multiple builtin calls in vectorizable loop */
        out[i] = sinf(in[i]) * cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* 
 * Function 2: Memory operations with builtin memcpy
 * Uses __builtin_memcpy in vectorizable context
 */
HIDDEN_VIS static void memory_operations_vectorized(void) {
    char local_buf[SMALL_SIZE * 8] ALIGN_32;
    
    #pragma GCC ivdep
    for (int i = 0; i < SMALL_SIZE; i++) {
        /* Vectorizable builtin memcpy operations */
        __builtin_memcpy(&local_buf[i * 8], &str_buf[i * 8], 8);
    }
    
    /* Use strlen builtin in a way that might vectorize */
    volatile int len = 0;
    #pragma omp simd reduction(+:len)
    for (int i = 0; i < SMALL_SIZE; i++) {
        len += __builtin_strlen(&local_buf[i * 8]);
    }
}

/* 
 * Function 3: Conditional architecture-specific paths
 * Both paths contain vectorizable builtin calls
 */
ALWAYS_INLINE static inline void conditional_math_operations(double* out, const double* in, int n) {
    /* Complex control flow with multiple vectorization candidates */
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 path - compiler may consider vectorized builtin alternatives */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = exp(in[i]) * log(fabs(in[i]) + 1.0);
        }
    } else 
    #endif
    {
        /* Scalar fallback path with same builtins */
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            out[i] = pow(in[i], 1.5) + exp(in[i] * 0.5);
        }
    }
}

/* 
 * Function 4: Hidden visibility helper with mixed types
 * Uses __attribute__((visibility("hidden")))
 */
HIDDEN_VIS static void hidden_visibility_helper(void) {
    double temp[SIZE] ALIGN_64;
    
    /* Loop with multiple builtin math functions */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        temp[i] = sin(arr_d1[i]) * cos(arr_d2[i]) + sqrt(fabs(arr_d1[i] - arr_d2[i]));
    }
    
    /* Type-punning through union may trigger builtin vectorization */
    union {
        double d;
        long long ll;
    } converter;
    
    #pragma GCC ivdep
    for (int i = 0; i < SIZE; i++) {
        converter.d = temp[i];
        arr_i1[i] = __builtin_ilogb(converter.d);
    }
}

/* 
 * Function 5: Multiple small inline functions with different builtins
 * Structured to ensure compiler analyzes all paths
 */
ALWAYS_INLINE static inline void builtin_pow_loop(float* out, const float* in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);
    }
}

ALWAYS_INLINE static inline void builtin_exp_loop(float* out, const float* in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i] * 0.5f);
    }
}

ALWAYS_INLINE static inline void builtin_fabs_loop(float* out, const float* in, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = fabsf(in[i] - 0.5f);
    }
}

/* 
 * Function 6: Complex control flow with switch statement
 * Presents multiple vectorization opportunities to compiler
 */
static void switch_based_vectorization(int mode, float* out, const float* in, int n) {
    switch (mode) {
        case 0:
            builtin_pow_loop(out, in, n);
            break;
        case 1:
            builtin_exp_loop(out, in, n);
            break;
        case 2:
            builtin_fabs_loop(out, in, n);
            break;
        default:
            /* Dead code path that still contains vectorizable builtins */
            if (0) {  /* Always false, but compiler still parses */
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    out[i] = sinf(in[i]) / cosf(in[i]);
                }
            }
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sqrtf(fabsf(in[i]));
            }
    }
}

/* 
 * Function 7: OpenMP parallel region with SIMD reduction
 * Creates complex vectorization context
 */
#pragma omp declare simd
static float simd_reduction_function(const float* data, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += sinf(data[i]) * cosf(data[i]);
    }
    
    return sum;
}

/* Main function orchestrates all test cases */
int main(void) {
    float result_f[SIZE] ALIGN_32;
    double result_d[SIZE] ALIGN_64;
    float reduction_result = 0.0f;
    
    /* Initialize data */
    init_data();
    
    printf("Starting vectorization tests...\n");
    
    /* Test 1: Math-intensive vectorization */
    math_intensive_vectorized(result_f, arr_f1, SIZE);
    
    /* Test 2: Memory operations */
    memory_operations_vectorized();
    
    /* Test 3: Conditional architecture paths */
    conditional_math_operations(result_d, arr_d1, SIZE);
    
    /* Test 4: Hidden visibility helper */
    hidden_visibility_helper();
    
    /* Test 5: Switch-based vectorization */
    for (int mode = 0; mode < 4; mode++) {
        switch_based_vectorization(mode, result_f, arr_f2, SIZE);
    }
    
    /* Test 6: OpenMP SIMD reduction */
    #pragma omp parallel for reduction(+:reduction_result)
    for (int i = 0; i < 4; i++) {
        reduction_result += simd_reduction_function(arr_f1 + i * (SIZE/4), SIZE/4);
    }
    
    /* Prevent dead code elimination */
    volatile float checksum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        checksum += result_f[i] + (float)result_d[i];
    }
    
    printf("Tests completed. Checksum: %f, Reduction: %f\n", 
           (double)checksum, (double)reduction_result);
    
    return 0;
}
