/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define HAS_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#endif

#if defined(__ARM_ARCH) || defined(__aarch64__)
#define HAS_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#endif

#if defined(__powerpc__) || defined(__PPC__)
#define HAS_PPC 1
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
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    asm volatile (
        /* Template doesn't matter much - we just need the operand count */
        "mov %[out], %[in1] \n\t"
        "add %[out], %[in2] \n\t"
        "add %[out], %[in3] \n\t"
        "add %[out], %[in4] \n\t"
        "add %[out], %[in5] \n\t"
        "add %[out], %[in6] \n\t"
        "add %[out], %[in7] \n\t"
        "add %[out], %[in8] \n\t"
        "add %[out], %[in9] \n\t"
        "add %[out], %[in10]"
        : [out] "=r" (result)
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
          [in4] "r" (d), [in5] "r" (e), [in6] "r" (f),
          [in7] "r" (g), [in8] "r" (h), [in9] "r" (i),
          [in10] "r" (j)
        : "cc"
    );
    
    return result + k; /* Total of 11 operands used */
}

#ifdef HAS_X86
/* AVX-512 intrinsic that uses many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __m512d f,
                                           __mmask8 k, int round) {
    /* AVX-512 masked FMA with rounding control - potentially 8+ operands */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    
    /* This intrinsic uses mask, 3 source vectors, and rounding control */
    return _mm512_mask_add_pd(t1, k, t2, _mm512_set1_pd(1.0));
}

/* Use _mm512_mask3_fmadd_round_pd which has 6 explicit operands + implicit ones */
static inline __m512d avx512_complex_fma(__m512d a, __m512d b, __m512d c,
                                         __mmask8 k, int round) {
    /* This builtin expands to an instruction with multiple operands */
    return _mm512_mask_fmadd_round_pd(a, k, b, c, round);
}
#endif

#ifdef HAS_ARM
/* ARM SVE2 intrinsic with lane selection - potentially many operands */
#ifdef __ARM_FEATURE_SVE
static inline svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                       svint32_t d, svint32_t e, svint32_t f,
                                       svint32_t g, uint64_t lane) {
    /* Chain multiple SVE operations that might combine */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane);
    svint32_t t2 = svmla_lane_s32(d, e, f, lane);
    return svadd_s32(t1, t2);
}
#endif

/* NEON intrinsic with multiple vector registers */
static inline int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b,
                                           int32x4_t c, int32x4_t d,
                                           int32x4_t e, int32x4_t f) {
    /* vmla_lane with multiple registers */
    int32x4_t t1 = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    int32x4_t t2 = vmlaq_lane_s32(d, e, vget_low_s32(f), 1);
    return vaddq_s32(t1, t2);
}
#endif

/* Atomic built-in with many parameters */
static int atomic_multi_operand(int *ptr, int *expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, expands to complex RTL */
    __atomic_compare_exchange(ptr, expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return *expected;
}

/* Decimal floating-point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                         _Decimal128 c, _Decimal128 d) {
    /* Some decimal FP builtins have many operands */
    return __bid128_add(a, __bid128_mul(b, __bid128_add(c, d)));
}
#endif

int main() {
    uint64_t result = 0;
    
    /* 1. Complex arithmetic expression with 10 variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += complex_expression(vars[0], vars[1], vars[2], vars[3], vars[4],
                                 vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* 2. Inline assembly with 11 operands */
    result += multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* 3. Atomic operation with multiple parameters */
    int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    atomic_multi_operand(&atomic_var, &expected, desired);
    result += atomic_var;
    
#ifdef HAS_X86
    /* 4. AVX-512 intrinsics */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __m512d avx_f = _mm512_set1_pd(6.0);
        
        __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                                  avx_d, avx_e, avx_f,
                                                  0xFF, _MM_FROUND_TO_NEAREST_INT);
        
        /* Extract a value to use in result */
        double avx_vals[8];
        _mm512_storeu_pd(avx_vals, avx_result);
        result += (uint64_t)avx_vals[0];
    }
#endif
    
#ifdef HAS_ARM
    /* 5. NEON intrinsics */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    
    int32x4_t neon_result = neon_multi_operand(neon_a, neon_b, neon_c,
                                               neon_d, neon_e, neon_f);
    
    int32_t neon_vals[4];
    vst1q_s32(neon_vals, neon_result);
    result += neon_vals[0];
#endif
    
    /* 6. Another complex expression mixing operations */
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10, x11 = 11;
    
    /* Expression that might be combined into multi-operand form */
    int expr_result = (x1 * x2) + (x3 * x4) + (x5 * x6) + 
                      (x7 * x8) + (x9 * x10) + x11;
    result += expr_result;
    
    printf("Result: %lu\n", (unsigned long)result);
    
    /* Return non-zero to indicate success */
    return result != 0 ? 0 : 1;
}
