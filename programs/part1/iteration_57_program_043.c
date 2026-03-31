/* test_multi_operand.c - Test program for 10/11-operand RTL expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture-specific headers */
#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>
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
static inline uint64_t inline_asm_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline asm with 11 operands - forces RTL expansion */
    __asm__ volatile (
        "/* 11-operand test */\n\t"
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

#ifdef __x86_64__
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __mmask8 m) {
    /* FMA with rounding control - potentially expands to multi-operand */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, t1);
    
    /* Masked operation with multiple operands */
    return _mm512_mask_blend_pd(m, a, t2);
}

/* AVX-512 intrinsic that takes many parameters */
static inline __m512i avx512_ternary_logic(__m512i a, __m512i b, __m512i c,
                                           __m512i d, __m512i e) {
    /* Chain of operations that might combine */
    __m512i t1 = _mm512_and_si512(a, b);
    __m512i t2 = _mm512_or_si512(c, d);
    __m512i t3 = _mm512_xor_si512(t1, t2);
    return _mm512_add_epi64(t3, e);
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsic with lane selection - potentially many operands */
static inline svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                       svint32_t d, svint32_t e, uint32_t lane) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane);
    svint32_t t2 = svmla_lane_s32(d, e, t1, lane);
    return svadd_s32_z(svptrue_b32(), t1, t2);
}
#endif

/* NEON intrinsic with multiple operands */
static inline int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b, int32x4_t c,
                                           int32x4_t d, int32x4_t e) {
    /* vmlaq_lane with multiple registers */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, 0);
    int32x4_t t2 = vmlaq_laneq_s32(d, e, t1, 1);
    return vaddq_s32(t1, t2);
}
#endif

/* Atomic built-in with many parameters */
static inline int atomic_multi_operand(int *ptr, int *expected, int desired) {
    int weak = 0;
    int success = __atomic_compare_exchange(ptr, expected, &desired,
                                            weak, __ATOMIC_SEQ_CST,
                                            __ATOMIC_SEQ_CST);
    return success;
}

/* Decimal floating-point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                                _Decimal128 c, _Decimal128 d) {
    /* Chain of decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    return __bid128_add(t1, t2);
}
#endif

/* Main test driver */
int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10 operands */
    int expr_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    printf("Complex expression result: %d\n", expr_result);
    
    /* Test 2: Inline assembly with 11 operands */
    uint64_t asm_result = inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    printf("Inline asm result: %lu\n", asm_result);
    
    /* Test 3: Architecture-specific intrinsics */
#ifdef __x86_64__
    __m512d avx_vec1 = _mm512_set1_pd(1.0);
    __m512d avx_vec2 = _mm512_set1_pd(2.0);
    __m512d avx_vec3 = _mm512_set1_pd(3.0);
    __m512d avx_vec4 = _mm512_set1_pd(4.0);
    __m512d avx_vec5 = _mm512_set1_pd(5.0);
    __mmask8 mask = 0xFF;
    
    __m512d avx_result = avx512_multi_operand(avx_vec1, avx_vec2, avx_vec3,
                                              avx_vec4, avx_vec5, mask);
    double avx_sum = _mm512_reduce_add_pd(avx_result);
    result += (int)avx_sum;
    printf("AVX-512 result: %f\n", avx_sum);
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    /* SVE test - will compile on SVE-capable ARM */
    svint32_t sve_a = svdup_s32(1);
    svint32_t sve_b = svdup_s32(2);
    svint32_t sve_c = svdup_s32(3);
    svint32_t sve_d = svdup_s32(4);
    svint32_t sve_e = svdup_s32(5);
    
    svint32_t sve_result = sve_multi_lane(sve_a, sve_b, sve_c, sve_d, sve_e, 0);
    /* Extract first element */
    int32_t sve_first = svlastb_s32(svptrue_b32(), sve_result);
    result += sve_first;
    printf("SVE result: %d\n", sve_first);
#endif
#endif
    
    /* Test 4: Atomic operation with many parameters */
    int atomic_var = 100;
    int expected = 100;
    int desired = 200;
    int atomic_result = atomic_multi_operand(&atomic_var, &expected, desired);
    result += atomic_result;
    printf("Atomic result: %d (atomic_var=%d)\n", atomic_result, atomic_var);
    
    /* Test 5: Vector element extraction and recombination */
    uint64_t vectors[5] = {0x1111111111111111, 0x2222222222222222,
                          0x3333333333333333, 0x4444444444444444,
                          0x5555555555555555};
    
    /* Complex bit manipulation across multiple words */
    uint64_t combined = (vectors[0] & 0xFF) |
                       ((vectors[1] & 0xFF00) << 8) |
                       ((vectors[2] & 0xFF0000) << 16) |
                       ((vectors[3] & 0xFF000000) << 24) |
                       ((vectors[4] & 0xFF00000000) << 32);
    result += (int)combined;
    printf("Bitfield combined: 0x%lx\n", combined);
    
    return result;
}
