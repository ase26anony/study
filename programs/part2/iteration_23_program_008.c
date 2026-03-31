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
    *sum = (*sum * 31) ^ val;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    unsigned long local_sum = 0;
    
    /* Test UNORDERED (unord) - unordered comparison */
    res = isunordered(d1, d2);
    mix(&local_sum, res);
    
    /* Test ORDERED (ord) - ordered comparison */
    res = isordered(d1, d2);
    mix(&local_sum, res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    /* Use complex expression to force condition code generation */
    if ((isunordered(f1, f2) || (f1 == f2))) {
        res = 1;
    } else {
        res = 0;
    }
    mix(&local_sum, res);
    
    /* Test UNGE (nlt) - unordered or greater than or equal */
    if ((isunordered(d1, d2) || (d1 >= d2))) {
        res = 1;
    } else {
        res = 0;
    }
    mix(&local_sum, res);
    
    /* Test UNGT (nle) - unordered or greater than */
    if ((isunordered(f1, f2) || (f1 > f2))) {
        res = 1;
    } else {
        res = 0;
    }
    mix(&local_sum, res);
    
    /* Test UNLE (ule) - unordered or less than or equal */
    if ((isunordered(d1, d2) || (d1 <= d2))) {
        res = 1;
    } else {
        res = 0;
    }
    mix(&local_sum, res);
    
    /* Test UNLT (ult) - unordered or less than */
    if ((isunordered(f1, f2) || (f1 < f2))) {
        res = 1;
    } else {
        res = 0;
    }
    mix(&local_sum, res);
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    res = ((d1 < d2) || (d1 > d2)) && (!isunordered(d1, d2));
    mix(&local_sum, res);
    
    g_checksum ^= local_sum;
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    volatile __m128d vres;
    volatile __m128 fres;
    unsigned long local_sum = 0;
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    vres = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* Test ORDERED - _CMP_ORD_Q */
    vres = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* Test UNEQ - _CMP_EQ_UQ */
    vres = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* Test UNGE - _CMP_NLT_UQ */
    vres = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* Test UNGT - _CMP_NLE_UQ */
    vres = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    /* Test UNLE - _CMP_LE_UQ */
    fres = _mm_cmp_ps(f1, f2, _CMP_LE_UQ);
    mix(&local_sum, _mm_movemask_ps(fres));
    
    /* Test UNLT - _CMP_LT_UQ */
    fres = _mm_cmp_ps(f1, f2, _CMP_LT_UQ);
    mix(&local_sum, _mm_movemask_ps(fres));
    
    /* Test LTGT - _CMP_NEQ_OQ */
    vres = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    mix(&local_sum, _mm_movemask_pd(vres));
    
    g_checksum ^= local_sum;
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    volatile int result = 0;
    unsigned long local_sum = 0;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomiss %1, %2\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(fa), "x"(fb)
        : "al", "cc"
    );
    mix(&local_sum, result);
    
    /* Test UNEQ (ueq) with extended asm template */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}\n\t"
        "movmskpd %1, %0"
        : "=r"(result)
        : "x"(_mm_set_pd(a, b)), "i"(_CMP_EQ_UQ)
        : "cc"
    );
    mix(&local_sum, result);
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        "movmskpd %1, %0"
        : "=r"(result)
        : "x"(_mm_set_pd(a, b)), "i"(_CMP_NLT_UQ)
        : "cc"
    );
    mix(&local_sum, result);
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}\n\t"
        "movmskpd %1, %0"
        : "=r"(result)
        : "x"(_mm_set_pd(a, b)), "i"(_CMP_NLE_UQ)
        : "cc"
    );
    mix(&local_sum, result);
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmpps %2, %1, %{%0|ule}\n\t"
        "movmskps %1, %0"
        : "=r"(result)
        : "x"(_mm_set_ps(fa, fb, fa, fb)), "i"(_CMP_LE_UQ)
        : "cc"
    );
    mix(&local_sum, result);
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmpps %2, %1, %{%0|ult}\n\t"
        "movmskps %1, %0"
        : "=r"(result)
        : "x"(_mm_set_ps(fa, fb, fa, fb)), "i"(_CMP_LT_UQ)
        : "cc"
    );
    mix(&local_sum, result);
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        "movmskpd %1, %0"
        : "=r"(result)
        : "x"(_mm_set_pd(a, b)), "i"(_CMP_NEQ_OQ)
        : "cc"
    );
    mix(&local_sum, result);
    
    g_checksum ^= local_sum;
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), noinline))
void test_optimized_conditions(double *darr, float *farr, int n) {
    volatile double dsum = 0.0;
    volatile float fsum = 0.0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Complex conditional expressions to force condition code generation */
        if ((isunordered(darr[i], darr[i+1]) || (darr[i] >= darr[i+1]))) {
            dsum += darr[i];
        }
        
        if ((!isunordered(farr[i], farr[i+1]) && (farr[i] != farr[i+1]))) {
            fsum += farr[i];
        }
        
        /* Nested conditions */
        if (isordered(darr[i], darr[i+1])) {
            if (darr[i] < darr[i+1]) {
                dsum -= darr[i+1];
            } else if (darr[i] > darr[i+1]) {
                dsum += darr[i+1];
            }
        }
    }
    
    mix((unsigned long*)&g_checksum, *(unsigned long*)&dsum);
    mix((unsigned long*)&g_checksum, *(unsigned int*)&fsum);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[8];
    float farr[8];
    
    for (int i = 0; i < 8; i++) {
        switch (i % 4) {
            case 0: darr[i] = (double)rand() / RAND_MAX; break;
            case 1: darr[i] = -(double)rand() / RAND_MAX; break;
            case 2: darr[i] = 0.0 / 0.0; /* NaN */ break;
            case 3: darr[i] = 1.0 / 0.0; /* Inf */ break;
        }
        
        switch (i % 4) {
            case 0: farr[i] = (float)rand() / RAND_MAX; break;
            case 1: farr[i] = -(float)rand() / RAND_MAX; break;
            case 2: farr[i] = 0.0f / 0.0f; /* NaN */ break;
            case 3: farr[i] = 1.0f / 0.0f; /* Inf */ break;
        }
    }
    
    /* Call test functions with various parameter combinations */
    for (int i = 0; i < 4; i++) {
        test_scalar_conditions(darr[i], darr[i+1], farr[i], farr[i+1]);
        test_scalar_conditions(darr[i+2], darr[i+3], farr[i+2], farr[i+3]);
        
        __m128d v1 = _mm_set_pd(darr[i], darr[i+1]);
        __m128d v2 = _mm_set_pd(darr[i+2], darr[i+3]);
        __m128 f1 = _mm_set_ps(farr[i], farr[i+1], farr[i+2], farr[i+3]);
        __m128 f2 = _mm_set_ps(farr[i+3], farr[i+2], farr[i+1], farr[i]);
        
        test_vector_conditions(v1, v2, f1, f2);
        test_inline_asm_conditions(darr[i], darr[i+1], farr[i], farr[i+1]);
    }
    
    test_optimized_conditions(darr, farr, 8);
    
    printf("Final checksum: %lu\n", g_checksum);
    return 0;
}
