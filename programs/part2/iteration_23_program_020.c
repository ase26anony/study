/* Test program for x86 condition code mnemonics coverage */
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
    *c *= 0x9e3779b97f4a7c15ULL;
}

/* Test functions with different optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* Test UNORDERED (unord) */
    res = isunordered(a, b);
    mix(&checksum, res);
    
    /* Test ORDERED (ord) */
    res = !isunordered(a, b);
    mix(&checksum, res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = (isunordered(a, b) || (a == b));
    mix(&checksum, res);
    
    /* Test UNGE (nlt) - unordered or not less than */
    res = (isunordered(a, b) || !(a < b));
    mix(&checksum, res);
    
    /* Test UNGT (nle) - unordered or not less than or equal */
    res = (isunordered(a, b) || !(a <= b));
    mix(&checksum, res);
    
    /* Test UNLE (ule) - unordered or less than or equal */
    res = (isunordered(a, b) || (a <= b));
    mix(&checksum, res);
    
    /* Test UNLT (ult) - unordered or less than */
    res = (isunordered(a, b) || (a < b));
    mix(&checksum, res);
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    res = (!isunordered(a, b) && (a != b));
    mix(&checksum, res);
    
    /* Float versions to test different modes */
    res = isunordered(fa, fb);
    mix(&checksum, res);
    
    res = !isunordered(fa, fb);
    mix(&checksum, res);
}

__attribute__((optimize("O2"), target("sse2")))
void test_scalar_conditions_O2(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* Complex branching to force condition code generation */
    if (isunordered(a, b)) {
        res = 1;
    } else if (!isunordered(a, b) && (a != b)) {
        res = 2;
    } else if (isunordered(a, b) || (a == b)) {
        res = 3;
    } else if (isunordered(a, b) || !(a < b)) {
        res = 4;
    } else {
        res = 5;
    }
    mix(&checksum, res);
    
    /* Ternary operations */
    res = (isunordered(fa, fb) || (fa <= fb)) ? 6 : 7;
    mix(&checksum, res);
    
    res = (isunordered(fa, fb) || !(fa <= fb)) ? 8 : 9;
    mix(&checksum, res);
}

__attribute__((optimize("O3"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 fcmp_res;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test ORDERED for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNEQ for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNGE for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNGT for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNLE for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNLT for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test LTGT for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Float vector tests */
    fcmp_res = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, fcmp_res);
    for (int i = 0; i < 4; i++) {
        mix(&checksum, *(unsigned int*)&fres[i]);
    }
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b, __m128d v1, __m128d v2) {
    volatile double result;
    volatile __m128d vresult;
    
    /* Inline assembly with explicit condition code mnemonics */
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Vector version with cmppd */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movdqa %1, %0"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    mix(&checksum, ((unsigned long*)&vresult)[0]);
    mix(&checksum, ((unsigned long*)&vresult)[1]);
}

__attribute__((optimize("Os"), target("sse2")))
void test_mixed_conditions(volatile double *darr, volatile float *farr, int n) {
    volatile int count_unord = 0;
    volatile int count_ord = 0;
    volatile int count_ueq = 0;
    volatile int count_nlt = 0;
    volatile int count_nle = 0;
    volatile int count_ule = 0;
    volatile int count_ult = 0;
    volatile int count_une = 0;
    
    /* Loop to prevent optimization */
    for (int i = 0; i < n - 1; i++) {
        double a = darr[i];
        double b = darr[i + 1];
        float fa = farr[i];
        float fb = farr[i + 1];
        
        /* Count various conditions */
        if (isunordered(a, b)) count_unord++;
        if (!isunordered(a, b)) count_ord++;
        if (isunordered(a, b) || (a == b)) count_ueq++;
        if (isunordered(a, b) || !(a < b)) count_nlt++;
        if (isunordered(a, b) || !(a <= b)) count_nle++;
        if (isunordered(a, b) || (a <= b)) count_ule++;
        if (isunordered(a, b) || (a < b)) count_ult++;
        if (!isunordered(a, b) && (a != b)) count_une++;
        
        /* Float versions */
        if (isunordered(fa, fb)) count_unord++;
        if (!isunordered(fa, fb)) count_ord++;
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
    unsigned long seed = 0x12345678;
    
    /* Use argv for seed variation if available */
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    
    srand(seed);
    
    /* Initialize test data with various values including NaN, Inf, normal */
    double darr[10];
    float farr[10];
    
    for (int i = 0; i < 10; i++) {
        switch (i % 5) {
            case 0: darr[i] = 1.0 / (i + 1); break;
            case 1: darr[i] = (double)(i * i); break;
            case 2: darr[i] = 0.0 / 0.0; /* NaN */ break;
            case 3: darr[i] = 1.0 / 0.0; /* Inf */ break;
            case 4: darr[i] = -1.0 / 0.0; /* -Inf */ break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Initialize vectors */
    __m128d v1 = _mm_set_pd(darr[0], darr[1]);
    __m128d v2 = _mm_set_pd(darr[2], darr[3]);
    __m128 f1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 f2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    /* Run all tests */
    test_scalar_conditions_O0(darr[0], darr[1], farr[0], farr[1]);
    test_scalar_conditions_O2(darr[2], darr[3], farr[2], farr[3]);
    test_vector_conditions(v1, v2, f1, f2);
    test_inline_asm_conditions(darr[4], darr[5], v1, v2);
    test_mixed_conditions(darr, farr, 10);
    
    /* Print final checksum */
    printf("Final checksum: 0x%016lx\n", checksum);
    
    return 0;
}
