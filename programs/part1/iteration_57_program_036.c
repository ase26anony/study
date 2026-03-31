/* Test program to trigger 10/11-operand instruction expansion in GCC optabs */
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

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result1, result2, result3, result4, result5;
    
    /* Inline assembly with 11 operands (5 outputs + 6 inputs) */
    asm volatile (
        /* Simulated multi-operand instruction pattern */
        "mov %0, %5\n\t"           /* result1 = a */
        "add %0, %0, %6\n\t"       /* result1 += b */
        "mov %1, %7\n\t"           /* result2 = c */
        "add %1, %1, %8\n\t"       /* result2 += d */
        "mov %2, %9\n\t"           /* result3 = e */
        "add %2, %2, %10\n\t"      /* result3 += f */
        "mov %3, %11\n\t"          /* result4 = g */
        "add %3, %3, %12\n\t"      /* result4 += h */
        "mov %4, %13\n\t"          /* result5 = i */
        "add %4, %4, %14\n\t"      /* result5 += j */
        /* The template itself doesn't matter; operand count triggers expansion */
        : "=r"(result1), "=r"(result2), "=r"(result3), "=r"(result4), "=r"(result5)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
          "r"(k)  /* 11th operand */
        : "cc"
    );
    
    return result1 + result2 + result3 + result4 + result5 + k;
}

#ifdef __x86_64__
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __mmask8 k) {
    /* FMA with mask, rounding control - potentially expands to multi-operand */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, t1);
    
    /* Masked operation with multiple operands */
    return _mm512_mask_blend_pd(k, a, t2);
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
                                       svint32_t d, svint32_t e, svint32_t f,
                                       uint32_t lane1, uint32_t lane2) {
    /* Complex SVE pattern that might require many operands */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane1);
    svint32_t t2 = svmla_lane_s32(d, e, f, lane2);
    return svadd_s32(t1, t2);
}
#endif

/* NEON intrinsic sequence */
static inline int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b,
                                           int32x4_t c, int32x4_t d) {
    /* vmla_lane with multiple operands */
    int32x4_t t1 = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    int32x4_t t2 = vmlaq_lane_s32(t1, d, vget_high_s32(c), 0);
    return vaddq_s32(t2, b);
}
#endif

/* Atomic built-in with many parameters */
static inline int atomic_multi_operand(int64_t *ptr, int64_t *expected,
                                       int64_t desired) {
    int64_t old_exp = *expected;
    int result = __atomic_compare_exchange(ptr, expected, &desired,
                                           0, /* weak */
                                           __ATOMIC_SEQ_CST,
                                           __ATOMIC_SEQ_CST);
    return result ? (int)old_exp : (int)*expected;
}

/* Decimal float built-in (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                                _Decimal128 c, _Decimal128 d) {
    /* Chain of decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    return __bid128_add(t1, t2);
}
#endif

int main() {
    int result = 0;
    
    /* Test complex expression with 10 operands */
    int expr_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
#ifdef __x86_64__
    /* Test AVX-512 operations */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_vec1 = _mm512_set1_pd(1.0);
        __m512d avx_vec2 = _mm512_set1_pd(2.0);
        __m512d avx_vec3 = _mm512_set1_pd(3.0);
        __m512d avx_vec4 = _mm512_set1_pd(4.0);
        __m512d avx_vec5 = _mm512_set1_pd(5.0);
        
        __m512d avx_res = avx512_multi_operand(avx_vec1, avx_vec2, avx_vec3,
                                               avx_vec4, avx_vec5, 0xFF);
        double avx_sum = _mm512_reduce_add_pd(avx_res);
        result += (int)avx_sum;
    }
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    /* Test SVE operations if available */
    svint32_t sve_vec1 = svdup_s32(1);
    svint32_t sve_vec2 = svdup_s32(2);
    svint32_t sve_vec3 = svdup_s32(3);
    svint32_t sve_vec4 = svdup_s32(4);
    svint32_t sve_vec5 = svdup_s32(5);
    svint32_t sve_vec6 = svdup_s32(6);
    
    svint32_t sve_res = sve_multi_lane(sve_vec1, sve_vec2, sve_vec3,
                                       sve_vec4, sve_vec5, sve_vec6, 0, 1);
    /* Extract first element */
    int32_t sve_first;
    svst1_s32(svptrue_b32(), &sve_first, sve_res);
    result += sve_first;
#endif
    
    /* Test NEON operations */
    int32x4_t neon_vec1 = vdupq_n_s32(1);
    int32x4_t neon_vec2 = vdupq_n_s32(2);
    int32x4_t neon_vec3 = vdupq_n_s32(3);
    int32x4_t neon_vec4 = vdupq_n_s32(4);
    
    int32x4_t neon_res = neon_multi_operand(neon_vec1, neon_vec2,
                                            neon_vec3, neon_vec4);
    result += vgetq_lane_s32(neon_res, 0);
#endif
    
    /* Test atomic operation */
    int64_t atomic_var = 42;
    int64_t expected = 42;
    int64_t desired = 100;
    
    int atomic_result = atomic_multi_operand(&atomic_var, &expected, desired);
    result += atomic_result;
    
    /* Create a complex bitfield expression */
    struct {
        uint64_t a : 5;
        uint64_t b : 7;
        uint64_t c : 10;
        uint64_t d : 12;
        uint64_t e : 15;
        uint64_t f : 15;
    } bitfield = {1, 2, 3, 4, 5, 6};
    
    /* Complex bitfield manipulation that might expand to multi-operand */
    uint64_t bitfield_result = ((uint64_t)bitfield.a << 0) |
                               ((uint64_t)bitfield.b << 5) |
                               ((uint64_t)bitfield.c << 12) |
                               ((uint64_t)bitfield.d << 22) |
                               ((uint64_t)bitfield.e << 34) |
                               ((uint64_t)bitfield.f << 49);
    result += (int)bitfield_result;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
