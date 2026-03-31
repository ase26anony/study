/* test_vectorized_builtins.c */
/* Compile with: gcc -O3 -ftree-vectorize -mavx2 -fopenmp -march=native -fopt-info-vec -o test test_vectorized_builtins.c -lm */

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

/* Helper to prevent compile-time optimization */
static float __attribute__((noinline)) simple_rand(int seed) {
    return (float)((seed * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff;
}

/* Function with hidden visibility attribute - aligns with DECL_VISIBILITY(t) = VISIBILITY_HIDDEN */
static void __attribute__((visibility("hidden"))) 
__attribute__((used)) 
__attribute__((nothrow))
process_hidden(float *restrict a, float *restrict b, int n) {
    /* This should trigger vectorization of sinf/cosf */
    #pragma omp simd aligned(a, b:ALIGN)
    for (int i = 0; i < n; i++) {
        a[i] = sinf(b[i]) + cosf(b[i]);
    }
}

/* Always inline function with multiple builtins */
static inline __attribute__((always_inline)) 
void process_pow_exp(float *restrict a, float *restrict b, int n) {
    #pragma GCC ivdep
    for (int i = 0; i < n; i++) {
        a[i] = powf(b[i], 2.0f) + expf(b[i]);
    }
}

/* Function with OpenMP declare simd - creates SIMD variants */
#pragma omp declare simd uniform(n) linear(a, b:1)
void __attribute__((used)) vectorized_sqrt(float *restrict a, float *restrict b, int n) {
    #pragma omp simd reduction(+:a[:0])
    for (int i = 0; i < n; i++) {
        a[i] = sqrtf(fabsf(b[i]));
    }
}

/* Mixed data types with alignment hints */
static void process_mixed_types(
    double *restrict d_arr,
    float *restrict f_arr,
    int *restrict i_arr,
    int n) {
    
    /* Aligned arrays as recommended */
    double __attribute__((aligned(32))) local_d[SIZE];
    float __attribute__((aligned(32))) local_f[SIZE];
    
    /* Type punning through memcpy - may trigger builtin vectorization */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        __builtin_memcpy(&local_f[i], &f_arr[i], sizeof(float));
        d_arr[i] = log(local_f[i] + 1.0);
    }
    
    /* Integer builtin */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        i_arr[i] = __builtin_ilogb(d_arr[i]);
    }
}

/* Architecture-specific path with fallback */
static void __attribute__((noinline)) 
arch_specific_processing(float *restrict a, float *restrict b, int n) {
    
    /* Conditional chain with dead code path */
    int path = 1; /* Could be parameterized */
    
    switch (path) {
        case 0: /* Dead code path but still analyzed */
            if (0) { /* Dead code */
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    a[i] = sinf(b[i]) * cosf(b[i]);
                }
            }
            break;
            
        case 1: /* Main path with CPU feature check */
            #ifdef __x86_64__
            if (__builtin_cpu_supports("avx2")) {
                /* Use AVX intrinsics - compiler may still consider builtin vectorization */
                for (int i = 0; i < n; i += 8) {
                    __m256 b_vec = _mm256_load_ps(&b[i]);
                    __m256 result = _mm256_add_ps(_mm256_sin_ps(b_vec), 
                                                 _mm256_cos_ps(b_vec));
                    _mm256_store_ps(&a[i], result);
                }
            } else 
            #endif
            {
                /* Fallback with scalar builtins - should trigger vectorization */
                #pragma omp simd
                for (int i = 0; i < n; i++) {
                    a[i] = sinf(b[i]) + cosf(b[i]);
                }
            }
            break;
            
        case 2: /* Alternative path with different builtins */
            process_pow_exp(a, b, n);
            break;
    }
}

