/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define X86_ARCH 1
#include <x86intrin.h>
#include <immintrin.h>
#endif

#if defined(__aarch64__) || defined(__arm__)
#define ARM_ARCH 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

#if defined(__powerpc__) || defined(__PPC__)
#define PPC_ARCH 1
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single multi-operand
       instruction at high optimization levels */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline void multi_operand_asm(void) {
    int64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int64_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int64_t out = 0;
    
    /* Inline assembly with exactly 11 operands to trigger the case */
    __asm__ volatile (
        "add %[out], %[in1], %[in2]\n\t"
        "add %[out], %[out], %[in3]\n\t"
        "add %[out], %[out], %[in4]\n\t"
        "add %[out], %[out], %[in5]\n\t"
        "add %[out], %[out], %[in6]\n\t"
        "add %[out], %[out], %[in7]\n\t"
        "add %[out], %[out], %[in8]\n\t"
        "add %[out], %[out], %[in9]\n\t"
        "add %[out], %[out], %[in10]"
        : [out] "=r" (out)
        : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3),
          [in4] "r" (in4), [in5] "r" (in5), [in6] "r" (in6),
          [in7] "r" (in7), [in8] "r" (in8), [in9] "r" (in9),
          [in10] "r" (in10)
        : "cc"
    );
    
    (void)out; /* Prevent unused variable warning */
}

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(void) {
    int64_t val = 0, expected = 0, desired = 42;
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which might expand
       to a multi-operand instruction on some architectures */
    __atomic_compare_exchange(&val, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return (int)val;
}

#if defined(X86_ARCH) && defined(__AVX512F__)
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __m512d f,
                                           __mmask8 k1, __mmask8 k2) {
    /* Chain multiple FMA operations that might combine */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    
    /* Masked operation with multiple source registers */
    __m512d result = _mm512_mask_add_pd(t1, k1, t2, _mm512_set1_pd(1.0));
    result = _mm512_mask_mul_pd(result, k2, result, _mm512_set1_pd(2.0));
    
    return result;
}

/* AVX-512 instruction with rounding control - up to 11 operands */
static inline __m512d avx512_rounding_multi_operand(__m512d a, __m512d b,
                                                    __m512d c, __mmask8 k) {
    /* This intrinsic takes: a, b, c, k, rounding - potentially 5 operands
       but the RTL expansion might need more */
    return _mm512_mask3_fmadd_round_pd(a, b, c, k, _MM_FROUND_TO_NEAREST_INT);
}
#endif

#if defined(ARM_ARCH)
/* ARM NEON/SVE operations with lane selection */
static inline int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b,
                                        int32x4_t c, int32x4_t d,
                                        int32x4_t e, int32x4_t f) {
    /* Multiple vector operations that might combine */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    
    /* Lane extraction and insertion operations */
    int32_t lane0 = vgetq_lane_s32(t1, 0);
    int32_t lane1 = vgetq_lane_s32(t1, 1);
    int32_t lane2 = vgetq_lane_s32(t2, 2);
    int32_t lane3 = vgetq_lane_s32(t2, 3);
    
    int32x4_t result = vsetq_lane_s32(lane0 + lane1, t1, 0);
    result = vsetq_lane_s32(lane2 + lane3, result, 1);
    
    return result;
}

#ifdef __ARM_FEATURE_SVE
/* SVE2 multi-operand intrinsic simulation */
static inline svint32_t sve_multi_operand(svint32_t a, svint32_t b,
                                          svint32_t c, svint32_t d,
                                          svint32_t e, svint32_t f,
                                          svint32_t g, svint32_t h) {
    /* Complex SVE expression that might require many operands */
    svint32_t t1 = svmla_s32_x(svptrue_b32(), a, b, c);
    svint32_t t2 = svmla_s32_x(svptrue_b32(), d, e, f);
    svint32_t result = svadd_s32_x(svptrue_b32(), t1, t2);
    result = svmla_s32_x(svptrue_b32(), result, g, h);
    
    return result;
}
#endif
#endif

#if defined(PPC_ARCH)
/* PowerPC Altivec/VSX operations */
static inline vector signed int ppc_multi_operand(vector signed int a,
                                                  vector signed int b,
                                                  vector signed int c,
                                                  vector signed int d,
                                                  vector signed int e) {
    /* Complex permute and compute */
    vector signed int t1 = vec_madd(a, b, c);
    vector signed int t2 = vec_madd(d, e, a);
    
    /* Permute with multiple source vectors */
    vector unsigned char perm = {0,1,2,3, 16,17,18,19, 4,5,6,7, 20,21,22,23};
    vector signed int result = vec_perm(t1, t2, perm);
    
    return result;
}
#endif

/* Decimal floating-point built-in with many operands (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                                _Decimal128 c, _Decimal128 d) {
    /* Chain decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    _Decimal128 result = __bid128_add(t1, t2);
    
    return result;
}
#endif

int main(void) {
    int result = 0;
    
    /* 1. Complex arithmetic expression with 10+ variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += complex_expression(vars[0], vars[1], vars[2], vars[3], vars[4],
                                 vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* 2. Inline assembly with 11 operands */
    multi_operand_asm();
    
    /* 3. Atomic operation with multiple parameters */
    result += atomic_multi_operand();
    
#if defined(X86_ARCH) && defined(__AVX512F__)
    /* 4. AVX-512 operations */
    __m512d avx_a = _mm512_set1_pd(1.0);
    __m512d avx_b = _mm512_set1_pd(2.0);
    __m512d avx_c = _mm512_set1_pd(3.0);
    __m512d avx_d = _mm512_set1_pd(4.0);
    __m512d avx_e = _mm512_set1_pd(5.0);
    __m512d avx_f = _mm512_set1_pd(6.0);
    
    __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                              avx_d, avx_e, avx_f,
                                              0xFF, 0x0F);
    
    /* Extract a scalar from the result */
    double avx_scalar = _mm512_cvtsd_f64(_mm512_castpd512_pd128(avx_result));
    result += (int)avx_scalar;
#endif

#if defined(ARM_ARCH)
    /* 5. ARM NEON operations */
    int32x4_t neon_a = {1, 2, 3, 4};
    int32x4_t neon_b = {5, 6, 7, 8};
    int32x4_t neon_c = {9, 10, 11, 12};
    int32x4_t neon_d = {13, 14, 15, 16};
    int32x4_t neon_e = {17, 18, 19, 20};
    int32x4_t neon_f = {21, 22, 23, 24};
    
    int32x4_t neon_result = neon_multi_lane(neon_a, neon_b, neon_c,
                                            neon_d, neon_e, neon_f);
    
    /* Sum elements */
    int32x2_t sum2 = vadd_s32(vget_low_s32(neon_result),
                              vget_high_s32(neon_result));
    int32_t neon_sum = vget_lane_s32(vpadd_s32(sum2, sum2), 0);
    result += neon_sum;
#endif

#if defined(PPC_ARCH)
    /* 6. PowerPC operations */
    vector signed int ppc_a = {1, 2, 3, 4};
    vector signed int ppc_b = {5, 6, 7, 8};
    vector signed int ppc_c = {9, 10, 11, 12};
    vector signed int ppc_d = {13, 14, 15, 16};
    vector signed int ppc_e = {17, 18, 19, 20};
    
    vector signed int ppc_result = ppc_multi_operand(ppc_a, ppc_b, ppc_c,
                                                     ppc_d, ppc_e);
    
    /* Extract and sum */
    int ppc_sum = 0;
    for (int i = 0; i < 4; i++) {
        ppc_sum += ((int*)&ppc_result)[i];
    }
    result += ppc_sum;
#endif

    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
