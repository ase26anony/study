/* test_multi_operand.c - Test program to trigger 10/11-operand expansion in GCC optabs */
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

/* x86 AVX-512 specific code */
#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test AVX-512 masked operations with many operands */
__attribute__((target("avx512f")))
void test_avx512_multi_operand() {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __m512d c = _mm512_set1_pd(3.0);
    __m512d d = _mm512_set1_pd(4.0);
    __m512d e = _mm512_set1_pd(5.0);
    __m512d f = _mm512_set1_pd(6.0);
    __m512d g = _mm512_set1_pd(7.0);
    __m512d h = _mm512_set1_pd(8.0);
    __m512d i = _mm512_set1_pd(9.0);
    __m512d j = _mm512_set1_pd(10.0);
    
    __mmask8 mask = 0xFF;
    
    /* Complex expression that might combine into multi-operand instruction */
    __m512d result = _mm512_fmadd_pd(a, b, 
                     _mm512_fmadd_pd(c, d,
                     _mm512_fmadd_pd(e, f,
                     _mm512_fmadd_pd(g, h,
                     _mm512_add_pd(i, j)))));
    
    /* Use inline assembly with 11 operands */
    double out1, out2, out3, out4, out5;
    double in1 = 1.0, in2 = 2.0, in3 = 3.0, in4 = 4.0, in5 = 5.0;
    double in6 = 6.0, in7 = 7.0, in8 = 8.0, in9 = 9.0, in10 = 10.0;
    
    asm volatile (
        /* Template with 11 operands - forces optabs to handle 11-operand case */
        "mov %0, %1\n\t"
        "mov %2, %3\n\t"
        "mov %4, %5\n\t"
        "mov %6, %7\n\t"
        "mov %8, %9\n\t"
        "add %10, %0"
        : "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4), "=r"(out5)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "r"(in6), "r"(in7), "r"(in8), "r"(in9), "r"(in10),
          "0"(out1), "1"(out2), "2"(out3), "3"(out4), "4"(out5)
        : "memory"
    );
    
    /* Store to prevent optimization */
    _mm512_storeu_pd((double*)&result, result);
}
#endif

/* ARM SVE2 specific code */
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

void test_arm_sve_multi_operand() {
    /* SVE vectors with potentially many operands */
    svint32_t a = svdup_s32(1);
    svint32_t b = svdup_s32(2);
    svint32_t c = svdup_s32(3);
    svint32_t d = svdup_s32(4);
    svint32_t e = svdup_s32(5);
    svint32_t f = svdup_s32(6);
    svint32_t g = svdup_s32(7);
    svint32_t h = svdup_s32(8);
    svint32_t i = svdup_s32(9);
    svint32_t j = svdup_s32(10);
    
    /* Complex expression that might use multi-operand instructions */
    svint32_t result = svadd_s32_z(svptrue_b32(),
                          svmla_s32_z(svptrue_b32(), a, b, c),
                          svmla_s32_z(svptrue_b32(), d, e, f));
    
    /* Additional operations to increase operand count */
    result = svmad_s32_z(svptrue_b32(), result, g, h);
    result = svmls_s32_z(svptrue_b32(), result, i, j);
    
    /* Inline assembly with 10 operands */
    uint32_t out1, out2, out3, out4, out5;
    uint32_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    uint32_t in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    
    asm volatile (
        /* 10-operand inline assembly */
        "add %0, %1, %2\n\t"
        "add %3, %4, %5\n\t"
        "add %6, %7, %8\n\t"
        "add %9, %0, %3"
        : "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4), "=r"(out5)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "r"(in6), "r"(in7), "r"(in8), "r"(in9), "r"(in10)
        : "cc"
    );
}
#endif
#endif

/* PowerPC VSX specific code */
#ifdef __powerpc64__
#include <altivec.h>

void test_powerpc_vsx_multi_operand() {
    vector double a = {1.0, 2.0};
    vector double b = {3.0, 4.0};
    vector double c = {5.0, 6.0};
    vector double d = {7.0, 8.0};
    vector double e = {9.0, 10.0};
    vector double f = {11.0, 12.0};
    vector double g = {13.0, 14.0};
    vector double h = {15.0, 16.0};
    vector double i = {17.0, 18.0};
    vector double j = {19.0, 20.0};
    
    /* Complex VSX expression */
    vector double result = vec_madd(a, b, 
                           vec_madd(c, d,
                           vec_madd(e, f,
                           vec_madd(g, h,
                           vec_add(i, j)))));
    
    /* Inline assembly with mixed constraints */
    double out1, out2, out3, out4, out5;
    double in1 = 1.0, in2 = 2.0, in3 = 3.0, in4 = 4.0, in5 = 5.0;
    double in6 = 6.0, in7 = 7.0, in8 = 8.0, in9 = 9.0, in10 = 10.0;
    double *mem_ptr = &in1;
    
    asm volatile (
        /* 11 operands with mixed constraints */
        "lfd %0, 0(%1)\n\t"
        "lfd %2, 8(%1)\n\t"
        "lfd %3, 16(%1)\n\t"
        "lfd %4, 24(%1)\n\t"
        "lfd %5, 32(%1)\n\t"
        "fmadd %0, %2, %3, %4\n\t"
        "fmadd %5, %6, %7, %8\n\t"
        "fmadd %9, %10, %0, %5"
        : "=f"(out1), "=f"(out2), "=f"(out3), "=f"(out4), "=f"(out5)
        : "r"(mem_ptr), "f"(in1), "f"(in2), "f"(in3), "f"(in4),
          "f"(in5), "f"(in6), "f"(in7), "f"(in8), "f"(in9), "f"(in10)
        : "memory"
    );
}
#endif