/* Memory operation with builtin memcpy in loop */
static void __attribute__((used))
memory_operations(char *restrict dst, char *restrict src, int n) {
    /* Multiple memcpy operations that could be vectorized */
    #pragma omp simd
    for (int i = 0; i < n; i += 64) {
        __builtin_memcpy(dst + i, src + i, 64);
    }
    
    /* strlen in vectorized context */
    int total_len = 0;
    #pragma omp simd reduction(+:total_len)
    for (int i = 0; i < n; i += 128) {
        total_len += __builtin_strlen(src + i);
    }
}

/* Complex control flow with multiple vectorization candidates */
static float __attribute__((noinline))
complex_control_flow(int selector, float *arr, int n) {
    float result = 0.0f;
    
    /* Multiple small functions inlined */
    auto inline void process_exp(float *a, float *b, int m) __attribute__((always_inline)) {
        #pragma omp simd
        for (int i = 0; i < m; i++) {
            a[i] = expf(b[i]);
        }
    }
    
    auto inline void process_log(float *a, float *b, int m) __attribute__((always_inline)) {
        #pragma omp simd
        for (int i = 0; i < m; i++) {
            a[i] = logf(b[i] + 1.0f);
        }
    }
    
    float tmp[SIZE] __attribute__((aligned(32)));
    
    if (selector & 1) {
        process_exp(tmp, arr, n);
        #pragma omp simd reduction(+:result)
        for (int i = 0; i < n; i++) {
            result += tmp[i];
        }
    }
    
    if (selector & 2) {
        process_log(tmp, arr, n);
        #pragma omp simd reduction(+:result)
        for (int i = 0; i < n; i++) {
            result += tmp[i];
        }
    }
    
    return result;
}

/* Main test driver */
int main() {
    /* Initialize with aligned arrays */
    float __attribute__((aligned(ALIGN))) a[SIZE];
    float __attribute__((aligned(ALIGN))) b[SIZE];
    double __attribute__((aligned(ALIGN))) d[SIZE];
    int __attribute__((aligned(ALIGN))) i_arr[SIZE];
    char __attribute__((aligned(ALIGN))) src[SIZE * 4];
    char __attribute__((aligned(ALIGN))) dst[SIZE * 4];
    
    srand(time(NULL));
    
    /* Initialize arrays with non-constant data */
    for (int j = 0; j < SIZE; j++) {
        b[j] = simple_rand(j);
        d[j] = (double)b[j] * 2.0;
        src[j * 4] = 'A' + (j % 26);
        src[j * 4 + 1] = '\0';
    }
    
    float total = 0.0f;
    
    /* Test 1: Math-intensive with OpenMP SIMD */
    process_hidden(a, b, SIZE);
    for (int j = 0; j < SIZE; j++) total += a[j];
    
    /* Test 2: Mixed types with alignment */
    process_mixed_types(d, a, i_arr, SIZE);
    for (int j = 0; j < SIZE; j++) total += (float)d[j] + i_arr[j];
    
    /* Test 3: Architecture-specific with fallback */
    arch_specific_processing(a, b, SIZE);
    for (int j = 0; j < SIZE; j++) total += a[j];
    
    /* Test 4: OpenMP declare simd function */
    vectorized_sqrt(a, b, SIZE);
    for (int j = 0; j < SIZE; j++) total += a[j];
    
    /* Test 5: Memory operations */
    memory_operations(dst, src, SIZE);
    total += (float)dst[0];
    
    /* Test 6: Complex control flow */
    total += complex_control_flow(3, b, SIZE);
    
    /* Test 7: Nested OpenMP parallel + SIMD */
    #pragma omp parallel for
    for (int j = 0; j < SIZE; j++) {
        float local_sum = 0.0f;
        #pragma omp simd reduction(+:local_sum)
        for (int k = 0; k < 16; k++) {
            local_sum += sinf(b[(j + k) % SIZE] * 0.1f * k);
        }
        a[j] = local_sum;
    }
    
    for (int j = 0; j < SIZE; j++) total += a[j];
    
    printf("Result: %f\n", total);
    return 0;
}
