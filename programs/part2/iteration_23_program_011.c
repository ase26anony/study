/* Test program to cover x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *c, unsigned long v) {
    *c = (*c * 33) ^ v;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    
    /* Test UNORDERED (unord) */
    res = isunordered(d1, d2);
    mix(&checksum, *(unsigned long*)&res);
    
    /* Test ORDERED (ord) */
    res = !isunordered(d1, d2);
    mix(&checksum, *(unsigned long*)&res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = (isunordered(d1, d2) || (d1 == d2));
    mix(&checksum, *(unsigned long*)&res);
    
    /* Test UNGE (nlt) - unordered or greater or equal */
    res = (isunordered(d1, d2) || (d1 >= d2));
    mix(&checksum, *(unsigned long*)&res);
    
    /* Test UNGT (nle) - unordered or greater */
    res = (isunordered(d1, d2) || (d1 > d2));
    mix(&checksum, *(unsigned long*)&res);
    
    /* Test UNLE (ule) - unordered or less or equal */
    res = (isunordered(d1, d2) || (d1 <= d2));
    mix(&checksum, *(unsigned long*)&res);
    
    /* Test UNLT (ult) - unordered or less */
    res = (isunordered(d1, d2) || (d1 < d2));
    mix(&checksum, *(unsigned long*)&res);
    
    /* Test LTGT (une) - less or greater (ordered and not equal) */
    res = (!isunordered(d1, d2) && (d1 != d2));
    mix(&checksum, *(unsigned long*)&res);
    
    /* Repeat with floats */
    res = isunordered(f1, f2);
    mix(&checksum, *(unsigned long*)&res);
    
    res = !isunordered(f1, f2);
    mix(&checksum, *(unsigned long*)&res);
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 fv1, __m128 fv2) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test ORDERED - _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNEQ - _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNGE - _CMP_GE_OQ (ordered greater or equal) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_GE_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNGT - _CMP_GT_OQ (ordered greater) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_GT_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNLE - _CMP_LE_OQ (ordered less or equal) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNLT - _CMP_LT_OQ (ordered less) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test LTGT - _CMP_NEQ_OQ (ordered not equal) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test with float vectors */
    cmp_resf = _mm_cmp_ps(fv1, fv2, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++)
        mix(&checksum, *(unsigned long*)&fres[i]);
    
    cmp_resf = _mm_cmp_ps(fv1, fv2, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++)
        mix(&checksum, *(unsigned long*)&fres[i]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, __m128d va, __m128d vb) {
    volatile double result;
    volatile __m128d vresult;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc");
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc");
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc");
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc");
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc");
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc");
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc");
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(a), "x"(b) : "cc");
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test with vector version */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movapd %1, %0"
        : "=x"(vresult) : "x"(va), "x"(vb) : "cc");
    mix(&checksum, *(unsigned long*)&vresult);
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movapd %1, %0"
        : "=x"(vresult) : "x"(va), "x"(vb) : "cc");
    mix(&checksum, *(unsigned long*)&vresult);
}

/* Complex branching to force condition code generation */
__attribute__((optimize("O2"), noinline))
void test_complex_branching(double *arr, int n) {
    volatile int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Complex condition using UNORDERED */
        if (isunordered(a, b)) {
            count++;
        } else if (!isunordered(a, b) && (a != b)) { /* LTGT */
            count += 2;
        } else if (isunordered(a, b) || (a == b)) { /* UNEQ */
            count += 3;
        } else if (isunordered(a, b) || (a >= b)) { /* UNGE */
            count += 4;
        } else if (isunordered(a, b) || (a > b)) { /* UNGT */
            count += 5;
        } else if (isunordered(a, b) || (a <= b)) { /* UNLE */
            count += 6;
        } else if (isunordered(a, b) || (a < b)) { /* UNLT */
            count += 7;
        }
    }
    
    mix(&checksum, count);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = (argc > 1) ? strtoul(argv[1], NULL, 0) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[16];
    float farr[16];
    
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: darr[i] = (double)rand() / RAND_MAX; break;
            case 1: darr[i] = -(double)rand() / RAND_MAX; break;
            case 2: darr[i] = 0.0 / 0.0; /* NaN */ break;
            case 3: darr[i] = 1.0 / 0.0; /* +Inf */ break;
            case 4: darr[i] = -1.0 / 0.0; /* -Inf */ break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Test scalar conditions */
    for (int i = 0; i < 15; i++) {
        test_scalar_conditions(darr[i], darr[i+1], farr[i], farr[i+1]);
    }
    
    /* Test vector conditions */
    for (int i = 0; i < 14; i += 2) {
        __m128d v1 = _mm_set_pd(darr[i], darr[i+1]);
        __m128d v2 = _mm_set_pd(darr[i+2], darr[i+3]);
        __m128 fv1 = _mm_set_ps(farr[i], farr[i+1], farr[i+2], farr[i+3]);
        __m128 fv2 = _mm_set_ps(farr[i+4], farr[i+5], farr[i+6], farr[i+7]);
        test_vector_conditions(v1, v2, fv1, fv2);
    }
    
    /* Test inline assembly */
    for (int i = 0; i < 15; i++) {
        __m128d v1 = _mm_set_pd(darr[i], darr[i+1]);
        __m128d v2 = _mm_set_pd(darr[i+2], darr[i+3]);
        test_inline_asm_conditions(darr[i], darr[i+1], v1, v2);
    }
    
    /* Test complex branching */
    test_complex_branching(darr, 16);
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
