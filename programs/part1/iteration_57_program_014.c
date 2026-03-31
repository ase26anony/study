/* Test program to trigger 10/11-operand instruction expansion in GCC optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent aggressive optimization from removing our test code */
#pragma GCC optimize("O0")

/* Architecture-specific includes */
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

#ifdef __powerpc__
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression_10_operands(int a, int b, int c, int d, 
                                                 int e, int f, int g, int h,
                                                 int i, int j) {
    /* This expression might be combined into a single instruction
       with many operands during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static inline void inline_asm_11_operands(uint64_t *out, uint64_t a, uint64_t b,
                                          uint64_t c, uint64_t d, uint64_t e,
                                          uint64_t f, uint64_t g, uint64_t h,
                                          uint64_t i, uint64_t j) {
    /* Inline assembly with 11 operands to force optabs expansion */
    __asm__ volatile (
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

/* Atomic built-in with many parameters */
static inline int atomic_compare_exchange_many_args(int *ptr, int *expected,
                                                    int desired, int weak,
                                                    int success_memorder,
                                                    int failure_memorder) {
    /* __atomic_compare_exchange has 6 parameters, which when expanded
       might become 10+ operands including temporaries */
    return __atomic_compare_exchange(ptr, expected, &desired, weak,
                                     success_memorder, failure_memorder);
}

#ifdef __x86_64__
/* AVX-512 masked operation with many operands */
static inline __m512d avx512_masked_fma(__m512d a, __m512d b, __m512d c,
                                        __mmask8 k, __m512d d, __m512d e) {
    /* Complex expression that might use multiple masked operations */
    __m512d t1 = _mm512_mask_fmadd_pd(a, k, b, c);
    __m512d t2 = _mm512_mask_fmadd_pd(d, k, e, t1);
    return _mm512_mask_add_pd(t2, k, t2, _mm512_set1_pd(1.0));
}

/* AVX-512 instruction with explicit rounding control */
static inline __m512 avx512_rounding_fma(__m512 a, __m512 b, __m512 c) {
    /* _mm512_fmadd_round_ps has 4 operands plus implicit rounding control */
    return _mm512_fmadd_round_ps(a, b, c, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 lane operations with multiple vectors */
static inline svint32_t sve_mla_lane_complex(svint32_t a, svint32_t b,
                                             svint32_t c, svint32_t d,
                                             svint32_t e, svint32_t f) {
    /* Complex SVE expression that might use multiple lane operations */
    svint32_t t1 = svmla_lane_s32(a, b, c, 0);
    svint32_t t2 = svmla_lane_s32(d, e, f, 2);
    return svadd_s32(t1, t2);
}
#endif

/* NEON complex multiply-accumulate */
static inline int32x4_t neon_mla_lane_complex(int32x4_t a, int32x4_t b,
                                              int32x4_t c, int32x4_t d,
                                              int32x4_t e, int32x4_t f) {
    /* Multiple vmla_lane operations */
    int32x4_t t1 = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    int32x4_t t2 = vmlaq_lane_s32(d, e, vget_high_s32(f), 1);
    return vaddq_s32(t1, t2);
}
#endif

#ifdef __powerpc__
/* Altivec/VSX complex permute and compute */
static inline vector signed int altivec_complex_op(vector signed int a,
                                                   vector signed int b,
                                                   vector signed int c,
                                                   vector signed int d) {
    /* Complex Altivec expression */
    vector signed int t1 = vec_madd(a, b, c);
    vector signed int t2 = vec_madd(d, a, b);
    return vec_add(t1, vec_perm(t2, t1, (vector unsigned char){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}));
}
#endif

/* Main test driver */
int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10 variables */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5;
        int f = 6, g = 7, h = 8, i = 9, j = 10;
        result += complex_expression_10_operands(a, b, c, d, e, f, g, h, i, j);
    }
    
    /* Test 2: Inline assembly with 11 operands */
    {
        uint64_t out;
        inline_asm_11_operands(&out, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        result += (int)out;
    }
    
    /* Test 3: Atomic built-in with many parameters */
    {
        int atomic_var = 42;
        int expected = 42;
        int desired = 100;
        result += atomic_compare_exchange_many_args(&atomic_var, &expected,
                                                    desired, 0, 5, 5);
    }
    
    /* Architecture-specific tests */
#ifdef __x86_64__
    /* Test 4: AVX-512 masked operations */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __mmask8 mask = 0xFF;
        
        __m512d avx_result = avx512_masked_fma(avx_a, avx_b, avx_c, mask, avx_d, avx_e);
        double avx_sum = _mm512_reduce_add_pd(avx_result);
        result += (int)avx_sum;
    }
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    /* Test 5: SVE lane operations */
    {
        svint32_t sve_a = svdup_s32(1);
        svint32_t sve_b = svdup_s32(2);
        svint32_t sve_c = svdup_s32(3);
        svint32_t sve_d = svdup_s32(4);
        svint32_t sve_e = svdup_s32(5);
        svint32_t sve_f = svdup_s32(6);
        
        svint32_t sve_result = sve_mla_lane_complex(sve_a, sve_b, sve_c,
                                                    sve_d, sve_e, sve_f);
        /* Extract first element */
        int32_t sve_first;
        svst1_s32(svptrue_b32(), &sve_first, sve_result);
        result += sve_first;
    }
#endif
    
    /* Test 6: NEON operations */
    {
        int32x4_t neon_a = {1, 2, 3, 4};
        int32x4_t neon_b = {5, 6, 7, 8};
        int32x4_t neon_c = {9, 10, 11, 12};
        int32x4_t neon_d = {13, 14, 15, 16};
        int32x4_t neon_e = {17, 18, 19, 20};
        int32x4_t neon_f = {21, 22, 23, 24};
        
        int32x4_t neon_result = neon_mla_lane_complex(neon_a, neon_b, neon_c,
                                                      neon_d, neon_e, neon_f);
        result += vgetq_lane_s32(neon_result, 0);
    }
#endif
    
#ifdef __powerpc__
    /* Test 7: Altivec operations */
    {
        vector signed int altivec_a = {1, 2, 3, 4};
        vector signed int altivec_b = {5, 6, 7, 8};
        vector signed int altivec_c = {9, 10, 11, 12};
        vector signed int altivec_d = {13, 14, 15, 16};
        
        vector signed int altivec_result = altivec_complex_op(altivec_a, altivec_b,
                                                              altivec_c, altivec_d);
        result += ((int*)&altivec_result)[0];
    }
#endif
    
    /* Additional test: Chain of operations that might combine */
    {
        volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
        volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
        
        /* This complex expression might be combined during optimization */
        int chain_result = v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8 + v9 * v10 * v11;
        result += chain_result;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
