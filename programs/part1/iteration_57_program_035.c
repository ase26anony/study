/* test_multi_operand.c - Test program for GCC optabs 10/11-operand expansion */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Generic fallback for architectures without specific intrinsics */
#ifndef __x86_64__
#ifndef __ARM_ARCH
#ifndef __powerpc64__
#define GENERIC_FALLBACK 1
#endif
#endif
#endif

/* x86 AVX-512 intrinsics */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* ARM SVE/SVE2 intrinsics */
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif
#include <arm_neon.h>
#endif

/* PowerPC Altivec/VSX intrinsics */
#ifdef __powerpc64__
#include <altivec.h>
#endif

/* Atomic operations */
#include <stdatomic.h>

/* Complex expression that might combine into multi-operand instruction */
static inline int64_t complex_expression(int64_t a, int64_t b, int64_t c,
                                         int64_t d, int64_t e, int64_t f,
                                         int64_t g, int64_t h, int64_t i,
                                         int64_t j, int64_t k) {
    /* This complex chain might be combined into a multi-operand instruction */
    return a * b + c * d + e * f + g * h + i * j + k;
}

/* Function using inline assembly with 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with exactly 11 operands to trigger the 11-operand case */
    __asm__ volatile (
        "/* Multi-operand test with 11 operands */\n\t"
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

/* x86 AVX-512 specific test with masked operations */
#ifdef __x86_64__
static __m512d avx512_multi_operand_test(__m512d a, __m512d b, __m512d c,
                                         __m512d d, __m512d e, __m512d f,
                                         __mmask8 m) {
    /* AVX-512 masked fused multiply-add with rounding control
       This can expand to instructions with many operands */
    __m512d result;
    
    /* Chain multiple FMA operations that might combine */
    result = _mm512_fmadd_pd(a, b, c);
    result = _mm512_fmadd_pd(result, d, e);
    result = _mm512_mask_fmadd_pd(result, m, f, a);
    
    /* Try to use an intrinsic with many parameters */
    result = _mm512_mask3_fmadd_round_pd(b, c, d, m, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    
    return result;
}
#endif

/* ARM SVE/NEON specific test */
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
static svint32_t sve_multi_operand_test(svint32_t a, svint32_t b, svint32_t c,
                                        svint32_t d, svint32_t e, svint32_t f,
                                        svint32_t g, svint32_t h) {
    /* SVE operations with lane selection and predication */
    svint32_t result;
    svbool_t pg = svptrue_b32();
    
    /* Complex SVE expression that might use multi-operand instructions */
    result = svmla_lane_s32(a, b, c, 0);
    result = svmla_lane_s32(result, d, e, 2);
    result = svdot_lane_s32(result, f, g, 1);
    
    return result;
}
#endif

/* ARM NEON test */
static int32x4_t neon_multi_operand_test(int32x4_t a, int32x4_t b, int32x4_t c,
                                         int32x4_t d, int32x4_t e, int32x4_t f,
                                         int32x4_t g, int32x4_t h) {
    /* NEON operations with lane extraction and multiple vectors */
    int32x4_t result;
    
    /* vmla_lane with multiple vectors */
    result = vmlaq_lane_s32(a, b, vget_low_s32(c), 0);
    result = vmlaq_lane_s32(result, d, vget_high_s32(e), 1);
    
    /* Complex expression combining multiple operations */
    int32x4_t temp1 = vmulq_s32(f, g);
    int32x4_t temp2 = vmlaq_s32(temp1, h, a);
    result = vaddq_s32(result, temp2);
    
    return result;
}
#endif

/* PowerPC VSX specific test */
#ifdef __powerpc64__
static vector double vsx_multi_operand_test(vector double a, vector double b,
                                            vector double c, vector double d,
                                            vector double e, vector double f) {
    /* VSX complex permute and compute operations */
    vector double result;
    
    /* xvmadd* instructions with multiple operands */
    result = vec_madd(a, b, c);
    result = vec_madd(result, d, e);
    
    /* Additional operations that might combine */
    vector double temp = vec_perm(a, b, (vector unsigned char){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
    result = vec_madd(result, temp, f);
    
    return result;
}
#endif

/* Atomic operation with many parameters */
static int atomic_multi_operand_test(_Atomic int64_t *shared, int64_t expected,
                                     int64_t desired, int weak, int success_memorder,
                                     int failure_memorder) {
    int64_t expected_local = expected;
    
    /* __atomic_compare_exchange with 6 parameters + return value
       Might expand to multi-operand instruction on some architectures */
    return __atomic_compare_exchange_n(shared, &expected_local, desired,
                                       weak, success_memorder, failure_memorder);
}

/* Decimal floating-point built-in test (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_operand_test(_Decimal128 a, _Decimal128 b,
                                              _Decimal128 c, _Decimal128 d,
                                              _Decimal128 e) {
    /* Decimal floating-point operations that might use multi-operand instructions */
    _Decimal128 result;
    
    /* Chain of decimal operations */
    result = __bid128_add(a, b);
    result = __bid128_mul(result, c);
    result = __bid128_fma(d, e, result);
    
    return result;
}
#endif

/* Main test driver */
int main() {
    uint64_t result = 0;
    
    printf("Testing multi-operand instruction expansion...\n");
    
    /* Test 1: Complex arithmetic expression (10+ operands) */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 2: Inline assembly with 11 operands */
    result += multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 3: Architecture-specific vector intrinsics */
#if defined(__x86_64__)
    {
        __m512d vec_a = _mm512_set1_pd(1.0);
        __m512d vec_b = _mm512_set1_pd(2.0);
        __m512d vec_c = _mm512_set1_pd(3.0);
        __m512d vec_d = _mm512_set1_pd(4.0);
        __m512d vec_e = _mm512_set1_pd(5.0);
        __m512d vec_f = _mm512_set1_pd(6.0);
        __mmask8 mask = 0xFF;
        
        __m512d vec_result = avx512_multi_operand_test(vec_a, vec_b, vec_c,
                                                       vec_d, vec_e, vec_f, mask);
        double res_array[8];
        _mm512_storeu_pd(res_array, vec_result);
        result += (uint64_t)res_array[0];
    }
#elif defined(__ARM_ARCH) && defined(__ARM_FEATURE_SVE)
    {
        svint32_t sve_a = svdup_s32(1);
        svint32_t sve_b = svdup_s32(2);
        svint32_t sve_c = svdup_s32(3);
        svint32_t sve_d = svdup_s32(4);
        svint32_t sve_e = svdup_s32(5);
        svint32_t sve_f = svdup_s32(6);
        svint32_t sve_g = svdup_s32(7);
        svint32_t sve_h = svdup_s32(8);
        
        svint32_t sve_result = sve_multi_operand_test(sve_a, sve_b, sve_c,
                                                      sve_d, sve_e, sve_f,
                                                      sve_g, sve_h);
        int32_t sve_res;
        svst1_s32(svptrue_b32(), &sve_res, sve_result);
        result += sve_res;
    }
#elif defined(__ARM_ARCH)
    {
        int32x4_t neon_a = vdupq_n_s32(1);
        int32x4_t neon_b = vdupq_n_s32(2);
        int32x4_t neon_c = vdupq_n_s32(3);
        int32x4_t neon_d = vdupq_n_s32(4);
        int32x4_t neon_e = vdupq_n_s32(5);
        int32x4_t neon_f = vdupq_n_s32(6);
        int32x4_t neon_g = vdupq_n_s32(7);
        int32x4_t neon_h = vdupq_n_s32(8);
        
        int32x4_t neon_result = neon_multi_operand_test(neon_a, neon_b, neon_c,
                                                        neon_d, neon_e, neon_f,
                                                        neon_g, neon_h);
        int32_t neon_res = vgetq_lane_s32(neon_result, 0);
        result += neon_res;
    }
#elif defined(__powerpc64__)
    {
        vector double vsx_a = (vector double){1.0, 2.0};
        vector double vsx_b = (vector double){3.0, 4.0};
        vector double vsx_c = (vector double){5.0, 6.0};
        vector double vsx_d = (vector double){7.0, 8.0};
        vector double vsx_e = (vector double){9.0, 10.0};
        vector double vsx_f = (vector double){11.0, 12.0};
        
        vector double vsx_result = vsx_multi_operand_test(vsx_a, vsx_b, vsx_c,
                                                          vsx_d, vsx_e, vsx_f);
        double vsx_res[2];
        vec_st(vsx_result, 0, (vector double *)vsx_res);
        result += (uint64_t)vsx_res[0];
    }
#endif
    
    /* Test 4: Atomic operation with many parameters */
    {
        _Atomic int64_t atomic_var = 100;
        int atomic_result = atomic_multi_operand_test(&atomic_var, 100, 200,
                                                      0, __ATOMIC_SEQ_CST,
                                                      __ATOMIC_SEQ_CST);
        result += atomic_result;
    }
    
    /* Test 5: Another inline assembly with mixed constraints (10 operands) */
    {
        uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
        uint64_t f = 6, g = 7, h = 8, i = 9, j = 10;
        uint64_t asm_result;
        
        /* 10 operands: 5 inputs in registers, 5 in memory/immediate */
        __asm__ volatile (
            "/* 10-operand test with mixed constraints */\n\t"
            "mov %[out], %[in1]\n\t"
            "add %[out], %[out], %[in2]\n\t"
            "add %[out], %[out], %[in3]\n\t"
            "add %[out], %[out], %[in4]\n\t"
            "add %[out], %[out], %[in5]"
            : [out] "=r" (asm_result)
            : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c),
              [in4] "m" (d), [in5] "i" (5),
              "m" (f), "m" (g), "m" (h), "m" (i), "m" (j)
            : "cc"
        );
        
        result += asm_result;
    }
    
    printf("Final result: %lu\n", (unsigned long)result);
    printf("Test completed. Check RTL dumps for multi-operand expansion.\n");
    
    return (int)(result % 256);  /* Return non-zero to indicate execution */
}
