/* Test program to cover condition code output logic in i386.cc */
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
    
    /* UNORDERED/ORDERED tests with NaN */
    double nan_val = 0.0/0.0;
    res = isunordered(d1, nan_val);
    mix(&local_sum, res);
    
    res = isordered(d1, d2);
    mix(&local_sum, res);
    
    /* UNEQ (unordered or equal) */
    res = !isgreater(d1, d2) && !isless(d1, d2);
    mix(&local_sum, res);
    
    /* UNGE (not less than) */
    res = !isless(d1, d2);
    mix(&local_sum, res);
    
    /* UNGT (not less than or equal) */
    res = !islessequal(d1, d2);
    mix(&local_sum, res);
    
    /* UNLE (unordered or less than or equal) */
    res = isunordered(f1, f2) || islessequal(f1, f2);
    mix(&local_sum, res);
    
    /* UNLT (unordered or less than) */
    res = isunordered(f1, f2) || isless(f1, f2);
    mix(&local_sum, res);
    
    /* LTGT (less than or greater than, but not equal and not unordered) */
    res = (isless(d1, d2) || isgreater(d1, d2)) && !isunordered(d1, d2);
    mix(&local_sum, res);
    
    /* Complex branching to force condition code generation */
    if (isunordered(d1, d2)) {
        local_sum += 1;
    } else if (!isless(d1, d2)) {
        local_sum += 2;
    } else if (!islessequal(d1, d2)) {
        local_sum += 3;
    }
    
    g_checksum += local_sum;
}

/* Test vector conditions with SSE */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    unsigned long local_sum = 0;
    volatile __m128d vres;
    volatile __m128 vresf;
    
    /* UNORDERED */
    vres = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* ORDERED */
    vres = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* UNEQ */
    vres = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* UNGE */
    vres = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* UNGT */
    vres = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* UNLE */
    vresf = _mm_cmp_ps(v3, v4, _CMP_LE_UQ);
    mix(&local_sum, _mm_movemask_ps(vresf));
    
    /* UNLT */
    vresf = _mm_cmp_ps(v3, v4, _CMP_LT_UQ);
    mix(&local_sum, _mm_movemask_ps(vresf));
    
    /* LTGT (UNE) */
    vres = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* Conditional moves based on vector comparisons */
    __m128d mask = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    vres = _mm_or_pd(_mm_and_pd(mask, v1), _mm_andnot_pd(mask, v2));
    mix(&local_sum, _mm_movemask_pd(vres));
    
    g_checksum += local_sum;
}

