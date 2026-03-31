/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Test functions with different optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* Test UNORDERED (unord) - unordered comparison */
    result = isunordered(a, b);
    g_checksum += result;
    
    /* Test ORDERED (ord) - ordered comparison */
    result = !isunordered(a, b);
    g_checksum += result;
    
    /* Test UNEQ (ueq) - unordered or equal */
    result = !isgreater(a, b) && !isless(a, b);
    g_checksum += result;
    
    /* Test UNGE (nlt) - not less than (greater or equal or unordered) */
    result = !isless(a, b);
    g_checksum += result;
    
    /* Test UNGT (nle) - not less than or equal (greater or unordered) */
    result = !islessequal(a, b);
    g_checksum += result;
    
    /* Test UNLE (ule) - less than or equal or unordered */
    result = islessequal(a, b) || isunordered(a, b);
    g_checksum += result;
    
    /* Test UNLT (ult) - less than or unordered */
    result = isless(a, b) || isunordered(a, b);
    g_checksum += result;
    
    /* Test LTGT (une) - less than or greater than (not equal and ordered) */
    result = (isless(a, b) || isgreater(a, b)) && !isunordered(a, b);
    g_checksum += result;
    
    /* Force branching with volatile variables */
    volatile double v1 = a;
    volatile double v2 = b;
    
    if (isunordered(v1, v2)) {
        g_checksum += 1;
    }
    if (!isunordered(v1, v2)) {
        g_checksum += 2;
    }
    if (!isgreater(v1, v2) && !isless(v1, v2)) {
        g_checksum += 3;
    }
    if (!isless(v1, v2)) {
        g_checksum += 4;
    }
    if (!islessequal(v1, v2)) {
        g_checksum += 5;
    }
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d va, __m128d vb, __m128 vfa, __m128 vfb) {
    __m128d cmp_result;
    __m128 cmp_result_f;
    volatile double d_result[2];
    volatile float f_result[4];
    
    /* Test UNORDERED (unord) for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_UNORD_Q);
    _mm_storeu_pd(d_result, cmp_result);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
    
    /* Test ORDERED (ord) for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_ORD_Q);
    _mm_storeu_pd(d_result, cmp_result);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
    
    /* Test UNEQ (ueq) for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_EQ_UQ);
    _mm_storeu_pd(d_result, cmp_result);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
    
    /* Test UNGE (nlt) for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_NLT_UQ);
    _mm_storeu_pd(d_result, cmp_result);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
    
    /* Test UNGT (nle) for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_NLE_UQ);
    _mm_storeu_pd(d_result, cmp_result);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
    
    /* Test UNLE (ule) for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_LE_OS);
    _mm_storeu_pd(d_result, cmp_result);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
    
    /* Test UNLT (ult) for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_LT_OS);
    _mm_storeu_pd(d_result, cmp_result);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
    
    /* Test LTGT (une) for vectors */
    cmp_result = _mm_cmp_pd(va, vb, _CMP_NEQ_OS);
    _mm_storeu_pd(d_result, cmp_result);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
    
    /* Test with float vectors */
    cmp_result_f = _mm_cmp_ps(vfa, vfb, _CMP_UNORD_Q);
    _mm_storeu_ps(f_result, cmp_result_f);
    g_checksum += (unsigned long)f_result[0] + (unsigned long)f_result[1] +
                  (unsigned long)f_result[2] + (unsigned long)f_result[3];
    
    /* Conditional moves based on vector comparisons */
    __m128d mask = _mm_cmp_pd(va, vb, _CMP_UNORD_Q);
    __m128d blended = _mm_blendv_pd(va, vb, mask);
    _mm_storeu_pd(d_result, blended);
    g_checksum += (unsigned long)d_result[0] + (unsigned long)d_result[1];
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b, __m128d va, __m128d vb) {
    double result_d;
    __m128d result_vec;
    volatile double v_result[2];
    
    /* Inline assembly with explicit condition code mnemonics */
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        : "=x"(result_d)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += (unsigned long)result_d;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        : "=x"(result_d)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += (unsigned long)result_d;
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        : "=x"(result_d)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += (unsigned long)result_d;
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        : "=x"(result_d)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += (unsigned long)result_d;
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        : "=x"(result_d)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += (unsigned long)result_d;
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        : "=x"(result_d)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += (unsigned long)result_d;
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        : "=x"(result_d)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += (unsigned long)result_d;
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        : "=x"(result_d)
        : "x"(a), "x"(b)
        : "cc"
    );
    g_checksum += (unsigned long)result_d;
    
    /* Vector version with cmppd */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        : "=x"(result_vec)
        : "x"(va), "x"(vb)
        : "cc"
    );
    _mm_storeu_pd(v_result, result_vec);
    g_checksum += (unsigned long)v_result[0] + (unsigned long)v_result[1];
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        : "=x"(result_vec)
        : "x"(va), "x"(vb)
        : "cc"
    );
    _mm_storeu_pd(v_result, result_vec);
    g_checksum += (unsigned long)v_result[0] + (unsigned long)v_result[1];
}

