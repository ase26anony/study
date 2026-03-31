/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Test scalar floating-point conditions using math.h macros */
__attribute__((optimize("O0")))
void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* UNORDERED: isunordered */
    res = isunordered(a, b);
    g_checksum += res;
    
    /* ORDERED: !isunordered */
    res = !isunordered(fa, fb);
    g_checksum += res;
    
    /* UNEQ: unordered or equal - simulate with isunordered || a == b */
    res = (isunordered(a, b) || (a == b));
    g_checksum += res;
    
    /* UNGE: !isless - not less than (greater or equal or unordered) */
    res = !isless(a, b);
    g_checksum += res;
    
    /* UNGT: !islessequal - greater than or unordered */
    res = !islessequal(a, b);
    g_checksum += res;
    
    /* UNLE: islessequal or unordered */
    res = (islessequal(a, b) || isunordered(a, b));
    g_checksum += res;
    
    /* UNLT: isless or unordered */
    res = (isless(a, b) || isunordered(a, b));
    g_checksum += res;
    
    /* LTGT: less or greater (ordered and not equal) */
    res = (isless(a, b) || isgreater(a, b));
    g_checksum += res;
}

/* Test with SSE2 vector comparisons */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* UNORDERED: _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    /* ORDERED: _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    /* UNEQ: _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    /* UNGE: _CMP_NLT_UQ (not less than) */
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_NLT_UQ);
    _mm_storeu_ps((float*)fres, cmp_resf);
    g_checksum += (int)fres[0] + (int)fres[1] + (int)fres[2] + (int)fres[3];
    
    /* UNGT: _CMP_NLE_UQ (not less or equal) */
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_NLE_UQ);
    _mm_storeu_ps((float*)fres, cmp_resf);
    g_checksum += (int)fres[0] + (int)fres[1] + (int)fres[2] + (int)fres[3];
    
    /* UNLE: _CMP_LE_UQ (less or equal or unordered) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    /* UNLT: _CMP_LT_UQ (less than or unordered) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    /* LTGT: _CMP_NEQ_OQ (not equal and ordered) */
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_NEQ_OQ);
    _mm_storeu_ps((float*)fres, cmp_resf);
    g_checksum += (int)fres[0] + (int)fres[1] + (int)fres[2] + (int)fres[3];
}

/* Test inline assembly with condition code mnemonics */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, __m128d v1, __m128d v2) {
    volatile int res_int = 0;
    volatile __m128d res_vec;
    volatile double dres[2];
    
    /* UNORDERED: "unord" */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movd %0, %k0"
        : "=r"(res_int) : "x"(a), "x"(b) : "cc"
    );
    g_checksum += res_int;
    
    /* ORDERED: "ord" */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movd %0, %k0"
        : "=r"(res_int) : "x"(a), "x"(b) : "cc"
    );
    g_checksum += res_int;
    
    /* UNEQ: "ueq" */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movd %0, %k0"
        : "=r"(res_int) : "x"(a), "x"(b) : "cc"
    );
    g_checksum += res_int;
    
    /* UNGE: "nlt" */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movd %0, %k0"
        : "=r"(res_int) : "x"(a), "x"(b) : "cc"
    );
    g_checksum += res_int;
    
    /* UNGT: "nle" */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movd %0, %k0"
        : "=r"(res_int) : "x"(a), "x"(b) : "cc"
    );
    g_checksum += res_int;
    
    /* UNLE: "ule" */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        : "=x"(res_vec) : "x"(v1), "x"(v2)
    );
    _mm_storeu_pd((double*)dres, res_vec);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    /* UNLT: "ult" */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}\n\t"
        : "=x"(res_vec) : "x"(v1), "x"(v2)
    );
    _mm_storeu_pd((double*)dres, res_vec);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    /* LTGT: "une" */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        : "=x"(res_vec) : "x"(v1), "x"(v2)
    );
    _mm_storeu_pd((double*)dres, res_vec);
    g_checksum += (int)dres[0] + (int)dres[1];
}

/* Complex branching to force condition code generation */
__attribute__((optimize("O3"), noinline))
void test_complex_branching(double* arr, int n) {
    volatile int count = 0;
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Mix of different conditions in branching */
        if (isunordered(a, b)) {
            count += 1;  /* UNORDERED */
        } else if (!isless(a, b) && !isgreater(a, b)) {
            count += 2;  /* UNEQ (ordered equal) */
        } else if (!isless(a, b)) {
            count += 3;  /* UNGE */
        } else if (isless(a, b) || isunordered(a, b)) {
            count += 4;  /* UNLT */
        }
        
        /* Ternary with LTGT */
        double c = (isless(a, b) || isgreater(a, b)) ? a : b;
        g_checksum += (int)c;
    }
    g_checksum += count;
}

int main(int argc, char** argv) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[10];
    float farr[10];
    for (int i = 0; i < 10; i++) {
        darr[i] = (rand() % 100) * 0.1;
        farr[i] = (rand() % 100) * 0.1f;
        if (i == 3) darr[i] = 0.0 / 0.0;  /* NaN */
        if (i == 5) darr[i] = 1.0 / 0.0;  /* +Inf */
        if (i == 7) darr[i] = -1.0 / 0.0; /* -Inf */
    }
    
    /* Test scalar conditions */
    test_scalar_conditions(darr[0], darr[1], farr[0], farr[1]);
    test_scalar_conditions(darr[2], darr[3], farr[2], farr[3]);  /* Includes NaN */
    
    /* Test vector conditions */
    __m128d v1 = _mm_set_pd(darr[0], darr[1]);
    __m128d v2 = _mm_set_pd(darr[2], darr[3]);
    __m128 f1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 f2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    test_vector_conditions(v1, v2, f1, f2);
    
    /* Test inline assembly */
    test_inline_asm_conditions(darr[4], darr[5], v1, v2);
    
    /* Test complex branching */
    test_complex_branching(darr, 10);
    
    printf("Final checksum: %lu\n", g_checksum);
    return 0;
}
