/* Test program to cover x86 condition code output in i386.cc */
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

/* Test scalar floating-point conditions with different optimizations */
__attribute__((optimize("O0")))
void test_scalar_conditions_O0(double a, double b, float c, float d) {
    volatile int res;
    
    /* Test UNORDERED (unord) */
    res = isunordered(a, b);
    mix(res);
    
    /* Test ORDERED (ord) */
    res = !isunordered(a, b);
    mix(res);
    
    /* Complex branching to force condition code generation */
    if (isunordered(a, b)) {
        mix(1);
        if (!isunordered(c, d)) {
            mix(2);
        }
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    if (isunordered(a, b) || a == b) {
        mix(3);
    }
    
    /* Test UNGE (nlt) - unordered or not less than */
    if (isunordered(a, b) || !(a < b)) {
        mix(4);
    }
    
    /* Test UNGT (nle) - unordered or not less than or equal */
    if (isunordered(a, b) || !(a <= b)) {
        mix(5);
    }
    
    /* Test UNLE (ule) - unordered or less than or equal */
    if (isunordered(a, b) || a <= b) {
        mix(6);
    }
    
    /* Test UNLT (ult) - unordered or less than */
    if (isunordered(a, b) || a < b) {
        mix(7);
    }
    
    /* Test LTGT (une) - not equal and ordered */
    if (!isunordered(a, b) && a != b) {
        mix(8);
    }
}

__attribute__((optimize("O2"), target("sse2")))
void test_scalar_conditions_O2(double a, double b, float c, float d) {
    volatile int res;
    
    /* Use ternary operators that may generate conditional moves */
    res = isunordered(a, b) ? 100 : 200;
    mix(res);
    
    res = !isunordered(a, b) ? 101 : 201;
    mix(res);
    
    /* Test all conditions with different expressions */
    mix((isunordered(a, b) || a == b) ? 1 : 0);
    mix((isunordered(a, b) || !(a < b)) ? 1 : 0);
    mix((isunordered(a, b) || !(a <= b)) ? 1 : 0);
    mix((isunordered(a, b) || a <= b) ? 1 : 0);
    mix((isunordered(a, b) || a < b) ? 1 : 0);
    mix((!isunordered(a, b) && a != b) ? 1 : 0);
}

/* Test vector conditions using SSE intrinsics */
__attribute__((optimize("O3"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd(dres, cmp_res);
    mix(*(unsigned long long*)&dres[0]);
    mix(*(unsigned long long*)&dres[1]);
    
    /* Test ORDERED - _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd(dres, cmp_res);
    mix(*(unsigned long long*)&dres[0]);
    mix(*(unsigned long long*)&dres[1]);
    
    /* Test UNEQ - _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd(dres, cmp_res);
    mix(*(unsigned long long*)&dres[0]);
    mix(*(unsigned long long*)&dres[1]);
    
    /* Test UNGE - _CMP_NGE_UQ (nlt) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NGE_UQ);
    _mm_storeu_pd(dres, cmp_res);
    mix(*(unsigned long long*)&dres[0]);
    mix(*(unsigned long long*)&dres[1]);
    
    /* Test UNGT - _CMP_NGT_UQ (nle) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NGT_UQ);
    _mm_storeu_pd(dres, cmp_res);
    mix(*(unsigned long long*)&dres[0]);
    mix(*(unsigned long long*)&dres[1]);
    
    /* Test UNLE - _CMP_LE_OS (ule) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_OS);
    _mm_storeu_pd(dres, cmp_res);
    mix(*(unsigned long long*)&dres[0]);
    mix(*(unsigned long long*)&dres[1]);
    
    /* Test UNLT - _CMP_LT_OS (ult) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_OS);
    _mm_storeu_pd(dres, cmp_res);
    mix(*(unsigned long long*)&dres[0]);
    mix(*(unsigned long long*)&dres[1]);
    
    /* Test LTGT - _CMP_NEQ_OS (une) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    _mm_storeu_pd(dres, cmp_res);
    mix(*(unsigned long long*)&dres[0]);
    mix(*(unsigned long long*)&dres[1]);
    
    /* Test float vectors as well */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    _mm_storeu_ps(fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(*(unsigned*)&fres[i]);
    
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    _mm_storeu_ps(fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(*(unsigned*)&fres[i]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile __m128d va, vb, vres;
    
    /* Initialize vectors */
    va = _mm_set_pd(a, b);
    vb = _mm_set_pd(b, a);
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        : "=x"(vres) : "x"(va), "x"(vb) : "cc"
    );
    _mm_storeu_pd(&result, vres);
    mix(*(unsigned long long*)&result);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        : "=x"(vres) : "x"(va), "x"(vb) : "cc"
    );
    _mm_storeu_pd(&result, vres);
    mix(*(unsigned long long*)&result);
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}\n\t"
        : "=x"(vres) : "x"(va), "x"(vb) : "cc"
    );
    _mm_storeu_pd(&result, vres);
    mix(*(unsigned long long*)&result);
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        : "=x"(vres) : "x"(va), "x"(vb) : "cc"
    );
    _mm_storeu_pd(&result, vres);
    mix(*(unsigned long long*)&result);
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}\n\t"
        : "=x"(vres) : "x"(va), "x"(vb) : "cc"
    );
    _mm_storeu_pd(&result, vres);
    mix(*(unsigned long long*)&result);
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        : "=x"(vres) : "x"(va), "x"(vb) : "cc"
    );
    _mm_storeu_pd(&result, vres);
    mix(*(unsigned long long*)&result);
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}\n\t"
        : "=x"(vres) : "x"(va), "x"(vb) : "cc"
    );
    _mm_storeu_pd(&result, vres);
    mix(*(unsigned long long*)&result);
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        : "=x"(vres) : "x"(va), "x"(vb) : "cc"
    );
    _mm_storeu_pd(&result, vres);
    mix(*(unsigned long long*)&result);
}

