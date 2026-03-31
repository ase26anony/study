/* Test program to trigger 10/11-operand instruction expansion in GCC optabs.cc */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Generic fallback for architectures without specific intrinsics */
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

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
#elif defined(__powerpc64__) || defined(__powerpc__)
#define TARGET_PPC 1
#include <altivec.h>
#endif

/* For atomic operations */
#include <stdatomic.h>

/* Function to prevent dead code elimination */
static void escape(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Complex expression that might combine into multi-operand instruction */
int complex_expression(int a, int b, int c, int d, int e, 
                       int f, int g, int h, int i, int j, int k) {
    /* This complex chain of operations might be combined */
    int result = a * b + c * d + e * f + g * h + i * j + k;
    result = result * a + b * c + d * e + f * g + h * i + j * k;
    result = result + (a & b) | (c & d) ^ (e & f) & (g & h) | (i & j);
    return result;
}

/* Test with inline assembly having exactly 11 operands */
int inline_asm_11_operands(int a, int b, int c, int d, int e,
                           int f, int g, int h, int i, int j, int k) {
    int result1, result2, result3, result4, result5;
    
    /* Inline asm with 11 operands - forces expansion */
    asm volatile(
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
        : "=r"(result1)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j)
        : "cc"
    );
    
    /* Another with mixed constraints */
    asm volatile(
        "/* Mixed constraints asm */\n\t"
        "imul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "sub %0, %0, %4"
        : "=&r"(result2)
        : "r"(k), "i"(10), "m"(a), "r"(b)
        : "cc"
    );
    
    return result1 + result2;
}

#if TARGET_X86
/* AVX-512 intrinsics that can generate multi-operand instructions */
__m512i avx512_multi_operand_test(__m512i a, __m512i b, __m512i c,
                                  __m512i d, __m512i e, __m512i f,
                                  __mmask16 k1, __mmask16 k2) {
    /* AVX-512 masked operations with multiple operands */
    __m512i result;
    
    /* Fused multiply-add with masking and rounding - potentially many operands */
    result = _mm512_mask_mullo_epi32(a, k1, b, c);
    result = _mm512_mask_add_epi32(result, k2, result, d);
    
    /* Chain operations that might combine */
    __m512i temp = _mm512_madd_epi16(a, b);
    temp = _mm512_add_epi32(temp, _mm512_madd_epi16(c, d));
    temp = _mm512_add_epi32(temp, _mm512_madd_epi16(e, f));
    
    return _mm512_add_epi32(result, temp);
}

/* Test AVX-512 with 10+ operands via intrinsics */
void test_avx512_multi_operand() {
    __m512i v1 = _mm512_set1_epi32(1);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    __m512i v5 = _mm512_set1_epi32(5);
    __m512i v6 = _mm512_set1_epi32(6);
    
    __mmask16 k1 = 0xAAAA;
    __mmask16 k2 = 0x5555;
    
    __m512i result = avx512_multi_operand_test(v1, v2, v3, v4, v5, v6, k1, k2);
    escape(&result);
}
#endif

#if TARGET_ARM
/* ARM SVE/NEON intrinsics that use many operands */
#ifdef __ARM_FEATURE_SVE
svint32_t sve_multi_operand_test(svint32_t a, svint32_t b, svint32_t c,
                                 svint32_t d, svint32_t e, svbool_t pg) {
    /* SVE operations with predication - can have many operands */
    svint32_t result = svmla_m(pg, a, b, c);  /* a + b * c with mask */
    result = svmad_m(pg, result, d, e);       /* result + d * e with mask */
    
    /* Lane operations with multiple vector registers */
    svint32_t temp = svmla_lane(svdup_n_s32(0), a, b, 2);
    temp = svmla_lane(temp, c, d, 3);
    temp = svmla_lane(temp, e, a, 1);
    
    return svadd_m(pg, result, temp);
}
#endif

/* NEON intrinsics with lane operations */
int32x4_t neon_multi_lane_test(int32x4_t a, int32x4_t b, int32x4_t c,
                               int32x4_t d, int32x4_t e, int32x4_t f) {
    /* Multi-lane operations that might expand to many operands */
    int32x4_t result = vmlaq_laneq_s32(a, b, c, 0);
    result = vmlaq_laneq_s32(result, d, e, 1);
    result = vmlaq_laneq_s32(result, f, a, 2);
    result = vmlaq_laneq_s32(result, b, c, 3);
    
    return result;
}
#endif

/* Atomic operation with many parameters */
int atomic_multi_operand_test(_Atomic int *ptr, int *expected, int desired) {
    int weak = 0;
    int success_memorder = __ATOMIC_SEQ_CST;
    int failure_memorder = __ATOMIC_ACQUIRE;
    
    /* __atomic_compare_exchange has 6 parameters, might expand further */
    int result = __atomic_compare_exchange_n(ptr, expected, desired,
                                             weak, success_memorder,
                                             failure_memorder);
    
    /* Another atomic with multiple order parameters */
    int val = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    __atomic_store_n(ptr, val + 1, __ATOMIC_RELEASE);
    
    return result;
}

/* Decimal floating point builtins (if available) */
#if __has_builtin(__builtin_addsd3) || defined(__DECIMAL_BID_FORMAT__)
void decimal_float_test(void) {
    /* Decimal float builtins can have many operands */
    _Decimal64 d1 = 1.0dl;
    _Decimal64 d2 = 2.0dl;
    _Decimal64 d3 = 3.0dl;
    _Decimal64 d4 = 4.0dl;
    
    /* Chain of decimal operations */
    _Decimal64 result = d1 + d2 * d3 - d4 / d1;
    escape(&result);
}
#endif

/* Vector reduction across multiple registers */
int vector_reduction_test(int *data, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    /* Unrolled reduction that might combine */
    for (int i = 0; i < n; i += 4) {
        sum1 += data[i];
        sum2 += data[i + 1];
        sum3 += data[i + 2];
        sum4 += data[i + 3];
    }
    
    /* Complex final reduction */
    int result = (sum1 * sum2) + (sum3 * sum4) - (sum1 + sum2) * (sum3 - sum4);
    return result;
}

/* Main test driver */
int main(void) {
    int result = 0;
    
    /* Test complex expression */
    result += complex_expression(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test inline assembly with many operands */
    result += inline_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    
    /* Test architecture-specific intrinsics */
#if TARGET_X86
    test_avx512_multi_operand();
#endif
    
#if TARGET_ARM
    {
        int32x4_t a = {1, 2, 3, 4};
        int32x4_t b = {5, 6, 7, 8};
        int32x4_t c = {9, 10, 11, 12};
        int32x4_t d = {13, 14, 15, 16};
        int32x4_t e = {17, 18, 19, 20};
        int32x4_t f = {21, 22, 23, 24};
        
        int32x4_t neon_result = neon_multi_lane_test(a, b, c, d, e, f);
        escape(&neon_result);
    }
#endif
    
    /* Test atomic operations */
    _Atomic int atomic_var = 42;
    int expected = 42;
    result += atomic_multi_operand_test(&atomic_var, &expected, 100);
    
    /* Test vector reduction */
    int data[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    result += vector_reduction_test(data, 16);
    
    /* Test decimal float if available */
#if __has_builtin(__builtin_addsd3) || defined(__DECIMAL_BID_FORMAT__)
    decimal_float_test();
#endif
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
