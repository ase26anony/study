#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Architecture-specific headers */
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

#ifdef __PPC64__
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single multi-operand
       instruction during optimization */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with 11 operands */
static inline void multi_operand_asm(void) {
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result;
    
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3),
          [d] "r" (op4), [e] "r" (op5), [f] "r" (op6),
          [g] "r" (op7), [h] "r" (op8), [i] "r" (op9),
          [j] "r" (op10)
        : "cc"
    );
    
    printf("ASM result: %ld\n", result);
}

#ifdef __x86_64__
/* AVX-512 operations with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __m512d f,
                                           __m512d g, __m512d h, __m512d i,
                                           __m512d j, __mmask8 k) {
    /* Chain of FMA operations that might combine */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    __m512d t3 = _mm512_fmadd_pd(g, h, i);
    
    /* Masked operation with many parameters */
    __m512d result = _mm512_mask3_fmadd_pd(t1, t2, t3, k);
    
    /* Another operation with immediate operand */
    result = _mm512_add_pd(result, j);
    
    return result;
}

/* AVX-512 masked store with many parameters */
static inline void avx512_masked_store(double* ptr, __m512d data,
                                       __mmask8 mask, int offset) {
    /* This intrinsic expands to instruction with multiple operands */
    _mm512_mask_storeu_pd(ptr + offset, mask, data);
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 operations with lane selection */
static inline svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                       svint32_t d, svint32_t e, svint32_t f,
                                       svint32_t g, svint32_t h, svint32_t i,
                                       svint32_t j) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_lane_s32(a, b, c, 0);
    svint32_t t2 = svmla_lane_s32(d, e, f, 1);
    svint32_t t3 = svmla_lane_s32(g, h, i, 2);
    
    /* Combine results */
    return svadd_s32(t1, svadd_s32(t2, t3));
}
#endif

/* NEON operations with multiple vectors */
static inline int32x4_t neon_multi_vector(int32x4_t a, int32x4_t b,
                                          int32x4_t c, int32x4_t d,
                                          int32x4_t e, int32x4_t f,
                                          int32x4_t g, int32x4_t h) {
    /* Multiple vector operations that might combine */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    return vaddq_s32(t1, vaddq_s32(t2, vaddq_s32(g, h)));
}
#endif

#ifdef __PPC64__
/* PowerPC VSX operations */
static inline vector double vsx_multi_operand(vector double a, vector double b,
                                              vector double c, vector double d,
                                              vector double e, vector double f,
                                              vector double g, vector double h) {
    /* Complex VSX operation chain */
    vector double t1 = vec_madd(a, b, c);
    vector double t2 = vec_madd(d, e, f);
    return vec_add(t1, vec_add(t2, vec_add(g, h)));
}
#endif

/* Atomic operation with many parameters */
static inline int atomic_multi_operand(_Atomic int* ptr, int expected,
                                       int desired, int memorder1,
                                       int memorder2, int weak) {
    int result;
    /* __atomic_compare_exchange has 6 parameters, which might expand
       to a multi-operand instruction on some architectures */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              memorder1, memorder2);
    return expected;
}

/* Decimal floating point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                                _Decimal128 c, _Decimal128 d,
                                                _Decimal128 e, _Decimal128 f) {
    /* Chain of decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    return __bid128_add(t1, __bid128_add(t2, __bid128_add(e, f)));
}
#endif

int main(void) {
    int result = 0;
    
    printf("Testing multi-operand instruction expansion...\n");
    
    /* 1. Complex arithmetic expression with 10+ operands */
    int expr_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    printf("Complex expression result: %d\n", expr_result);
    result += expr_result;
    
    /* 2. Inline assembly with 11 operands */
    multi_operand_asm();
    
    /* 3. Atomic operation with multiple parameters */
    _Atomic int atomic_var = 42;
    int atomic_result = atomic_multi_operand(&atomic_var, 42, 100,
                                             __ATOMIC_SEQ_CST,
                                             __ATOMIC_SEQ_CST, 0);
    printf("Atomic operation result: %d\n", atomic_result);
    result += atomic_result;
    
