/* Condition code test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Test function with O0 optimization */
__attribute__((optimize("O0"), noinline))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int result = 0;
    
    /* UNORDERED/ORDERED tests with NaNs */
    if (isunordered(d1, d2)) {
        result |= 1;  /* Should generate unord condition */
    }
    
    if (!isunordered(d1, d2)) {
        result |= 2;  /* Should generate ord condition */
    }
    
    /* UNEQ: unordered or equal */
    if (!(d1 > d2) && !(d1 < d2)) {
        result |= 4;  /* May generate ueq for floating point */
    }
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(d1 < d2)) {
        result |= 8;  /* May generate nlt */
    }
    
    /* UNGT: not less or equal (unordered or greater) */
    if (!(d1 <= d2)) {
        result |= 16; /* May generate nle */
    }
    
    /* UNLE: unordered or less or equal */
    if (islessequal(f1, f2) || isunordered(f1, f2)) {
        result |= 32; /* May generate ule */
    }
    
    /* UNLT: unordered or less than */
    if (isless(f1, f2) || isunordered(f1, f2)) {
        result |= 64; /* May generate ult */
    }
    
    /* LTGT: less or greater (ordered and not equal) */
    if ((d1 < d2) || (d1 > d2)) {
        result |= 128; /* May generate une */
    }
    
    global_counter += result;
}

/* Test function with SSE2 and O2 optimization */
__attribute__((optimize("O2"), target("sse2"), noinline))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    volatile int result = 0;
    
    /* Vector comparisons that should generate condition codes */
    __m128d cmp1 = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);   /* UNORDERED */
    __m128d cmp2 = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);     /* ORDERED */
    __m128d cmp3 = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);     /* UNEQ */
    __m128d cmp4 = _mm_cmp_pd(v1, v2, _CMP_NGE_UQ);    /* UNGE -> nlt */
    __m128d cmp5 = _mm_cmp_pd(v1, v2, _CMP_NGT_UQ);    /* UNGT -> nle */
    __m128d cmp6 = _mm_cmp_pd(v1, v2, _CMP_LE_OS);     /* UNLE -> ule? */
    __m128d cmp7 = _mm_cmp_pd(v1, v2, _CMP_LT_OS);     /* UNLT -> ult? */
    __m128d cmp8 = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);    /* LTGT -> une */
    
    /* Force use of results */
    double d1[2], d2[2];
    _mm_store_pd(d1, cmp1);
    _mm_store_pd(d2, cmp8);
    
    result = (int)(d1[0] + d1[1] + d2[0] + d2[1]);
    
    /* Float vector comparisons */
    __m128 cmp9 = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    __m128 cmp10 = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    
    float f1[4], f2[4];
    _mm_store_ps(f1, cmp9);
    _mm_store_ps(f2, cmp10);
    
    result += (int)(f1[0] + f1[1] + f2[0] + f2[1]);
    
    global_counter += result;
}

/* Test function with inline assembly */
__attribute__((optimize("O1"), noinline))
void test_inline_asm_conditions(double a, double b) {
    volatile double result = 0.0;
    
    /* Inline assembly with condition code mnemonics */
    /* Note: Using different template variations */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setp %%al\n\t"
        : : "x" (a), "x" (b) : "al", "cc"
    );
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setnp %%al\n\t"
        : : "x" (a), "x" (b) : "al", "cc"
    );
    
    /* Using extended asm with template substitution */
    int res1, res2;
    
    /* UNEQ - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "sete %%dl\n\t"
        "or %%dl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (res1) : "x" (a), "x" (b) : "al", "dl", "cc"
    );
    
    /* UNGE - not less than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setae %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (res2) : "x" (a), "x" (b) : "al", "cc"
    );
    
    result = res1 + res2;
    global_counter += (int)result;
}

/* Additional test with branching */
__attribute__((optimize("O3"), noinline))
void test_branching_conditions(float *fa, double *da, int n) {
    volatile int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        /* Complex branching to force condition code generation */
        if (isunordered(da[i], da[i+1])) {
            count++;
        } else if (!isgreater(da[i], da[i+1]) && !isless(da[i], da[i+1])) {
            count += 2;  /* UNEQ-like */
        } else if (islessequal(fa[i], fa[i+1])) {
            count += 3;  /* UNLE-like */
        } else if (!(da[i] <= da[i+1])) {
            count += 4;  /* UNGT-like */
        }
        
        /* LTGT: ordered and not equal */
        if ((da[i] < da[i+1]) || (da[i] > da[i+1])) {
            count += 5;
        }
    }
    
    global_counter += count;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity */
    double darray[10];
    float farray[10];
    
    for (int i = 0; i < 10; i++) {
        farray[i] = (rand() % 100) / 10.0f;
        darray[i] = (rand() % 100) / 5.0;
        
        /* Introduce some special values */
        if (i == 2) darray[i] = 0.0 / 0.0;  /* NaN */
        if (i == 5) darray[i] = 1.0 / 0.0;  /* Inf */
        if (i == 7) farray[i] = 0.0f / 0.0f; /* NaN */
    }
    
    /* Test scalar conditions */
    for (int i = 0; i < 9; i++) {
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
    for (int i = 0; i < 5; i++) {
        test_inline_asm_conditions(darray[i], darray[i+1]);
    }
    
    /* Test branching */
    test_branching_conditions(farray, darray, 10);
    
    /* Print checksum */
    printf("Final checksum: %d\n", global_counter);
    
    return 0;
}
