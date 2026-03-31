/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define X86_TARGET 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define ARM_TARGET 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc64__) || defined(__powerpc__)
#define PPC_TARGET 1
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
static inline void multi_operand_asm(uint64_t *out, uint64_t a, uint64_t b,
                                     uint64_t c, uint64_t d, uint64_t e,
                                     uint64_t f, uint64_t g, uint64_t h,
                                     uint64_t i, uint64_t j) {
    /* Inline assembly with exactly 11 operands to force expansion */
    asm volatile (
        "/* 11-operand asm block */\n\t"
        "add %[out], %[a], %[b]\n\t"
        "add %[out], %[out], %[c]\n\t"
        "add %[out], %[out], %[d]\n\t"
        "add %[out], %[out], %[e]\n\t"
        "add %[out], %[out], %[f]\n\t"
        "add %[out], %[out], %[g]\n\t"
        "add %[out], %[out], %[h]\n\t"
        "add %[out], %[out], %[i]\n\t"
        "add %[out], %[out], %[j]"
        : [out] "=r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
}

#if X86_TARGET
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __mmask8 k) {
    /* This could expand to a multi-operand instruction with:
       - 3 source vectors (a, b, c)
       - 1 destination vector
       - 1 mask register
       - 1 rounding control
       - 1 exception control
       Total: 7+ operands, might combine with others */
    __m512d t1 = _mm512_mask3_fmadd_pd(a, b, c, k);
    __m512d t2 = _mm512_mask_fmadd_pd(d, k, e, t1);
    return t2;
}

/* Complex AVX-512 expression with many operands */
static inline __m512i avx512_permute_combo(__m512i a, __m512i b, __m512i c,
                                           __m512i d, __m512i e, __m512i f) {
    /* Multiple operations that might combine */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    __m512i t4 = _mm512_add_epi32(t1, t2);
    return _mm512_add_epi32(t3, t4);
}
#endif

#if ARM_TARGET
#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsic with lane selection - potentially many operands */
static inline svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                       svint32_t d, svint32_t e, svint32_t f,
                                       uint64_t lane1, uint64_t lane2) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane1);
    svint32_t t2 = svmla_lane_s32(d, e, f, lane2);
    return svadd_s32(t1, t2);
}
#endif

/* NEON complex multiply-accumulate with lane */
static inline int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                        int32x4_t d, int32x4_t e, int32x4_t f,
                                        int lane) {
    /* Multiple operations that might combine */
    int32x4_t t1 = vmlaq_lane_s32(a, b, c, lane);
    int32x4_t t2 = vmlaq_lane_s32(d, e, f, lane);
    return vaddq_s32(t1, t2);
}
#endif

#if PPC_TARGET
/* PowerPC Altivec complex permute and compute */
static inline vector signed int ppc_multi_operand(vector signed int a,
                                                  vector signed int b,
                                                  vector signed int c,
                                                  vector signed int d,
                                                  vector signed int e,
                                                  vector signed int f) {
    /* Multiple vector operations */
    vector signed int t1 = vec_add(a, b);
    vector signed int t2 = vec_add(c, d);
    vector signed int t3 = vec_add(e, f);
    vector signed int t4 = vec_add(t1, t2);
    return vec_add(t3, t4);
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(uint64_t *ptr, uint64_t *expected,
                                       uint64_t desired) {
    /* __atomic_compare_exchange has 6 parameters which might expand
       to a multi-operand instruction on some architectures */
    int weak = 0;
    return __atomic_compare_exchange(ptr, expected, &desired,
                                     weak, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Decimal floating-point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                         _Decimal128 c, _Decimal128 d) {
    /* Complex decimal operation chain */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    return __bid128_add(t1, t2);
}
#endif

int main() {
    int result = 0;
    
    /* Test complex expression with 10 variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += complex_expression(vars[0], vars[1], vars[2], vars[3], vars[4],
                                 vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_result = 0;
    multi_operand_asm(&asm_result, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += (int)asm_result;
    
    /* Test atomic operation */
    uint64_t atomic_var = 42;
    uint64_t expected = 42;
    uint64_t desired = 43;
    result += atomic_multi_operand(&atomic_var, &expected, desired);
    
#if X86_TARGET
    /* Test AVX-512 operations if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __mmask8 mask = 0xFF;
        
        __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                                  avx_d, avx_e, mask);
        double avx_sum = _mm512_reduce_add_pd(avx_result);
        result += (int)avx_sum;
    }
#endif
    
#if ARM_TARGET
    /* Test NEON operations */
    int32x4_t neon_a = {1, 2, 3, 4};
    int32x4_t neon_b = {5, 6, 7, 8};
    int32x4_t neon_c = {9, 10, 11, 12};
    int32x4_t neon_d = {13, 14, 15, 16};
    int32x4_t neon_e = {17, 18, 19, 20};
    int32x4_t neon_f = {21, 22, 23, 24};
    
    int32x4_t neon_result = neon_multi_lane(neon_a, neon_b, neon_c,
                                            neon_d, neon_e, neon_f, 1);
    result += vgetq_lane_s32(neon_result, 0);
#endif
    
    /* Additional complex expression to encourage instruction combining */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int k = 11, l = 12, m = 13, n = 14, o = 15;
    
    /* Very complex expression that might trigger multi-operand combining */
    int complex_result = 
        a * b + c * d + e * f + g * h + i * j +
        k * l + m * n + o * a + b * c + d * e;
    
    result += complex_result;
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
