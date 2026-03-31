/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define HAS_X86 1
#include <immintrin.h>
#include <x86intrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#define HAS_ARM 1
#include <arm_neon.h>
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#elif defined(__powerpc__) || defined(__PPC__)
#define HAS_PPC 1
#include <altivec.h>
#endif

/* Atomic operations header */
#include <stdatomic.h>

/* Complex expression to force instruction combining */
static inline int complex_expression_10_operands(int a, int b, int c, int d, int e,
                                                  int f, int g, int h, int i, int j) {
    /* This expression might be combined into a multi-operand instruction */
    return a * b + c * d + e * f + g * h + i * j;
}

static inline int complex_expression_11_operands(int a, int b, int c, int d, int e,
                                                  int f, int g, int h, int i, int j, int k) {
    /* 11-operand expression */
    return a * b + c * d + e * f + g * h + i * j + k;
}

/* Function using inline assembly with exactly 11 operands */
static int inline_asm_11_operands(int a, int b, int c, int d, int e,
                                  int f, int g, int h, int i, int j, int k) {
    int result;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    __asm__ volatile (
        /* Simple template - the important part is the operand count */
        "add %[res], %[a], %[b] \n\t"
        "add %[res], %[res], %[c] \n\t"
        "add %[res], %[res], %[d] \n\t"
        "add %[res], %[res], %[e] \n\t"
        "add %[res], %[res], %[f] \n\t"
        "add %[res], %[res], %[g] \n\t"
        "add %[res], %[res], %[h] \n\t"
        "add %[res], %[res], %[i] \n\t"
        "add %[res], %[res], %[j] \n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

#ifdef HAS_X86
/* AVX-512 operations with many operands */
static __m512d avx512_multi_operand_test(__m512d a, __m512d b, __m512d c,
                                         __m512d d, __m512d e, __m512d f,
                                         __m512d g, __m512d h, __m512d i,
                                         __m512d j, __mmask8 k) {
    /* Chain of FMA operations that might be combined */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    __m512d t3 = _mm512_fmadd_pd(g, h, i);
    
    /* Masked operation with many operands */
    __m512d result = _mm512_mask_fmadd_pd(t1, k, t2, t3, j);
    
    return result;
}

/* AVX-512 intrinsic with many parameters (10 operands) */
static __m512i avx512_10_operand_intrinsic(__m512i a, __m512i b, __m512i c,
                                           __m512i d, __m512i e, __m512i f,
                                           __m512i g, __m512i h, __m512i i,
                                           __mmask16 mask) {
    /* Complex permute and blend operation */
    __m512i t1 = _mm512_add_epi32(a, b);
    __m512i t2 = _mm512_add_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(e, f);
    __m512i t4 = _mm512_add_epi32(g, h);
    
    /* Blend with mask - this might expand to multi-operand instruction */
    __m512i result = _mm512_mask_blend_epi32(mask, t1, t2);
    result = _mm512_add_epi32(result, t3);
    result = _mm512_add_epi32(result, t4);
    result = _mm512_add_epi32(result, i);
    
    return result;
}
#endif

#ifdef HAS_ARM
/* ARM SVE2 multi-operand intrinsic simulation */
#ifdef __ARM_FEATURE_SVE
static svint32_t sve_multi_operand_test(svint32_t a, svint32_t b, svint32_t c,
                                        svint32_t d, svint32_t e, svint32_t f,
                                        svint32_t g, svint32_t h, svint32_t i,
                                        svint32_t j, svint32_t k) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_s32(a, b, c);
    svint32_t t2 = svmla_s32(d, e, f);
    svint32_t t3 = svmla_s32(g, h, i);
    
    /* Combine results - might trigger multi-operand expansion */
    svint32_t result = svadd_s32(t1, t2);
    result = svadd_s32(result, t3);
    result = svadd_s32(result, j);
    result = svadd_s32(result, k);
    
    return result;
}
#endif

/* ARM NEON multi-operand operations */
static int32x4_t neon_multi_operand_test(int32x4_t a, int32x4_t b, int32x4_t c,
                                         int32x4_t d, int32x4_t e, int32x4_t f,
                                         int32x4_t g, int32x4_t h, int32x4_t i,
                                         int32x4_t j) {
    /* Chain of MLA (multiply-accumulate) operations */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vmlaq_s32(g, h, i);
    
    /* Combine - might be optimized into multi-operand instruction */
    int32x4_t result = vaddq_s32(t1, t2);
    result = vaddq_s32(result, t3);
    result = vaddq_s32(result, j);
    
    return result;
}
#endif

#ifdef HAS_PPC
/* PowerPC Altivec/VSX multi-operand operations */
static vector int ppc_multi_operand_test(vector int a, vector int b, vector int c,
                                         vector int d, vector int e, vector int f,
                                         vector int g, vector int h, vector int i,
                                         vector int j) {
    /* Complex Altivec operation chain */
    vector int t1 = vec_madd(a, b, c);
    vector int t2 = vec_madd(d, e, f);
    vector int t3 = vec_madd(g, h, i);
    
    /* Combine results */
    vector int result = vec_add(t1, t2);
    result = vec_add(result, t3);
    result = vec_add(result, j);
    
    return result;
}
#endif

/* Atomic operation with many parameters */
static int atomic_multi_operand_test(_Atomic int* ptr, int expected, int desired) {
    int old_value = expected;
    
    /* __atomic_compare_exchange has 6 parameters, which might expand
       to a multi-operand instruction on some architectures */
    __atomic_compare_exchange(ptr, &old_value, &desired,
                              0, /* weak */
                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return old_value;
}

/* Decimal floating-point built-in (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_operand_test(_Decimal128 a, _Decimal128 b,
                                              _Decimal128 c, _Decimal128 d,
                                              _Decimal128 e, _Decimal128 f,
                                              _Decimal128 g, _Decimal128 h,
                                              _Decimal128 i, _Decimal128 j) {
    /* Chain of decimal operations */
    _Decimal128 t1 = __bid128_add(a, b, 0);
    _Decimal128 t2 = __bid128_mul(c, d, 0);
    _Decimal128 t3 = __bid128_add(e, f, 0);
    _Decimal128 t4 = __bid128_mul(g, h, 0);
    
    _Decimal128 result = __bid128_add(t1, t2, 0);
    result = __bid128_add(result, t3, 0);
    result = __bid128_add(result, t4, 0);
    result = __bid128_add(result, i, 0);
    result = __bid128_add(result, j, 0);
    
    return result;
}
#endif

int main() {
    int result = 0;
    
    /* Test 10-operand complex expression */
    result += complex_expression_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test 11-operand complex expression */
    result += complex_expression_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test inline assembly with 11 operands */
    result += inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
#ifdef HAS_X86
    /* AVX-512 tests */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_vec1 = _mm512_set1_pd(1.0);
        __m512d avx_vec2 = _mm512_set1_pd(2.0);
        __m512d avx_vec3 = _mm512_set1_pd(3.0);
        __m512d avx_vec4 = _mm512_set1_pd(4.0);
        __m512d avx_vec5 = _mm512_set1_pd(5.0);
        __m512d avx_vec6 = _mm512_set1_pd(6.0);
        __m512d avx_vec7 = _mm512_set1_pd(7.0);
        __m512d avx_vec8 = _mm512_set1_pd(8.0);
        __m512d avx_vec9 = _mm512_set1_pd(9.0);
        __m512d avx_vec10 = _mm512_set1_pd(10.0);
        
        __m512d avx_result = avx512_multi_operand_test(
            avx_vec1, avx_vec2, avx_vec3, avx_vec4, avx_vec5,
            avx_vec6, avx_vec7, avx_vec8, avx_vec9, avx_vec10, 0xFF);
        
        /* Extract a scalar from vector to prevent dead code elimination */
        double avx_scalar = _mm512_cvtsd_f64(_mm512_castpd512_pd128(avx_result));
        result += (int)avx_scalar;
    }
#endif
    
#ifdef HAS_ARM
    /* ARM NEON tests */
    int32x4_t neon_vec1 = vdupq_n_s32(1);
    int32x4_t neon_vec2 = vdupq_n_s32(2);
    int32x4_t neon_vec3 = vdupq_n_s32(3);
    int32x4_t neon_vec4 = vdupq_n_s32(4);
    int32x4_t neon_vec5 = vdupq_n_s32(5);
    int32x4_t neon_vec6 = vdupq_n_s32(6);
    int32x4_t neon_vec7 = vdupq_n_s32(7);
    int32x4_t neon_vec8 = vdupq_n_s32(8);
    int32x4_t neon_vec9 = vdupq_n_s32(9);
    int32x4_t neon_vec10 = vdupq_n_s32(10);
    
    int32x4_t neon_result = neon_multi_operand_test(
        neon_vec1, neon_vec2, neon_vec3, neon_vec4, neon_vec5,
        neon_vec6, neon_vec7, neon_vec8, neon_vec9, neon_vec10);
    
    /* Extract scalar */
    result += vgetq_lane_s32(neon_result, 0);
#endif
    
    /* Atomic operation test */
    _Atomic int atomic_var = 42;
    result += atomic_multi_operand_test(&atomic_var, 42, 100);
    
    /* Additional complex expression to encourage instruction combining */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    /* This complex expression might be combined into multi-operand instruction */
    int complex_result = a * b + c * d + e * f + g * h + i * j + k +
                         (a + b) * (c + d) + (e + f) * (g + h) +
                         (i * j) - (k * a) + (b * c) - (d * e);
    
    result += complex_result;
    
    printf("Result: %d\n", result);
    return result;
}
