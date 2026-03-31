/* Test program to cover x86 condition code output logic in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Test functions with different optimization attributes */
__attribute__((optimize("O0")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* Test UNORDERED (unord) - unordered comparison */
    result = isunordered(a, b);
    checksum += result;
    
    /* Test ORDERED (ord) - ordered comparison */
    result = !isunordered(a, b);
    checksum += result;
    
    /* Test UNEQ (ueq) - unordered or equal */
    result = !isgreater(a, b) && !isless(a, b);
    checksum += result;
    
    /* Test UNGE (nlt) - not less than (greater or equal or unordered) */
    result = !isless(a, b);
    checksum += result;
    
    /* Test UNGT (nle) - not less than or equal (greater or unordered) */
    result = !islessequal(a, b);
    checksum += result;
    
    /* Test UNLE (ule) - unordered or less than or equal */
    result = islessequal(a, b) || isunordered(a, b);
    checksum += result;
    
    /* Test UNLT (ult) - unordered or less than */
    result = isless(a, b) || isunordered(a, b);
    checksum += result;
    
    /* Test LTGT (une) - less than or greater than (not equal and ordered) */
    result = (isless(a, b) || isgreater(a, b)) && !isunordered(a, b);
    checksum += result;
    
    /* Use results in conditional branches to force code generation */
    if (isunordered(fa, fb)) checksum += 1;
    if (!isunordered(fa, fb)) checksum += 2;
    if (!isgreater(fa, fb) && !isless(fa, fb)) checksum += 3;
    if (!isless(fa, fb)) checksum += 4;
    if (!islessequal(fa, fb)) checksum += 5;
    if (islessequal(fa, fb) || isunordered(fa, fb)) checksum += 6;
    if (isless(fa, fb) || isunordered(fa, fb)) checksum += 7;
    if ((isless(fa, fb) || isgreater(fa, fb)) && !isunordered(fa, fb)) checksum += 8;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_result_d;
    __m128 cmp_result_f;
    volatile double d_result[2];
    volatile float f_result[4];
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    cmp_result_d = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)d_result, cmp_result_d);
    checksum += (int)d_result[0] + (int)d_result[1];
    
    /* Test ORDERED - _CMP_ORD_Q */
    cmp_result_d = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)d_result, cmp_result_d);
    checksum += (int)d_result[0] + (int)d_result[1];
    
    /* Test UNEQ - _CMP_EQ_UQ */
    cmp_result_d = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)d_result, cmp_result_d);
    checksum += (int)d_result[0] + (int)d_result[1];
    
    /* Test UNGE - _CMP_NGE_UQ (not less than) */
    cmp_result_d = _mm_cmp_pd(v1, v2, _CMP_NGE_UQ);
    _mm_storeu_pd((double*)d_result, cmp_result_d);
    checksum += (int)d_result[0] + (int)d_result[1];
    
    /* Test UNGT - _CMP_NGT_UQ (not less than or equal) */
    cmp_result_d = _mm_cmp_pd(v1, v2, _CMP_NGT_UQ);
    _mm_storeu_pd((double*)d_result, cmp_result_d);
    checksum += (int)d_result[0] + (int)d_result[1];
    
    /* Test UNLE - _CMP_NLE_UQ (unordered or less than or equal) */
    cmp_result_d = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)d_result, cmp_result_d);
    checksum += (int)d_result[0] + (int)d_result[1];
    
    /* Test UNLT - _CMP_NLT_UQ (unordered or less than) */
    cmp_result_d = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)d_result, cmp_result_d);
    checksum += (int)d_result[0] + (int)d_result[1];
    
    /* Test LTGT - _CMP_NEQ_OQ (not equal and ordered) */
    cmp_result_d = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)d_result, cmp_result_d);
    checksum += (int)d_result[0] + (int)d_result[1];
    
    /* Repeat for single precision floats */
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)f_result, cmp_result_f);
    checksum += (int)f_result[0] + (int)f_result[1] + (int)f_result[2] + (int)f_result[3];
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    _mm_storeu_ps((float*)f_result, cmp_result_f);
    checksum += (int)f_result[0] + (int)f_result[1] + (int)f_result[2] + (int)f_result[3];
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_EQ_UQ);
    _mm_storeu_ps((float*)f_result, cmp_result_f);
    checksum += (int)f_result[0] + (int)f_result[1] + (int)f_result[2] + (int)f_result[3];
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_NGE_UQ);
    _mm_storeu_ps((float*)f_result, cmp_result_f);
    checksum += (int)f_result[0] + (int)f_result[1] + (int)f_result[2] + (int)f_result[3];
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_NGT_UQ);
    _mm_storeu_ps((float*)f_result, cmp_result_f);
    checksum += (int)f_result[0] + (int)f_result[1] + (int)f_result[2] + (int)f_result[3];
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_NLE_UQ);
    _mm_storeu_ps((float*)f_result, cmp_result_f);
    checksum += (int)f_result[0] + (int)f_result[1] + (int)f_result[2] + (int)f_result[3];
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_NLT_UQ);
    _mm_storeu_ps((float*)f_result, cmp_result_f);
    checksum += (int)f_result[0] + (int)f_result[1] + (int)f_result[2] + (int)f_result[3];
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_NEQ_OQ);
    _mm_storeu_ps((float*)f_result, cmp_result_f);
    checksum += (int)f_result[0] + (int)f_result[1] + (int)f_result[2] + (int)f_result[3];
}