/* Test inline assembly with explicit condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    unsigned long local_sum = 0;
    volatile int res;
    volatile double dres;
    volatile float fres;
    
    /* Test each condition code with inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (res)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    mix(&local_sum, res);
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (res)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    mix(&local_sum, res);
    
    /* UNEQ using cmppd */
    {
        __m128d va = _mm_set_sd(a);
        __m128d vb = _mm_set_sd(b);
        int mask;
        __asm__ volatile (
            "cmppd %3, %2, %{%1|ueq}\n\t"
            "movmskpd %1, %0"
            : "=r" (mask)
            : "x" (va), "x" (vb), "i" (8)
            : "cc"
        );
        mix(&local_sum, mask);
    }
    
    /* UNGE using cmppd with nlt */
    {
        __m128d va = _mm_set_sd(a);
        __m128d vb = _mm_set_sd(b);
        int mask;
        __asm__ volatile (
            "cmppd %3, %2, %{%1|nlt}\n\t"
            "movmskpd %1, %0"
            : "=r" (mask)
            : "x" (va), "x" (vb), "i" (5)
            : "cc"
        );
        mix(&local_sum, mask);
    }
    
    /* UNGT using cmppd with nle */
    {
        __m128d va = _mm_set_sd(a);
        __m128d vb = _mm_set_sd(b);
        int mask;
        __asm__ volatile (
            "cmppd %3, %2, %{%1|nle}\n\t"
            "movmskpd %1, %0"
            : "=r" (mask)
            : "x" (va), "x" (vb), "i" (6)
            : "cc"
        );
        mix(&local_sum, mask);
    }
    
    /* UNLE using cmpss with ule */
    {
        __m128 va = _mm_set_ss(fa);
        __m128 vb = _mm_set_ss(fb);
        int mask;
        __asm__ volatile (
            "cmpss %3, %2, %{%1|ule}\n\t"
            "movmskps %1, %0"
            : "=r" (mask)
            : "x" (va), "x" (vb), "i" (2)
            : "cc"
        );
        mix(&local_sum, mask);
    }
    
    /* UNLT using cmpss with ult */
    {
        __m128 va = _mm_set_ss(fa);
        __m128 vb = _mm_set_ss(fb);
        int mask;
        __asm__ volatile (
            "cmpss %3, %2, %{%1|ult}\n\t"
            "movmskps %1, %0"
            : "=r" (mask)
            : "x" (va), "x" (vb), "i" (1)
            : "cc"
        );
        mix(&local_sum, mask);
    }
    
    /* LTGT using cmppd with une */
    {
        __m128d va = _mm_set_sd(a);
        __m128d vb = _mm_set_sd(b);
        int mask;
        __asm__ volatile (
            "cmppd %3, %2, %{%1|une}\n\t"
            "movmskpd %1, %0"
            : "=r" (mask)
            : "x" (va), "x" (vb), "i" (12)
            : "cc"
        );
        mix(&local_sum, mask);
    }
    
    g_checksum += local_sum;
}

/* Test with aggressive optimization */
__attribute__((optimize("O3"), target("sse2")))
void test_optimized_conditions(double *darray, float *farray, int n) {
    unsigned long local_sum = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Mix of different conditions in loop */
        if (isunordered(darray[i], darray[i+1])) {
            local_sum += i;
        } else if (!isless(darray[i], darray[i+1])) {
            local_sum += i * 2;
        }
        
        /* Ternary with uncommon condition */
        float res = (isunordered(farray[i], farray[i+1]) || 
                    islessequal(farray[i], farray[i+1])) ? 
                   farray[i] : farray[i+1];
        mix(&local_sum, *(unsigned*)&res);
    }
    
    g_checksum += local_sum;
}

int main(int argc, char *argv[]) {
    /* Initialize with some variation */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with NaNs, normals, and special values */
    double dvals[8];
    float fvals[8];
    
    for (int i = 0; i < 8; i++) {
        dvals[i] = (rand() % 100) / 10.0;
        fvals[i] = (rand() % 100) / 10.0f;
    }
    
    /* Add some NaN values */
    dvals[2] = 0.0/0.0;
    fvals[3] = 0.0f/0.0f;
    
    /* Test scalar conditions */
    for (int i = 0; i < 7; i++) {
        test_scalar_conditions(dvals[i], dvals[i+1], fvals[i], fvals[i+1]);
    }
    
    /* Test vector conditions */
    __m128d vd1 = _mm_set_pd(dvals[0], dvals[1]);
    __m128d vd2 = _mm_set_pd(dvals[2], dvals[3]);
    __m128 vf1 = _mm_set_ps(fvals[0], fvals[1], fvals[2], fvals[3]);
    __m128 vf2 = _mm_set_ps(fvals[4], fvals[5], fvals[6], fvals[7]);
    
    test_vector_conditions(vd1, vd2, vf1, vf2);
    
    /* Test inline assembly */
    for (int i = 0; i < 4; i++) {
        test_inline_asm_conditions(dvals[i], dvals[i+1], fvals[i], fvals[i+1]);
    }
    
    /* Test optimized version */
    test_optimized_conditions(dvals, fvals, 8);
    
    printf("Final checksum: %lu\n", g_checksum);
    return 0;
}
