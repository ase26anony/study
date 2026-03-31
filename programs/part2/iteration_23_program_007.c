/* Test program to cover x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

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
    
    /* Test UNORDERED (unord) */
    res = isunordered(d1, d2);
    mix(res);
    
    /* Test ORDERED (ord) */
    res = !isunordered(d1, d2);
    mix(res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = (d1 != d1) || (d2 != d2) || (d1 == d2);
    mix(res);
    
    /* Test UNGE (nlt) - unordered or greater or equal */
    res = (d1 != d1) || (d2 != d2) || (d1 >= d2);
    mix(res);
    
    /* Test UNGT (nle) - unordered or greater */
    res = (d1 != d1) || (d2 != d2) || (d1 > d2);
    mix(res);
    
    /* Test UNLE (ule) - unordered or less or equal */
    res = (d1 != d1) || (d2 != d2) || (d1 <= d2);
    mix(res);
    
    /* Test UNLT (ult) - unordered or less */
    res = (d1 != d1) || (d2 != d2) || (d1 < d2);
    mix(res);
    
    /* Test LTGT (une) - less or greater (ordered, not equal) */
    res = (d1 == d1) && (d2 == d2) && (d1 != d2);
    mix(res);
    
    /* Complex branching to force condition code generation */
    if (isunordered(f1, f2)) {
        mix(1);
    } else if (!isunordered(f1, f2) && (f1 == f2)) {
        mix(2);
    } else if ((f1 != f1) || (f2 != f2) || (f1 >= f2)) {
        mix(3);
    } else if ((f1 != f1) || (f2 != f2) || (f1 > f2)) {
        mix(4);
    }
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test ORDERED - _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNEQ - _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNGE - _CMP_GE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_GE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNGT - _CMP_GT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_GT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNLE - _CMP_LE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNLT - _CMP_LT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test LTGT - _CMP_NEQ_OQ (ordered, not equal) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test with float vectors */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(*(unsigned*)&fres[i]);
    
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_EQ_UQ);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(*(unsigned*)&fres[i]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    volatile double result;
    volatile float fresult;
    volatile int iresult;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(*(unsigned long*)&result);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(*(unsigned long*)&result);
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(*(unsigned long*)&result);
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(*(unsigned long*)&result);
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(*(unsigned long*)&result);
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(*(unsigned long*)&result);
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(*(unsigned long*)&result);
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(*(unsigned long*)&result);
    
    /* Test with float using cmpps */
    __asm__ volatile (
        "cmpss %2, %1, %{%0|unord}\n\t"
        "movd %1, %0"
        : "=x"(fresult) : "x"(_mm_load_ss(&fa)), "x"(_mm_load_ss(&fb)) : "cc"
    );
    mix(*(unsigned*)&fresult);
    
    /* Conditional move based on comparison */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "setp %0\n\t"
        "sete %b0"
        : "=r"(iresult) : "x"(*(__m128d*)&a), "x"(*(__m128d*)&b) : "cc"
    );
    mix(iresult);
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d av1, __m256d av2) {
    __m256d cmp_res;
    volatile double dres[4];
    
    /* Test various conditions with AVX */
    cmp_res = _mm256_cmp_pd(av1, av2, _CMP_UNORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) mix(*(unsigned long*)&dres[i]);
    
    cmp_res = _mm256_cmp_pd(av1, av2, _CMP_EQ_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) mix(*(unsigned long*)&dres[i]);
    
    cmp_res = _mm256_cmp_pd(av1, av2, _CMP_GE_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) mix(*(unsigned long*)&dres[i]);
    
    cmp_res = _mm256_cmp_pd(av1, av2, _CMP_NEQ_OQ);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) mix(*(unsigned long*)&dres[i]);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with NaNs, infinities, and normal numbers */
    double darray[8];
    float farray[16];
    
    for (int i = 0; i < 8; i++) {
        darray[i] = (rand() % 100) * 0.1;
        if (i == 2) darray[i] = 0.0 / 0.0;  /* NaN */
        if (i == 3) darray[i] = 1.0 / 0.0;  /* +Inf */
        if (i == 4) darray[i] = -1.0 / 0.0; /* -Inf */
    }
    
    for (int i = 0; i < 16; i++) {
        farray[i] = (rand() % 100) * 0.1f;
        if (i == 5) farray[i] = 0.0f / 0.0f;  /* NaN */
        if (i == 6) farray[i] = 1.0f / 0.0f;  /* +Inf */
    }
    
    /* Test scalar conditions */
    for (int i = 0; i < 7; i++) {
        test_scalar_conditions(darray[i], darray[(i+1)%8], 
                              farray[i], farray[(i+1)%16]);
    }
    
    /* Test vector conditions */
    __m128d v1 = _mm_set_pd(darray[0], darray[1]);
    __m128d v2 = _mm_set_pd(darray[2], darray[3]);
    __m128 v3 = _mm_set_ps(farray[0], farray[1], farray[2], farray[3]);
    __m128 v4 = _mm_set_ps(farray[4], farray[5], farray[6], farray[7]);
    
    test_vector_conditions(v1, v2, v3, v4);
    
    /* Test inline assembly */
    for (int i = 0; i < 4; i++) {
        test_inline_asm_conditions(darray[i], darray[i+4], 
                                  farray[i], farray[i+8]);
    }
    
    /* Test AVX if available */
    #ifdef __AVX__
    __m256d av1 = _mm256_set_pd(darray[0], darray[1], darray[2], darray[3]);
    __m256d av2 = _mm256_set_pd(darray[4], darray[5], darray[6], darray[7]);
    test_avx_conditions(av1, av2);
    #endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
