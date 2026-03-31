/* test_multi_operand_expansion.c
 * Compile with: gcc -O3 -march=native -ftree-vectorize -funsafe-math-optimizations -fdump-rtl-expand -fdump-rtl-combine test_multi_operand_expansion.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent dead code elimination */
volatile int sink;

/* Complex expression that might combine into multi-operand instruction */
int complex_expression(int a, int b, int c, int d, int e, 
                       int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single multi-operand
     * instruction at high optimization levels */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Inline assembly with exactly 11 operands */
void inline_asm_11_operands(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int64_t out;
    
    /* 11 operands: 10 inputs, 1 output */
    asm volatile (
        "/* 11-operand asm block */\n\t"
        "mov %[out], %[in1]\n\t"
        "add %[out], %[in2]\n\t"
        "add %[out], %[in3]\n\t"
        "add %[out], %[in4]\n\t"
        "add %[out], %[in5]\n\t"
        "add %[out], %[in6]\n\t"
        "add %[out], %[in7]\n\t"
        "add %[out], %[in8]\n\t"
        "add %[out], %[in9]\n\t"
        "add %[out], %[in10]"
        : [out] "=r" (out)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [in5] "r" (in5), [in6] "r" (in6),
          [in7] "r" (in7), [in8] "r" (in8), [in9] "r" (in9),
          [in10] "r" (in10)
        : "cc"
    );
    
    sink = out;
}

/* Inline assembly with exactly 10 operands */
void inline_asm_10_operands(void) {
    int32_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int32_t in6 = 6, in7 = 7, in8 = 8, in9 = 9;
    int32_t out1, out2;
    
    /* 10 operands: 8 inputs, 2 outputs */
    asm volatile (
        "/* 10-operand asm block */\n\t"
        "mov %[out1], %[in1]\n\t"
        "mov %[out2], %[in2]\n\t"
        "add %[out1], %[in3]\n\t"
        "add %[out2], %[in4]\n\t"
        "imul %[out1], %[in5]\n\t"
        "imul %[out2], %[in6]\n\t"
        "add %[out1], %[in7]\n\t"
        "add %[out2], %[in8]\n\t"
        "add %[out1], %[in9]"
        : [out1] "=&r" (out1), [out2] "=&r" (out2)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [in5] "r" (in5), [in6] "r" (in6),
          [in7] "r" (in7), [in8] "r" (in8), [in9] "r" (in9)
        : "cc"
    );
    
    sink = out1 + out2;
}

/* Atomic built-in with many parameters */
void atomic_multi_operand(void) {
    int64_t atomic_var = 0;
    int64_t expected = 0;
    int64_t desired = 42;
    int weak = 0;
    
    /* __atomic_compare_exchange has 6 parameters, which might expand
     * to more operands when considering memory ordering */
    __atomic_compare_exchange(&atomic_var, &expected, &desired,
                              weak, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    sink = atomic_var;
}

#ifdef __x86_64__
#include <immintrin.h>

/* AVX-512 intrinsics that can use many operands */
void avx512_multi_operand(void) {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __mmask8 mask = 0xFF;
    
    /* Some AVX-512 instructions can have many operands:
     * dest, src1, src2, src3, mask, rounding control */
    __m512d result = _mm512_mask_fmadd_pd(a, mask, b, c);
    
    /* Force use of result */
    double temp[8];
    _mm512_storeu_pd(temp, result);
    sink = (int)temp[0];
}

/* Complex FMA chain that might combine */
void avx512_complex_fma(void) {
    __m512d v1 = _mm512_set1_pd(1.0);
    __m512d v2 = _mm512_set1_pd(2.0);
    __m512d v3 = _mm512_set1_pd(3.0);
    __m512d v4 = _mm512_set1_pd(4.0);
    __m512d v5 = _mm512_set1_pd(5.0);
    __m512d v6 = _mm512_set1_pd(6.0);
    __m512d v7 = _mm512_set1_pd(7.0);
    __m512d v8 = _mm512_set1_pd(8.0);
    __m512d v9 = _mm512_set1_pd(9.0);
    __m512d v10 = _mm512_set1_pd(10.0);
    
    /* Complex expression that might be optimized into
     * multiple FMA operations or combined */
    __m512d result = _mm512_fmadd_pd(v1, v2, 
                       _mm512_fmadd_pd(v3, v4,
                         _mm512_fmadd_pd(v5, v6,
                           _mm512_fmadd_pd(v7, v8,
                             _mm512_mul_pd(v9, v10)))));
    
    double temp[8];
    _mm512_storeu_pd(temp, result);
    sink = (int)temp[0];
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>
#include <arm_acle.h>

/* ARM SVE/SVE2 style multi-operand operation simulation */
void arm_multi_operand(void) {
    /* Using NEON intrinsics - actual SVE intrinsics would require
     * -march=armv8-a+sve and appropriate headers */
    int32x4_t a = vdupq_n_s32(1);
    int32x4_t b = vdupq_n_s32(2);
    int32x4_t c = vdupq_n_s32(3);
    int32x4_t d = vdupq_n_s32(4);
    int32x4_t e = vdupq_n_s32(5);
    int32x4_t f = vdupq_n_s32(6);
    int32x4_t g = vdupq_n_s32(7);
    int32x4_t h = vdupq_n_s32(8);
    int32x4_t i = vdupq_n_s32(9);
    int32x4_t j = vdupq_n_s32(10);
    
    /* Complex chain of operations */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vmlaq_s32(g, h, i);
    int32x4_t result = vaddq_s32(vaddq_s32(t1, t2), vaddq_s32(t3, j));
    
    int32_t temp[4];
    vst1q_s32(temp, result);
    sink = temp[0];
}
#endif

#ifdef __powerpc64__
#include <altivec.h>

/* PowerPC VSX/Altivec operations */
void powerpc_multi_operand(void) {
    vector float a = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float b = {5.0f, 6.0f, 7.0f, 8.0f};
    vector float c = {9.0f, 10.0f, 11.0f, 12.0f};
    vector float d = {13.0f, 14.0f, 15.0f, 16.0f};
    vector float e = {17.0f, 18.0f, 19.0f, 20.0f};
    
    /* Complex permute and compute */
    vector float t1 = vec_madd(a, b, c);
    vector float t2 = vec_madd(d, e, a);
    vector float result = vec_add(t1, t2);
    
    float temp[4];
    vec_st(result, 0, temp);
    sink = (int)temp[0];
}
#endif

/* Bit-field operations that might combine */
uint64_t bitfield_multi_operand(uint64_t word) {
    /* Multiple bit-field extractions and insertions */
    uint64_t a = (word >> 0) & 0xFF;
    uint64_t b = (word >> 8) & 0xFF;
    uint64_t c = (word >> 16) & 0xFF;
    uint64_t d = (word >> 24) & 0xFF;
    uint64_t e = (word >> 32) & 0xFF;
    uint64_t f = (word >> 40) & 0xFF;
    uint64_t g = (word >> 48) & 0xFF;
    uint64_t h = (word >> 56) & 0xFF;
    
    /* Recombine in different order */
    return (h << 0) | (g << 8) | (f << 16) | (e << 24) |
           (d << 32) | (c << 40) | (b << 48) | (a << 56);
}

int main(void) {
    int result = 0;
    
    /* Test complex expression with 10 operands */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test inline assembly paths */
    inline_asm_10_operands();
    inline_asm_11_operands();
    
    /* Test atomic operation */
    atomic_multi_operand();
    
    /* Test architecture-specific intrinsics */
#ifdef __x86_64__
    avx512_multi_operand();
    avx512_complex_fma();
#endif
    
#ifdef __aarch64__
    arm_multi_operand();
#endif
    
#ifdef __powerpc64__
    powerpc_multi_operand();
#endif
    
    /* Test bit-field operations */
    result += bitfield_multi_operand(0x0123456789ABCDEFULL);
    
    printf("Result: %d (sink: %d)\n", result, sink);
    
    return 0;
}
