#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Prevent constant folding and dead code elimination */
volatile int global_counter = 0;
volatile double volatile_double = 3.14159;
volatile float volatile_float = 2.71828f;

/* Test function with specific optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions(double d1, double d2, float f1, float f2, int* checksum) {
    /* UNORDERED (unord) - test with NaN */
    double nan_val = 0.0 / 0.0;  /* Generate NaN */
    int unord_result = isnan(d1) || isnan(d2) ? 1 : 0;
    if (isunordered(d1, d2)) {
        *checksum += 1;
    }
    
    /* ORDERED (ord) */
    if (isordered(d1, d2)) {
        *checksum += 2;
    }
    
    /* UNEQ (ueq) - unordered or equal */
    if (d1 == d2 || isunordered(d1, d2)) {
        *checksum += 4;
    }
    
    /* UNGE (nlt) - unordered or not less than (greater than or equal) */
    if (!(d1 < d2) || isunordered(d1, d2)) {
        *checksum += 8;
    }
    
    /* UNGT (nle) - unordered or not less than or equal (greater than) */
    if (!(d1 <= d2) || isunordered(d1, d2)) {
        *checksum += 16;
    }
    
    /* UNLE (ule) - unordered or less than or equal */
    if (d1 <= d2 || isunordered(d1, d2)) {
        *checksum += 32;
    }
    
    /* UNLT (ult) - unordered or less than */
    if (d1 < d2 || isunordered(d1, d2)) {
        *checksum += 64;
    }
    
    /* LTGT (une) - less than or greater than (not equal and ordered) */
    if ((d1 < d2 || d1 > d2) && isordered(d1, d2)) {
        *checksum += 128;
    }
    
    /* Force volatile usage */
    if (volatile_double > 0.0) {
        *checksum += 256;
    }
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2, int* checksum) {
    /* Vector comparisons using SSE intrinsics */
    
    /* UNORDERED - _CMP_UNORD_Q */
    __m128d unord_mask = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    double unord_result[2];
    _mm_store_pd(unord_result, unord_mask);
    *checksum += (int)(unord_result[0] + unord_result[1]);
    
    /* ORDERED - _CMP_ORD_Q */
    __m128d ord_mask = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    double ord_result[2];
    _mm_store_pd(ord_result, ord_mask);
    *checksum += (int)(ord_result[0] + ord_result[1]);
    
    /* UNEQ - _CMP_EQ_UQ */
    __m128d ueq_mask = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    double ueq_result[2];
    _mm_store_pd(ueq_result, ueq_mask);
    *checksum += (int)(ueq_result[0] + ueq_result[1]);
    
    /* UNGE - _CMP_NLT_UQ */
    __m128d unge_mask = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    double unge_result[2];
    _mm_store_pd(unge_result, unge_mask);
    *checksum += (int)(unge_result[0] + unge_result[1]);
    
    /* UNGT - _CMP_NLE_UQ */
    __m128d ungt_mask = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    double ungt_result[2];
    _mm_store_pd(ungt_result, ungt_mask);
    *checksum += (int)(ungt_result[0] + ungt_result[1]);
    
    /* UNLE - _CMP_LE_UQ */
    __m128d unle_mask = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    double unle_result[2];
    _mm_store_pd(unle_result, unle_mask);
    *checksum += (int)(unle_result[0] + unle_result[1]);
    
    /* UNLT - _CMP_LT_UQ */
    __m128d unlt_mask = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    double unlt_result[2];
    _mm_store_pd(unlt_result, unlt_mask);
    *checksum += (int)(unlt_result[0] + unlt_result[1]);
    
    /* LTGT - _CMP_NEQ_OQ */
    __m128d ltgt_mask = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    double ltgt_result[2];
    _mm_store_pd(ltgt_result, ltgt_mask);
    *checksum += (int)(ltgt_result[0] + ltgt_result[1]);
    
    /* Float vector comparisons */
    __m128 f_unord = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    float f_result[4];
    _mm_store_ps(f_result, f_unord);
    *checksum += (int)(f_result[0] + f_result[1] + f_result[2] + f_result[3]);
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double d1, double d2, __m128d v1, __m128d v2, int* checksum) {
    double result1, result2;
    __m128d vresult;
    
    /* Inline assembly with condition code mnemonics */
    
    /* UNORDERED (unord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        : "=x"(result1)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    *checksum += (int)result1;
    
    /* ORDERED (ord) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        : "=x"(result2)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    *checksum += (int)result2;
    
    /* UNEQ (ueq) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}\n\t"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    double vres[2];
    _mm_store_pd(vres, vresult);
    *checksum += (int)(vres[0] + vres[1]);
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    _mm_store_pd(vres, vresult);
    *checksum += (int)(vres[0] + vres[1]);
    
    /* UNGT (nle) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}\n\t"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    _mm_store_pd(vres, vresult);
    *checksum += (int)(vres[0] + vres[1]);
    
    /* UNLE (ule) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    _mm_store_pd(vres, vresult);
    *checksum += (int)(vres[0] + vres[1]);
    
    /* UNLT (ult) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}\n\t"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    _mm_store_pd(vres, vresult);
    *checksum += (int)(vres[0] + vres[1]);
    
    /* LTGT (une) */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        : "=x"(vresult)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    _mm_store_pd(vres, vresult);
    *checksum += (int)(vres[0] + vres[1]);
}

