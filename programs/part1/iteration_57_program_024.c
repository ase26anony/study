/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

#ifdef __PPC64__
#include <altivec.h>
#endif

/* Complex expression that might combine into multi-operand instruction */
static inline int complex_expression(int a, int b, int c, int d, int e,
                                     int f, int g, int h, int i, int j) {
    /* Fused multiply-add chain with 10 operands */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                         uint64_t d, uint64_t e, uint64_t f,
                                         uint64_t g, uint64_t h, uint64_t i,
                                         uint64_t j, uint64_t k) {
    uint64_t result1, result2;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    asm volatile (
        /* Template doesn't matter much - we care about operand count */
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r1], %[r1], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "add %[r1], %[r1], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r2], %[r2], %[j]\n\t"
        "add %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[k]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1;
}

#ifdef __x86_64__
/* AVX-512 example with many operands */
static inline __m512d avx512_multi_operand(__m512d a, __m512d b, __m512d c,
                                           __m512d d, __m512d e, __m512d f,
                                           __mmask8 k, int rounding) {
    /* This could expand to a multi-operand instruction with mask and rounding */
    __m512d t1 = _mm512_fmadd_pd(a, b, c);
    __m512d t2 = _mm512_fmadd_pd(d, e, f);
    
    /* Masked operation with multiple operands */
    return _mm512_mask_add_pd(t1, k, t1, t2);
}

/* AVX-512 masked multiply-add with rounding control - up to 11 operands */
static inline __m512d avx512_complex_fma(__m512d a, __m512d b, __m512d c,
                                         __m512d d, __m512d e, __m512d f,
                                         __mmask8 k1, __mmask8 k2,
                                         int rc1, int rc2) {
    /* Chain of operations that might combine */
    __m512d t1 = _mm512_fmadd_round_pd(a, b, c, rc1);
    __m512d t2 = _mm512_fmadd_round_pd(d, e, f, rc2);
    
    /* Final masked blend - many operands */
    return _mm512_mask_blend_pd(k1, t1, _mm512_mask_blend_pd(k2, t2, a));
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 example with lane operations - can have many operands */
static inline svint32_t sve_multi_lane(svint32_t a, svint32_t b, svint32_t c,
                                       svint32_t d, svint32_t e, svint32_t f,
                                       svint32_t g, uint32_t lane1,
                                       uint32_t lane2, uint32_t lane3) {
    /* Complex SVE pattern with multiple lane selections */
    svint32_t t1 = svmla_lane_s32(a, b, c, lane1);
    svint32_t t2 = svmla_lane_s32(d, e, f, lane2);
    
    /* Final operation with another lane */
    return svadd_lane_s32(t1, t2, g, lane3);
}
#endif

/* NEON example with complex operations */
static inline int32x4_t neon_multi_operand(int32x4_t a, int32x4_t b,
                                           int32x4_t c, int32x4_t d,
                                           int32x4_t e, int32x4_t f,
                                           int32x4_t g, int32x4_t h) {
    /* Chain of operations that might combine */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    int32x4_t t3 = vaddq_s32(t1, t2);
    
    /* Final multiply-add with lane */
    return vmlaq_laneq_s32(t3, g, h, 2);
}
#endif

/* Atomic built-in with many parameters */
static inline int atomic_multi_operand(int *ptr, int *expected, int desired,
                                       int success_memorder, int failure_memorder,
                                       int weak) {
    /* __atomic_compare_exchange has 6 parameters, but expands to more operands */
    return __atomic_compare_exchange_n(ptr, expected, desired, weak,
                                       success_memorder, failure_memorder);
}

/* Decimal floating-point built-in (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static inline _Decimal128 decimal_multi_operand(_Decimal128 a, _Decimal128 b,
                                                _Decimal128 c, _Decimal128 d,
                                                _Decimal128 e, _Decimal128 f) {
    /* Complex decimal arithmetic that might use hardware instructions */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    _Decimal128 t3 = __bid128_fma(e, f, t1);
    
    return __bid128_add(t2, t3);
}
#endif

/* Vector reduction across multiple registers */
static inline int multi_vector_reduction(int32_t *data1, int32_t *data2,
                                         int32_t *data3, int32_t *data4,
                                         size_t n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    for (size_t i = 0; i < n; i++) {
        sum1 += data1[i];
        sum2 += data2[i];
        sum3 += data3[i];
        sum4 += data4[i];
    }
    
    /* Complex expression with many operands */
    return sum1 * sum2 + sum3 * sum4 + (sum1 + sum2) * (sum3 + sum4);
}

int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10 operands */
    int expr_result = complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += expr_result;
    printf("Complex expression result: %d\n", expr_result);
    
    /* Test 2: Inline assembly with 11 operands */
    uint64_t asm_result = multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    result += (int)asm_result;
    printf("Assembly result: %lu\n", asm_result);
    
    /* Test 3: Atomic operation with many parameters */
    int atomic_var = 42;
    int expected = 42;
    int desired = 100;
    int atomic_success = atomic_multi_operand(&atomic_var, &expected, desired,
                                              __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST, 0);
    result += atomic_success;
    printf("Atomic exchange %s\n", atomic_success ? "succeeded" : "failed");
    
#ifdef __x86_64__
    /* Test 4: AVX-512 operations */
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
        
        double avx_sum = _mm512_reduce_add_pd(avx_result);
        result += (int)avx_sum;
        printf("AVX-512 result sum: %f\n", avx_sum);
    }
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_NEON
    /* Test 5: NEON operations */
    int32x4_t neon_a = vdupq_n_s32(1);
    int32x4_t neon_b = vdupq_n_s32(2);
    int32x4_t neon_c = vdupq_n_s32(3);
    int32x4_t neon_d = vdupq_n_s32(4);
    int32x4_t neon_e = vdupq_n_s32(5);
    int32x4_t neon_f = vdupq_n_s32(6);
    int32x4_t neon_g = vdupq_n_s32(7);
    int32x4_t neon_h = vdupq_n_s32(8);
    
    int32x4_t neon_result = neon_multi_operand(neon_a, neon_b, neon_c, neon_d,
                                               neon_e, neon_f, neon_g, neon_h);
    
    int32_t neon_sum = vaddvq_s32(neon_result);
    result += neon_sum;
    printf("NEON result sum: %d\n", neon_sum);
#endif
#endif
    
    /* Test 6: Vector reduction across multiple arrays */
    int32_t data1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int32_t data2[8] = {2, 3, 4, 5, 6, 7, 8, 9};
    int32_t data3[8] = {3, 4, 5, 6, 7, 8, 9, 10};
    int32_t data4[8] = {4, 5, 6, 7, 8, 9, 10, 11};
    
    int reduction_result = multi_vector_reduction(data1, data2, data3, data4, 8);
    result += reduction_result;
    printf("Vector reduction result: %d\n", reduction_result);
    
    /* Test 7: Bit-field manipulation with many operands */
    uint64_t bitfield_test = 0;
    for (int i = 0; i < 10; i++) {
        /* Complex bit manipulation that might combine */
        bitfield_test = (bitfield_test << 3) | ((bitfield_test >> 5) & 0x7);
        bitfield_test ^= (1ULL << i);
    }
    result += (int)bitfield_test;
    printf("Bitfield result: %lu\n", bitfield_test);
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
