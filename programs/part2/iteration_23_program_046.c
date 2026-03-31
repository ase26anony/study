/* Test program to cover condition code output logic in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *c, unsigned long v) {
    *c ^= v;
    *c = (*c << 13) | (*c >> (64 - 13));
    *c = *c * 0x9e3779b97f4a7c15ULL;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    
    /* Test UNORDERED (unord) */
    res = isunordered(d1, d2);
    mix(&checksum, res);
    
    /* Test ORDERED (ord) */
    res = !isunordered(d1, d2);
    mix(&checksum, res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = (isunordered(f1, f2) || (f1 == f2));
    mix(&checksum, res);
    
    /* Test UNGE (nlt) - unordered or greater than or equal */
    res = (isunordered(d1, d2) || (d1 >= d2));
    mix(&checksum, res);
    
    /* Test UNGT (nle) - unordered or greater than */
    res = (isunordered(f1, f2) || (f1 > f2));
    mix(&checksum, res);
    
    /* Test UNLE (ule) - unordered or less than or equal */
    res = (isunordered(d1, d2) || (d1 <= d2));
    mix(&checksum, res);
    
    /* Test UNLT (ult) - unordered or less than */
    res = (isunordered(f1, f2) || (f1 < f2));
    mix(&checksum, res);
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    res = (!isunordered(d1, d2) && (d1 != d2));
    mix(&checksum, res);
    
    /* Complex branching to force condition code generation */
    if (isunordered(d1, d2)) {
        mix(&checksum, 1);
    } else if (d1 > d2) {
        mix(&checksum, 2);
    } else if (d1 < d2) {
        mix(&checksum, 3);
    } else {
        mix(&checksum, 4);
    }
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED with _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test ORDERED with _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNEQ with _CMP_EQ_UQ */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_EQ_UQ);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) {
        mix(&checksum, *(unsigned int*)&fres[i]);
    }
    
    /* Test UNGE with _CMP_GE_OQ (ordered greater-than-or-equal) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_GE_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNGT with _CMP_GT_OQ (ordered greater-than) */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_GT_OQ);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) {
        mix(&checksum, *(unsigned int*)&fres[i]);
    }
    
    /* Test UNLE with _CMP_LE_OQ (ordered less-than-or-equal) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNLT with _CMP_LT_OQ (ordered less-than) */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_LT_OQ);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) {
        mix(&checksum, *(unsigned int*)&fres[i]);
    }
    
    /* Test LTGT with _CMP_NEQ_OQ (ordered not-equal) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    volatile int result;
    volatile __m128d va, vb;
    volatile __m128 vfa, vfb;
    
    va = _mm_set_pd(a, b);
    vb = _mm_set_pd(b, a);
    vfa = _mm_set_ps(fa, fb, fa, fb);
    vfb = _mm_set_ps(fb, fa, fb, fa);
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movmskpd %1, %k0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(&checksum, result);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movmskpd %1, %k0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(&checksum, result);
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmpps %2, %1, %{%0|ueq}\n\t"
        "movmskps %1, %k0"
        : "=r"(result) : "x"(vfa), "x"(vfb) : "cc"
    );
    mix(&checksum, result);
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        "movmskpd %1, %k0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(&checksum, result);
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmpps %2, %1, %{%0|nle}\n\t"
        "movmskps %1, %k0"
        : "=r"(result) : "x"(vfa), "x"(vfb) : "cc"
    );
    mix(&checksum, result);
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        "movmskpd %1, %k0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(&checksum, result);
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmpps %2, %1, %{%0|ult}\n\t"
        "movmskps %1, %k0"
        : "=r"(result) : "x"(vfa), "x"(vfb) : "cc"
    );
    mix(&checksum, result);
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        "movmskpd %1, %k0"
        : "=r"(result) : "x"(va), "x"(vb) : "cc"
    );
    mix(&checksum, result);
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), noinline))
void test_optimized_conditions(double *darr, float *farr, int n) {
    volatile double sum = 0.0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Use various conditions in loop to force code generation */
        if (isunordered(darr[i], darr[i+1])) {
            sum += 1.0;
        } else if (darr[i] > darr[i+1]) {
            sum += 2.0;
        } else if (darr[i] < darr[i+1]) {
            sum += 3.0;
        } else {
            sum += 4.0;
        }
        
        /* Ternary operator with uncommon conditions */
        double temp = (isunordered(farr[i], farr[i+1]) || farr[i] >= farr[i+1]) ? 
                     darr[i] : darr[i+1];
        sum += temp;
    }
    
    mix(&checksum, *(unsigned long*)&sum);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = (argc > 1) ? strtoul(argv[1], NULL, 0) : 0x12345678;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[8];
    float farr[16];
    
    for (int i = 0; i < 8; i++) {
        switch (i % 4) {
            case 0: darr[i] = (double)rand() / RAND_MAX * 100.0; break;
            case 1: darr[i] = -((double)rand() / RAND_MAX * 100.0); break;
            case 2: darr[i] = 0.0 / 0.0; /* NaN */ break;
            case 3: darr[i] = 1.0 / 0.0; /* Inf */ break;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        switch (i % 4) {
            case 0: farr[i] = (float)rand() / RAND_MAX * 100.0f; break;
            case 1: farr[i] = -((float)rand() / RAND_MAX * 100.0f); break;
            case 2: farr[i] = 0.0f / 0.0f; /* NaN */ break;
            case 3: farr[i] = 1.0f / 0.0f; /* Inf */ break;
        }
    }
    
    /* Create vector values */
    __m128d vd1 = _mm_set_pd(darr[0], darr[1]);
    __m128d vd2 = _mm_set_pd(darr[2], darr[3]);
    __m128 vf1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 vf2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    /* Run all tests */
    test_scalar_conditions(darr[0], darr[1], farr[0], farr[1]);
    test_vector_conditions(vd1, vd2, vf1, vf2);
    test_inline_asm_conditions(darr[2], darr[3], farr[2], farr[3]);
    test_optimized_conditions(darr, farr, 8);
    
    /* Additional tests with different value combinations */
    for (int i = 0; i < 4; i++) {
        test_scalar_conditions(darr[i], darr[i+4], farr[i], farr[i+8]);
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