__attribute__((optimize("O3"), target("sse2")))
void test_complex_branching(double* darray, float* farray, int size, int* checksum) {
    /* Complex branching to force condition code generation */
    for (int i = 0; i < size - 1; i++) {
        double d1 = darray[i];
        double d2 = darray[i + 1];
        float f1 = farray[i];
        float f2 = farray[i + 1];
        
        /* Nested conditionals with uncommon comparisons */
        if (isunordered(d1, d2)) {
            *checksum += i * 2;
            if (isordered(f1, f2)) {
                *checksum += i * 3;
            }
        } else if (!(d1 < d2) || isunordered(d1, d2)) {  /* UNGE */
            *checksum += i * 5;
        } else if (d1 <= d2 || isunordered(d1, d2)) {    /* UNLE */
            *checksum += i * 7;
        }
        
        /* Ternary operator with complex conditions */
        int temp = ((d1 < d2 || isunordered(d1, d2)) ? 
                   ((f1 > f2 || isunordered(f1, f2)) ? 11 : 13) : 
                   ((d1 == d2 || isunordered(d1, d2)) ? 17 : 19));
        *checksum += temp;
    }
}

int main(int argc, char* argv[]) {
    int checksum = 0;
    
    /* Initialize with non-uniform values */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Create arrays with mixed values including NaN and Inf */
    double darray[10];
    float farray[10];
    
    for (int i = 0; i < 10; i++) {
        darray[i] = (rand() % 100) / 10.0;
        farray[i] = (rand() % 100) / 10.0f;
        
        /* Introduce some special values */
        if (i == 2) darray[i] = 0.0 / 0.0;  /* NaN */
        if (i == 5) darray[i] = 1.0 / 0.0;  /* Inf */
        if (i == 7) farray[i] = 0.0f / 0.0f; /* NaN */
    }
    
    /* Test scalar conditions */
    test_scalar_conditions(darray[0], darray[1], farray[0], farray[1], &checksum);
    
    /* Test vector conditions */
    __m128d v1 = _mm_set_pd(darray[0], darray[1]);
    __m128d v2 = _mm_set_pd(darray[2], darray[3]);
    __m128 f1 = _mm_set_ps(farray[0], farray[1], farray[2], farray[3]);
    __m128 f2 = _mm_set_ps(farray[4], farray[5], farray[6], farray[7]);
    test_vector_conditions(v1, v2, f1, f2, &checksum);
    
    /* Test inline assembly */
    test_inline_asm_conditions(darray[4], darray[5], v1, v2, &checksum);
    
    /* Test complex branching */
    test_complex_branching(darray, farray, 10, &checksum);
    
    /* Additional volatile operations to prevent optimization */
    volatile_double = checksum / 100.0;
    volatile_float = checksum / 100.0f;
    
    printf("Final checksum: %d\n", checksum);
    return checksum == 0 ? 1 : 0;  /* Return non-zero if checksum is 0 */
}
