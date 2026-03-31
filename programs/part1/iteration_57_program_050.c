/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Architecture-specific headers */
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__ARM_ARCH) || defined(__aarch64__)
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Chain of operations that could be combined */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline void multi_operand_asm(void) {
    int64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    int64_t f = 6, g = 7, h = 8, i = 9, j = 10;
    int64_t result;
    
    /* Inline assembly with exactly 11 operands */
    __asm__ volatile (
        "/* 11-operand asm block */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    (void)result; /* Prevent unused variable warning */
}

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(void) {
    int64_t val = 42;
    int64_t expected = 42;
    int64_t desired = 43;
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, which may expand further */
    return __atomic_compare_exchange(&val, &expected, &desired, weak,
                                     success_memorder, failure_memorder);
}

#if defined(__x86_64__) || defined(__i386__)
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __m512d f,
                                           __mmask8 k1, __mmask8 k2) {
    /* Complex chain of FMA operations */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    
    /* Masked operation with many parameters */
    return _mm512_mask_add_pd(t1, k1, t2, _mm512_set1_pd(1.0));
}

/* AVX-512 ternary operation with rounding control */
static inline __m512 avx512_ternary_fma(__m512 a, __m512 b, __m512 c,
                                        __m512 d, __m512 e, __m512 f,
                                        __mmask16 k) {
    /* This could potentially expand to a multi-operand instruction */
    __m512 t1 = _mm512_fmadd_ps(a, b, c);
    __m512 t2 = _mm512_fmadd_ps(d, e, f);
    return _mm512_mask_blend_ps(k, t1, t2);
}
#endif

#if defined(__ARM_ARCH) || defined(__aarch64__)
/* ARM NEON/SVE operations with lane selection */
static inline int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                        int32x4_t d, int32x4_t e, int32x4_t f,
                                        int32x4_t g, int32x4_t h) {
    /* Complex sequence that might combine */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vaddq_s32(t1, t2);
    return vmlaq_s32(t3, g, h);
}

#ifdef __ARM_FEATURE_SVE
/* SVE2 operation with multiple vectors and predicates */
static inline svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                          svint32_t d, svint32_t e, svint32_t f,
                                          svint32_t g, svint32_t h,
                                          svbool_t pg) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_s32_z(pg, a, b, c);
    svint32_t t2 = svmla_s32_z(pg, d, e, f);
    return svadd_s32_z(pg, t1, t2);
}
#endif
#endif

#if defined(__powerpc__) || defined(__PPC__)
/* PowerPC Altivec complex permute operation */
static inline vector signed int altivec_multi_operand(vector signed int a,
                                                      vector signed int b,
                                                      vector signed int c,
                                                      vector signed int d,
                                                      vector signed int e,
                                                      vector signed int f) {
    /* Complex Altivec operation that might require many operands */
    vector signed int t1 = vec_madd(a, b, c);
    vector signed int t2 = vec_madd(d, e, f);
    return vec_add(t1, t2);
}
#endif

/* Main driver function */
int main(void) {
    int result = 0;
    
    /* 1. Complex expression with 10 variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += complex_expression(vars[0], vars[1], vars[2], vars[3], vars[4],
                                 vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* 2. Inline assembly with 11 operands */
    multi_operand_asm();
    
    /* 3. Atomic operation with multiple parameters */
    result += atomic_multi_operand();
    
#if defined(__x86_64__) || defined(__i386__)
    /* 4. AVX-512 operations */
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
    
#if defined(__ARM_ARCH) || defined(__aarch64__)
    /* 5. NEON operations */
    int32x4_t neon_a = {1, 2, 3, 4};
    int32x4_t neon_b = {5, 6, 7, 8};
    int32x4_t neon_c = {9, 10, 11, 12};
    int32x4_t neon_d = {13, 14, 15, 16};
    int32x4_t neon_e = {17, 18, 19, 20};
    int32x4_t neon_f = {21, 22, 23, 24};
    int32x4_t neon_g = {25, 26, 27, 28};
    int32x4_t neon_h = {29, 30, 31, 32};
    
    int32x4_t neon_result = neon_multi_lane(neon_a, neon_b, neon_c, neon_d,
                                            neon_e, neon_f, neon_g, neon_h);
    int32_t neon_sum[4];
    vst1q_s32(neon_sum, neon_result);
    result += neon_sum[0];
#endif
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
