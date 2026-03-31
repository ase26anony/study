/* Test program to cover condition code output in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function with specific optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions(double d1, double d2, float f1, float f2, int *checksum) {
    volatile int local_sum = 0;
    
    /* Test UNORDERED (unord) - unordered comparison */
    if (isunordered(d1, d2)) {
        local_sum += 1;
    }
    
    /* Test ORDERED (ord) - ordered comparison */
    if (!isunordered(d1, d2)) {  /* Equivalent to ordered */
        local_sum += 2;
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    if (isunordered(d1, d2) || (d1 == d2)) {
        local_sum += 4;
    }
    
    /* Test UNGE (nlt) - unordered or greater-or-equal */
    if (isunordered(d1, d2) || (d1 >= d2)) {
        local_sum += 8;
    }
    
    /* Test UNGT (nle) - unordered or greater */
    if (isunordered(d1, d2) || (d1 > d2)) {
        local_sum += 16;
    }
    
    /* Test UNLE (ule) - unordered or less-or-equal */
    if (isunordered(d1, d2) || (d1 <= d2)) {
        local_sum += 32;
    }
    
    /* Test UNLT (ult) - unordered or less */
    if (isunordered(d1, d2) || (d1 < d2)) {
        local_sum += 64;
    }
    
    /* Test LTGT (une) - less or greater (ordered and not equal) */
    if ((d1 < d2) || (d1 > d2)) {
        local_sum += 128;
    }
    
    /* Repeat with floats to test different mode */
    if (isunordered(f1, f2)) local_sum += 256;
    if (!isunordered(f1, f2)) local_sum += 512;
    
    *checksum += local_sum;
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 fv1, __m128 fv2, int *checksum) {
    volatile int local_sum = 0;
    
    /* Vector comparisons with different predicates */
    __m128d cmp_result;
    __m128 fcmp_result;
    
    /* UNORDERED - _CMP_UNORD_Q */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    local_sum += _mm_movemask_pd(cmp_result);
    
    /* ORDERED - _CMP_ORD_Q */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    local_sum += _mm_movemask_pd(cmp_result);
    
    /* UNEQ - _CMP_EQ_UQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    local_sum += _mm_movemask_pd(cmp_result);
    
    /* UNGE - _CMP_GE_UQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_GE_UQ);
    local_sum += _mm_movemask_pd(cmp_result);
    
    /* UNGT - _CMP_GT_UQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_GT_UQ);
    local_sum += _mm_movemask_pd(cmp_result);
    
    /* UNLE - _CMP_LE_UQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    local_sum += _mm_movemask_pd(cmp_result);
    
    /* UNLT - _CMP_LT_UQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    local_sum += _mm_movemask_pd(cmp_result);
    
    /* LTGT - _CMP_NEQ_OQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    local_sum += _mm_movemask_pd(cmp_result);
    
    /* Test with float vectors */
    fcmp_result = _mm_cmp_ps(fv1, fv2, _CMP_UNORD_Q);
    local_sum += _mm_movemask_ps(fcmp_result);
    
    fcmp_result = _mm_cmp_ps(fv1, fv2, _CMP_ORD_Q);
    local_sum += _mm_movemask_ps(fcmp_result);
    
    *checksum += local_sum;
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double d1, double d2, __m128d v1, __m128d v2, int *checksum) {
    volatile int local_sum = 0;
    volatile double result_d;
    volatile __m128d result_v;
    
    /* Inline assembly with explicit condition code mnemonics */
    
    /* Test unord (UNORDERED) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|unord}\n\t"
        "movq %1, %0"
        : "=x"(result_d)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    local_sum += (int)result_d;
    
    /* Test ord (ORDERED) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ord}\n\t"
        "movq %1, %0"
        : "=x"(result_d)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    local_sum += (int)result_d;
    
    /* Test ueq (UNEQ) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ueq}\n\t"
        "movq %1, %0"
        : "=x"(result_d)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    local_sum += (int)result_d;
    
    /* Test nlt (UNGE) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nlt}\n\t"
        "movq %1, %0"
        : "=x"(result_d)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    local_sum += (int)result_d;
    
    /* Test nle (UNGT) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|nle}\n\t"
        "movq %1, %0"
        : "=x"(result_d)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    local_sum += (int)result_d;
    
    /* Test ule (UNLE) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ule}\n\t"
        "movq %1, %0"
        : "=x"(result_d)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    local_sum += (int)result_d;
    
    /* Test ult (UNLT) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|ult}\n\t"
        "movq %1, %0"
        : "=x"(result_d)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    local_sum += (int)result_d;
    
    /* Test une (LTGT) */
    __asm__ volatile (
        "cmpsd %2, %1, %{%0|une}\n\t"
        "movq %1, %0"
        : "=x"(result_d)
        : "x"(d1), "x"(d2)
        : "cc"
    );
    local_sum += (int)result_d;
    
    /* Vector version with cmppd */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movapd %1, %0"
        : "=x"(result_v)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movapd %1, %0"
        : "=x"(result_v)
        : "x"(v1), "x"(v2)
        : "cc"
    );
    
    *checksum += local_sum;
}

