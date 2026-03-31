/* Test program to cover x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *sum, unsigned long val) {
    *sum = (*sum * 31) + val;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    unsigned long local_sum = 0;
    
    /* UNORDERED/UNORD: Test with NaN */
    res = isunordered(d1, d2);
    mix(&local_sum, res);
    
    /* ORDERED/ORD: Test with normal numbers */
    res = !isunordered(d1, d2);
    mix(&local_sum, res);
    
    /* UNEQ: Unordered or equal */
    res = !isgreater(d1, d2) && !isless(d1, d2);
    mix(&local_sum, res);
    
    /* UNGE: Unordered or greater than or equal */
    res = !isless(d1, d2);
    mix(&local_sum, res);
    
    /* UNGT: Unordered or greater than */
    res = !islessequal(d1, d2);
    mix(&local_sum, res);
    
    /* UNLE: Unordered or less than or equal */
    res = !isgreater(d1, d2);
    mix(&local_sum, res);
    
    /* UNLT: Unordered or less than */
    res = !isgreaterequal(d1, d2);
    mix(&local_sum, res);
    
    /* LTGT/UNE: Not equal and ordered */
    res = islessgreater(d1, d2);
    mix(&local_sum, res);
    
    /* Complex branching to force condition code generation */
    if (isunordered(f1, f2)) {
        local_sum += 1;
    } else if (!isgreater(f1, f2) && !isless(f1, f2)) {
        local_sum += 2;
    } else if (islessgreater(f1, f2)) {
        local_sum += 3;
    }
    
    g_checksum += local_sum;
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    volatile __m128d resd;
    volatile __m128 resf;
    unsigned long local_sum = 0;
    
    /* UNORDERED/UNORD: _CMP_UNORD_Q */
    resd = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    mix(&local_sum, _mm_movemask_pd(resd));
    
    /* ORDERED/ORD: _CMP_ORD_Q */
    resd = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    mix(&local_sum, _mm_movemask_pd(resd));
    
    /* UNEQ: _CMP_EQ_UQ */
    resd = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    mix(&local_sum, _mm_movemask_pd(resd));
    
    /* UNGE: _CMP_GE_UQ */
    resd = _mm_cmp_pd(v1, v2, _CMP_GE_UQ);
    mix(&local_sum, _mm_movemask_pd(resd));
    
    /* UNGT: _CMP_GT_UQ */
    resd = _mm_cmp_pd(v1, v2, _CMP_GT_UQ);
    mix(&local_sum, _mm_movemask_pd(resd));
    
    /* UNLE: _CMP_LE_UQ */
    resd = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    mix(&local_sum, _mm_movemask_pd(resd));
    
    /* UNLT: _CMP_LT_UQ */
    resd = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    mix(&local_sum, _mm_movemask_pd(resd));
    
    /* LTGT/UNE: _CMP_NEQ_UQ */
    resd = _mm_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    mix(&local_sum, _mm_movemask_pd(resd));
    
    /* Test with float vectors */
    resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    mix(&local_sum, _mm_movemask_ps(resf));
    
    resf = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    mix(&local_sum, _mm_movemask_ps(resf));
    
    g_checksum += local_sum;
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float c, float d) {
    volatile int result;
    unsigned long local_sum = 0;
    
    /* Test various condition codes in inline assembly */
    
    /* UNORD */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* ORD */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* UEQ - unordered or equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* NLT - unordered or greater than or equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* NLE - unordered or greater than */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* ULE - unordered or less than or equal */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setna %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* ULT - unordered or less than */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* UNE - not equal and ordered */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setne %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* Test with AT&T/Intel syntax variations using template strings */
    __asm__ volatile (
        "cmppd %{%2|unord}, %1, %0"
        : "=x"(result)
        : "x"(a), "i"(0x03)  /* _CMP_UNORD_Q */
        : "cc"
    );
    mix(&local_sum, result);
    
    __asm__ volatile (
        "cmppd %{%2|ord}, %1, %0"
        : "=x"(result)
        : "x"(a), "i"(0x07)  /* _CMP_ORD_Q */
        : "cc"
    );
    mix(&local_sum, result);
    
    g_checksum += local_sum;
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d v1, __m256d v2) {
    volatile __m256d res;
    unsigned long local_sum = 0;
    
    /* Use AVX comparisons if available */
    res = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    mix(&local_sum, _mm256_movemask_pd(res));
    
    res = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    mix(&local_sum, _mm256_movemask_pd(res));
    
    res = _mm256_cmp_pd(v1, v2, _CMP_EQ_UQ);
    mix(&local_sum, _mm256_movemask_pd(res));
    
    g_checksum += local_sum;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darray[8];
    float farray[8];
    
    for (int i = 0; i < 8; i++) {
        farray[i] = (rand() % 100) / 10.0f;
        darray[i] = (rand() % 100) / 10.0;
        
        /* Introduce some special values */
        if (i == 2) {
            farray[i] = 0.0f / 0.0f;  /* NaN */
            darray[i] = 0.0 / 0.0;    /* NaN */
        }
        if (i == 4) {
            farray[i] = 1.0f / 0.0f;  /* +Inf */
            darray[i] = 1.0 / 0.0;    /* +Inf */
        }
        if (i == 6) {
            farray[i] = -1.0f / 0.0f; /* -Inf */
            darray[i] = -1.0 / 0.0;   /* -Inf */
        }
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
        test_inline_asm_conditions(darray[i], darray[i+1], 
                                  farray[i], farray[i+1]);
    }
    
    /* Test AVX if available */
    #ifdef __AVX__
    __m256d avx1 = _mm256_set_pd(darray[0], darray[1], darray[2], darray[3]);
    __m256d avx2 = _mm256_set_pd(darray[4], darray[5], darray[6], darray[7]);
    test_avx_conditions(avx1, avx2);
    #endif
    
    printf("Final checksum: %lu\n", g_checksum);
    return 0;
}
