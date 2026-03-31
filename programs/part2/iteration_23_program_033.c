/* Test program to cover condition code output logic in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *chk, unsigned long val) {
    *chk = (*chk * 31) ^ val;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    
    /* Test UNORDERED (unord) - using isnan() */
    res = isnan(d1) || isnan(d2);
    mix(&g_checksum, res);
    
    /* Test ORDERED (ord) - using !isnan() */
    res = !isnan(d1) && !isnan(d2);
    mix(&g_checksum, res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = (isnan(d1) || isnan(d2)) || (d1 == d2);
    mix(&g_checksum, res);
    
    /* Test UNGE (nlt) - unordered or not less than (>=) */
    res = (isnan(d1) || isnan(d2)) || !(d1 < d2);
    mix(&g_checksum, res);
    
    /* Test UNGT (nle) - unordered or not less than or equal (>) */
    res = (isnan(d1) || isnan(d2)) || !(d1 <= d2);
    mix(&g_checksum, res);
    
    /* Test UNLE (ule) - unordered or less than or equal (<=) */
    res = (isnan(d1) || isnan(d2)) || (d1 <= d2);
    mix(&g_checksum, res);
    
    /* Test UNLT (ult) - unordered or less than (<) */
    res = (isnan(d1) || isnan(d2)) || (d1 < d2);
    mix(&g_checksum, res);
    
    /* Test LTGT (une) - less than or greater than (not equal and ordered) */
    res = (!isnan(d1) && !isnan(d2)) && (d1 != d2);
    mix(&g_checksum, res);
    
    /* Repeat with float */
    res = isnan(f1) || isnan(f2);
    mix(&g_checksum, res);
    
    res = !isnan(f1) && !isnan(f2);
    mix(&g_checksum, res);
    
    /* Complex branching to force condition code generation */
    volatile int control = (int)(d1 * 1000) % 7;
    double result = 0.0;
    
    switch(control) {
        case 0:
            if (isunordered(d1, d2)) result = 1.0;  /* UNORDERED */
            break;
        case 1:
            if (!isunordered(d1, d2)) result = 2.0; /* ORDERED */
            break;
        case 2:
            if (isgreater(d1, d2)) result = 3.0;    /* GT (not UNLE) */
            break;
        case 3:
            if (isless(d1, d2)) result = 4.0;       /* LT (not UNGE) */
            break;
        case 4:
            if (islessgreater(d1, d2)) result = 5.0; /* LTGT */
            break;
        case 5:
            if (islessequal(d1, d2)) result = 6.0;  /* LE (not UNGT) */
            break;
        case 6:
            if (isgreaterequal(d1, d2)) result = 7.0; /* GE (not UNLT) */
            break;
    }
    
    mix(&g_checksum, (unsigned long)result);
}

/* Test with SSE/AVX vector operations */
__attribute__((target("sse2")))
__attribute__((optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED (unord) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
    
    /* Test ORDERED (ord) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
    
    /* Test UNEQ (ueq) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
    
    /* Test UNGE (nlt) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
    
    /* Test UNGT (nle) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
    
    /* Test UNLE (ule) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
    
    /* Test UNLT (ult) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
    
    /* Test LTGT (une) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
    
    /* Repeat with single precision */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(&g_checksum, (unsigned long)fres[i]);
    
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(&g_checksum, (unsigned long)fres[i]);
    
    /* Conditional moves based on vector comparisons */
    __m128d mask = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    __m128d result = _mm_or_pd(_mm_and_pd(mask, v1), 
                               _mm_andnot_pd(mask, v2));
    _mm_storeu_pd((double*)dres, result);
    mix(&g_checksum, (unsigned long)dres[0]);
    mix(&g_checksum, (unsigned long)dres[1]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    volatile int result_int = 0;
    volatile double result_dbl = 0.0;
    volatile float result_flt = 0.0f;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result_int)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&g_checksum, result_int);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result_int)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    mix(&g_checksum, result_int);
    
    /* Test using condition codes in templates */
    __asm__ volatile (
        "cmpsd %[unord], %1, %0\n\t"
        : "=x"(result_dbl)
        : "x"(a), "x"(b)
        : "cc"
        : [unord] "i"(_CMP_UNORD_Q)
    );
    mix(&g_checksum, (unsigned long)result_dbl);
    
    /* Test with AT&T/Intel syntax variations */
    int cmp_result;
    __asm__ volatile (
        "ucomiss %2, %1\n\t"
        "%{%0|ueq}\n\t"
        "movzbl %%al, %0"
        : "=r"(cmp_result)
        : "x"(fa), "x"(fb)
        : "al", "cc"
    );
    mix(&g_checksum, cmp_result);
    
    /* More inline assembly with different condition codes */
    unsigned long flags;
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "pushf\n\t"
        "pop %0"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
}

/* Test with aggressive optimization */
__attribute__((optimize("O3")))
__attribute__((noinline))
void test_optimized_conditions(double *arr, int n) {
    volatile double sum = 0.0;
    volatile int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Use various conditions in loop */
        if (isunordered(a, b)) {
            sum += 1.0;  /* UNORDERED */
        } else if (!isunordered(a, b) && (a == b)) {
            sum += 2.0;  /* ORDERED and equal */
        } else if (islessgreater(a, b)) {
            sum += 3.0;  /* LTGT */
        } else if (isless(a, b)) {
            sum += 4.0;  /* UNLT (unordered or less) */
        } else if (isgreater(a, b)) {
            sum += 5.0;  /* UNGT (unordered or greater) */
        }
        
        /* Ternary with complex condition */
        double temp = (isunordered(a, b) || a <= b) ? a : b;  /* UNLE */
        sum += temp;
        
        count++;
    }
    
    mix(&g_checksum, (unsigned long)sum);
    mix(&g_checksum, count);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = 12345;
    if (argc > 1) seed = atoi(argv[1]);
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal */
    double darr[10];
    float farr[10];
    
    for (int i = 0; i < 10; i++) {
        switch (i % 5) {
            case 0: darr[i] = 1.0 / (i + 1); break;
            case 1: darr[i] = (double)rand() / RAND_MAX; break;
            case 2: darr[i] = -darr[i-1]; break;
            case 3: darr[i] = 0.0 / 0.0; break;  /* NaN */
            case 4: darr[i] = 1.0 / 0.0; break;  /* Inf */
        }
        farr[i] = (float)darr[i];
    }
    
    /* Initialize vector values */
    __m128d vd1 = _mm_set_pd(darr[0], darr[1]);
    __m128d vd2 = _mm_set_pd(darr[2], darr[3]);
    __m128 vf1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 vf2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    /* Run all tests */
    test_scalar_conditions(darr[0], darr[1], farr[0], farr[1]);
    test_vector_conditions(vd1, vd2, vf1, vf2);
    test_inline_asm_conditions(darr[2], darr[3], farr[2], farr[3]);
    test_optimized_conditions(darr, 10);
    
    /* Additional tests with different value combinations */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            test_scalar_conditions(darr[i], darr[j], farr[i], farr[j]);
        }
    }
    
    printf("Final checksum: %lu\n", g_checksum);
    return (int)(g_checksum % 256);
}