__attribute__((optimize("O3"), target("sse2")))
void test_mixed_conditions(double *darray, float *farray, int size, int *checksum) {
    volatile int local_sum = 0;
    
    for (int i = 0; i < size - 1; i++) {
        /* Complex nested conditions to prevent optimization */
        if (isunordered(darray[i], darray[i+1])) {
            local_sum += i;
            if (!isunordered(farray[i], farray[i+1])) {
                local_sum += i * 2;
                if (darray[i] >= darray[i+1] || isunordered(darray[i], darray[i+1])) {
                    local_sum += i * 3;
                }
            }
        } else {
            local_sum -= i;
            if (darray[i] < darray[i+1] || darray[i] > darray[i+1]) {
                local_sum += i * 4;
            }
        }
        
        /* Ternary operator with uncommon conditions */
        double temp = (isunordered(darray[i], darray[i+1]) || 
                      (darray[i] <= darray[i+1])) ? darray[i] : darray[i+1];
        local_sum += (int)temp;
    }
    
    *checksum += local_sum;
}

int main(int argc, char *argv[]) {
    int checksum = 0;
    unsigned int seed = 12345;
    
    /* Use argv for seed variation if available */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    /* Initialize test data with varied values including NaN, Inf, normal */
    double d1 = (rand() % 100) / 10.0;
    double d2 = (rand() % 100) / 10.0;
    float f1 = (rand() % 100) / 10.0f;
    float f2 = (rand() % 100) / 10.0f;
    
    /* Introduce special floating-point values */
    if (rand() % 3 == 0) d1 = 0.0 / 0.0;  /* NaN */
    if (rand() % 3 == 1) d2 = 1.0 / 0.0;  /* Inf */
    if (rand() % 3 == 2) f1 = -1.0 / 0.0; /* -Inf */
    
    /* Vector data */
    __m128d v1 = _mm_set_pd(d1, d2);
    __m128d v2 = _mm_set_pd(d2, d1);
    __m128 fv1 = _mm_set_ps(f1, f2, f1, f2);
    __m128 fv2 = _mm_set_ps(f2, f1, f2, f1);
    
    /* Array data for loop tests */
    const int ARRAY_SIZE = 32;
    double darray[ARRAY_SIZE];
    float farray[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        darray[i] = (rand() % 200 - 100) / 10.0;
        farray[i] = (rand() % 200 - 100) / 10.0f;
        /* Sprinkle some special values */
        if (i % 7 == 0) darray[i] = 0.0 / 0.0;
        if (i % 11 == 0) farray[i] = 1.0 / 0.0f;
    }
    
    /* Run all test functions */
    test_scalar_conditions(d1, d2, f1, f2, &checksum);
    test_vector_conditions(v1, v2, fv1, fv2, &checksum);
    test_inline_asm_conditions(d1, d2, v1, v2, &checksum);
    test_mixed_conditions(darray, farray, ARRAY_SIZE, &checksum);
    
    /* Use global volatile to prevent dead code elimination */
    global_counter = checksum;
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;
}