__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile int cc_result;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += cc_result;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += cc_result;
    
    /* Test UNEQ (ueq) - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "setp %%bl\n\t"
        "orb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "bl", "cc"
    );
    checksum += cc_result;
    
    /* Test UNGE (nlt) - not less than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setae %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += cc_result;
    
    /* Test UNGT (nle) - not less than or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += cc_result;
    
    /* Test UNLE (ule) - unordered or less than or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += cc_result;
    
    /* Test UNLT (ult) - unordered or less than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += cc_result;
    
    /* Test LTGT (une) - not equal and ordered */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "setnp %%bl\n\t"
        "andb %%bl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "al", "bl", "cc"
    );
    checksum += cc_result;
}

__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions_O3(__m256d v1, __m256d v2) {
    __m256d cmp_result;
    volatile double d_result[4];
    
    /* Test various conditions with AVX */
    cmp_result = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm256_storeu_pd((double*)d_result, cmp_result);
    checksum += (int)d_result[0] + (int)d_result[1] + (int)d_result[2] + (int)d_result[3];
    
    cmp_result = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm256_storeu_pd((double*)d_result, cmp_result);
    checksum += (int)d_result[0] + (int)d_result[1] + (int)d_result[2] + (int)d_result[3];
    
    cmp_result = _mm256_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm256_storeu_pd((double*)d_result, cmp_result);
    checksum += (int)d_result[0] + (int)d_result[1] + (int)d_result[2] + (int)d_result[3];
    
    cmp_result = _mm256_cmp_pd(v1, v2, _CMP_NGE_UQ);
    _mm256_storeu_pd((double*)d_result, cmp_result);
    checksum += (int)d_result[0] + (int)d_result[1] + (int)d_result[2] + (int)d_result[3];
    
    cmp_result = _mm256_cmp_pd(v1, v2, _CMP_NGT_UQ);
    _mm256_storeu_pd((double*)d_result, cmp_result);
    checksum += (int)d_result[0] + (int)d_result[1] + (int)d_result[2] + (int)d_result[3];
    
    cmp_result = _mm256_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm256_storeu_pd((double*)d_result, cmp_result);
    checksum += (int)d_result[0] + (int)d_result[1] + (int)d_result[2] + (int)d_result[3];
    
    cmp_result = _mm256_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm256_storeu_pd((double*)d_result, cmp_result);
    checksum += (int)d_result[0] + (int)d_result[1] + (int)d_result[2] + (int)d_result[3];
    
    cmp_result = _mm256_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm256_storeu_pd((double*)d_result, cmp_result);
    checksum += (int)d_result[0] + (int)d_result[1] + (int)d_result[2] + (int)d_result[3];
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values including NaN, Inf, normal numbers */
    volatile double d_values[] = {
        1.0, 2.0, -1.0, 0.0, 
        INFINITY, -INFINITY, 
        NAN, -NAN,
        3.14, -2.71
    };
    
    volatile float f_values[] = {
        1.0f, 2.0f, -1.0f, 0.0f,
        INFINITY, -INFINITY,
        NAN, -NAN,
        3.14f, -2.71f
    };
    
    /* Initialize vector values */
    __m128d vd1 = _mm_set_pd(1.0, 2.0);
    __m128d vd2 = _mm_set_pd(NAN, -INFINITY);
    __m128 vf1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vf2 = _mm_set_ps(NAN, INFINITY, -INFINITY, 0.0f);
    __m256d avx_v1 = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    __m256d avx_v2 = _mm256_set_pd(NAN, INFINITY, -INFINITY, 0.0);
    
    printf("Starting condition code coverage test...\n");
    
    /* Test with various combinations of values */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            test_scalar_conditions_O0(d_values[i], d_values[j], f_values[i], f_values[j]);
            test_inline_asm_conditions(d_values[i], d_values[j]);
        }
    }
    
    /* Test vector conditions */
    test_vector_conditions_O2(vd1, vd2, vf1, vf2);
    
    /* Test AVX conditions if available */
    #ifdef __AVX__
    test_avx_conditions_O3(avx_v1, avx_v2);
    #endif
    
    printf("Final checksum: %lu\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
