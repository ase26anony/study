/* Test program to cover x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long val) {
    checksum = checksum * 31 + val;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    
    /* Test UNORDERED (unord) - unordered comparison */
    res = isunordered(d1, d2);
    mix(res);
    
    /* Test ORDERED (ord) - ordered comparison */
    res = !isunordered(d1, d2);
    mix(res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = !isgreater(d1, d2) && !isless(d1, d2);
    mix(res);
    
    /* Test UNGE (nlt) - not less than (greater or equal or unordered) */
    res = !isless(d1, d2);
    mix(res);
    
    /* Test UNGT (nle) - not less than or equal (greater or unordered) */
    res = !islessequal(d1, d2);
    mix(res);
    
    /* Test UNLE (ule) - unordered or less or equal */
    res = islessequal(d1, d2) || isunordered(d1, d2);
    mix(res);
    
    /* Test UNLT (ult) - unordered or less than */
    res = isless(d1, d2) || isunordered(d1, d2);
    mix(res);
    
    /* Test LTGT (une) - less than or greater than (not equal and ordered) */
    res = (d1 < d2) || (d1 > d2);
    mix(res);
    
    /* Repeat with float */
    res = isunordered(f1, f2);
    mix(res);
    res = !isunordered(f1, f2);
    mix(res);
    res = !isgreater(f1, f2) && !isless(f1, f2);
    mix(res);
}

/* Test with SSE2 vector conditions */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED (unord) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test ORDERED (ord) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNEQ (ueq) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNGE (nlt) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNGT (nle) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNLE (ule) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNLT (ult) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test LTGT (une) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Repeat with float vectors */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(*(unsigned*)&fres[i]);
    
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(*(unsigned*)&fres[i]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    volatile int result;
    volatile __m128d va, vb;
    volatile __m128 vfa, vfb;
    
    /* Initialize vectors */
    va = _mm_set_pd(a, b);
    vb = _mm_set_pd(b, a);
    vfa = _mm_set_ps(fa, fb, fa, fb);
    vfb = _mm_set_ps(fb, fa, fb, fa);
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(result);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(result);
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(result);
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(result);
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(result);
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(result);
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(result);
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(result);
    
    /* Test with float (cmpps) */
    __asm__ volatile (
        "cmpps %2, %1, %{%0|unord}\n\t"
        "movmskps %1, %0"
        : "=r"(result) : "x"(vfa), "x"(vfb) : "cc"
    );
    mix(result);
    
    __asm__ volatile (
        "cmpps %2, %1, %{%0|ord}\n\t"
        "movmskps %1, %0"
        : "=r"(result) : "x"(vfa), "x"(vfb) : "cc"
    );
    mix(result);
}

/* Complex branching with uncommon conditions */
__attribute__((optimize("O3"), noinline))
void test_complex_branching(double* arr, int n) {
    volatile int count_unord = 0;
    volatile int count_ord = 0;
    volatile int count_ueq = 0;
    volatile int count_nlt = 0;
    volatile int count_nle = 0;
    volatile int count_ule = 0;
    volatile int count_ult = 0;
    volatile int count_une = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Complex nested conditions to prevent optimization */
        if (isunordered(a, b)) {
            count_unord++;
            if (!isgreater(a, b) && !isless(a, b)) {
                count_ueq += 2;
            }
        } else {
            count_ord++;
            if (!isless(a, b)) {
                count_nlt++;
                if (!islessequal(a, b)) {
                    count_nle++;
                }
            }
        }
        
        if (islessequal(a, b) || isunordered(a, b)) {
            count_ule++;
        }
        
        if (isless(a, b) || isunordered(a, b)) {
            count_ult++;
        }
        
        if ((a < b) || (a > b)) {
            count_une++;
        }
    }
    
    mix(count_unord);
    mix(count_ord);
    mix(count_ueq);
    mix(count_nlt);
    mix(count_nle);
    mix(count_ule);
    mix(count_ult);
    mix(count_une);
}

int main(int argc, char** argv) {
    /* Initialize with some variation */
    unsigned seed = (argc > 1) ? (unsigned)atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values (including some NaN/inf) */
    double darr[20];
    float farr[20];
    
    for (int i = 0; i < 20; i++) {
        darr[i] = (rand() % 100) * 0.1 - 5.0;
        farr[i] = (rand() % 100) * 0.1f - 5.0f;
    }
    
    /* Insert some special values */
    darr[5] = 0.0 / 0.0;  /* NaN */
    darr[10] = 1.0 / 0.0; /* +Inf */
    darr[15] = -1.0 / 0.0; /* -Inf */
    
    farr[3] = 0.0f / 0.0f;
    farr[8] = 1.0f / 0.0f;
    farr[12] = -1.0f / 0.0f;
    
    /* Test scalar conditions */
    for (int i = 0; i < 19; i++) {
        test_scalar_conditions(darr[i], darr[i+1], farr[i], farr[i+1]);
    }
    
    /* Test vector conditions */
    for (int i = 0; i < 18; i += 2) {
        __m128d vd1 = _mm_set_pd(darr[i], darr[i+1]);
        __m128d vd2 = _mm_set_pd(darr[i+1], darr[i]);
        __m128 vf1 = _mm_set_ps(farr[i], farr[i+1], farr[i], farr[i+1]);
        __m128 vf2 = _mm_set_ps(farr[i+1], farr[i], farr[i+1], farr[i]);
        test_vector_conditions(vd1, vd2, vf1, vf2);
    }
    
    /* Test inline assembly */
    for (int i = 0; i < 10; i++) {
        test_inline_asm_conditions(darr[i], darr[19-i], farr[i], farr[19-i]);
    }
    
    /* Test complex branching */
    test_complex_branching(darr, 20);
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
