#include <math.h>
#include <immintrin.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Test functions with different optimization attributes */
__attribute__((optimize("O0"), target("sse2")))
static void test_scalar_conditions(volatile double d1, volatile double d2, 
                                   volatile float f1, volatile float f2,
                                   volatile int* checksum) {
    /* UNORDERED/ORDERED tests with NaNs */
    volatile double nan = 0.0/0.0;
    volatile double inf = 1.0/0.0;
    
    /* Test UNORDERED (unord) */
    if (isunordered(d1, nan)) {
        *checksum += 1;
    }
    
    /* Test ORDERED (ord) */
    if (isordered(d1, d2)) {
        *checksum += 2;
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    volatile double a = d1;
    volatile double b = d2;
    if (!(a > b) && !(a < b)) {  /* This can generate UNEQ */
        *checksum += 4;
    }
    
    /* Test UNGE (nlt) - unordered or greater-or-equal */
    if (!(a < b)) {
        *checksum += 8;
    }
    
    /* Test UNGT (nle) - unordered or greater */
    if (!(a <= b)) {
        *checksum += 16;
    }
    
    /* Test UNLE (ule) - unordered or less-or-equal */
    if (!(a > b)) {
        *checksum += 32;
    }
    
    /* Test UNLT (ult) - unordered or less */
    if (!(a >= b)) {
        *checksum += 64;
    }
    
    /* Test LTGT (une) - less or greater (ordered and not equal) */
    if ((a < b) || (a > b)) {
        *checksum += 128;
    }
    
    /* More complex branching to prevent optimization */
    volatile int i;
    for (i = 0; i < 3; i++) {
        volatile double x = d1 + i;
        volatile double y = d2 + i;
        
        if (isunordered(x, y)) {
            *checksum += 256;
        } else if (isgreater(x, y)) {
            *checksum += 512;
        } else if (isless(x, y)) {
            *checksum += 1024;
        }
    }
}

__attribute__((optimize("O2"), target("sse2")))
static void test_vector_conditions(__m128d v1, __m128d v2, 
                                   __m128 f1, __m128 f2,
                                   volatile int* checksum) {
    /* Vector comparisons that generate condition codes */
    __m128d cmp_result;
    
    /* UNORDERED - _CMP_UNORD_Q */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    *checksum += _mm_movemask_pd(cmp_result);
    
    /* ORDERED - _CMP_ORD_Q */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    *checksum += _mm_movemask_pd(cmp_result) << 2;
    
    /* UNEQ - _CMP_EQ_UQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    *checksum += _mm_movemask_pd(cmp_result) << 4;
    
    /* UNGE - _CMP_NLT_UQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    *checksum += _mm_movemask_pd(cmp_result) << 6;
    
    /* UNGT - _CMP_NLE_UQ */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    *checksum += _mm_movemask_pd(cmp_result) << 8;
    
    /* UNLE - _CMP_LE_OS (ordered, signaling - can generate ule) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_LE_OS);
    *checksum += _mm_movemask_pd(cmp_result) << 10;
    
    /* UNLT - _CMP_LT_OS (ordered, signaling - can generate ult) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_LT_OS);
    *checksum += _mm_movemask_pd(cmp_result) << 12;
    
    /* LTGT - _CMP_NEQ_OS (ordered, signaling - can generate une) */
    cmp_result = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    *checksum += _mm_movemask_pd(cmp_result) << 14;
    
    /* Float vector tests */
    __m128 cmp_result_f;
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    *checksum += _mm_movemask_ps(cmp_result_f) << 16;
    
    cmp_result_f = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    *checksum += _mm_movemask_ps(cmp_result_f) << 20;
}

__attribute__((optimize("O1"), target("sse2")))
static void test_inline_asm_conditions(volatile double d1, volatile double d2,
                                       __m128d v1, __m128d v2,
                                       volatile int* checksum) {
    /* Inline assembly with explicit condition code mnemonics */
    int result;
    double a = d1, b = d2;
    
    /* Test various condition codes in inline assembly */
    /* Note: These templates will be processed by output_operand */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    *checksum += result;
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "al", "cc"
    );
    *checksum += result << 1;
    
    /* Using condition codes in cmov */
    long long ll_result;
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "mov $1, %0\n\t"
        "cmovnp %3, %0"
        : "=r"(ll_result)
        : "x"(a), "x"(b), "r"(0LL)
        : "cc"
    );
    *checksum += (int)ll_result;
    
    /* Vector comparisons with inline assembly */
    __m128d cmp;
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}"
        : "=x"(cmp)
        : "x"(v1), "x"(v2)
    );
    *checksum += _mm_movemask_pd(cmp);
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}"
        : "=x"(cmp)
        : "x"(v1), "x"(v2)
    );
    *checksum += _mm_movemask_pd(cmp) << 2;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}"
        : "=x"(cmp)
        : "x"(v1), "x"(v2)
    );
    *checksum += _mm_movemask_pd(cmp) << 4;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}"
        : "=x"(cmp)
        : "x"(v1), "x"(v2)
    );
    *checksum += _mm_movemask_pd(cmp) << 6;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}"
        : "=x"(cmp)
        : "x"(v1), "x"(v2)
    );
    *checksum += _mm_movemask_pd(cmp) << 8;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}"
        : "=x"(cmp)
        : "x"(v1), "x"(v2)
    );
    *checksum += _mm_movemask_pd(cmp) << 10;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}"
        : "=x"(cmp)
        : "x"(v1), "x"(v2)
    );
    *checksum += _mm_movemask_pd(cmp) << 12;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}"
        : "=x"(cmp)
        : "x"(v1), "x"(v2)
    );
    *checksum += _mm_movemask_pd(cmp) << 14;
}

