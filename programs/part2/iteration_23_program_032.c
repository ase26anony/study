/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *sum, unsigned long val) {
    *sum ^= val;
    *sum = (*sum << 13) | (*sum >> (64 - 13));
    *sum *= 0x9e3779b97f4a7c15UL;
}

/* Test scalar floating-point conditions with different optimizations */
__attribute__((optimize("O0")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* Test UNORDERED (unord) */
    result = isunordered(a, b);
    mix(&g_checksum, result);
    
    /* Test ORDERED (ord) */
    result = !isunordered(a, b);
    mix(&g_checksum, result);
    
    /* Complex branching to force condition code generation */
    if (isunordered(fa, fb)) {
        mix(&g_checksum, 0x1234);
    } else if (!isunordered(fa, fb)) {
        mix(&g_checksum, 0x5678);
    }
    
    /* Test UNEQ (ueq) via islessgreater negation */
    result = !islessgreater(a, b);
    mix(&g_checksum, result);
    
    /* Test UNGE (nlt) via !isless */
    result = !isless(a, b);
    mix(&g_checksum, result);
    
    /* Test UNGT (nle) via !islessequal */
    result = !islessequal(a, b);
    mix(&g_checksum, result);
    
    /* Test UNLE (ule) via islessequal with unordered */
    result = islessequal(a, b) || isunordered(a, b);
    mix(&g_checksum, result);
    
    /* Test UNLT (ult) via isless with unordered */
    result = isless(a, b) || isunordered(a, b);
    mix(&g_checksum, result);
    
    /* Test LTGT (une) via islessgreater */
    result = islessgreater(a, b);
    mix(&g_checksum, result);
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_sse2(__m128d va, __m128d vb, __m128 vfa, __m128 vfb) {
    __m128d mask;
    __m128i imask;
    volatile long long store[2];
    
    /* Test UNORDERED (unord) - CMP_UNORD_Q */
    mask = _mm_cmp_pd(va, vb, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)store, mask);
    mix(&g_checksum, store[0]);
    mix(&g_checksum, store[1]);
    
    /* Test ORDERED (ord) - CMP_ORD_Q */
    mask = _mm_cmp_pd(va, vb, _CMP_ORD_Q);
    _mm_storeu_pd((double*)store, mask);
    mix(&g_checksum, store[0] + store[1]);
    
    /* Test UNEQ (ueq) - CMP_EQ_UQ */
    mask = _mm_cmp_pd(va, vb, _CMP_EQ_UQ);
    imask = _mm_castpd_si128(mask);
    mix(&g_checksum, _mm_extract_epi32(imask, 0));
    
    /* Test UNGE (nlt) - CMP_NLT_UQ */
    mask = _mm_cmp_pd(va, vb, _CMP_NLT_UQ);
    imask = _mm_castpd_si128(mask);
    mix(&g_checksum, _mm_extract_epi32(imask, 1));
    
    /* Test UNGT (nle) - CMP_NLE_UQ */
    mask = _mm_cmp_pd(va, vb, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)store, mask);
    mix(&g_checksum, store[0] ^ store[1]);
    
    /* Test UNLE (ule) - CMP_LE_UQ */
    mask = _mm_cmp_pd(va, vb, _CMP_LE_UQ);
    imask = _mm_castpd_si128(mask);
    mix(&g_checksum, _mm_extract_epi32(imask, 0) + _mm_extract_epi32(imask, 1));
    
    /* Test UNLT (ult) - CMP_LT_UQ */
    mask = _mm_cmp_pd(va, vb, _CMP_LT_UQ);
    _mm_storeu_pd((double*)store, mask);
    mix(&g_checksum, store[0] | store[1]);
    
    /* Test LTGT (une) - CMP_NEQ_UQ */
    mask = _mm_cmp_pd(va, vb, _CMP_NEQ_UQ);
    imask = _mm_castpd_si128(mask);
    mix(&g_checksum, _mm_extract_epi32(imask, 0) ^ _mm_extract_epi32(imask, 1));
    
    /* Repeat for single precision */
    __m128 fmask = _mm_cmp_ps(vfa, vfb, _CMP_UNORD_Q);
    imask = _mm_castps_si128(fmask);
    mix(&g_checksum, _mm_extract_epi32(imask, 2));
}

