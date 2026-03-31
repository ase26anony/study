/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Target-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

/* GCC vector extensions for complex operations */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Complex reduction using GCC vector extensions */
FORCE_INLINE v8sf complex_vector_operation(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex expression that might expand to multi-operand instructions */
    return a * b + c * d + (a + b) * (c - d) + a * c + b * d;
}

#ifdef __x86_64__
/* AVX-512 intrinsics with many operands */
FORCE_INLINE __m512 test_avx512_multi_operand(__m512 a, __m512 b, __m512 c, 
                                              __m512 d, __mmask16 k) {
    /* Use mask operations that take many operands */
    __m512 result;
    
    /* FMA with mask - can expand to many operands */
    result = _mm512_mask_fmadd_ps(a, k, b, c);
    
    /* Additional masked operation */
    result = _mm512_mask_add_ps(result, k, result, d);
    
    /* Complex blend operation */
    result = _mm512_mask_blend_ps(k, result, a);
    
    return result;
}

/* Test AVX-512 gather with many parameters */
FORCE_INLINE __m512i test_gather(__m512i index, __m512i src, __mmask16 k, 
                                 void const *base, __m512i scale) {
    /* Gather operation with mask, base, scale, etc. */
    return _mm512_mask_i32gather_epi32(src, k, index, base, 4);
}
#endif

#ifdef __aarch64__
/* ARM NEON/SVE style operations with many operands */
FORCE_INLINE float32x4x4_t test_neon_multi_lane(float32x4x4_t a, float32x4x4_t b,
                                                float32x4x4_t c, float32x4x4_t d) {
    /* Complex multi-vector operation */
    float32x4x4_t result;
    
    /* Multiple vector operations that might combine */
    result.val[0] = vmlaq_f32(a.val[0], b.val[0], c.val[0]);
    result.val[1] = vmlaq_f32(a.val[1], b.val[1], c.val[1]);
    result.val[2] = vmlaq_f32(a.val[2], b.val[2], c.val[2]);
    result.val[3] = vmlaq_f32(a.val[3], b.val[3], c.val[3]);
    
    /* Additional operations */
    result.val[0] = vaddq_f32(result.val[0], d.val[0]);
    result.val[1] = vaddq_f32(result.val[1], d.val[1]);
    
    return result;
}
#endif

/* Inline assembly with many operands */
FORCE_INLINE void multi_operand_asm(void) {
    /* Create 11 distinct variables to use as operands */
    uint64_t out1, out2, out3;
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6, in7 = 7;
    
    /* Extended asm with 10 operands (3 outputs, 7 inputs) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %6\n\t"
        "mov %2, %7\n\t"
        "add %2, %8\n\t"
        "add %0, %9\n\t"
        : "=&r" (out1), "=&r" (out2), "=&r" (out3)
        : "r" (in1), "r" (in2), "r" (in3), "r" (in4), 
          "r" (in5), "r" (in6), "r" (in7)
        : "cc"
    );
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(out1), "r"(out2), "r"(out3));
}

/* Built-in functions for complex math */
FORCE_INLINE double complex_builtin_operations(double a, double b, double c, 
                                               double d, double e) {
    /* Nested FMA operations that might expand to multi-operand patterns */
    double result;
    
    /* Use __builtin_fma which takes 3 operands */
    result = __builtin_fma(a, b, c);
    result = __builtin_fma(result, d, e);
    
    /* Additional complex operation */
    result = __builtin_fma(a, c, __builtin_fma(b, d, e));
    
    return result;
}

/* OpenMP SIMD reduction with vector types */
void omp_vector_reduction(v8sf *array, int n, v8sf *result) {
    v8sf sum = {0};
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Complex operation that might require multi-operand expansion */
        sum = sum + array[i] * array[i] + array[i];
    }
    
    *result = sum;
}

/* Main test function */
int main(void) {
    /* Initialize test data */
    float array_data[32];
    v8sf vector_array[4];
    v8sf result_vec = {0};
    
    for (int i = 0; i < 32; i++) {
        array_data[i] = (float)i;
    }
    
    /* Load data into vectors */
    for (int i = 0; i < 4; i++) {
        memcpy(&vector_array[i], &array_data[i * 8], sizeof(v8sf));
    }
    
    /* Test 1: GCC vector extensions */
    v8sf a = vector_array[0];
    v8sf b = vector_array[1];
    v8sf c = vector_array[2];
    v8sf d = vector_array[3];
    
    v8sf vec_result = complex_vector_operation(a, b, c, d);
    
    /* Test 2: Inline assembly with many operands */
    multi_operand_asm();
    
    /* Test 3: Built-in complex math */
    double builtin_result = complex_builtin_operations(1.0, 2.0, 3.0, 4.0, 5.0);
    
    /* Test 4: OpenMP reduction */
    omp_vector_reduction(vector_array, 4, &result_vec);
    
    #ifdef __x86_64__
    /* Test 5: AVX-512 operations */
    __m512 avx_a = _mm512_loadu_ps(array_data);
    __m512 avx_b = _mm512_loadu_ps(array_data + 16);
    __m512 avx_c = _mm512_set1_ps(2.0f);
    __m512 avx_d = _mm512_set1_ps(3.0f);
    __mmask16 mask = 0xAAAA;
    
    __m512 avx_result = test_avx512_multi_operand(avx_a, avx_b, avx_c, avx_d, mask);
    
    /* Test gather */
    __m512i indices = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i gather_src = _mm512_set1_epi32(0);
    __m512i scale = _mm512_set1_epi32(1);
    __m512i gather_result = test_gather(indices, gather_src, mask, array_data, scale);
    
    /* Store to prevent optimization */
    _mm512_storeu_ps(array_data, avx_result);
    #endif
    
    #ifdef __aarch64__
    /* Test 6: ARM NEON operations */
    float32x4x4_t neon_a, neon_b, neon_c, neon_d;
    for (int i = 0; i < 4; i++) {
        neon_a.val[i] = vld1q_f32(array_data + i * 4);
        neon_b.val[i] = vld1q_f32(array_data + 16 + i * 4);
        neon_c.val[i] = vld1q_f32(array_data + 8 + i * 4);
        neon_d.val[i] = vld1q_f32(array_data + 24 + i * 4);
    }
    
    float32x4x4_t neon_result = test_neon_multi_lane(neon_a, neon_b, neon_c, neon_d);
    
    /* Store results */
    for (int i = 0; i < 4; i++) {
        vst1q_f32(array_data + i * 4, neon_result.val[i]);
    }
    #endif
    
    /* Use results to prevent dead code elimination */
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += array_data[i];
    }
    
    printf("Result checksum: %f\n", sum + (float)builtin_result);
    return (int)(sum * 1000);
}
