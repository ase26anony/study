/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Test function with O0 optimization */
__attribute__((optimize("O0"), noinline))
void test_scalar_conditions(double d1, double d2, float f1, float f2, int *checksum) {
    volatile int local_sum = 0;
    
    /* UNORDERED/ORDERED tests using isnan/isunordered */
    if (isunordered(d1, d2)) {
        local_sum += 1;  /* Triggers unord */
    }
    
    if (!isunordered(d1, d2)) {
        local_sum += 2;  /* Triggers ord */
    }
    
    /* UNEQ test - unordered or equal */
    if (isunordered(d1, d2) || (d1 == d2)) {
        local_sum += 4;  /* Triggers ueq */
    }
    
    /* UNGE test - unordered or greater-or-equal */
    if (isunordered(d1, d2) || (d1 >= d2)) {
        local_sum += 8;  /* Triggers nlt */
    }
    
    /* UNGT test - unordered or greater */
    if (isunordered(d1, d2) || (d1 > d2)) {
        local_sum += 16;  /* Triggers nle */
    }
    
    /* UNLE test - unordered or less-or-equal */
    if (isunordered(d1, d2) || (d1 <= d2)) {
        local_sum += 32;  /* Triggers ule */
    }
    
    /* UNLT test - unordered or less */
    if (isunordered(d1, d2) || (d1 < d2)) {
        local_sum += 64;  /* Triggers ult */
    }
    
    /* LTGT test - less or greater (ordered and not equal) */
    if ((d1 < d2) || (d1 > d2)) {
        local_sum += 128;  /* Triggers une */
    }
    
    /* Repeat with float types */
    if (isunordered(f1, f2)) local_sum += 256;
    if (!isunordered(f1, f2)) local_sum += 512;
    
    *checksum += local_sum;
}

/* Test function with SSE2 vector operations */
__attribute__((optimize("O2"), target("sse2"), noinline))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 fv1, __m128 fv2, int *checksum) {
    volatile int local_sum = 0;
    
    /* UNORDERED - _CMP_UNORD_Q */
    __m128d cmp_unord = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    __m128d cmp_ord = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    __m128d cmp_ueq = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    __m128d cmp_nlt = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    __m128d cmp_nle = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    __m128d cmp_ule = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    __m128d cmp_ult = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    __m128d cmp_une = _mm_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    
    /* Convert mask results to integers to use in checksum */
    double res[2];
    _mm_storeu_pd(res, cmp_unord);
    local_sum += (int)(res[0] != 0.0) + (int)(res[1] != 0.0);
    
    _mm_storeu_pd(res, cmp_ord);
    local_sum += (int)(res[0] != 0.0) + (int)(res[1] != 0.0);
    
    _mm_storeu_pd(res, cmp_ueq);
    local_sum += (int)(res[0] != 0.0) + (int)(res[1] != 0.0);
    
    _mm_storeu_pd(res, cmp_nlt);
    local_sum += (int)(res[0] != 0.0) + (int)(res[1] != 0.0);
    
    _mm_storeu_pd(res, cmp_nle);
    local_sum += (int)(res[0] != 0.0) + (int)(res[1] != 0.0);
    
    _mm_storeu_pd(res, cmp_ule);
    local_sum += (int)(res[0] != 0.0) + (int)(res[1] != 0.0);
    
    _mm_storeu_pd(res, cmp_ult);
    local_sum += (int)(res[0] != 0.0) + (int)(res[1] != 0.0);
    
    _mm_storeu_pd(res, cmp_une);
    local_sum += (int)(res[0] != 0.0) + (int)(res[1] != 0.0);
    
    /* Repeat with float vectors */
    __m128 cmp_unord_f = _mm_cmp_ps(fv1, fv2, _CMP_UNORD_Q);
    __m128 cmp_ord_f = _mm_cmp_ps(fv1, fv2, _CMP_ORD_Q);
    __m128 cmp_ueq_f = _mm_cmp_ps(fv1, fv2, _CMP_EQ_UQ);
    
    float fres[4];
    _mm_storeu_ps(fres, cmp_unord_f);
    local_sum += (int)(fres[0] != 0.0f);
    
    *checksum += local_sum;
}

