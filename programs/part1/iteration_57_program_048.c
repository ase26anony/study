/* Test program to trigger 10/11-operand instruction expansion in GCC optabs */
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

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single instruction
       with many operands during optimization */
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

/* Atomic built-in with many parameters */
static int atomic_test(int *ptr, int expected, int desired) {
    int weak = 0;
    /* __atomic_compare_exchange has 6 parameters, which when expanded
       with memory order parameters might reach 10+ operands */
    return __atomic_compare_exchange(ptr, &expected, &desired,
                                     weak, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#if HAS_X86
/* AVX-512 masked operation with many operands */
static __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                    __m512d d, __m512d e, __mmask8 k) {
    /* Fused multiply-add with mask and rounding control */
    __m512d result;
    
    /* Chain of operations that might combine */
    result = _mm512_fmadd_pd(a, b, c);
    result = _mm512_fmadd_pd(result, d, e);
    
    /* Masked operation with many parameters */
    result = _mm512_mask_add_pd(result, k, result, a);
    
    return result;
}

/* AVX-512 intrinsic that takes many arguments */
static __m512i avx512_permute2var(__m512i a, __m512i idx, __m512i b,
                                  __mmask64 k1, __mmask64 k2) {
    /* Complex permute operation */
    __m512i tmp = _mm512_permutex2var_epi64(a, idx, b);
    
    /* Additional masked operations */
    tmp = _mm512_maskz_expand_epi64(k1, tmp);
    tmp = _mm512_mask_expand_epi64(tmp, k2, tmp);
    
    return tmp;
}
#endif

#if HAS_ARM
/* ARM SVE/SVE2 style multi-operand operation simulation */
#ifdef __ARM_FEATURE_SVE
static svint32_t sve_multi_operand(svint32_t a, svint32_t b, svint32_t c,
                                   svint32_t d, svint32_t e, svint32_t f,
                                   svbool_t pg) {
    /* Simulate complex SVE operation chain */
    svint32_t result = svmla_s32_x(pg, a, b, c);
    result = svmla_lane_s32(result, d, e, 2);
    result = svadd_s32_x(pg, result, f);
    return result;
}
#endif

/* ARM NEON multi-lane operation */
static int32x4_t neon_multi_lane(int32x4_t a, int32x4_t b, int32x4_t c,
                                 int32x4_t d, int32x4_t e) {
    /* Complex NEON operation chain */
    int32x4_t result = vmlaq_s32(a, b, c);
    result = vmlaq_laneq_s32(result, d, e, 3);
    result = vaddq_s32(result, a);
    return result;
}
#endif

#if HAS_PPC
/* PowerPC VSX/Altivec complex operation */
static vector signed int ppc_multi_operand(vector signed int a,
                                           vector signed int b,
                                           vector signed int c,
                                           vector signed int d,
                                           vector signed int e) {
    /* Complex Altivec operation chain */
    vector signed int result = vec_madd(a, b, c);
    result = vec_add(result, d);
    result = vec_madd(result, e, a);
    return result;
}
#endif

/* Decimal floating-point built-in (if supported) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                         _Decimal128 c, _Decimal128 d) {
    /* Chain of decimal operations */
    _Decimal128 result = __bid128_add(a, b);
    result = __bid128_mul(result, c);
    result = __bid128_div(result, d);
    return result;
}
#endif

int main() {
    int result = 0;
    
    /* Test complex expression with 10 variables */
    int vars[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += complex_expression(vars[0], vars[1], vars[2], vars[3], vars[4],
                                 vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    /* Test inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    
    /* Test atomic built-in */
    int atomic_var = 42;
    atomic_test(&atomic_var, 42, 100);
    result += atomic_var;
    
#if HAS_X86
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
        double avx_sum[8];
        _mm512_storeu_pd(avx_sum, avx_result);
        result += (int)avx_sum[0];
    }
#endif
    
#if HAS_ARM
    /* Test ARM NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    
    int32x4_t neon_result = neon_multi_lane(neon_a, neon_b, neon_c,
                                            neon_d, neon_e);
    int32_t neon_sum[4];
    vst1q_s32(neon_sum, neon_result);
    result += neon_sum[0];
#endif
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