#ifdef __x86_64__
    /* 4. AVX-512 operations */
    if (__builtin_cpu_supports("avx512f")) {
        __m512d avx_a = _mm512_set1_pd(1.0);
        __m512d avx_b = _mm512_set1_pd(2.0);
        __m512d avx_c = _mm512_set1_pd(3.0);
        __m512d avx_d = _mm512_set1_pd(4.0);
        __m512d avx_e = _mm512_set1_pd(5.0);
        __m512d avx_f = _mm512_set1_pd(6.0);
        __m512d avx_g = _mm512_set1_pd(7.0);
        __m512d avx_h = _mm512_set1_pd(8.0);
        __m512d avx_i = _mm512_set1_pd(9.0);
        __m512d avx_j = _mm512_set1_pd(10.0);
        
        __m512d avx_result = avx512_multi_operand(avx_a, avx_b, avx_c,
                                                  avx_d, avx_e, avx_f,
                                                  avx_g, avx_h, avx_i,
                                                  avx_j, 0xFF);
        
        double avx_sum[8];
        _mm512_storeu_pd(avx_sum, avx_result);
        printf("AVX-512 result sum: %f\n", avx_sum[0]);
        result += (int)avx_sum[0];
    }
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    /* 5. SVE operations */
    svint32_t sve_a = svdup_s32(1);
    svint32_t sve_b = svdup_s32(2);
    svint32_t sve_c = svdup_s32(3);
    svint32_t sve_d = svdup_s32(4);
    svint32_t sve_e = svdup_s32(5);
    svint32_t sve_f = svdup_s32(6);
    svint32_t sve_g = svdup_s32(7);
    svint32_t sve_h = svdup_s32(8);
    svint32_t sve_i = svdup_s32(9);
    svint32_t sve_j = svdup_s32(10);
    
    svint32_t sve_result = sve_multi_lane(sve_a, sve_b, sve_c, sve_d,
                                          sve_e, sve_f, sve_g, sve_h,
                                          sve_i, sve_j);
    
    int32_t sve_scalar = svaddv_s32(svptrue_b32(), sve_result);
    printf("SVE result: %d\n", sve_scalar);
    result += sve_scalar;
#endif
    
    /* 6. NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    int32x4_t neon_g = vdupq_n_s32(7);
    int32x4_t neon_h = vdupq_n_s32(8);
    
    int32x4_t neon_result = neon_multi_vector(neon_a, neon_b, neon_c, neon_d,
                                              neon_e, neon_f, neon_g, neon_h);
    
    int32_t neon_sum = vaddvq_s32(neon_result);
    printf("NEON result: %d\n", neon_sum);
    result += neon_sum;
#endif
    
#ifdef __PPC64__
    /* 7. VSX operations */
    vector double vsx_a = (vector double){1.0, 2.0};
    vector double vsx_b = (vector double){3.0, 4.0};
    vector double vsx_c = (vector double){5.0, 6.0};
    vector double vsx_d = (vector double){7.0, 8.0};
    vector double vsx_e = (vector double){9.0, 10.0};
    vector double vsx_f = (vector double){11.0, 12.0};
    vector double vsx_g = (vector double){13.0, 14.0};
    vector double vsx_h = (vector double){15.0, 16.0};
    
    vector double vsx_result = vsx_multi_operand(vsx_a, vsx_b, vsx_c, vsx_d,
                                                 vsx_e, vsx_f, vsx_g, vsx_h);
    
    double vsx_sum[2];
    vec_st(vsx_result, 0, vsx_sum);
    printf("VSX result sum: %f\n", vsx_sum[0] + vsx_sum[1]);
    result += (int)(vsx_sum[0] + vsx_sum[1]);
#endif
    
#ifdef __DECIMAL_BID_FORMAT__
    /* 8. Decimal floating point operations */
    _Decimal128 dec_a = 1.0DL;
    _Decimal128 dec_b = 2.0DL;
    _Decimal128 dec_c = 3.0DL;
    _Decimal128 dec_d = 4.0DL;
    _Decimal128 dec_e = 5.0DL;
    _Decimal128 dec_f = 6.0DL;
    
    _Decimal128 dec_result = decimal_multi_operand(dec_a, dec_b, dec_c,
                                                   dec_d, dec_e, dec_f);
    printf("Decimal operation performed\n");
#endif
    
    /* Return final result to prevent optimization */
    return result % 256;
}