__attribute__((optimize("O3"), target("sse2")))
void test_mixed_conditions_O3(double a, double b, float fa, float fb) {
    volatile double vd1 = a;
    volatile double vd2 = b;
    volatile float vf1 = fa;
    volatile float vf2 = fb;
    
    /* Complex nested conditions to prevent optimization */
    for (int i = 0; i < 3; i++) {
        if (isunordered(vd1, vd2)) {
            g_checksum += i * 10;
            if (!isgreater(vd1, vd2) && !isless(vd1, vd2)) {
                g_checksum += i * 20;
            }
        } else {
            g_checksum += i * 30;
            if (isless(vd1, vd2) || isgreater(vd1, vd2)) {
                g_checksum += i * 40;
            }
        }
        
        /* Ternary operator with uncommon conditions */
        double temp = (!isless(vd1, vd2)) ? vd1 * 2.0 : vd2 * 3.0;
        g_checksum += (unsigned long)temp;
        
        temp = (!islessequal(vd1, vd2)) ? vd1 * 4.0 : vd2 * 5.0;
        g_checksum += (unsigned long)temp;
        
        temp = (islessequal(vd1, vd2) || isunordered(vd1, vd2)) ? 
               vd1 * 6.0 : vd2 * 7.0;
        g_checksum += (unsigned long)temp;
        
        temp = (isless(vd1, vd2) || isunordered(vd1, vd2)) ? 
               vd1 * 8.0 : vd2 * 9.0;
        g_checksum += (unsigned long)temp;
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = (argc > 1) ? (unsigned long)argv[1] : 12345;
    srand(seed);
    
    /* Create test data with various values including NaN, infinity, normal */
    double d_values[] = {
        1.0, 2.0, 0.0, -1.0, 
        INFINITY, -INFINITY, NAN, 3.14159
    };
    
    float f_values[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        INFINITY, -INFINITY, NAN, 2.71828f
    };
    
    /* Initialize vector data */
    __m128d vec_d1 = _mm_set_pd(d_values[0], d_values[1]);
    __m128d vec_d2 = _mm_set_pd(d_values[2], d_values[3]);
    __m128d vec_d3 = _mm_set_pd(d_values[4], d_values[5]);
    __m128d vec_d4 = _mm_set_pd(d_values[6], d_values[7]);
    
    __m128 vec_f1 = _mm_set_ps(f_values[0], f_values[1], f_values[2], f_values[3]);
    __m128 vec_f2 = _mm_set_ps(f_values[4], f_values[5], f_values[6], f_values[7]);
    
    /* Run all test functions with different combinations */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i != j) {  /* Avoid comparing identical values */
                test_scalar_conditions_O0(d_values[i], d_values[j], 
                                         f_values[i], f_values[j]);
                test_mixed_conditions_O3(d_values[i], d_values[j],
                                        f_values[i], f_values[j]);
            }
        }
    }
    
    /* Test vector conditions */
    test_vector_conditions_O2(vec_d1, vec_d2, vec_f1, vec_f2);
    test_vector_conditions_O2(vec_d3, vec_d4, vec_f1, vec_f2);
    
    /* Test inline assembly */
    test_inline_asm_conditions(d_values[0], d_values[6], vec_d1, vec_d3);
    test_inline_asm_conditions(d_values[4], d_values[7], vec_d2, vec_d4);
    
    printf("Final checksum: %lu\n", g_checksum);
    return 0;
}
