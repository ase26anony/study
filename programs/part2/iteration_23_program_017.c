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
    *c ^= v;
    *c = (*c << 13) | (*c >> (64 - 13));
    *c *= 0x9e3779b97f4a7c15UL;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    
    /* UNORDERED (unord) - using isnan() */
    res = isnan(d1) || isnan(d2);
    mix(&checksum, res);
    
    /* ORDERED (ord) - using !isnan() */
    res = !isnan(d1) && !isnan(d2);
    mix(&checksum, res);
    
    /* UNEQ (ueq) - unordered or equal */
    res = isnan(d1) || isnan(d2) || (d1 == d2);
    mix(&checksum, res);
    
    /* UNGE (nlt) - unordered or not less than */
    res = isnan(d1) || isnan(d2) || !(d1 < d2);
    mix(&checksum, res);
    
    /* UNGT (nle) - unordered or not less than or equal */
    res = isnan(d1) || isnan(d2) || !(d1 <= d2);
    mix(&checksum, res);
    
    /* UNLE (ule) - unordered or less than or equal */
    res = isnan(d1) || isnan(d2) || (d1 <= d2);
    mix(&checksum, res);
    
    /* UNLT (ult) - unordered or less than */
    res = isnan(d1) || isnan(d2) || (d1 < d2);
    mix(&checksum, res);
    
    /* LTGT (une) - less than or greater than (ordered and not equal) */
    res = (!isnan(d1) && !isnan(d2)) && (d1 != d2);
    mix(&checksum, res);
    
    /* Complex branching to force condition code generation */
    if (isunordered(f1, f2)) {
        mix(&checksum, 0x1);
    } else if (isgreater(f1, f2)) {
        mix(&checksum, 0x2);
    } else if (isless(f1, f2)) {
        mix(&checksum, 0x3);
    } else if (isgreaterequal(f1, f2)) {
        mix(&checksum, 0x4);
    } else if (islessequal(f1, f2)) {
        mix(&checksum, 0x5);
    }
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test various comparison predicates */
    
    /* UNORDERED - _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* ORDERED - _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNEQ - _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNGE - _CMP_NLT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNGT - _CMP_NLE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNLE - _CMP_LE_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_OS);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNLT - _CMP_LT_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_OS);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* LTGT - _CMP_NEQ_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Float vector comparisons */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) {
        mix(&checksum, *(unsigned*)&fres[i]);
    }
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    volatile double result;
    volatile float fresult;
    volatile int iresult;
    
    /* UNORDERED - unord */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* ORDERED - ord */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNEQ - ueq */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNGE - nlt */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNGT - nle */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNLE - ule */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNLT - ult */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* LTGT - une */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Float comparisons */
    __asm__ volatile (
        "cmpss %2, %1, %{%0|unord}\n\t"
        "movd %1, %0"
        : "=x"(fresult)
        : "x"(fa), "x"(fb)
        : "cc"
    );
    mix(&checksum, *(unsigned*)&fresult);
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d v1, __m256d v2) {
    __m256d cmp_res;
    volatile double dres[4];
    
    /* Test various AVX comparison predicates */
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) {
        mix(&checksum, *(unsigned long*)&dres[i]);
    }
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) {
        mix(&checksum, *(unsigned long*)&dres[i]);
    }
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) {
        mix(&checksum, *(unsigned long*)&dres[i]);
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = 0x12345678;
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    
    srand(seed);
    
    /* Create test data with NaN, infinity, and normal numbers */
    double darray[8];
    float farray[8];
    
    for (int i = 0; i < 8; i++) {
        darray[i] = (rand() % 100) / 10.0;
        farray[i] = (rand() % 100) / 10.0f;
    }
    
    /* Add some special values */
    darray[2] = 0.0 / 0.0;  /* NaN */
    darray[3] = 1.0 / 0.0;  /* +Inf */
    darray[4] = -1.0 / 0.0; /* -Inf */
    
    farray[2] = 0.0f / 0.0f;  /* NaN */
    farray[3] = 1.0f / 0.0f;  /* +Inf */
    
    /* Test scalar conditions */
    for (int i = 0; i < 7; i++) {
        test_scalar_conditions(darray[i], darray[i+1], 
                              farray[i], farray[i+1]);
    }
    
    /* Test vector conditions */
    __m128d vd1 = _mm_set_pd(darray[0], darray[1]);
    __m128d vd2 = _mm_set_pd(darray[2], darray[3]);
    __m128 vf1 = _mm_set_ps(farray[0], farray[1], farray[2], farray[3]);
    __m128 vf2 = _mm_set_ps(farray[4], farray[5], farray[6], farray[7]);
    
    test_vector_conditions(vd1, vd2, vf1, vf2);
    
    /* Test inline assembly */
    for (int i = 0; i < 5; i++) {
        test_inline_asm_conditions(darray[i], darray[i+1], 
                                  farray[i], farray[i+1]);
    }
    
    /* Test AVX if available */
    #ifdef __AVX__
    __m256d avx1 = _mm256_set_pd(darray[0], darray[1], darray[2], darray[3]);
    __m256d avx2 = _mm256_set_pd(darray[4], darray[5], darray[6], darray[7]);
    test_avx_conditions(avx1, avx2);
    #endif
    
    printf("Final checksum: 0x%016lx\n", checksum);
    return 0;
}
