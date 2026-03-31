/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *c, unsigned long v) {
    *c ^= v;
    *c = (*c << 13) | (*c >> (64 - 13));
    *c = *c * 0x9e3779b97f4a7c15UL;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    unsigned long local_sum = 0;
    
    /* UNORDERED/ORDERED tests with NaN */
    double nan_val = 0.0/0.0;
    res = isunordered(d1, nan_val);
    mix(&local_sum, res);
    
    res = isordered(d1, d2);
    mix(&local_sum, res);
    
    /* UNEQ (unordered or equal) */
    res = !(isgreater(d1, d2) || isless(d1, d2));
    mix(&local_sum, res);
    
    /* UNGE (not less than) */
    res = !isless(d1, d2);
    mix(&local_sum, res);
    
    /* UNGT (not less than or equal) */
    res = !islessequal(d1, d2);
    mix(&local_sum, res);
    
    /* UNLE (unordered or less than or equal) */
    res = islessequal(d1, d2) || isunordered(d1, d2);
    mix(&local_sum, res);
    
    /* UNLT (unordered or less than) */
    res = isless(d1, d2) || isunordered(d1, d2);
    mix(&local_sum, res);
    
    /* LTGT (less than or greater than, ordered) */
    res = (isless(d1, d2) || isgreater(d1, d2)) && !isunordered(d1, d2);
    mix(&local_sum, res);
    
    /* Complex branching to force condition code generation */
    if (isunordered(f1, f2)) {
        local_sum += 1;
    } else if (!isless(f1, f2)) {  /* UNGE */
        local_sum += 2;
    } else if (!islessequal(f1, f2)) {  /* UNGT */
        local_sum += 3;
    }
    
    g_checksum ^= local_sum;
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    unsigned long local_sum = 0;
    
    /* UNORDERED */
    __m128d cmp_unord = _mm_cmpunord_pd(v1, v2);
    volatile double d = ((double*)&cmp_unord)[0];
    mix(&local_sum, (unsigned long)d);
    
    /* ORDERED */
    __m128d cmp_ord = _mm_cmpord_pd(v1, v2);
    d = ((double*)&cmp_ord)[0];
    mix(&local_sum, (unsigned long)d);
    
    /* UNEQ (unordered or equal) */
    __m128d cmp_ueq = _mm_cmpneq_pd(v1, v2);
    __m128d cmp_unord2 = _mm_cmpunord_pd(v1, v2);
    __m128d result = _mm_or_pd(cmp_ueq, cmp_unord2);
    d = ((double*)&result)[0];
    mix(&local_sum, (unsigned long)d);
    
    /* UNGE (not less than) */
    __m128d cmp_nlt = _mm_cmpnlt_pd(v1, v2);
    d = ((double*)&cmp_nlt)[0];
    mix(&local_sum, (unsigned long)d);
    
    /* UNGT (not less than or equal) */
    __m128d cmp_nle = _mm_cmpnle_pd(v1, v2);
    d = ((double*)&cmp_nle)[0];
    mix(&local_sum, (unsigned long)d);
    
    /* UNLE (unordered or less than or equal) */
    __m128d cmp_ule = _mm_cmple_pd(v1, v2);
    __m128d unord = _mm_cmpunord_pd(v1, v2);
    result = _mm_or_pd(cmp_ule, unord);
    d = ((double*)&result)[0];
    mix(&local_sum, (unsigned long)d);
    
    /* UNLT (unordered or less than) */
    __m128d cmp_ult = _mm_cmplt_pd(v1, v2);
    unord = _mm_cmpunord_pd(v1, v2);
    result = _mm_or_pd(cmp_ult, unord);
    d = ((double*)&result)[0];
    mix(&local_sum, (unsigned long)d);
    
    /* LTGT (unordered, not equal) */
    __m128d cmp_une = _mm_cmpneq_pd(v1, v2);
    __m128d ord = _mm_cmpord_pd(v1, v2);
    result = _mm_and_pd(cmp_une, ord);
    d = ((double*)&result)[0];
    mix(&local_sum, (unsigned long)d);
    
    /* Test with float vectors */
    __m128 cmp_unord_ps = _mm_cmpunord_ps(v3, v4);
    volatile float f = ((float*)&cmp_unord_ps)[0];
    mix(&local_sum, (unsigned long)f);
    
    g_checksum ^= local_sum;
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    unsigned long local_sum = 0;
    volatile double result;
    volatile float fresult;
    
    /* Test various condition codes in inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&local_sum, (unsigned long)result);
    
    /* ORDERED */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&local_sum, (unsigned long)result);
    
    /* UNEQ */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&local_sum, (unsigned long)result);
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&local_sum, (unsigned long)result);
    
    /* UNGT (nle) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&local_sum, (unsigned long)result);
    
    /* UNLE (ule) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&local_sum, (unsigned long)result);
    
    /* UNLT (ult) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&local_sum, (unsigned long)result);
    
    /* LTGT (une) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&local_sum, (unsigned long)result);
    
    /* Test with float (scalar) */
    __asm__ volatile (
        "cmpss %2, %1, %{%0|unord}\n\t"
        "movd %1, %0"
        : "=x"(fresult)
        : "x"(fa), "x"(fb)
        : "cc"
    );
    mix(&local_sum, (unsigned long)fresult);
    
    g_checksum ^= local_sum;
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d av1, __m256d av2) {
    unsigned long local_sum = 0;
    
    /* Use AVX comparisons (will be lowered to SSE for non-AVX targets) */
    __m256d cmp = _mm256_cmp_pd(av1, av2, _CMP_UNORD_Q);  /* UNORDERED */
    volatile double d = ((double*)&cmp)[0];
    mix(&local_sum, (unsigned long)d);
    
    cmp = _mm256_cmp_pd(av1, av2, _CMP_ORD_Q);  /* ORDERED */
    d = ((double*)&cmp)[1];
    mix(&local_sum, (unsigned long)d);
    
    cmp = _mm256_cmp_pd(av1, av2, _CMP_EQ_UQ);  /* UNEQ */
    d = ((double*)&cmp)[2];
    mix(&local_sum, (unsigned long)d);
    
    cmp = _mm256_cmp_pd(av1, av2, _CMP_NLT_UQ); /* UNGE */
    d = ((double*)&cmp)[3];
    mix(&local_sum, (unsigned long)d);
    
    g_checksum ^= local_sum;
}

int main(int argc, char *argv[]) {
    /* Initialize with some non-uniform values */
    unsigned long seed = 0x12345678;
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    
    srand(seed);
    
    /* Create arrays with varied values including NaN and Inf */
    double darray[8];
    float farray[8];
    
    for (int i = 0; i < 8; i++) {
        darray[i] = (rand() % 100) * 1.2345;
        farray[i] = (rand() % 100) * 0.9876f;
        
        /* Introduce some special values */
        if (i == 2) darray[i] = 0.0/0.0;  /* NaN */
        if (i == 3) darray[i] = 1.0/0.0;  /* Inf */
        if (i == 4) farray[i] = 0.0f/0.0f; /* NaN */
    }
    
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
    for (int i = 0; i < 4; i++) {
        test_inline_asm_conditions(darray[i], darray[i+4], 
                                  farray[i], farray[i+4]);
    }
    
    /* Test AVX if available */
    __m256d av1 = _mm256_set_pd(darray[0], darray[1], darray[2], darray[3]);
    __m256d av2 = _mm256_set_pd(darray[4], darray[5], darray[6], darray[7]);
    test_avx_conditions(av1, av2);
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", g_checksum);
    
    return 0;
}
