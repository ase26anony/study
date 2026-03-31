/* 
 * Program to trigger GCC's default_builtin_vectorized_function
 * and cover flag-setting lines in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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

/* Simple PRNG to prevent compile-time computation */
static unsigned int seed = 12345;
static inline float rand_float(void) {
    seed = seed * 1103515245 + 12345;
    return (float)(seed % 1000) / 1000.0f;
}

/* Function with hidden visibility containing vectorizable math operations */
__attribute__((visibility("hidden")))
__attribute__((used))
__attribute__((nothrow))
static void hidden_visibility_math(float *restrict out, 
                                   const float *restrict in, 
                                   int n) {
    #pragma omp simd simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls that can be vectorized */
        out[i] = sinf(in[i]) + cosf(in[i]) + sqrtf(fabsf(in[i]));
    }
}

/* Static function with always_inline attribute */
static __attribute__((always_inline)) 
inline void inline_math_operations(double *restrict arr, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        /* Built-in math functions */
        arr[i] = exp(arr[i]) * log(fabs(arr[i]) + 1.0);
    }
}

/* Function using __builtin_memcpy in vectorizable context */
__attribute__((used))
static void vectorized_memcpy_ops(char *restrict dest, 
                                  const char *restrict src, 
                                  int n) {
    const int chunk = 64;
    #pragma omp simd
    for (int i = 0; i < n; i += chunk) {
        int size = (i + chunk <= n) ? chunk : n - i;
        /* __builtin_memcpy that could be vectorized */
        __builtin_memcpy(dest + i, src + i, size);
    }
}

/* OpenMP declare simd function */
#pragma omp declare simd uniform(b) linear(i:1)
__attribute__((always_inline))
inline float simd_pow_operation(float a, float b, int i) {
    return powf(a + i, b);
}

/* Function with architecture-specific intrinsics */
__attribute__((noinline))
void architecture_specific_vectorization(float *restrict out, 
                                         const float *restrict in, 
                                         int n) {
#ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        /* AVX2 path - compiler may consider built-in alternatives */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(in + i);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(out + i, result);
        }
    } else 
#endif
    {
        /* Fallback scalar path with built-in calls */
        #pragma omp simd reduction(+:out[:n])
        for (int i = 0; i < n; i++) {
            out[i] = sqrtf(in[i]) + sinf(in[i] * 0.5f);
        }
    }
}

/* Multiple built-in functions in switch-controlled paths */
__attribute__((flatten))
static void multi_builtin_switch(int mode, float *arr, int n) {
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = sinf(arr[i]) * cosf(arr[i]);
            }
            break;
            
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = sqrtf(fabsf(arr[i])) + logf(arr[i] + 1.0f);
            }
            break;
            
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                arr[i] = expf(arr[i] * 0.5f) - tanf(arr[i]);
            }
            break;
            
        default:
            /* Dead code path that still contains vectorizable built-ins */
            if (0) {
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    arr[i] = powf(arr[i], 2.0f) + asinf(arr[i] * 0.5f);
                }
            }
            break;
    }
}

/* Function with mixed data types */
__attribute__((optimize("tree-vectorize")))
static void mixed_type_operations(float *floats, double *doubles, int *ints, int n) {
    /* Type-punning union */
    union {
        float f;
        int i;
    } converter;
    
    #pragma omp parallel for simd
    for (int idx = 0; idx < n; idx++) {
        /* Mixed built-in calls */
        floats[idx] = sinf(floats[idx]);
        doubles[idx] = cos(doubles[idx]);
        
        /* __builtin_ilogb */
        ints[idx] = __builtin_ilogb(floats[idx]);
        
        /* Type punning that might use built-in memcpy */
        converter.f = floats[idx];
        ints[idx] ^= converter.i;
    }
}

/* Main test function */
int main(void) {
    const int N = 1024;
    const int ITERATIONS = 5;
    
    /* Aligned arrays */
    float ALIGN_32 arr1[N], arr2[N], arr3[N];
    double ALIGN_64 darr1[N], darr2[N];
    int ALIGN_32 iarr[N];
    char ALIGN_32 str1[N], str2[N];
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_float() * 2.0f - 1.0f;
        arr2[i] = rand_float() * 3.0f;
        darr1[i] = (double)rand_float() * 4.0 - 2.0;
        iarr[i] = (int)(rand_float() * 1000);
        str1[i] = 'A' + (char)(rand_float() * 26);
    }
    
    /* Ensure null termination for string ops */
    str1[N-1] = '\0';
    
    float total_sum = 0.0f;
    
    /* Test 1: Math-intensive function with OpenMP SIMD */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        #pragma omp parallel for simd reduction(+:total_sum)
        for (int i = 0; i < N; i++) {
            arr3[i] = sinf(arr1[i]) + cosf(arr2[i]);
            total_sum += arr3[i];
        }
    }
    
    /* Test 2: Hidden visibility function */
    hidden_visibility_math(arr2, arr1, N);
    for (int i = 0; i < N; i++) {
        total_sum += arr2[i];
    }
    
    /* Test 3: Memory/copy operations */
    vectorized_memcpy_ops(str2, str1, N);
    
    /* Test 4: Architecture-specific path selection */
    architecture_specific_vectorization(arr3, arr1, N);
    for (int i = 0; i < N; i++) {
        total_sum += arr3[i];
    }
    
    /* Test 5: Inline math operations */
    inline_math_operations(darr1, N);
    for (int i = 0; i < N; i++) {
        total_sum += (float)darr1[i];
    }
    
    /* Test 6: Switch with multiple built-in paths */
    for (int mode = 0; mode < 3; mode++) {
        multi_builtin_switch(mode, arr1, N);
        for (int i = 0; i < N; i++) {
            total_sum += arr1[i];
        }
    }
    
    /* Test 7: Mixed type operations */
    mixed_type_operations(arr1, darr1, iarr, N);
    for (int i = 0; i < N; i++) {
        total_sum += arr1[i] + (float)darr1[i] + (float)iarr[i];
    }
    
    /* Test 8: OpenMP declare simd function usage */
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        arr2[i] = simd_pow_operation(arr1[i], 1.5f, i);
        total_sum += arr2[i];
    }
    
    /* Test 9: __builtin_strlen in vectorizable context */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < N/64; i++) {
        total_len += __builtin_strlen(str1 + i * 64);
    }
    total_sum += (float)total_len;
    
    /* Prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    printf("String length sum: %d\n", total_len);
    
    /* Use results to prevent optimization */
    volatile float sink = total_sum;
    (void)sink;
    
    return 0;
}