/* Test function with inline assembly templates */
__attribute__((optimize("O1"), noinline))
void test_inline_asm_conditions(double a, double b, float fa, float fb, int *checksum) {
    volatile int local_sum = 0;
    volatile double result_d;
    volatile float result_f;
    
    /* Inline assembly with condition code mnemonics */
    /* UNORDERED */
    __asm__ volatile ("cmpsd %2, %1, %{%0|unord}\n\t"
                      "movq %1, %0"
                      : "=x"(result_d) : "x"(a), "x"(b) : "cc");
    local_sum += (int)(result_d != 0.0);
    
    /* ORDERED */
    __asm__ volatile ("cmpsd %2, %1, %{%0|ord}\n\t"
                      "movq %1, %0"
                      : "=x"(result_d) : "x"(a), "x"(b) : "cc");
    local_sum += (int)(result_d != 0.0);
    
    /* UNEQ */
    __asm__ volatile ("cmpsd %2, %1, %{%0|ueq}\n\t"
                      "movq %1, %0"
                      : "=x"(result_d) : "x"(a), "x"(b) : "cc");
    local_sum += (int)(result_d != 0.0);
    
    /* UNGE (nlt) */
    __asm__ volatile ("cmpsd %2, %1, %{%0|nlt}\n\t"
                      "movq %1, %0"
                      : "=x"(result_d) : "x"(a), "x"(b) : "cc");
    local_sum += (int)(result_d != 0.0);
    
    /* UNGT (nle) */
    __asm__ volatile ("cmpsd %2, %1, %{%0|nle}\n\t"
                      "movq %1, %0"
                      : "=x"(result_d) : "x"(a), "x"(b) : "cc");
    local_sum += (int)(result_d != 0.0);
    
    /* UNLE (ule) */
    __asm__ volatile ("cmpsd %2, %1, %{%0|ule}\n\t"
                      "movq %1, %0"
                      : "=x"(result_d) : "x"(a), "x"(b) : "cc");
    local_sum += (int)(result_d != 0.0);
    
    /* UNLT (ult) */
    __asm__ volatile ("cmpsd %2, %1, %{%0|ult}\n\t"
                      "movq %1, %0"
                      : "=x"(result_d) : "x"(a), "x"(b) : "cc");
    local_sum += (int)(result_d != 0.0);
    
    /* LTGT (une) */
    __asm__ volatile ("cmpsd %2, %1, %{%0|une}\n\t"
                      "movq %1, %0"
                      : "=x"(result_d) : "x"(a), "x"(b) : "cc");
    local_sum += (int)(result_d != 0.0);
    
    /* Float version with different syntax */
    __asm__ volatile ("cmpss %2, %1, %{%0|unord}\n\t"
                      "movd %1, %0"
                      : "=x"(result_f) : "x"(fa), "x"(fb) : "cc");
    local_sum += (int)(result_f != 0.0f);
    
    *checksum += local_sum;
}

/* Test with O3 optimization and vectorization */
__attribute__((optimize("O3"), noinline))
void test_optimized_conditions(double *darray, float *farray, int size, int *checksum) {
    volatile int local_sum = 0;
    
    for (int i = 0; i < size - 1; i++) {
        /* Mix of different conditions in loop to prevent optimization */
        if (isunordered(darray[i], darray[i+1])) {
            local_sum += i;
        }
        
        if (!isunordered(darray[i], darray[i+1])) {
            local_sum -= i;
        }
        
        if (isunordered(farray[i], farray[i+1]) || (farray[i] == farray[i+1])) {
            local_sum += i * 2;
        }
        
        if ((darray[i] < darray[i+1]) || (darray[i] > darray[i+1])) {
            local_sum += i * 3;
        }
    }
    
    *checksum += local_sum;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Initialize with non-uniform values */
    double d1 = 1.0, d2 = 2.0, d3 = 0.0/0.0;  /* NaN */
    float f1 = 3.0f, f2 = 4.0f, f3 = 0.0f/0.0f;  /* NaN */
    
    /* Initialize arrays with varying values */
    double darray[10];
    float farray[10];
    
    for (int i = 0; i < 10; i++) {
        darray[i] = (i % 3 == 0) ? (double)i : 
                    (i % 3 == 1) ? (double)i * 1.5 : 
                    (double)(0.0/0.0);  /* Some NaNs */
        farray[i] = (i % 4 == 0) ? (float)i : 
                    (i % 4 == 1) ? (float)i * 1.5f :
                    (i % 4 == 2) ? (float)(0.0f/0.0f) :  /* Some NaNs */
                    (float)-i;
    }
    
    /* Initialize vector values */
    __m128d v1 = _mm_set_pd(1.0, 2.0);
    __m128d v2 = _mm_set_pd(3.0, 0.0/0.0);  /* One NaN */
    __m128 fv1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 fv2 = _mm_set_ps(5.0f, 0.0f/0.0f, 6.0f, 7.0f);  /* One NaN */
    
    /* Run all test functions */
    test_scalar_conditions(d1, d2, f1, f2, &checksum);
    test_scalar_conditions(d3, d1, f3, f1, &checksum);  /* With NaN */
    test_scalar_conditions(d2, d3, f2, f3, &checksum);  /* With NaN */
    
    test_vector_conditions(v1, v2, fv1, fv2, &checksum);
    
    test_inline_asm_conditions(d1, d2, f1, f2, &checksum);
    test_inline_asm_conditions(d1, d3, f1, f3, &checksum);  /* With NaN */
    
    test_optimized_conditions(darray, farray, 10, &checksum);
    
    /* Print final checksum */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
