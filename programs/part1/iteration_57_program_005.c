/* Test program to trigger 10/11-operand instruction expansion in GCC optabs */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Enable architecture-specific intrinsics */
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
static inline int complex_expression_10_operands(int a, int b, int c, int d, int e,
                                                 int f, int g, int h, int i, int j) {
    /* This expression might be combined into a single instruction with many operands */
    return a * b + c * d + e * f + g * h + i * j;
}

/* Function using inline assembly with exactly 11 operands */
static inline uint64_t inline_asm_11_operands(uint64_t a, uint64_t b, uint64_t c,
                                              uint64_t d, uint64_t e, uint64_t f,
                                              uint64_t g, uint64_t h, uint64_t i,
                                              uint64_t j, uint64_t k) {
    uint64_t result;
    
    /* Inline assembly with 11 operands to force 11-operand expansion */
    asm volatile (
        /* Template doesn't matter much - we just need the operand count */
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j),
          [k] "r" (k)
        : "cc"
    );
    
    return result;
}

#ifdef __x86_64__
/* AVX-512 example that could use many operands */
static __m512d avx512_multi_operand_test(__m512d a, __m512d b, __m512d c,
                                         __m512d d, __m512d e, __m512d f,
                                         __mmask8 mask) {
    /* FMA with mask and rounding control could expand to many operands */
    __m512d result;
    
    /* Chain of operations that might combine */
    result = _mm512_fmadd_pd(a, b, c);
    result = _mm512_fmadd_pd(result, d, e);
    result = _mm512_fmadd_pd(result, f, a);
    
    /* Masked operation with many parameters */
    result = _mm512_mask_mov_pd(result, mask, _mm512_set1_pd(0.0));
    
    return result;
}
#endif

#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
/* SVE2 intrinsics that use many operands */
static svint32_t sve_multi_operand_test(svint32_t a, svint32_t b, svint32_t c,
                                        svint32_t d, svint32_t e, svint32_t f,
                                        svint32_t g, svint32_t h) {
    /* Complex SVE operation chain */
    svint32_t t1 = svmla_s32_x(svptrue_b32(), a, b, c);
    svint32_t t2 = svmla_s32_x(svptrue_b32(), d, e, f);
    svint32_t result = svadd_s32_x(svptrue_b32(), t1, t2);
    result = svmla_s32_x(svptrue_b32(), result, g, h);
    
    return result;
}
#endif

/* NEON intrinsics */
static int32x4_t neon_multi_operand_test(int32x4_t a, int32x4_t b, int32x4_t c,
                                         int32x4_t d, int32x4_t e, int32x4_t f) {
    /* Chain of operations that might combine */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlaq_s32(d, e, f);
    return vaddq_s32(t1, t2);
}
#endif

/* Atomic built-in with many parameters */
static int atomic_multi_operand_test(int *ptr, int expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_SEQ_CST;
    
    /* __atomic_compare_exchange has 6 parameters, might expand to multi-operand */
    __atomic_compare_exchange(ptr, &expected, &desired, weak,
                              success_memorder, failure_memorder);
    
    return expected;
}

/* Decimal floating point built-ins (if available) */
#ifdef __DECIMAL_BID_FORMAT__
static _Decimal128 decimal_multi_operand_test(_Decimal128 a, _Decimal128 b,
                                              _Decimal128 c, _Decimal128 d) {
    /* Chain of decimal operations */
    _Decimal128 t1 = __bid128_add(a, b);
    _Decimal128 t2 = __bid128_mul(c, d);
    return __bid128_add(t1, t2);
}
#endif

int main() {
    int result = 0;
    
    /* Test 1: Complex expression with 10 operands */
    result += complex_expression_10_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Test 2: Inline assembly with 11 operands */
    result += inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test 3: Architecture-specific vector intrinsics */
#ifdef __x86_64__
    {
        __m512d v1 = _mm512_set1_pd(1.0);
        __m512d v2 = _mm512_set1_pd(2.0);
        __m512d v3 = _mm512_set1_pd(3.0);
        __m512d v4 = _mm512_set1_pd(4.0);
        __m512d v5 = _mm512_set1_pd(5.0);
        __m512d v6 = _mm512_set1_pd(6.0);
        
        __m512d vresult = avx512_multi_operand_test(v1, v2, v3, v4, v5, v6, 0xFF);
        double temp[8];
        _mm512_storeu_pd(temp, vresult);
        result += (int)temp[0];
    }
#endif
    
#ifdef __ARM_ARCH
    {
        int32x4_t a = {1, 2, 3, 4};
        int32x4_t b = {5, 6, 7, 8};
        int32x4_t c = {9, 10, 11, 12};
        int32x4_t d = {13, 14, 15, 16};
        int32x4_t e = {17, 18, 19, 20};
        int32x4_t f = {21, 22, 23, 24};
        
        int32x4_t neon_result = neon_multi_operand_test(a, b, c, d, e, f);
        result += vgetq_lane_s32(neon_result, 0);
    }
#endif
    
    /* Test 4: Atomic operation with many parameters */
    {
        int atomic_var = 42;
        result += atomic_multi_operand_test(&atomic_var, 42, 100);
    }
    
    /* Test 5: Another complex expression that might combine */
    {
        /* Fused multiply-add chain with 10 variables */
        int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
        int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
        
        int chain_result = v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8 + v9 * v10;
        result += chain_result;
    }
    
    /* Test 6: Bit-field operations across multiple words */
    {
        uint64_t bits1 = 0xAAAAAAAAAAAAAAAA;
        uint64_t bits2 = 0x5555555555555555;
        uint64_t bits3 = 0xFFFFFFFFFFFFFFFF;
        uint64_t bits4 = 0x0000000000000000;
        uint64_t bits5 = 0x123456789ABCDEF0;
        
        /* Complex bit manipulation that might combine */
        uint64_t bit_result = ((bits1 & bits2) | (bits3 & ~bits4)) ^ bits5;
        bit_result = (bit_result >> 32) | (bit_result << 32);
        bit_result = ((bit_result & 0xFF00FF00FF00FF00) >> 8) |
                     ((bit_result & 0x00FF00FF00FF00FF) << 8);
        
        result += (int)bit_result;
    }
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
