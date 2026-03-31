/* Test program to trigger 10/11-operand instruction expansion in GCC optabs */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define TARGET_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#endif

#if defined(__aarch64__) || defined(__arm__)
#define TARGET_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

#if defined(__powerpc__) || defined(__PPC__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single multi-operand
       instruction at high optimization levels */
    return a * b + c * d + e * f + g * h + i * j +
           (a + b) * (c + d) + (e + f) * (g + h);
}

/* Function using inline assembly with 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result1, result2, result3;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    asm volatile (
        /* Template doesn't matter much - we just need 11 operands */
        "mov %[r1], %[a] \n\t"
        "add %[r1], %[r1], %[b] \n\t"
        "add %[r1], %[r1], %[c] \n\t"
        "mov %[r2], %[d] \n\t"
        "add %[r2], %[r2], %[e] \n\t"
        "add %[r2], %[r2], %[f] \n\t"
        "mov %[r3], %[g] \n\t"
        "add %[r3], %[r3], %[h] \n\t"
        "add %[r3], %[r3], %[i] \n\t"
        "add %[r1], %[r1], %[r2] \n\t"
        "add %[r1], %[r1], %[r3] \n\t"
        "add %[r1], %[r1], %[j] \n\t"
        "add %[r1], %[r1], %[k]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1;
}

#ifdef TARGET_X86
/* AVX-512 intrinsic with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __m512d f,
                                           __mmask8 k1, __mmask8 k2) {
    /* FMA with mask and rounding control - potentially expands to many operands */
    __m512d t1 = _mm512_mask3_fmadd_pd(a, b, c, k1);
    __m512d t2 = _mm512_mask3_fmadd_pd(d, e, f, k2);
    return _mm512_add_pd(t1, t2);
}

/* Complex permute operation with many operands */
static inline __m512i avx512_complex_permute(__m512i a, __m512i b, __m512i c,
                                             __m512i d, __m512i e) {
    /* Multi-operand permute pattern */
    __m512i t1 = _mm512_alignr_epi32(a, b, 4);
    __m512i t2 = _mm512_alignr_epi32(c, d, 8);
    __m512i t3 = _mm512_alignr_epi32(t1, t2, 12);
    return _mm512_add_epi32(t3, e);
}
#endif

#ifdef TARGET_ARM
/* ARM SVE2 multi-operand intrinsic simulation */
static inline int32x4_t arm_multi_lane_op(int32x4_t a, int32x4_t b,
                                          int32x4_t c, int32x4_t d,
                                          int32x4_t e, int32x4_t f,
                                          int lane1, int lane2) {
    /* Simulate multi-operand lane operations */
    int32x4_t t1 = vmlaq_laneq_s32(a, b, c, lane1);
    int32x4_t t2 = vmlaq_laneq_s32(d, e, f, lane2);
    return vaddq_s32(t1, t2);
}

/* Complex vector operation with many operands */
static inline int32x4_t arm_complex_vector_op(int32x4_t a, int32x4_t b,
                                              int32x4_t c, int32x4_t d,
                                              int32x4_t e, int32x4_t f,
                                              int32x4_t g, int32x4_t h) {
    /* Chain of operations that might combine */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vmulq_s32(c, d);
    int32x4_t t3 = vmlaq_s32(e, f, g);
    int32x4_t t4 = vaddq_s32(t1, t2);
    return vaddq_s32(t3, t4);
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(int *ptr, int expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange with 6 parameters */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return expected;
}

/* Decimal floating point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_op(_Decimal128 a, _Decimal128 b,
                                           _Decimal128 c, _Decimal128 d) {
    /* Chain of decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    return __bid128_add(t1, t2);
}
#endif

int main() {
    int result = 0;
    
    /* Test complex expression with 10+ variables */
    int vars[12];
    for (int i = 0; i < 12; i++) {
        vars[i] = i + 1;
    }
    
    result += complex_expression(vars[0], vars[1], vars[2], vars[3],
                                 vars[4], vars[5], vars[6], vars[7],
                                 vars[8], vars[9]);
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test atomic operation */
    int atomic_var = 42;
    result += atomic_multi_operand(&atomic_var, 42, 100);
    
#ifdef TARGET_X86
    /* Test AVX-512 operations if available */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __m512d avx_f = _mm512_set1_pd(6.0);
        
        __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                                  avx_d, avx_e, avx_f,
                                                  0xFF, 0xFF);
        double avx_sum[8];
        _mm512_storeu_pd(avx_sum, avx_result);
        result += (int)avx_sum[0];
    }
#endif
    
#ifdef TARGET_ARM
    /* Test ARM vector operations */
    int32x4_t arm_a = vdupq_n_s32(1);
    int32x4_t arm_b = vdupq_n_s32(2);
    int32x4_t arm_c = vdupq_n_s32(3);
    int32x4_t arm_d = vdupq_n_s32(4);
    int32x4_t arm_e = vdupq_n_s32(5);
    int32x4_t arm_f = vdupq_n_s32(6);
    int32x4_t arm_g = vdupq_n_s32(7);
    int32x4_t arm_h = vdupq_n_s32(8);
    
    int32x4_t arm_result = arm_complex_vector_op(arm_a, arm_b, arm_c, arm_d,
                                                 arm_e, arm_f, arm_g, arm_h);
    int32_t arm_sum[4];
    vst1q_s32(arm_sum, arm_result);
    result += arm_sum[0];
#endif
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
