/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Enable architecture-specific intrinsics */
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__ARM_ARCH) || defined(__aarch64__)
#include <arm_neon.h>
#include <arm_acle.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Fused multiply-add chain that could be combined */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    /* Inline assembly with 11 operands to force 11-operand expansion */
    __asm__ volatile (
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

#if defined(__x86_64__) || defined(__i386__)
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __mmask8 k) {
    /* This could expand to instruction with mask, rounding control, etc. */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, t1);
    return _mm512_mask_blend_pd(k, a, t2);
}

/* Use AVX-512 ternary operation with mask */
static inline __m512i avx512_ternary_op(__m512i a, __m512i b, __m512i c,
                                        __m512i d, __m512i e, __mmask16 k) {
    /* Multiple operations that might combine */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(t1, t2);
    return _mm512_mask_add_epi32(e, k, t3, _mm512_set1_epi32(1));
}
#endif

#if defined(__ARM_ARCH) && __ARM_ARCH >= 8
/* ARM SVE/SVE2 multi-operand intrinsic simulation */
static inline int32x4_t arm_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                       int32x4_t d, int32x4_t e, int32x4_t f,
                                       int lane) {
    /* Complex sequence that might use multi-operand instructions */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, lane);
    int32x4_t t2 = vmlaq_laneq_s32(d, e, f, lane);
    return vaddq_s32(t1, t2);
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(int *ptr, int *expected, int desired) {
    int weak = 0;
    int success = __atomic_compare_exchange(ptr, expected, &desired,
                                            weak, __ATOMIC_SEQ_CST,
                                            __ATOMIC_SEQ_CST);
    return success;
}

/* Decimal floating-point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_op(_Decimal128 a, _Decimal128 b,
                                           _Decimal128 c, _Decimal128 d) {
    /* Chain of decimal operations */
    return __bid128_add(a, __bid128_mul(b, __bid128_add(c, d)));
}
#endif

int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10 variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += complex_expression(vars[0], vars[1], vars[2], vars[3], vars[4],
                                 vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* Test 2: Inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test 3: Architecture-specific vector operations */
#if defined(__x86_64__) || defined(__i386__)
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_vec1 = _mm512_set1_pd(1.0);
        __m512d avx_vec2 = _mm512_set1_pd(2.0);
        __m512d avx_vec3 = _mm512_set1_pd(3.0);
        __m512d avx_vec4 = _mm512_set1_pd(4.0);
        __m512d avx_vec5 = _mm512_set1_pd(5.0);
        
        __m512d avx_res = avx512_multi_operand(avx_vec1, avx_vec2, avx_vec3,
                                               avx_vec4, avx_vec5, 0xFF);
        double avx_sum[8];
        _mm512_storeu_pd(avx_sum, avx_res);
        result += (int)avx_sum[0];
    }
#endif
    
#if defined(__ARM_ARCH) && __ARM_ARCH >= 8
    int32x4_t arm_vec1 = {1, 2, 3, 4};
    int32x4_t arm_vec2 = {5, 6, 7, 8};
    int32x4_t arm_vec3 = {9, 10, 11, 12};
    int32x4_t arm_vec4 = {13, 14, 15, 16};
    int32x4_t arm_vec5 = {17, 18, 19, 20};
    int32x4_t arm_vec6 = {21, 22, 23, 24};
    
    int32x4_t arm_res = arm_multi_lane(arm_vec1, arm_vec2, arm_vec3,
                                       arm_vec4, arm_vec5, arm_vec6, 1);
    result += vgetq_lane_s32(arm_res, 0);
#endif
    
    /* Test 4: Atomic operation */
    int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    result += atomic_multi_operand(&atomic_var, &expected, desired);
    
    /* Test 5: Bit-field operations across multiple words */
    struct {
        uint64_t a, b, c, d, e;
    } bitfields = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0, 0xFEDCBA98};
    
    /* Complex bit manipulation that might combine */
    uint64_t bit_result = ((bitfields.a & 0xFF) << 56) |
                         ((bitfields.b & 0xFF00) << 40) |
                         ((bitfields.c & 0xFF0000) << 24) |
                         ((bitfields.d & 0xFF000000) << 8) |
                         (bitfields.e & 0xFF00000000);
    result += (int)(bit_result >> 32);
    
    /* Test 6: Vector reduction with many accumulators */
    int accum[10] = {0};
    for (int i = 0; i < 100; i++) {
        accum[0] += i;
        accum[1] += i * 2;
        accum[2] += i * 3;
        accum[3] += i * 4;
        accum[4] += i * 5;
        accum[5] += i * 6;
        accum[6] += i * 7;
        accum[7] += i * 8;
        accum[8] += i * 9;
        accum[9] += i * 10;
    }
    
    for (int i = 0; i < 10; i++) {
        result += accum[i];
    }
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
