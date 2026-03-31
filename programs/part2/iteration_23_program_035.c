/* Condition code test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile checksum to prevent optimization */
static volatile unsigned long g_checksum = 0;

/* Test function for scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    
    /* UNORDERED - using isnan() */
    res = isnan(d1) || isnan(d2);
    g_checksum += res;
    
    /* ORDERED - using !isnan() */
    res = !isnan(d1) && !isnan(d2);
    g_checksum += res;
    
    /* UNEQ - unordered or equal */
    res = isnan(d1) || isnan(d2) || (d1 == d2);
    g_checksum += res;
    
    /* UNGE - unordered or greater-or-equal */
    res = isnan(d1) || isnan(d2) || (d1 >= d2);
    g_checksum += res;
    
    /* UNGT - unordered or greater */
    res = isnan(d1) || isnan(d2) || (d1 > d2);
    g_checksum += res;
    
    /* UNLE - unordered or less-or-equal */
    res = isnan(d1) || isnan(d2) || (d1 <= d2);
    g_checksum += res;
    
    /* UNLT - unordered or less */
    res = isnan(d1) || isnan(d2) || (d1 < d2);
    g_checksum += res;
    
    /* LTGT - less or greater (ordered and not equal) */
    res = (!isnan(d1) && !isnan(d2)) && (d1 != d2);
    g_checksum += res;
    
    /* Additional tests using math.h comparison macros */
    res = isunordered(f1, f2);
    g_checksum += res;
    
    res = !isunordered(f1, f2);
    g_checksum += res;
    
    res = isgreater(f1, f2);
    g_checksum += res;
    
    res = isless(f1, f2);
    g_checksum += res;
    
    res = isgreaterequal(f1, f2);
    g_checksum += res;
    
    res = islessequal(f1, f2);
    g_checksum += res;
}

/* Test function for SSE vector conditions */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test various comparison predicates */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);    /* UNORDERED */
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);      /* ORDERED */
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);      /* UNEQ */
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NGE_UQ);     /* UNGE (not less) */
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NGT_UQ);     /* UNGT (not less-or-equal) */
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_OS);      /* UNLE variant */
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_OS);      /* UNLT variant */
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);     /* LTGT (not equal) */
    _mm_storeu_pd((double*)dres, cmp_res);
    g_checksum += (int)dres[0] + (int)dres[1];
    
    /* Float vector tests */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    g_checksum += (int)fres[0] + (int)fres[1] + (int)fres[2] + (int)fres[3];
    
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    g_checksum += (int)fres[0] + (int)fres[1] + (int)fres[2] + (int)fres[3];
}

/* Test function using inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    volatile int result = 0;
    volatile __m128d va, vb;
    volatile __m128 vfa, vfb;
    
    va = _mm_set_pd(a, b);
    vb = _mm_set_pd(b, a);
    vfa = _mm_set_ps(fa, fb, fa, fb);
    vfb = _mm_set_ps(fb, fa, fb, fa);
    
    /* Test each condition code mnemonic in inline assembly */
    /* Using AT&T/Intel template syntax variation */
    
    /* UNORDERED */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        : "=x"(result)
        : "x"(va), "x"(vb)
        : "cc"
    );
    g_checksum += result;
    
    /* ORDERED */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        : "=x"(result)
        : "x"(va), "x"(vb)
        : "cc"
    );
    g_checksum += result;
    
    /* UNEQ */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}\n\t"
        : "=x"(result)
        : "x"(va), "x"(vb)
        : "cc"
    );
    g_checksum += result;
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        : "=x"(result)
        : "x"(va), "x"(vb)
        : "cc"
    );
    g_checksum += result;
    
    /* UNGT (nle) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}\n\t"
        : "=x"(result)
        : "x"(va), "x"(vb)
        : "cc"
    );
    g_checksum += result;
    
    /* UNLE (ule) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        : "=x"(result)
        : "x"(va), "x"(vb)
        : "cc"
    );
    g_checksum += result;
    
    /* UNLT (ult) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}\n\t"
        : "=x"(result)
        : "x"(va), "x"(vb)
        : "cc"
    );
    g_checksum += result;
    
    /* LTGT (une) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        : "=x"(result)
        : "x"(va), "x"(vb)
        : "cc"
    );
    g_checksum += result;
    
    /* Float comparisons */
    __asm__ volatile (
        "cmpps %2, %1, %{%0|unord}\n\t"
        : "=x"(result)
        : "x"(vfa), "x"(vfb)
        : "cc"
    );
    g_checksum += result;
    
    __asm__ volatile (
        "cmpps %2, %1, %{%0|ord}\n\t"
        : "=x"(result)
        : "x"(vfa), "x"(vfb)
        : "cc"
    );
    g_checksum += result;
}

/* Complex branching test to force condition code generation */
__attribute__((optimize("O3"), noinline))
void test_complex_branching(double* darr, float* farr, int n) {
    volatile int i, j;
    volatile double sum = 0.0;
    volatile float fsum = 0.0f;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            /* Create complex conditions that can't be easily optimized away */
            if (isunordered(darr[i], darr[j])) {
                sum += 1.0;
            } else if (!isunordered(darr[i], darr[j]) && (darr[i] != darr[j])) {
                sum += 2.0;
            }
            
            if (isgreater(farr[i], farr[j])) {
                fsum += 1.0f;
            } else if (isless(farr[i], farr[j])) {
                fsum += 2.0f;
            } else if (isunordered(farr[i], farr[j])) {
                fsum += 3.0f;
            }
        }
    }
    
    g_checksum += (int)sum + (int)fsum;
}

int main(int argc, char** argv) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values (normal, NaN, inf) */
    double darr[8];
    float farr[8];
    
    for (int i = 0; i < 8; i++) {
        darr[i] = (rand() % 100) * 0.1;
        farr[i] = (rand() % 100) * 0.1f;
    }
    
    /* Insert some special values */
    darr[2] = 0.0 / 0.0;  /* NaN */
    darr[5] = 1.0 / 0.0;  /* Inf */
    farr[3] = 0.0f / 0.0f; /* NaN */
    farr[6] = -1.0f / 0.0f; /* -Inf */
    
    /* Test scalar conditions */
    for (int i = 0; i < 8; i += 2) {
        test_scalar_conditions(darr[i], darr[i+1], farr[i], farr[i+1]);
    }
    
    /* Test vector conditions */
    for (int i = 0; i < 8; i += 4) {
        __m128d vd1 = _mm_set_pd(darr[i], darr[i+1]);
        __m128d vd2 = _mm_set_pd(darr[i+2], darr[i+3]);
        __m128 vf1 = _mm_set_ps(farr[i], farr[i+1], farr[i+2], farr[i+3]);
        __m128 vf2 = _mm_set_ps(farr[i+3], farr[i+2], farr[i+1], farr[i]);
        test_vector_conditions(vd1, vd2, vf1, vf2);
    }
    
    /* Test inline assembly */
    for (int i = 0; i < 8; i += 2) {
        test_inline_asm_conditions(darr[i], darr[i+1], farr[i], farr[i+1]);
    }
    
    /* Test complex branching */
    test_complex_branching(darr, farr, 8);
    
    printf("Final checksum: %lu\n", g_checksum);
    return 0;
}
