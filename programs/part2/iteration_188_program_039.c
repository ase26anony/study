/* 
 * Test program to trigger GCC's default_builtin_vectorized_function
 * and cover the flag-setting block for vectorized built-in declarations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Architecture-specific intrinsics */
#ifdef __x86_64__
#include <xmmintrin.h>
#include <immintrin.h>
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#endif

/* Alignment hints for vectorization */
#define ALIGN_32 __attribute__((aligned(32)))
#define ALIGN_64 __attribute__((aligned(64)))

/* Function attributes to influence declaration handling */
#define HIDDEN_VIS __attribute__((visibility("hidden")))
#define USED_FUNC __attribute__((used))
#define NOTHROW_FUNC __attribute__((nothrow))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vectorization pragmas */
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC ivdep

/* OpenMP SIMD function variant */
#pragma omp declare simd uniform(n) linear(a:1) linear(b:1)
static inline float simd_math_func(float a, float b, int n) ALWAYS_INLINE;

/* Test data size - known at compile time for vectorization */
#define SIZE 1024
#define ITERATIONS 100

/* Aligned arrays for vector operations */
static float arr1[SIZE] ALIGN_32;
static float arr2[SIZE] ALIGN_32;
static double darr1[SIZE] ALIGN_64;
static double darr2[SIZE] ALIGN_64;
static int iarr[SIZE] ALIGN_32;
static char str_buffer[SIZE * 2] ALIGN_32;

/* Simple random generator to prevent compile-time computation */
static float simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff;
}

/* Function 1: Math-intensive with OpenMP SIMD pragma */
HIDDEN_VIS USED_FUNC NOTHROW_FUNC
static void test_math_vectorization(float *out, const float *in, int n) {
    #pragma omp simd safelen(16)
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls to trigger vectorization */
        float val = in[i];
        out[i] = sinf(val) + cosf(val) + sqrtf(fabsf(val));
    }
}

/* Function 2: Memory operations with builtins */
static void test_memcpy_vectorization(void) {
    char local_buf[SIZE] ALIGN_32;
    
    #pragma omp simd
    for (int i = 0; i < SIZE - 32; i += 32) {
        /* Use __builtin_memcpy in vectorizable context */
        __builtin_memcpy(&local_buf[i], &str_buffer[i], 32);
    }
    
    /* strlen in loop - may get vectorized */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < SIZE; i += 64) {
        total_len += __builtin_strlen(&str_buffer[i]);
    }
}

/* Function 3: Conditional architecture-specific paths */
USED_FUNC
static double test_conditional_vectorization(double *arr, int n) {
    double sum = 0.0;
    
    /* Conditional path with CPU feature check */
    if (__builtin_cpu_supports("avx2")) {
        /* Vector intrinsic path - compiler may still analyze scalar fallback */
        #ifdef __x86_64__
        for (int i = 0; i < n; i += 4) {
            __m256d vec = _mm256_load_pd(&arr[i]);
            __m256d sqrt_vec = _mm256_sqrt_pd(vec);
            double temp[4] ALIGN_32;
            _mm256_store_pd(temp, sqrt_vec);
            sum += temp[0] + temp[1] + temp[2] + temp[3];
        }
        #endif
    } else {
        /* Scalar fallback with built-in calls */
        #pragma omp simd reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += sqrt(arr[i]) + log(arr[i] + 1.0);
        }
    }
    
    return sum;
}

/* Function 4: Multiple small inline functions with different builtins */
static inline float vec_func1(float x) ALWAYS_INLINE {
    return powf(x, 1.5f) + expf(x * 0.1f);
}

static inline float vec_func2(float x) ALWAYS_INLINE {
    return fabsf(x) + sinf(x * 0.5f);
}

static inline float vec_func3(float x) ALWAYS_INLINE {
    return logf(x + 1.0f) + cosf(x);
}

/* Function 5: Switch with different vectorization candidates */
HIDDEN_VIS
static void test_switch_vectorization(float *out, const float *in, int n, int mode) {
    switch (mode & 3) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = vec_func1(in[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = vec_func2(in[i]);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = vec_func3(in[i]);
            }
            break;
        default:
            /* Mixed operations */
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                out[i] = sinf(in[i]) * cosf(in[i]) + sqrtf(fabsf(in[i]));
            }
    }
}

/* Function 6: Dead code path with vectorizable builtins */
static void dead_code_vectorization(void) {
    /* This path is never executed but still analyzed by compiler */
    if (0) {  /* Always false */
        float dead_arr[SIZE] ALIGN_32;
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            dead_arr[i] = sinf(i * 0.1f) + expf(i * 0.01f);
        }
        
        /* Type punning through union */
        union {
            float f;
            int i;
        } converter;
        
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            converter.f = dead_arr[i];
            iarr[i] = __builtin_ilogb(dead_arr[i]);
        }
    }
}

/* Function 7: Nested OpenMP pragmas for complex context */
static double test_nested_vectorization(const double *arr, int n) {
    double global_sum = 0.0;
    
    #pragma omp parallel for reduction(+:global_sum)
    for (int j = 0; j < ITERATIONS; j++) {
        double local_sum = 0.0;
        
        #pragma omp simd reduction(+:local_sum) aligned(arr:64)
        for (int i = 0; i < n; i++) {
            local_sum += exp(arr[i] * 0.01) * log(arr[i] + 1.0);
        }
        
        global_sum += local_sum;
    }
    
    return global_sum;
}

/* Function 8: Mixed data types and alignment */
static void test_mixed_types(void) {
    /* Operate on different types with corresponding builtins */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        /* Float builtins */
        arr2[i] = sinf(arr1[i]) + cosf(arr1[i]);
        
        /* Double builtins */
        darr2[i] = sqrt(darr1[i]) + log(darr1[i] + 1.0);
        
        /* Integer builtins */
        iarr[i] = __builtin_abs(iarr[i]) + __builtin_clz(iarr[i] | 1);
    }
}

/* Main test driver */
int main(void) {
    /* Initialize with pattern data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = simple_rand(i) * 10.0f;
        darr1[i] = simple_rand(i + SIZE) * 10.0;
        iarr[i] = rand() % 1000;
        str_buffer[i] = 'A' + (i % 26);
    }
    str_buffer[SIZE - 1] = '\0';
    
    printf("Starting vectorization tests...\n");
    
    /* Run all vectorization tests */
    double total_result = 0.0;
    
    /* Test 1: Math vectorization */
    test_math_vectorization(arr2, arr1, SIZE);
    total_result += arr2[SIZE/2];
    
    /* Test 2: Memory builtins */
    test_memcpy_vectorization();
    
    /* Test 3: Conditional paths */
    total_result += test_conditional_vectorization(darr1, SIZE);
    
    /* Test 4: Switch-based vectorization */
    for (int mode = 0; mode < 4; mode++) {
        test_switch_vectorization(arr2, arr1, SIZE, mode);
        total_result += arr2[mode];
    }
    
    /* Test 5: Dead code (still analyzed) */
    dead_code_vectorization();
    
    /* Test 6: Nested OpenMP */
    total_result += test_nested_vectorization(darr1, SIZE);
    
    /* Test 7: Mixed types */
    test_mixed_types();
    total_result += arr2[0] + darr2[0] + iarr[0];
    
    /* Prevent dead code elimination */
    printf("Total result: %f\n", total_result);
    printf("Array samples: arr2[0]=%f, darr2[0]=%f\n", arr2[0], darr2[0]);
    
    return 0;
}

#pragma GCC pop_options
