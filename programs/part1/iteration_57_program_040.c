/* test_multi_operand.c - Test program for 10/11-operand instruction expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define TARGET_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define TARGET_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* For atomic operations */
#include <stdatomic.h>

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single instruction
       with many operands during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline void asm_11_operands(uint64_t *out, uint64_t a, uint64_t b,
                                   uint64_t c, uint64_t d, uint64_t e,
                                   uint64_t f, uint64_t g, uint64_t h,
                                   uint64_t i, uint64_t j) {
    /* Inline assembly with exactly 11 operands to force expansion */
    __asm__ volatile (
        "/* 11-operand assembly block */\n\t"
        "mov %[out], %[a]\n\t"
        "add %[out], %[b]\n\t"
        "add %[out], %[c]\n\t"
        "add %[out], %[d]\n\t"
        "add %[out], %[e]\n\t"
        "add %[out], %[f]\n\t"
        "add %[out], %[g]\n\t"
        "add %[out], %[h]\n\t"
        "add %[out], %[i]\n\t"
        "add %[out], %[j]"
        : [out] "=r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
}

/* Function using inline assembly with 10 operands */
static inline void asm_10_operands(uint64_t *out, uint64_t a, uint64_t b,
                                   uint64_t c, uint64_t d, uint64_t e,
                                   uint64_t f, uint64_t g, uint64_t h,
                                   uint64_t i) {
    __asm__ volatile (
        "/* 10-operand assembly block */\n\t"
        "mov %[out], %[a]\n\t"
        "imul %[out], %[b]\n\t"
        "add %[out], %[c]\n\t"
        "add %[out], %[d]\n\t"
        "add %[out], %[e]\n\t"
        "add %[out], %[f]\n\t"
        "add %[out], %[g]\n\t"
        "add %[out], %[h]\n\t"
        "add %[out], %[i]"
        : [out] "=r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i)
        : "cc"
    );
}

#if TARGET_X86
/* AVX-512 intrinsics with many operands */
static __m512i avx512_multi_operand(__m512i a, __m512i b, __m512i c,
                                    __m512i d, __m512i e, __m512i f,
                                    __mmask16 k1, __mmask16 k2) {
    /* Complex sequence that might use multi-operand instructions */
    __m512i t1 = _mm512_mask_add_epi32(a, k1, b, c);
    __m512i t2 = _mm512_mask_mul_epi32(t1, k2, d, e);
    return _mm512_mask_add_epi32(t2, k1, t2, f);
}

/* FMA with rounding control - up to 11 operands */
static __m512d avx512_fma_multi_op(__m512d a, __m512d b, __m512d c,
                                   __m512d d, __m512d e, __m512d f,
                                   __mmask8 k, int rc) {
    /* Chain of FMA operations */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    return _mm512_mask_add_pd(t1, k, t1, t2);
}
#endif

#if TARGET_ARM
/* ARM NEON/SVE intrinsics with lane operations */
#ifdef __ARM_FEATURE_SVE
/* SVE2 multi-operand intrinsics */
static svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                svint32_t d, svint32_t e, svint32_t f,
                                svint32_t g, uint64_t lane1, uint64_t lane2) {
    /* Complex SVE operation with multiple lane selections */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane1);
    svint32_t t2 = svmla_lane_s32(d, e, f, lane2);
    return svadd_s32(t1, t2);
}
#endif

/* ARM NEON multi-operand operations */
static int32x4_t neon_multi_op(int32x4_t a, int32x4_t b, int32x4_t c,
                               int32x4_t d, int32x4_t e, int32x4_t f,
                               int32x4_t g, int32x4_t h) {
    /* Complex NEON expression */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vaddq_s32(t1, t2);
    return vmlaq_s32(t3, g, h);
}
#endif

#if TARGET_PPC
/* PowerPC Altivec/VSX multi-operand operations */
static vector signed int ppc_multi_op(vector signed int a, vector signed int b,
                                      vector signed int c, vector signed int d,
                                      vector signed int e, vector signed int f,
                                      vector signed int g, vector signed int h) {
    /* Complex Altivec expression */
    vector signed int t1 = vec_madd(a, b, c);
    vector signed int t2 = vec_madd(d, e, f);
    vector signed int t3 = vec_add(t1, t2);
    return vec_madd(t3, g, h);
}
#endif

/* Atomic operation with many parameters */
static int atomic_multi_op(_Atomic int *ptr, int expected, int desired,
                           int mem_order1, int mem_order2, int weak) {
    int result = 0;
    /* __atomic_compare_exchange has 6 parameters, which when expanded
       might become 10+ operands with memory barriers and such */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              mem_order1, mem_order2);
    return result;
}

/* Decimal floating point builtins (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_op(_Decimal128 a, _Decimal128 b,
                                    _Decimal128 c, _Decimal128 d,
                                    _Decimal128 e, _Decimal128 f) {
    /* Chain of decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    _Decimal128 t3 = __bid128_fma(e, f, t1);
    return __bid128_add(t2, t3);
}
#endif

int main() {
    int result = 0;
    
    /* Test complex expression with 10 variables */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    result += complex_expression(a, b, c, d, e, f, g, h, i, j);
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_result;
    asm_11_operands(&asm_result, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test inline assembly with 10 operands */
    uint64_t asm_result2;
    asm_10_operands(&asm_result2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += (int)asm_result2;
    
#if TARGET_X86
    /* Test AVX-512 intrinsics */
    if (__builtin_cpu_supports("avx512f")) {
        __m512i avx_a = _mm512_set1_epi32(1);
        __m512i avx_b = _mm512_set1_epi32(2);
        __m512i avx_c = _mm512_set1_epi32(3);
        __m512i avx_d = _mm512_set1_epi32(4);
        __m512i avx_e = _mm512_set1_epi32(5);
        __m512i avx_f = _mm512_set1_epi32(6);
        
        __mmask16 k1 = 0xAAAA;
        __mmask16 k2 = 0x5555;
        
        __m512i avx_res = avx512_multi_operand(avx_a, avx_b, avx_c,
                                               avx_d, avx_e, avx_f,
                                               k1, k2);
        int avx_sum = _mm512_reduce_add_epi32(avx_res);
        result += avx_sum;
    }
#endif
    
#if TARGET_ARM
    /* Test ARM NEON intrinsics */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    int32x4_t neon_g = vdupq_n_s32(7);
    int32x4_t neon_h = vdupq_n_s32(8);
    
    int32x4_t neon_res = neon_multi_op(neon_a, neon_b, neon_c, neon_d,
                                       neon_e, neon_f, neon_g, neon_h);
    int32_t neon_sum = vaddvq_s32(neon_res);
    result += neon_sum;
#endif
    
    /* Test atomic operation */
    _Atomic int atomic_var = 42;
    result += atomic_multi_op(&atomic_var, 42, 100,
                              __ATOMIC_SEQ_CST, __ATOMIC_RELAXED, 0);
    
    /* Create another complex expression that might be combined */
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10, x11 = 11;
    
    /* This expression has 11 variables and might trigger
       the 11-operand case during RTL expansion */
    int complex_result = x1 * x2 + x3 * x4 + x5 * x6 +
                         x7 * x8 + x9 * x10 + x11;
    result += complex_result;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