/* Inline assembly tests with explicit condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, __m128d va, __m128d vb) {
    volatile double result;
    volatile __m128d vresult;
    volatile int iresult;
    
    /* Test unord (UNORDERED) */
    __asm__ volatile (
        "comisd %1, %0\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(iresult) : "x"(b), "0"(a) : "al", "cc"
    );
    mix(&g_checksum, iresult);
    
    /* Test ord (ORDERED) with AT&T/Intel template */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnp %{%0|b%}"
        : "=r"(iresult) : "x"(a), "x"(b) : "cc"
    );
    mix(&g_checksum, iresult);
    
    /* Test ueq (UNEQ) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %{%0|b%}"
        : "=r"(iresult) : "x"(a), "x"(b) : "cc"
    );
    mix(&g_checksum, iresult);
    
    /* Test nlt (UNGE) */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnb %{%0|b%}"
        : "=r"(iresult) : "x"(a), "x"(b) : "cc"
    );
    mix(&g_checksum, iresult);
    
    /* Test nle (UNGT) */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnbe %{%0|b%}"
        : "=r"(iresult) : "x"(a), "x"(b) : "cc"
    );
    mix(&g_checksum, iresult);
    
    /* Test ule (UNLE) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %{%0|b%}"
        : "=r"(iresult) : "x"(a), "x"(b) : "cc"
    );
    mix(&g_checksum, iresult);
    
    /* Test ult (UNLT) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %{%0|b%}"
        : "=r"(iresult) : "x"(a), "x"(b) : "cc"
    );
    mix(&g_checksum, iresult);
    
    /* Test une (LTGT) */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setne %{%0|b%}"
        : "=r"(iresult) : "x"(a), "x"(b) : "cc"
    );
    mix(&g_checksum, iresult);
    
    /* Vector comparisons with inline assembly */
    __asm__ volatile (
        "cmppd %{%2|unord%}, %1, %0"
        : "=x"(vresult) : "x"(va), "x"(vb)
    );
    _mm_storeu_pd((double*)&result, vresult);
    mix(&g_checksum, *(unsigned long*)&result);
    
    __asm__ volatile (
        "cmppd %{%2|ord%}, %1, %0"
        : "=x"(vresult) : "x"(va), "x"(vb)
    );
    _mm_storeu_pd((double*)&result, vresult);
    mix(&g_checksum, *(unsigned long*)&result);
}

/* Mixed optimization test */
__attribute__((optimize("O3"), target("avx")))
void test_mixed_conditions_avx(double *darr, float *farr, int n) {
    volatile double sum = 0.0;
    volatile int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Use various conditions in loop to force code generation */
        if (isunordered(darr[i], darr[i+1])) {
            sum += 1.0;
        } else if (!islessgreater(darr[i], darr[i+1])) {
            sum += 2.0;
        }
        
        if (!isless(darr[i], darr[i+1])) {
            count++;
        }
        
        if (islessequal(farr[i], farr[i+1]) || isunordered(farr[i], farr[i+1])) {
            count--;
        }
    }
    
    mix(&g_checksum, *(unsigned long*)&sum);
    mix(&g_checksum, count);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = 0x12345678;
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[16];
    float farr[16];
    
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: darr[i] = 1.0 / (i + 1); break;
            case 1: darr[i] = (double)(i * i); break;
            case 2: darr[i] = 0.0 / 0.0; break; /* NaN */
            case 3: darr[i] = 1.0 / 0.0; break; /* Inf */
            case 4: darr[i] = -1.0 / (i + 1); break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Initialize vector values */
    __m128d va = _mm_set_pd(darr[0], darr[1]);
    __m128d vb = _mm_set_pd(darr[2], darr[3]);
    __m128 vfa = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 vfb = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    /* Run all test functions */
    test_scalar_conditions_O0(darr[0], darr[1], farr[0], farr[1]);
    test_vector_conditions_sse2(va, vb, vfa, vfb);
    test_inline_asm_conditions(darr[4], darr[5], va, vb);
    test_mixed_conditions_avx(darr, farr, 16);
    
    /* Additional calls with different values */
    for (int i = 0; i < 4; i++) {
        va = _mm_set_pd(darr[i*2], darr[i*2+1]);
        vb = _mm_set_pd(darr[i*2+2], darr[i*2+3]);
        test_vector_conditions_sse2(va, vb, vfa, vfb);
    }
    
    printf("Final checksum: 0x%016lx\n", g_checksum);
    return (int)(g_checksum & 0x7FFFFFFF);
}