/* Test mixed conditions with loops to prevent optimization */
__attribute__((optimize("O2"), target("sse2")))
void test_mixed_conditions_loop(double* darr, float* farr, int n) {
    volatile int count_unord = 0;
    volatile int count_ord = 0;
    volatile int count_ueq = 0;
    volatile int count_nlt = 0;
    volatile int count_nle = 0;
    volatile int count_ule = 0;
    volatile int count_ult = 0;
    volatile int count_une = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Count UNORDERED */
        if (isunordered(darr[i], darr[i+1])) {
            count_unord++;
        }
        
        /* Count ORDERED */
        if (!isunordered(darr[i], darr[i+1])) {
            count_ord++;
        }
        
        /* Count UNEQ */
        if (isunordered(darr[i], darr[i+1]) || darr[i] == darr[i+1]) {
            count_ueq++;
        }
        
        /* Count UNGE (nlt) */
        if (isunordered(darr[i], darr[i+1]) || !(darr[i] < darr[i+1])) {
            count_nlt++;
        }
        
        /* Count UNGT (nle) */
        if (isunordered(darr[i], darr[i+1]) || !(darr[i] <= darr[i+1])) {
            count_nle++;
        }
        
        /* Count UNLE (ule) */
        if (isunordered(darr[i], darr[i+1]) || darr[i] <= darr[i+1]) {
            count_ule++;
        }
        
        /* Count UNLT (ult) */
        if (isunordered(darr[i], darr[i+1]) || darr[i] < darr[i+1]) {
            count_ult++;
        }
        
        /* Count LTGT (une) */
        if (!isunordered(darr[i], darr[i+1]) && darr[i] != darr[i+1]) {
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

int main(int argc, char* argv[]) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[20];
    float farr[20];
    
    for (int i = 0; i < 20; i++) {
        switch (i % 7) {
            case 0: darr[i] = 1.0 / (i + 1); break;
            case 1: darr[i] = (double)(i * i); break;
            case 2: darr[i] = -darr[i-1]; break;
            case 3: darr[i] = 0.0; break;
            case 4: darr[i] = -0.0; break;
            case 5: darr[i] = (i % 3 == 0) ? 0.0 / 0.0 : darr[i-1]; /* NaN sometimes */ break;
            case 6: darr[i] = 1.0 / 0.0; /* Infinity */ break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Create vector values */
    __m128d vd1 = _mm_set_pd(darr[0], darr[1]);
    __m128d vd2 = _mm_set_pd(darr[2], darr[3]);
    __m128 vf1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 vf2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    /* Run all tests */
    test_scalar_conditions_O0(darr[0], darr[1], farr[0], farr[1]);
    test_scalar_conditions_O2(darr[2], darr[3], farr[2], farr[3]);
    test_vector_conditions(vd1, vd2, vf1, vf2);
    test_inline_asm_conditions(darr[4], darr[5]);
    test_mixed_conditions_loop(darr, farr, 20);
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
