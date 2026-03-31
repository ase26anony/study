/* Test to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* For x86 AVX-512 */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

/* For ARM NEON */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Always inline to force expansion */
__attribute__((always_inline, hot))
static inline void complex_vector_operations(void) {
    /* Approach 1: Use AVX-512 masked operations (if available) */
#ifdef __AVX512F__
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __m512 c = _mm512_set1_ps(3.0f);
        __mmask16 mask = 0xAAAA;  /* Alternating pattern */
        
        /* _mm512_mask_fmadd_ps has 4 operands, but during RTL expansion
           it may decompose into more operands */
        __m512 result1 = _mm512_mask_fmadd_ps(a, mask, b, c);
        
        /* Complex expression with multiple masked operations */
        __m512 d = _mm512_set1_ps(4.0f);
        __m512 e = _mm512_set1_ps(5.0f);
        __m512 f = _mm512_set1_ps(6.0f);
        
        /* Chain operations to potentially create multi-operand patterns */
        __m512 temp = _mm512_mask_add_ps(a, mask, b, c);
        __m512 result2 = _mm512_mask_mul_ps(temp, mask, d, e);
        result2 = _mm512_mask_sub_ps(result2, mask, f, a);
        
        /* Use the results to prevent optimization */
        volatile __m512 sink1 = result1;
        volatile __m512 sink2 = result2;
    }
#endif

    /* Approach 2: GCC vector extensions with complex expressions */
    {
        v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        v8sf v2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
        v8sf v3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
        v8sf v4 = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
        v8sf v5 = {5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
        
        /* Complex expression that might expand to multi-operand instruction */
        v8sf result = v1 * v2 + v3 * v4 + v5;
        result = result * v1 - v2 / v3 + v4 * v5;
        
        /* Use built-in FMA functions */
        for (int i = 0; i < 8; i++) {
            /* __builtin_fmaf expands to FMA instruction with 3 operands,
               but the overall expression may require more */
            float temp = __builtin_fmaf(v1[i], v2[i], v3[i]);
            temp = __builtin_fmaf(temp, v4[i], v5[i]);
            result[i] = temp;
        }
        
        volatile v8sf sink = result;
    }

    /* Approach 3: Inline assembly with many operands */
    {
        int64_t a = 1, b = 2, c = 3, d = 4, e = 5;
        int64_t f = 6, g = 7, h = 8, i = 9, j = 10;
        int64_t k = 11;
        
        /* Extended asm with 11 operands (10 inputs + 1 output) */
        asm volatile (
            "add %[a], %[b], %[c]\n\t"
            "add %[d], %[e], %[f]\n\t"
            "add %[g], %[h], %[i]\n\t"
            "mul %[out], %[j], %[k]"
            : [out] "=r" (a)
            : [a] "r" (a), [b] "r" (b), [c] "r" (c),
              [d] "r" (d), [e] "r" (e), [f] "r" (f),
              [g] "r" (g), [h] "r" (h), [i] "r" (i),
              [j] "r" (j), [k] "r" (k)
            : "cc"
        );
        
        volatile int64_t sink = a + b + c + d + e + f + g + h + i + j + k;
    }
}

/* Approach 4: OpenMP SIMD reduction with vector types */
__attribute__((noinline, hot))
static float complex_reduction(float* arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex expression that might expand to multi-operand pattern */
        sum += arr[i] * arr[i] + arr[i] / (i + 1.0f) - arr[i] * 0.5f;
    }
    
    return sum;
}

/* Approach 5: ARM NEON multi-vector operations */
#ifdef __ARM_NEON
__attribute__((always_inline))
static inline void neon_complex_ops(void) {
    int8x16_t v1 = vdupq_n_s8(1);
    int8x16_t v2 = vdupq_n_s8(2);
    int8x16_t v3 = vdupq_n_s8(3);
    int8x16_t v4 = vdupq_n_s8(4);
    
    /* Table lookup operations can require many operands */
    int8x16_t indices = vaddq_s8(v1, v2);
    
    /* Complex sequence of operations */
    int8x16_t result = vaddq_s8(v1, v2);
    result = vmlaq_s8(result, v3, v4);
    result = vqdmulhq_s8(result, indices);
    
    volatile int8x16_t sink = result;
}
#endif

int main(void) {
    /* Initialize array for reduction */
    float* arr = (float*)malloc(1024 * sizeof(float));
    for (int i = 0; i < 1024; i++) {
        arr[i] = (float)i / 1024.0f;
    }
    
    /* Execute all approaches */
    complex_vector_operations();
    
    float sum = complex_reduction(arr, 1024);
    
#ifdef __ARM_NEON
    neon_complex_ops();
#endif
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f\n", sum);
    
    free(arr);
    return (int)(sum * 0.0f);  /* Return 0, but compiler doesn't know */
}
