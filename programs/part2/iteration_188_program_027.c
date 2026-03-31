/* test_vectorized_builtins.c */
/* Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fno-builtin -fno-math-errno -o test_vectorized_builtins test_vectorized_builtins.c -lm */

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

#define SIZE 1024
#define ALIGN 32

/* Helper to prevent compile-time computation */
static float simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff;
}

/* Function with visibility attribute matching the uncovered flags */
__attribute__((visibility("hidden"), used, nothrow))
static void hidden_visibility_math(float *restrict a, float *restrict b, int n) {
    #pragma omp simd aligned(a, b: ALIGN)
    for (int i = 0; i < n; i++) {
        /* Vectorized built-in calls */
        a[i] = sinf(b[i]) + cosf(b[i]);
    }
}

/* Static function with vectorized built-ins */
__attribute__((used, noinline))
static void static_vectorized_func(double *restrict arr, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        /* Multiple built-in calls to increase vectorization opportunities */
        arr[i] = sqrt(fabs(arr[i])) + log(fabs(arr[i]) + 1.0);
    }
}

/* Function with OpenMP declare simd to create SIMD variants */
#pragma omp declare simd uniform(n) linear(a, b:1)
__attribute__((always_inline))
inline void simd_math_function(float *a, float *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = powf(b[i], 2.0f) + expf(b[i]);
    }
}

/* Memory operation with built-in memcpy in vectorizable loop */
__attribute__((aligned(ALIGN)))
static void builtin_memcpy_vectorized(char *restrict dst, 
                                      const char *restrict src, 
                                      int n) {
    #pragma omp simd
    for (int i = 0; i < n; i += 16) {
        /* Vectorized built-in memcpy */
        __builtin_memcpy(dst + i, src + i, 16);
    }
}

/* Architecture-specific path with fallback */
__attribute__((target_clones("avx2", "default")))
void arch_specific_math(float *restrict a, float *restrict b, int n) {
    /* Conditional chain presenting multiple vectorization opportunities */
    int arch = 0;
    
    #ifdef __x86_64__
    if (__builtin_cpu_supports("avx2")) {
        arch = 1;
        /* Use AVX intrinsics - compiler may still consider built-in vectorization */
        for (int i = 0; i < n; i += 8) {
            __m256 vec = _mm256_load_ps(&b[i]);
            __m256 result = _mm256_sqrt_ps(vec);
            _mm256_store_ps(&a[i], result);
        }
    } else 
    #endif
    {
        /* Fallback with scalar built-ins - should trigger vectorization */
        #pragma omp simd reduction(+:arch)
        for (int i = 0; i < n; i++) {
            a[i] = sqrtf(b[i]);
            arch += 1;  /* Prevent loop elimination */
        }
    }
    
    /* Dead code path that still contains vectorizable built-ins */
    if (0) {
        float dummy[SIZE] __attribute__((aligned(ALIGN)));
        #pragma omp simd
        for (int i = 0; i < SIZE; i++) {
            dummy[i] = sinf(b[i]) * cosf(b[i]);
        }
    }
}

/* Mixed data types and type punning */
union vector_scalar {
    float f[8];
    #ifdef __x86_64__
    __m256 v;
    #endif
} __attribute__((aligned(32)));

/* Function with switch statement for multiple vectorization candidates */
__attribute__((flatten))
void multi_builtin_selector(int mode, float *a, float *b, int n) {
    switch (mode) {
        case 0:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                a[i] = sinf(b[i]);
            }
            break;
        case 1:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                a[i] = cosf(b[i]);
            }
            break;
        case 2:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                a[i] = sqrtf(b[i]);
            }
            break;
        case 3:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                a[i] = logf(fabsf(b[i]) + 1.0f);
            }
            break;
        default:
            #pragma omp simd
            for (int i = 0; i < n; i++) {
                a[i] = b[i] * b[i];
            }
    }
}

/* Complex OpenMP context with nested pragmas */
void complex_omp_context(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < n) {
                /* Built-in call in nested SIMD loop */
                c[idx] = a[idx] * sinf(b[idx]) + cosf(b[idx]);
                sum += c[idx];
            }
        }
    }
    
    /* Prevent dead code elimination */
    volatile float dummy = sum;
    (void)dummy;
}

/* Main test driver */
int main() {
    /* Aligned arrays as per requirements */
    float a[SIZE] __attribute__((aligned(ALIGN)));
    float b[SIZE] __attribute__((aligned(ALIGN)));
    float c[SIZE] __attribute__((aligned(ALIGN)));
    double d[SIZE] __attribute__((aligned(ALIGN)));
    char src[SIZE] __attribute__((aligned(ALIGN)));
    char dst[SIZE] __attribute__((aligned(ALIGN)));
    
    /* Initialize with pattern data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        b[i] = simple_rand(i);
        a[i] = 0.0f;
        c[i] = 0.0f;
        d[i] = (double)b[i];
        src[i] = (char)(i % 256);
    }
    
    printf("Testing vectorized built-in functions...\n");
    
    /* 1. Math-intensive function with OpenMP SIMD */
    hidden_visibility_math(a, b, SIZE);
    
    /* 2. Static function with vectorized built-ins */
    static_vectorized_func(d, SIZE);
    
    /* 3. Memory/copy function with builtin_memcpy */
    builtin_memcpy_vectorized(dst, src, SIZE);
    
    /* 4. Architecture-specific with fallback */
    arch_specific_math(c, b, SIZE);
    
    /* 5. Multiple built-in selector */
    for (int mode = 0; mode < 4; mode++) {
        multi_builtin_selector(mode, a, b, SIZE);
    }
    
    /* 6. SIMD declared function */
    simd_math_function(c, b, SIZE);
    
    /* 7. Complex OpenMP context */
    complex_omp_context(a, b, c, SIZE / 16);
    
    /* Aggregate results to prevent elimination */
    float total = 0.0f;
    #pragma omp simd reduction(+:total)
    for (int i = 0; i < SIZE; i++) {
        total += a[i] + c[i] + (float)d[i] + dst[i];
    }
    
    printf("Total: %f\n", total);
    printf("Test completed.\n");
    
    return 0;
}
