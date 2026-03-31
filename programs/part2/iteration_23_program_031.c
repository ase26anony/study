/* Test program for x86 condition code mnemonics coverage */
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
    *c = *c * 0x9e3779b97f4a7c15ULL;
}

/* Test scalar floating-point conditions with different optimizations */
__attribute__((optimize("O0")))
void test_scalar_conditions_O0(double a, double b, float c, float d) {
    volatile int res;
    
    /* UNORDERED (unord) - using isnan() */
    res = isnan(a) || isnan(b);
    mix(&checksum, res);
    
    /* ORDERED (ord) - using !isnan() */
    res = !isnan(a) && !isnan(b);
    mix(&checksum, res);
    
    /* UNEQ (ueq) - unordered or equal */
    res = isnan(a) || isnan(b) || (a == b);
    mix(&checksum, res);
    
    /* UNGE (nlt) - unordered or not less than */
    res = isnan(a) || isnan(b) || !(a < b);
    mix(&checksum, res);
    
    /* UNGT (nle) - unordered or not less than or equal */
    res = isnan(a) || isnan(b) || !(a <= b);
    mix(&checksum, res);
    
    /* UNLE (ule) - unordered or less than or equal */
    res = isnan(a) || isnan(b) || (a <= b);
    mix(&checksum, res);
    
    /* UNLT (ult) - unordered or less than */
    res = isnan(a) || isnan(b) || (a < b);
    mix(&checksum, res);
    
    /* LTGT (une) - less than or greater than (ordered and not equal) */
    res = (!isnan(a) && !isnan(b)) && (a != b);
    mix(&checksum, res);
    
    /* Use results in conditional branches */
    if (isnan(c) || isnan(d)) checksum += 1;
    if (!isnan(c) && !isnan(d)) checksum += 2;
    if (isnan(c) || isnan(d) || (c == d)) checksum += 4;
    if (isnan(c) || isnan(d) || !(c < d)) checksum += 8;
    if (isnan(c) || isnan(d) || !(c <= d)) checksum += 16;
    if (isnan(c) || isnan(d) || (c <= d)) checksum += 32;
    if (isnan(c) || isnan(d) || (c < d)) checksum += 64;
    if ((!isnan(c) && !isnan(d)) && (c != d)) checksum += 128;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_sse2(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 fcmp_res;
    volatile double dres[2];
    volatile float fres[4];
    
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
    
    /* UNLE - _CMP_LE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNLT - _CMP_LT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* LTGT - _CMP_NEQ_OQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test float vectors as well */
    fcmp_res = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, fcmp_res);
    for (int i = 0; i < 4; i++) mix(&checksum, *(unsigned*)&fres[i]);
    
    fcmp_res = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fres, fcmp_res);
    for (int i = 0; i < 4; i++) mix(&checksum, *(unsigned*)&fres[i]);
}

/* Inline assembly tests for condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile __m128d va, vb, vres;
    
    va = _mm_set1_pd(a);
    vb = _mm_set1_pd(b);
    
    /* Test each condition code mnemonic in inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|unord}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (0), "x" (va), "x" (vb)
        : "cc"
    );
    _mm_store_sd(&result, vres);
    mix(&checksum, *(unsigned long*)&result);
    
    /* ORDERED */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ord}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (1), "x" (va), "x" (vb)
        : "cc"
    );
    _mm_store_sd(&result, vres);
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNEQ */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (8), "x" (va), "x" (vb)
        : "cc"
    );
    _mm_store_sd(&result, vres);
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (5), "x" (va), "x" (vb)
        : "cc"
    );
    _mm_store_sd(&result, vres);
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNGT (nle) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nle}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (6), "x" (va), "x" (vb)
        : "cc"
    );
    _mm_store_sd(&result, vres);
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNLE (ule) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (2), "x" (va), "x" (vb)
        : "cc"
    );
    _mm_store_sd(&result, vres);
    mix(&checksum, *(unsigned long*)&result);
    
    /* UNLT (ult) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ult}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (1), "x" (va), "x" (vb)
        : "cc"
    );
    _mm_store_sd(&result, vres);
    mix(&checksum, *(unsigned long*)&result);
    
    /* LTGT (une) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (12), "x" (va), "x" (vb)
        : "cc"
    );
    _mm_store_sd(&result, vres);
    mix(&checksum, *(unsigned long*)&result);
}

/* Complex branching test with volatile variables */
__attribute__((optimize("O3"), target("avx")))
void test_complex_branching(volatile double *arr, int n) {
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
        
        /* Complex nested conditions to force code generation */
        if (isnan(a) || isnan(b)) {
            count_unord++;
            if (!(a < b)) count_nlt++;
            if (!(a <= b)) count_nle++;
        } else {
            count_ord++;
            if (a == b) count_ueq++;
            if (a <= b) count_ule++;
            if (a < b) count_ult++;
            if (a != b) count_une++;
        }
        
        /* More complex logic */
        if ((isnan(a) || isnan(b)) || (a == b)) {
            checksum += i;
        }
        if ((isnan(a) || isnan(b)) || !(a < b)) {
            checksum += i * 2;
        }
        if ((isnan(a) || isnan(b)) || !(a <= b)) {
            checksum += i * 3;
        }
    }
    
    mix(&checksum, count_unord);
    mix(&checksum, count_ord);
    mix(&checksum, count_ueq);
    mix(&checksum, count_nlt);
    mix(&checksum, count_nle);
    mix(&checksum, count_ule);
    mix(&checksum, count_ult);
    mix(&checksum, count_une);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = 0x12345678;
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[20];
    float farr[20];
    
    for (int i = 0; i < 20; i++) {
        switch (i % 7) {
            case 0: darr[i] = 1.0 / (i + 1); break;
            case 1: darr[i] = -2.0 * i; break;
            case 2: darr[i] = 0.0; break;
            case 3: darr[i] = -0.0; break;
            case 4: darr[i] = __builtin_nan(""); break;
            case 5: darr[i] = __builtin_inf(); break;
            case 6: darr[i] = -__builtin_inf(); break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Call test functions with different optimization levels */
    test_scalar_conditions_O0(darr[0], darr[1], farr[2], farr[3]);
    
    /* Test vector conditions */
    __m128d v1 = _mm_set_pd(darr[4], darr[5]);
    __m128d v2 = _mm_set_pd(darr[6], darr[7]);
    __m128 f1 = _mm_set_ps(farr[8], farr[9], farr[10], farr[11]);
    __m128 f2 = _mm_set_ps(farr[12], farr[13], farr[14], farr[15]);
    test_vector_conditions_sse2(v1, v2, f1, f2);
    
    /* Test inline assembly */
    test_inline_asm_conditions(darr[16], darr[17]);
    
    /* Test complex branching */
    test_complex_branching(darr, 20);
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
