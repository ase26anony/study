/* test_multi_operand.c - Test program for 10/11-operand instruction expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Enable architecture-specific intrinsics */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

#ifdef __ARM_ARCH
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

#ifdef __PPC64__
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Fused multiply-add chain with 10 operands */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands (5 inputs, 5 modifies, 1 output) */
    __asm__ volatile (
        /* Template doesn't matter much - we care about operand count */
        "mov %[out], %[in1] \n\t"
        "add %[out], %[in2] \n\t"
        "add %[out], %[in3] \n\t"
        "add %[out], %[in4] \n\t"
        "add %[out], %[in5]"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e),
          "0" (result), "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    return result + j + k; /* Use all 11 inputs */
}

#ifdef __x86_64__
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __mmask8 m) {
    /* _mm512_mask3_fmadd_round_pd has 6 explicit operands + implicit rounding control */
    __m512d t1 = _mm512_mask3_fmadd_round_pd(a, b, c, m, _MM_FROUND_TO_NEAREST_INT);
    __m512d t2 = _mm512_mask3_fmadd_round_pd(t1, d, e, m, _MM_FROUND_TO_NEAREST_INT);
    
    /* Chain more operations */
    __m512d t3 = _mm512_fmadd_round_pd(t2, a, b, _MM_FROUND_TO_NEAREST_INT);
    __m512d t4 = _mm512_fmadd_round_pd(t3, c, d, _MM_FROUND_TO_NEAREST_INT);
    
    return _mm512_add_pd(t4, e);
}

/* AVX-512 gather with many parameters */
static inline __m512i avx512_gather_multi(__m512i index, __m512i scale,
                                          void const *base, __mmask8 mask) {
    /* _mm512_mask_i64gather_epi64 has 6 operands */
    return _mm512_mask_i64gather_epi64(_mm512_setzero_si512(), mask,
                                       index, base, scale, 1);
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 multi-operand intrinsic simulation */
static inline svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                          svint32_t d, svint32_t e, svint32_t f) {
    /* Simulate complex operation with lane selection */
    svint32_t t1 = svmla_lane_s32(a, b, c, 0);
    svint32_t t2 = svmla_lane_s32(t1, d, e, 2);
    svint32_t t3 = svdot_lane_s32(t2, f, a, 1);
    
    /* Additional operations to increase operand count */
    svint32_t t4 = svmla_lane_s32(t3, b, c, 3);
    return svmla_lane_s32(t4, d, e, 1);
}
#endif

/* NEON multi-lane operations */
static inline int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                        int32x4_t d, int32x4_t e) {
    /* vmla_lane has 3 vector operands + lane index */
    int32x4_t t1 = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    int32x4_t t2 = vmlaq_lane_s32(t1, d, vget_high_s32(e), 1);
    int32x4_t t3 = vmlaq_lane_s32(t2, b, vget_low_s32(a), 0);
    return vmlaq_lane_s32(t3, c, vget_high_s32(d), 1);
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(int *ptr, int expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_ACQUIRE;
    
    /* __atomic_compare_exchange has 6 parameters */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return expected;
}

/* Decimal floating-point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                                _Decimal128 c, _Decimal128 d) {
    /* Chain multiple decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(t1, c);
    _Decimal128 t3 = __bid128_div(t2, d);
    _Decimal128 t4 = __bid128_fma(a, b, t3);
    
    return __bid128_add(t4, __bid128_sub(c, d));
}
#endif

/* Bit-field operations across multiple words */
static inline uint64_t bitfield_multi_operand(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e) {
    /* Complex bit-field insertion/extraction */
    uint64_t result = a;
    
    /* Insert multiple bit fields */
    result = (result & ~0xFF00FF00FF00FF00ULL) | 
             ((b & 0xFF00FF00FF00FF00ULL) >> 8);
    result = (result & ~0x00FF00FF00FF00FFULL) | 
             ((c & 0x00FF00FF00FF00FFULL) << 8);
    result = (result & ~0xFFFF0000FFFF0000ULL) | 
             ((d & 0xFFFF0000FFFF0000ULL) >> 16);
    result = (result & ~0x0000FFFF0000FFFFULL) | 
             ((e & 0x0000FFFF0000FFFFULL) << 16);
    
    return result;
}

int main() {
    int result = 0;
    
    /* Test 1: Complex arithmetic expression with 10 operands */
    int expr_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    
    /* Test 2: Inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test 3: Atomic operation with multiple parameters */
    int atomic_var = 42;
    int atomic_result = atomic_multi_operand(&atomic_var, 42, 100);
    result += atomic_result;
    
    /* Test 4: Bit-field operations */
    uint64_t bitfield_result = bitfield_multi_operand(0xFFFFFFFFFFFFFFFFULL,
                                                      0xAAAAAAAAAAAAAAAAULL,
                                                      0x5555555555555555ULL,
                                                      0xCCCCCCCCCCCCCCCCULL,
                                                      0x3333333333333333ULL);
    result += (int)bitfield_result;
    
#ifdef __x86_64__
    /* Test 5: AVX-512 operations */
    __m512d avx_a = _mm512_set1_pd(1.0);
    __m512d avx_b = _mm512_set1_pd(2.0);
    __m512d avx_c = _mm512_set1_pd(3.0);
    __m512d avx_d = _mm512_set1_pd(4.0);
    __m512d avx_e = _mm512_set1_pd(5.0);
    __mmask8 mask = 0xFF;
    
    __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c, avx_d, avx_e, mask);
    double avx_sum[8];
    _mm512_storeu_pd(avx_sum, avx_result);
    result += (int)avx_sum[0];
    
    /* Test 6: AVX-512 gather */
    __m512i gather_idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i gather_scale = _mm512_set1_epi64(8);
    uint64_t gather_base[64];
    for (int i = 0; i < 64; i++) gather_base[i] = i;
    
    __m512i gather_result = avx512_gather_multi(gather_idx, gather_scale, gather_base, mask);
    uint64_t gather_sum[8];
    _mm512_storeu_epi64(gather_sum, gather_result);
    result += (int)gather_sum[0];
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    /* Test 7: SVE operations */
    svint32_t sve_a = svdup_s32(1);
    svint32_t sve_b = svdup_s32(2);
    svint32_t sve_c = svdup_s32(3);
    svint32_t sve_d = svdup_s32(4);
    svint32_t sve_e = svdup_s32(5);
    svint32_t sve_f = svdup_s32(6);
    
    svint32_t sve_result = sve_multi_operand(sve_a, sve_b, sve_c, sve_d, sve_e, sve_f);
    int32_t sve_sum[16];
    svst1_s32(svptrue_b32(), sve_sum, sve_result);
    result += sve_sum[0];
#endif

    /* Test 8: NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    
    int32x4_t neon_result = neon_multi_lane(neon_a, neon_b, neon_c, neon_d, neon_e);
    int32_t neon_sum[4];
    vst1q_s32(neon_sum, neon_result);
    result += neon_sum[0];
#endif

#ifdef __DECIMAL_BID_FORMAT__
    /* Test 9: Decimal floating-point operations */
    _Decimal128 dec_a = 1.0dl;
    _Decimal128 dec_b = 2.0dl;
    _Decimal128 dec_c = 3.0dl;
    _Decimal128 dec_d = 4.0dl;
    
    _Decimal128 dec_result = decimal_multi_operand(dec_a, dec_b, dec_c, dec_d);
    result += (int)__bid128_to_string(NULL, dec_result)[0];
#endif

    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
