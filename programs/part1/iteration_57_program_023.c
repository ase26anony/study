/* test_multi_operand_expansion.c
 * Compile with: gcc -O3 -march=native -ftree-vectorize -funsafe-math-optimizations -fdump-rtl-expand -fdump-rtl-combine test_multi_operand_expansion.c -o test
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

#ifdef __ARM_ARCH
#include <arm_neon.h>
#include <arm_sve.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression_10_operands(int a, int b, int c, int d, int e,
                                                  int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single multi-operand instruction
     * during RTL combine pass at high optimization levels */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static inline void inline_asm_11_operands(uint64_t *out, uint64_t a, uint64_t b,
                                          uint64_t c, uint64_t d, uint64_t e,
                                          uint64_t f, uint64_t g, uint64_t h,
                                          uint64_t i, uint64_t j) {
    /* Inline assembly with 11 operands to force optabs expansion */
    __asm__ volatile (
        "/* 11-operand assembly template */\n\t"
        "mov %0, %1\n\t"
        "add %0, %2\n\t"
        "add %0, %3\n\t"
        "add %0, %4\n\t"
        "add %0, %5\n\t"
        "add %0, %6\n\t"
        "add %0, %7\n\t"
        "add %0, %8\n\t"
        "add %0, %9\n\t"
        "add %0, %10"
        : "=r" (*out)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
}

#ifdef __x86_64__
/* AVX-512 intrinsics that can generate multi-operand instructions */
static inline __m512d avx512_multi_operand_fma(__m512d a, __m512d b, __m512d c,
                                               __m512d d, __m512d e, __mmask8 k) {
    /* AVX-512 masked FMA with rounding control - can use many operands */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, t1);
    
    /* This intrinsic uses mask, 3 source operands, and rounding mode */
    __m512d result = _mm512_maskz_fmadd_round_pd(k, a, b, c, _MM_FROUND_TO_NEAREST_INT);
    
    return _mm512_add_pd(t2, result);
}

/* Complex vector operation that might combine */
static inline __m512i avx512_complex_operation(__m512i v0, __m512i v1, __m512i v2,
                                               __m512i v3, __m512i v4, __m512i v5,
                                               __m512i v6, __m512i v7) {
    /* Chain of operations that might be combined */
    __m512i t0 = _mm512_add_epi32(v0, v1);
    __m512i t1 = _mm512_mullo_epi32(v2, v3);
    __m512i t2 = _mm512_slli_epi32(v4, 3);
    __m512i t3 = _mm512_and_si512(v5, v6);
    
    /* Complex expression that could trigger multi-operand expansion */
    return _mm512_add_epi32(_mm512_add_epi32(t0, t1),
                           _mm512_add_epi32(t2, t3));
}
#endif

#ifdef __ARM_ARCH
/* ARM SVE2 intrinsics with lane selection - can use many operands */
#ifdef __ARM_FEATURE_SVE
static inline svint32_t sve_multi_operand_mla(svint32_t a, svint32_t b, svint32_t c,
                                              svint32_t d, svint32_t e, svint32_t f,
                                              svint32_t g, svint32_t h) {
    /* SVE2 MLA with lane selection uses multiple vector registers and indices */
    svint32_t t1 = svmla_lane_s32(a, b, c, 0);
    svint32_t t2 = svmla_lane_s32(d, e, f, 2);
    svint32_t t3 = svmla_lane_s32(g, h, t1, 1);
    
    return svadd_s32(t2, t3);
}
#endif

/* ARM Neon complex operation */
static inline int32x4_t neon_multi_operand_operation(int32x4_t a, int32x4_t b,
                                                     int32x4_t c, int32x4_t d,
                                                     int32x4_t e, int32x4_t f) {
    /* vmla_lane with multiple vectors */
    int32x4_t t1 = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    int32x4_t t2 = vmlaq_lane_s32(d, e, vget_high_s32(f), 1);
    
    return vaddq_s32(t1, t2);
}
#endif