__attribute__((optimize("O3"), target("sse2"), noinline))
static void test_mixed_conditions(volatile double* darray, volatile float* farray,
                                  int size, volatile int* checksum) {
    /* Mixed scalar and vector operations */
    for (int i = 0; i < size - 1; i++) {
        volatile double d1 = darray[i];
        volatile double d2 = darray[i + 1];
        volatile float f1 = farray[i];
        volatile float f2 = farray[i + 1];
        
        /* Generate various condition codes through branching */
        if (isunordered(d1, d2)) {
            *checksum += i * 3;
        } else if (d1 == d2) {
            *checksum += i * 5;
        } else if (d1 > d2) {
            *checksum += i * 7;
        } else {
            *checksum += i * 11;
        }
        
        /* Ternary operator that might generate condition codes */
        volatile double result = (f1 < f2) ? (d1 * 2.0) : (d2 / 2.0);
        *checksum += (int)result;
        
        /* Complex condition */
        if ((isgreater(d1, 0.0) && isless(d2, 1.0)) || isunordered(f1, f2)) {
            *checksum += 1000;
        }
    }
}

int main(int argc, char* argv[]) {
    volatile int checksum = 0;
    
    /* Initialize with non-uniform values */
    volatile double dvals[] = {1.0, 2.0, 0.0, -1.0, 3.14, 2.71, 100.0, 0.001};
    volatile float fvals[] = {1.0f, 2.0f, 0.0f, -1.0f, 3.14f, 2.71f, 100.0f, 0.001f};
    
    /* Create some NaN and Inf values */
    volatile double nan_val = 0.0/0.0;
    volatile double inf_val = 1.0/0.0;
    dvals[2] = nan_val;  /* Introduce NaN */
    dvals[6] = inf_val;  /* Introduce Inf */
    
    /* Vector values */
    __m128d v1 = _mm_set_pd(1.0, 2.0);
    __m128d v2 = _mm_set_pd(nan_val, 3.0);
    __m128 fv1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 fv2 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    
    /* Run all tests */
    test_scalar_conditions(dvals[0], dvals[1], fvals[0], fvals[1], &checksum);
    test_vector_conditions(v1, v2, fv1, fv2, &checksum);
    test_inline_asm_conditions(dvals[3], dvals[4], v1, v2, &checksum);
    test_mixed_conditions(dvals, fvals, 8, &checksum);
    
    /* Additional tests with different values */
    for (int i = 0; i < 4; i++) {
        v1 = _mm_set_pd(dvals[i], dvals[i+1]);
        v2 = _mm_set_pd(dvals[i+2], dvals[i+3]);
        test_vector_conditions(v1, v2, fv1, fv2, &checksum);
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
