/* Test for covering 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* For x86 AVX-512 */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

/* For ARM NEON */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* GCC vector extensions for generic testing */
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Hot function attribute to encourage complex instruction patterns */
__attribute__((hot, noinline))
void test_vector_operations() {
    /* Approach 1: Complex vector operations using GCC extensions */
    v8sf a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf b = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf c = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf d = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    
    /* Complex expression that might generate multi-operand FMA patterns */
    v8sf result = a * b + c * d + a * c + b * d;
    
    /* Use result to prevent elimination */
    volatile v8sf sink = result;
    
#ifdef __AVX512F__
    /* Approach 2: AVX-512 masked operations with many operands */
    __m512 avx_a = _mm512_set1_ps(1.0f);
    __m512 avx_b = _mm512_set1_ps(2.0f);
    __m512 avx_c = _mm512_set1_ps(3.0f);
    __m512 avx_d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;  /* Alternating mask */
    
    /* Complex masked FMA operation - may expand to many operands */
    __m512 avx_result = _mm512_mask_fmadd_ps(avx_a, mask, avx_b, avx_c);
    avx_result = _mm512_mask_add_ps(avx_result, mask, avx_result, avx_d);
    
    volatile __m512 avx_sink = avx_result;
#endif

#ifdef __ARM_NEON
    /* Approach 3: ARM NEON multi-vector operations */
    float32x4_t neon_a = vdupq_n_f32(1.0f);
    float32x4_t neon_b = vdupq_n_f32(2.0f);
    float32x4_t neon_c = vdupq_n_f32(3.0f);
    float32x4_t neon_d = vdupq_n_f32(4.0f);
    
    /* Complex FMA-like pattern using multiple intrinsics */
    float32x4_t neon_result = vmlaq_f32(neon_a, neon_b, neon_c);
    neon_result = vmlaq_f32(neon_result, neon_c, neon_d);
    
    volatile float32x4_t neon_sink = neon_result;
#endif
}

/* Approach 4: Complex reduction with many temporaries */
__attribute__((hot, noinline))
float test_complex_reduction(float* arr, int n) {
    v8sf acc1 = {0};
    v8sf acc2 = {0};
    v8sf acc3 = {0};
    v8sf acc4 = {0};
    
    for (int i = 0; i < n; i += 32) {
        /* Load and process multiple vectors */
        v8sf v1 = *(v8sf*)(arr + i);
        v8sf v2 = *(v8sf*)(arr + i + 8);
        v8sf v3 = *(v8sf*)(arr + i + 16);
        v8sf v4 = *(v8sf*)(arr + i + 24);
        
        /* Complex reduction pattern that might use multi-operand instructions */
        acc1 = acc1 + v1 * v2 + v3 * v4;
        acc2 = acc2 + v2 * v3 + v4 * v1;
        acc3 = acc3 + v3 * v4 + v1 * v2;
        acc4 = acc4 + v4 * v1 + v2 * v3;
    }
    
    /* Horizontal reduction */
    float sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += acc1[i] + acc2[i] + acc3[i] + acc4[i];
    }
    return sum;
}

/* Approach 5: Inline assembly with many operands */
__attribute__((hot, noinline))
void test_multi_operand_asm() {
    /* Create many distinct variables to use as operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long op11 = 11, op12 = 12;
    
    /* Extended asm with 11 operands (10 inputs + 1 output) */
    asm volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (op1)
        : [in1] "r" (op2), [in2] "r" (op3), [in3] "r" (op4),
          [in4] "r" (op5), [in5] "r" (op6), [in6] "r" (op7),
          [in7] "r" (op8), [in8] "r" (op9), [in9] "r" (op10),
          [in10] "r" (op11)
        : "cc"
    );
    
    volatile long sink = op1;
}

/* Approach 6: Built-in complex math operations */
__attribute__((hot, noinline))
double test_builtin_fma_chain() {
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0, e = 5.0;
    double f = 6.0, g = 7.0, h = 8.0, i = 9.0, j = 10.0;
    
    /* Chain of FMA operations that might be combined */
    double result = __builtin_fma(a, b, c);
    result = __builtin_fma(result, d, e);
    result = __builtin_fma(result, f, g);
    result = __builtin_fma(result, h, i);
    result = __builtin_fma(result, j, a);
    
    return result;
}

int main() {
    /* Initialize test data */
    float arr[1024];
    for (int i = 0; i < 1024; i++) {
        arr[i] = (i % 10) * 0.1f;
    }
    
    /* Execute all test approaches */
    test_vector_operations();
    
    float reduction_result = test_complex_reduction(arr, 1024);
    printf("Reduction result: %f\n", reduction_result);
    
    test_multi_operand_asm();
    
    double fma_result = test_builtin_fma_chain();
    printf("FMA chain result: %f\n", fma_result);
    
    return 0;
}
