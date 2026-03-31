#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <string.h>

/* Prevent constant folding and dead code elimination */
static volatile int global_counter = 0;
static volatile double checksum = 0.0;

/* Test functions with different optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* UNORDERED (unord) - test with NaN */
    result = isunordered(a, b);
    checksum += result;
    if (result) global_counter++;
    
    /* ORDERED (ord) */
    result = !isunordered(a, b);
    checksum += result;
    if (result) global_counter++;
    
    /* UNEQ (ueq) - unordered or equal */
    result = !isgreater(a, b) && !isless(a, b);
    checksum += result;
    if (result) global_counter++;
    
    /* UNGE (nlt) - unordered or greater or equal */
    result = !isless(a, b);
    checksum += result;
    if (result) global_counter++;
    
    /* UNGT (nle) - unordered or greater */
    result = !isless(a, b) && !(a == b);
    checksum += result;
    if (result) global_counter++;
    
    /* UNLE (ule) - unordered or less or equal */
    result = !isgreater(a, b);
    checksum += result;
    if (result) global_counter++;
    
    /* UNLT (ult) - unordered or less */
    result = !isgreater(a, b) && !(a == b);
    checksum += result;
    if (result) global_counter++;
    
    /* LTGT (une) - less or greater (ordered, not equal) */
    result = (a < b) || (a > b);
    checksum += result;
    if (result) global_counter++;
    
    /* Float versions to test different mode */
    result = isunordered(fa, fb);
    checksum += result;
    if (result) global_counter++;
    
    result = !isunordered(fa, fb);
    checksum += result;
    if (result) global_counter++;
}

__attribute__((optimize("O2"), target("sse2")))
void test_scalar_conditions_O2(double a, double b, float fa, float fb) {
    volatile int result;
    double temp;
    
    /* Complex branching to force condition code generation */
    if (isunordered(a, b)) {
        temp = 1.0;
    } else if (!isunordered(a, b)) {
        temp = 2.0;
    } else {
        temp = 3.0;
    }
    checksum += temp;
    
    /* Ternary operations with different conditions */
    temp = (!isgreater(a, b) && !isless(a, b)) ? 4.0 : 5.0;
    checksum += temp;
    
    temp = !isless(a, b) ? 6.0 : 7.0;
    checksum += temp;
    
    temp = (!isless(a, b) && !(a == b)) ? 8.0 : 9.0;
    checksum += temp;
    
    temp = !isgreater(a, b) ? 10.0 : 11.0;
    checksum += temp;
    
    temp = (!isgreater(a, b) && !(a == b)) ? 12.0 : 13.0;
    checksum += temp;
    
    temp = ((a < b) || (a > b)) ? 14.0 : 15.0;
    checksum += temp;
    
    /* Float version with nested conditions */
    if (isunordered(fa, fb)) {
        if (!isgreater(fa, fb)) {
            temp = 16.0;
        } else {
            temp = 17.0;
        }
    } else {
        temp = 18.0;
    }
    checksum += temp;
}