/* Atomic built-in with many parameters */
static inline int atomic_compare_exchange_multi_operand(int64_t *ptr, int64_t *expected,
                                                        int64_t desired) {
    int64_t old_exp = *expected;
    /* __atomic_compare_exchange with 6 parameters */
    return __atomic_compare_exchange(ptr, expected, &desired, 0,
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Decimal floating-point built-in (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                                _Decimal128 c, _Decimal128 d) {
    /* Decimal floating-point operations that might expand to multi-operand insns */
    return __bid128_add(a, __bid128_mul(b, __bid128_add(c, d)));
}
#endif

int main() {
    int result = 0;
    
    /* 1. Complex expression with 10 operands */
    int expr_result = complex_expression_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    
    /* 2. Inline assembly with 11 operands */
    uint64_t asm_result = 0;
    inline_asm_11_operands(&asm_result, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
#ifdef __x86_64__
    /* 3. AVX-512 multi-operand intrinsics */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_vec1 = _mm512_set1_pd(1.0);
        __m512d avx_vec2 = _mm512_set1_pd(2.0);
        __m512d avx_vec3 = _mm512_set1_pd(3.0);
        __m512d avx_vec4 = _mm512_set1_pd(4.0);
        __m512d avx_vec5 = _mm512_set1_pd(5.0);
        
        __m512d avx_result = avx512_multi_operand_fma(avx_vec1, avx_vec2, avx_vec3,
                                                      avx_vec4, avx_vec5, 0xFF);
        
        /* Extract a scalar result */
        double avx_scalar[8];
        _mm512_storeu_pd(avx_scalar, avx_result);
        result += (int)avx_scalar[0];
    }
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    /* 4. ARM SVE multi-operand intrinsics */
    svint32_t sve_vec1 = svdup_s32(1);
    svint32_t sve_vec2 = svdup_s32(2);
    svint32_t sve_vec3 = svdup_s32(3);
    svint32_t sve_vec4 = svdup_s32(4);
    svint32_t sve_vec5 = svdup_s32(5);
    svint32_t sve_vec6 = svdup_s32(6);
    svint32_t sve_vec7 = svdup_s32(7);
    svint32_t sve_vec8 = svdup_s32(8);
    
    svint32_t sve_result = sve_multi_operand_mla(sve_vec1, sve_vec2, sve_vec3,
                                                 sve_vec4, sve_vec5, sve_vec6,
                                                 sve_vec7, sve_vec8);
    
    int32_t sve_scalar[16];
    svst1_s32(svptrue_b32(), sve_scalar, sve_result);
    result += sve_scalar[0];
#endif
#endif
    
    /* 5. Atomic operation with multiple parameters */
    int64_t atomic_var = 42;
    int64_t expected = 42;
    int64_t desired = 100;
    
    int atomic_result = atomic_compare_exchange_multi_operand(&atomic_var, &expected, desired);
    result += atomic_result;
    
    /* 6. Another complex expression that might combine */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    /* Expression with 11 variables that might be combined */
    int combined_result = a * b + c * d + e * f + g * h + i * j + k;
    result += combined_result;
    
    /* 7. Vector operations that might generate multi-operand RTL */
#ifdef __SSE2__
    __m128i sse_vec1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i sse_vec2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i sse_vec3 = _mm_set_epi32(9, 10, 11, 12);
    __m128i sse_vec4 = _mm_set_epi32(13, 14, 15, 16);
    
    /* Chain of operations */
    __m128i sse_tmp1 = _mm_add_epi32(sse_vec1, sse_vec2);
    __m128i sse_tmp2 = _mm_mullo_epi32(sse_vec3, sse_vec4);
    __m128i sse_tmp3 = _mm_slli_epi32(sse_tmp1, 2);
    __m128i sse_result = _mm_add_epi32(sse_tmp2, sse_tmp3);
    
    int sse_scalar[4];
    _mm_storeu_si128((__m128i*)sse_scalar, sse_result);
    result += sse_scalar[0];
#endif
    
    printf("Result: %d\n", result);
    return result;
}
