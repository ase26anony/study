/* test_multi_operand_expansion.c
 * Compile with: gcc -O3 -march=native -ftree-vectorize -funsafe-math-optimizations -fdump-rtl-expand -fdump-rtl-combine test_multi_operand_expansion.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

#ifdef __ARM_ARCH
#include <arm_neon.h>
#include <arm_acle.h>
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
    /* This expression might be combined into a single instruction
     * with many operands during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    __asm__ volatile (
        "/* Multi-operand test - template doesn't matter */ \n\t"
        "mov %[out], %[in1] \n\t"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i),
          [in10] "r" (j), [in11] "r" (k)
        : "memory"
    );
    
    return result;
}

#ifdef __x86_64__
/* AVX-512 intrinsic with many operands */
static __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                    __m512d d, __m512d e, __mmask8 k) {
    /* _mm512_mask3_fmadd_round_pd has 6 explicit operands + implicit rounding control
     * When combined with other operations, might reach 10+ operands */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, t1);
    
    /* Masked operation with multiple source registers */
    __m512d result = _mm512_mask_blend_pd(k, t1, t2);
    
    /* Chain more operations */
    result = _mm512_add_pd(result, a);
    result = _mm512_mul_pd(result, b);
    
    return result;
}

/* AVX-512 masked gather with many parameters */
static __m512i avx512_gather_test(__m512i index, __m512i src,
                                  __mmask16 mask, void const* base,
                                  int scale) {
    /* _mm512_mask_i32gather_epi32 has many operands when expanded */
    return _mm512_mask_i32gather_epi32(src, mask, index, base, scale);
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsic with lane selection - potentially many operands */
static svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                svint32_t d, svint32_t e, uint64_t lane) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane);
    svint32_t t2 = svmla_lane_s32(d, e, t1, lane);
    
    /* Additional operations to increase operand count */
    t2 = svadd_s32_z(svptrue_b32(), t2, a);
    t2 = svmul_s32_z(svptrue_b32(), t2, b);
    
    return t2;
}
#endif

/* ARM NEON intrinsic with multiple lane operations */
static int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                 int32x4_t d, int32x4_t e) {
    /* vmla_lane_s32 has 4 operands, chain multiple */
    int32x4_t t1 = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    int32x4_t t2 = vmlaq_lane_s32(d, e, vget_high_s32(t1), 1);
    
    /* Additional operations */
    t2 = vaddq_s32(t2, a);
    t2 = vmulq_s32(t2, b);
    
    return t2;
}
#endif

#ifdef __PPC64__
/* PowerPC VSX/Altivec complex permute */
static vector int altivec_complex_op(vector int a, vector int b,
                                     vector int c, vector int d) {
    /* Complex permute and compute */
    vector int t1 = vec_add(a, b);
    vector int t2 = vec_add(c, d);
    vector int t3 = vec_mul(t1, t2);
    
    /* Permute with many operands */
    vector int result = vec_perm(t1, t2, t3);
    
    return result;
}
#endif

/* Atomic operation with many parameters */
static int atomic_multi_operand(int* ptr, int* expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which when expanded
     * with memory ordering might reach higher operand counts */
    return __atomic_compare_exchange(ptr, expected, &desired,
                                     weak, success_memorder,
                                     failure_memorder);
}

/* Decimal floating point built-in (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_op(_Decimal128 a, _Decimal128 b,
                                    _Decimal128 c, _Decimal128 d) {
    /* Chain multiple decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    _Decimal128 result = __bid128_add(t1, t2);
    
    return result;
}
#endif

int main() {
    int result = 0;
    
    /* Test 1: Complex arithmetic expression with 10 variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += complex_expression(vars[0], vars[1], vars[2], vars[3], vars[4],
                                 vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* Test 2: Inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test 3: Atomic operation with many parameters */
    int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    result += atomic_multi_operand(&atomic_var, &expected, desired);
    
#ifdef __x86_64__
    /* Test 4: AVX-512 intrinsics */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __mmask8 mask = 0xFF;
        
        __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                                  avx_d, avx_e, mask);
        
        /* Extract some value from result */
        double avx_sum = _mm512_reduce_add_pd(avx_result);
        result += (int)avx_sum;
    }
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    /* Test 5: SVE intrinsics */
    if (svcntb() > 0) {
        svint32_t sve_a = svdup_s32(1);
        svint32_t sve_b = svdup_s32(2);
        svint32_t sve_c = svdup_s32(3);
        svint32_t sve_d = svdup_s32(4);
        svint32_t sve_e = svdup_s32(5);
        
        svint32_t sve_result = sve_multi_lane(sve_a, sve_b, sve_c,
                                              sve_d, sve_e, 0);
        
        /* Extract sum */
        int32_t sve_sum = svaddv_s32(svptrue_b32(), sve_result);
        result += sve_sum;
    }
#endif
    
    /* Test 6: NEON intrinsics */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    
    int32x4_t neon_result = neon_multi_lane(neon_a, neon_b, neon_c,
                                            neon_d, neon_e);
    
    /* Extract sum */
    int32_t neon_sum = vaddvq_s32(neon_result);
    result += neon_sum;
#endif
    
#ifdef __PPC64__
    /* Test 7: Altivec/VSX intrinsics */
    vector int altivec_a = {1, 2, 3, 4};
    vector int altivec_b = {5, 6, 7, 8};
    vector int altivec_c = {9, 10, 11, 12};
    vector int altivec_d = {13, 14, 15, 16};
    
    vector int altivec_result = altivec_complex_op(altivec_a, altivec_b,
                                                   altivec_c, altivec_d);
    
    /* Extract sum */
    int altivec_sum = 0;
    for (int i = 0; i < 4; i++) {
        altivec_sum += ((int*)&altivec_result)[i];
    }
    result += altivec_sum;
#endif
    
#ifdef __DECIMAL_BID_FORMAT__
    /* Test 8: Decimal floating point */
    _Decimal128 dec_a = 1.0dl;
    _Decimal128 dec_b = 2.0dl;
    _Decimal128 dec_c = 3.0dl;
    _Decimal128 dec_d = 4.0dl;
    
    _Decimal128 dec_result = decimal_multi_op(dec_a, dec_b, dec_c, dec_d);
    result += (int)__bid128_to_string(NULL, dec_result)[0];
#endif
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