/* Generic fallback for testing atomic operations and complex expressions */
#ifdef GENERIC_FALLBACK
void test_generic_multi_operand() {
    /* Test atomic built-in with many parameters */
    uint64_t atomic_var = 0;
    uint64_t expected = 0;
    uint64_t desired = 42;
    uint64_t* weak = 0;
    
    /* __atomic_compare_exchange with 6 parameters - may expand to multi-operand */
    int atomic_result = __atomic_compare_exchange(&atomic_var, &expected, &desired,
                                                  1, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Complex arithmetic expression with 10+ variables */
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0, e = 5.0;
    double f = 6.0, g = 7.0, h = 8.0, i = 9.0, j = 10.0;
    double k = 11.0, l = 12.0, m = 13.0, n = 14.0, o = 15.0;
    
    /* Expression that might be combined into multi-operand instruction */
    double result = a * b + c * d + e * f + g * h + i * j + k * l + m * n + o;
    
    /* Bit-field operations across multiple words */
    struct {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
        unsigned int field4 : 16;
    } bitfields = {0};
    
    /* Multiple bit-field assignments */
    bitfields.field1 = 7;
    bitfields.field2 = 31;
    bitfields.field3 = 255;
    bitfields.field4 = 65535;
    
    /* Inline assembly with exactly 11 operands */
    int out1, out2, out3, out4, out5;
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int in11 = 11;
    
    asm volatile (
        /* 11-operand inline assembly */
        "add %0, %1, %2\n\t"
        "add %3, %4, %5\n\t"
        "add %6, %7, %8\n\t"
        "add %9, %10, %0\n\t"
        "add %9, %9, %3"
        : "=r"(out1), "=r"(out2), "=r"(out3), "=r"(out4), "=r"(out5)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), "r"(in5),
          "r"(in6), "r"(in7), "r"(in8), "r"(in9), "r"(in10), "r"(in11),
          "0"(out1), "1"(out2), "2"(out3), "3"(out4), "4"(out5)
        : "cc"
    );
}
#endif

/* Test decimal floating-point built-ins if available */
#ifdef __DECIMAL_BID_FORMAT__
#include <bid_decimal_data.h>
#include <bid_functions.h>

void test_decimal_multi_operand() {
    /* Decimal floating-point operations with many operands */
    BID_UINT128 a = {0}, b = {0}, c = {0}, d = {0}, e = {0};
    BID_UINT128 f = {0}, g = {0}, h = {0}, i = {0}, j = {0};
    
    /* Complex decimal expression */
    BID_UINT128 result = __bid128_add(a, 
                          __bid128_mul(b,
                          __bid128_add(c,
                          __bid128_mul(d,
                          __bid128_add(e,
                          __bid128_mul(f,
                          __bid128_add(g,
                          __bid128_mul(h,
                          __bid128_add(i, j)))))))));
}
#endif

/* Main driver function */
int main() {
    int total = 0;
    
    /* Call architecture-specific tests */
#ifdef __x86_64__
    test_avx512_multi_operand();
    total += 1;
#endif
    
#ifdef __ARM_ARCH
#ifdef __ARM_FEATURE_SVE
    test_arm_sve_multi_operand();
    total += 2;
#endif
#endif
    
#ifdef __powerpc64__
    test_powerpc_vsx_multi_operand();
    total += 3;
#endif
    
#ifdef GENERIC_FALLBACK
    test_generic_multi_operand();
    total += 4;
#endif
    
#ifdef __DECIMAL_BID_FORMAT__
    test_decimal_multi_operand();
    total += 5;
#endif
    
    /* Complex expression with many variables to force combining */
    double v1 = 1.1, v2 = 2.2, v3 = 3.3, v4 = 4.4, v5 = 5.5;
    double v6 = 6.6, v7 = 7.7, v8 = 8.8, v9 = 9.9, v10 = 10.10;
    double v11 = 11.11, v12 = 12.12, v13 = 13.13, v14 = 14.14, v15 = 15.15;
    
    /* This expression uses 15 variables - compiler might combine into multi-operand */
    double final_result = 
        v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8 + v9 * v10 +
        v11 * v12 + v13 * v14 + v15;
    
    /* Use result to prevent dead code elimination */
    printf("Test completed. Total architecture flags: %d\n", total);
    printf("Final computation result: %f\n", final_result);
    
    return (int)final_result + total;
}
