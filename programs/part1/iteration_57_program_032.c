/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture-specific includes */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#include <arm_neon.h>
#endif

#ifdef __powerpc__
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
int complex_expression(int a, int b, int c, int d, int e, 
                       int f, int g, int h, int i, int j, int k) {
    /* This expression might be combined into a single multi-operand instruction
       at high optimization levels */
    return a * b + c * d + e * f + g * h + i * j + k;
}

/* Function using inline assembly with exactly 11 operands */
int inline_asm_11_operands(int a, int b, int c, int d, int e,
                           int f, int g, int h, int i, int j, int k) {
    int result;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    __asm__ volatile (
        "/* 11-operand asm block */\n\t"
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j),
          [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Function using inline assembly with exactly 10 operands */
int inline_asm_10_operands(int a, int b, int c, int d, int e,
                           int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline assembly with 10 operands to force 10-operand expansion */
    __asm__ volatile (
        "/* 10-operand asm block */\n\t"
        "imul %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

#ifdef __x86_64__
/* AVX-512 example that could use many operands */
__m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c, __m512i d,
                             __m512i e, __m512i f, __m512i g, __m512i h,
                             __mmask16 mask, int rounding) {
    /* Complex expression that might combine into multi-operand instruction */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    __m512i t4 = _mm512_add_epi32(g, h);
    
    /* This might get combined at high optimization levels */
    return _mm512_add_epi32(_mm512_add_epi32(t1, t2), 
                           _mm512_add_epi32(t3, t4));
}

/* AVX-512 masked operation with many parameters */
__m512d avx512_masked_fma(__m512d a, __m512d b, __m512d c, 
                          __mmask8 k, int sae) {
    /* _mm512_mask3_fmadd_round_pd has 5 explicit parameters plus implicit ones */
    return _mm512_mask3_fmadd_round_pd(a, b, c, k, sae);
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsic with lane selection - could use many operands */
svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                         svint32_t d, svint32_t e, uint64_t lane1,
                         uint64_t lane2, uint64_t lane3) {
    /* Complex SVE expression */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane1);
    svint32_t t2 = svmla_lane_s32(d, e, t1, lane2);
    return svadd_s32_z(svptrue_b32(), t2, svdup_s32(42));
}
#endif

/* NEON intrinsic with multiple operands */
int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                             int32x4_t d, int32x4_t e, int32x4_t f,
                             int lane) {
    /* vmla_lane with multiple registers */
    int32x4_t t1 = vmlaq_lane_s32(a, b, vget_low_s32(c), lane);
    int32x4_t t2 = vmlaq_lane_s32(d, e, vget_low_s32(f), lane);
    return vaddq_s32(t1, t2);
}
#endif

/* Atomic operation with many parameters */
int atomic_multi_operand(int *ptr, int *expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters */
    return __atomic_compare_exchange(ptr, expected, &desired, weak,
                                     success_memorder, failure_memorder);
}

/* Decimal floating point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
_BID128 bid128_multi_operand(_BID128 a, _BID128 b, _BID128 c,
                             _BID128 d, _BID128 e, unsigned int rmode) {
    /* Chain of decimal operations */
    _BID128 t1 = __bid128_add(a, b, rmode);
    _BID128 t2 = __bid128_add(c, d, rmode);
    _BID128 t3 = __bid128_mul(t1, t2, rmode);
    return __bid128_add(t3, e, rmode);
}
#endif

/* Main driver function */
int main() {
    int result = 0;
    
    /* Test complex expression */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test inline assembly with many operands */
    result += inline_asm_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test atomic operation */
    int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    result += atomic_multi_operand(&atomic_var, &expected, desired);
    
#ifdef __x86_64__
    /* Test AVX-512 if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512i vec_a = _mm512_set1_epi32(1);
        __m512i vec_b = _mm512_set1_epi32(2);
        __m512i vec_c = _mm512_set1_epi32(3);
        __m512i vec_d = _mm512_set1_epi32(4);
        __m512i vec_e = _mm512_set1_epi32(5);
        __m512i vec_f = _mm512_set1_epi32(6);
        __m512i vec_g = _mm512_set1_epi32(7);
        __m512i vec_h = _mm512_set1_epi32(8);
        
        __m512i vec_result = avx512_multi_operand(vec_a, vec_b, vec_c, vec_d,
                                                  vec_e, vec_f, vec_g, vec_h,
                                                  0xFFFF, 0);
        
        /* Extract one element to prevent optimization away */
        int extracted = _mm512_extract_epi32(vec_result, 0);
        result += extracted;
    }
#endif

#ifdef __ARM_ARCH
    /* Test NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    
    int32x4_t neon_result = neon_multi_operand(neon_a, neon_b, neon_c,
                                               neon_d, neon_e, neon_f, 0);
    
    /* Extract one element */
    result += vgetq_lane_s32(neon_result, 0);
#endif

    /* Additional complex arithmetic that might combine */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    /* Fused multiply-add chain - might combine into multi-operand instruction */
    int fmadd_chain = a * b + c * d + e * f + g * h + i * j + k;
    result += fmadd_chain;
    
    /* Bit-field operations across multiple words */
    unsigned int bitfield1 = 0x12345678;
    unsigned int bitfield2 = 0x9ABCDEF0;
    unsigned int bitfield3 = 0x13579BDF;
    unsigned int bitfield4 = 0x2468ACE0;
    
    /* Complex bit manipulation */
    unsigned int combined = ((bitfield1 & 0xFF) << 24) |
                           ((bitfield2 & 0xFF00) << 8) |
                           ((bitfield3 & 0xFF0000) >> 8) |
                           ((bitfield4 & 0xFF000000) >> 24);
    result += combined;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