__attribute__((optimize("O3"), target("sse2")))
void test_vector_conditions(__m128d a, __m128d b, __m128 fa, __m128 fb) {
    __m128d mask_d;
    __m128 mask_f;
    volatile double temp[4];
    
    /* Vector comparisons with different predicates */
    /* _CMP_UNORD_Q = UNORDERED */
    mask_d = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
    _mm_store_pd((double*)temp, mask_d);
    checksum += temp[0] + temp[1];
    
    /* _CMP_ORD_Q = ORDERED */
    mask_d = _mm_cmp_pd(a, b, _CMP_ORD_Q);
    _mm_store_pd((double*)temp, mask_d);
    checksum += temp[0] + temp[1];
    
    /* _CMP_EQ_UQ = UNEQ */
    mask_d = _mm_cmp_pd(a, b, _CMP_EQ_UQ);
    _mm_store_pd((double*)temp, mask_d);
    checksum += temp[0] + temp[1];
    
    /* _CMP_NLT_UQ = UNGE (nlt) */
    mask_d = _mm_cmp_pd(a, b, _CMP_NLT_UQ);
    _mm_store_pd((double*)temp, mask_d);
    checksum += temp[0] + temp[1];
    
    /* _CMP_NLE_UQ = UNGT (nle) */
    mask_d = _mm_cmp_pd(a, b, _CMP_NLE_UQ);
    _mm_store_pd((double*)temp, mask_d);
    checksum += temp[0] + temp[1];
    
    /* _CMP_LE_OS = UNLE (ule) - Note: using ordered signaling version */
    mask_d = _mm_cmp_pd(a, b, _CMP_LE_OS);
    _mm_store_pd((double*)temp, mask_d);
    checksum += temp[0] + temp[1];
    
    /* _CMP_LT_OS = UNLT (ult) - ordered signaling */
    mask_d = _mm_cmp_pd(a, b, _CMP_LT_OS);
    _mm_store_pd((double*)temp, mask_d);
    checksum += temp[0] + temp[1];
    
    /* _CMP_NEQ_OS = LTGT (une) - ordered signaling, not equal */
    mask_d = _mm_cmp_pd(a, b, _CMP_NEQ_OS);
    _mm_store_pd((double*)temp, mask_d);
    checksum += temp[0] + temp[1];
    
    /* Float vector versions */
    /* _CMP_UNORD_Q */
    mask_f = _mm_cmp_ps(fa, fb, _CMP_UNORD_Q);
    _mm_store_ps((float*)temp, mask_f);
    checksum += temp[0] + temp[1] + temp[2] + temp[3];
    
    /* _CMP_ORD_Q */
    mask_f = _mm_cmp_ps(fa, fb, _CMP_ORD_Q);
    _mm_store_ps((float*)temp, mask_f);
    checksum += temp[0] + temp[1] + temp[2] + temp[3];
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b) {
    double result1, result2;
    __m128d va = _mm_set_pd(a, b);
    __m128d vb = _mm_set_pd(b, a);
    
    /* Inline assembly with explicit condition code mnemonics */
    /* Test various condition codes in AT&T syntax */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movsd %1, %0"
        : "=x"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result1;
    
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movsd %1, %0"
        : "=x"(result2)
        : "x"(a), "x"(b)
        : "cc"
    );
    checksum += result2;
    
    /* Test with vector registers */
    __m128d vresult;
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}\n\t"
        "movapd %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        "movapd %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}\n\t"
        "movapd %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        "movapd %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}\n\t"
        "movapd %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        "movapd %1, %0"
        : "=x"(vresult)
        : "x"(va), "x"(vb)
        : "cc"
    );
}

__attribute__((optimize("O2"), target("sse2")))
void test_mixed_conditions_loop(double *darray, float *farray, int n) {
    volatile int i;
    double d1, d2;
    float f1, f2;
    
    for (i = 0; i < n - 1; i++) {
        d1 = darray[i];
        d2 = darray[i + 1];
        f1 = farray[i];
        f2 = farray[i + 1];
        
        /* Mix of different conditions in loop to prevent optimization */
        if (isunordered(d1, d2)) {
            checksum += 1.0;
        } else if (!isunordered(d1, d2)) {
            checksum += 2.0;
        }
        
        if (!isgreater(d1, d2) && !isless(d1, d2)) {
            checksum += 3.0;
        }
        
        if (!isless(d1, d2)) {
            checksum += 4.0;
        }
        
        if (!isless(d1, d2) && !(d1 == d2)) {
            checksum += 5.0;
        }
        
        if (!isgreater(d1, d2)) {
            checksum += 6.0;
        }
        
        if (!isgreater(d1, d2) && !(d1 == d2)) {
            checksum += 7.0;
        }
        
        if ((d1 < d2) || (d1 > d2)) {
            checksum += 8.0;
        }
        
        /* Float conditions */
        checksum += isunordered(f1, f2) ? 9.0 : 10.0;
        checksum += !isunordered(f1, f2) ? 11.0 : 12.0;
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaN */
    double darray[20];
    float farray[20];
    
    for (int i = 0; i < 20; i++) {
        darray[i] = (double)(rand() % 1000) / 100.0;
        farray[i] = (float)(rand() % 1000) / 100.0;
        
        /* Insert some NaN values */
        if (i % 7 == 0) {
            darray[i] = 0.0 / 0.0;  /* NaN */
        }
        if (i % 5 == 0) {
            farray[i] = 0.0f / 0.0f;  /* NaN */
        }
    }
    
    /* Initialize vector values */
    __m128d vd1 = _mm_set_pd(darray[0], darray[1]);
    __m128d vd2 = _mm_set_pd(darray[2], darray[3]);
    __m128 vf1 = _mm_set_ps(farray[0], farray[1], farray[2], farray[3]);
    __m128 vf2 = _mm_set_ps(farray[4], farray[5], farray[6], farray[7]);
    
    /* Run all test functions */
    test_scalar_conditions_O0(darray[0], darray[1], farray[0], farray[1]);
    test_scalar_conditions_O2(darray[2], darray[3], farray[2], farray[3]);
    test_vector_conditions(vd1, vd2, vf1, vf2);
    test_inline_asm_conditions(darray[4], darray[5]);
    test_mixed_conditions_loop(darray, farray, 20);
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
