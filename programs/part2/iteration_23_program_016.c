/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Test functions with different optimization attributes */
__attribute__((optimize("O0"))) 
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* Test UNORDERED (unord) */
    res = isunordered(a, b);
    checksum += res;
    
    /* Test ORDERED (ord) */
    res = !isunordered(a, b);
    checksum += res;
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = (isunordered(a, b) || (a == b));
    checksum += res;
    
    /* Test UNGE (nlt) - unordered or greater than or equal */
    res = (isunordered(a, b) || (a >= b));
    checksum += res;
    
    /* Test UNGT (nle) - unordered or greater than */
    res = (isunordered(a, b) || (a > b));
    checksum += res;
    
    /* Test UNLE (ule) - unordered or less than or equal */
    res = (isunordered(a, b) || (a <= b));
    checksum += res;
    
    /* Test UNLT (ult) - unordered or less than */
    res = (isunordered(a, b) || (a < b));
    checksum += res;
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    res = (!isunordered(a, b) && (a != b));
    checksum += res;
    
    /* Complex branching to force condition code generation */
    if (isunordered(fa, fb)) {
        checksum += 1;
    } else if (fa > fb) {
        checksum += 2;
    } else if (fa < fb) {
        checksum += 3;
    } else {
        checksum += 4;
    }
}

__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions_O2(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    volatile __m128d resd;
    volatile __m128 resf;
    
    /* Test UNORDERED with vector comparisons */
    resd = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    checksum += _mm_movemask_pd(resd);
    
    /* Test ORDERED */
    resd = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    checksum += _mm_movemask_pd(resd);
    
    /* Test UNEQ */
    resd = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    checksum += _mm_movemask_pd(resd);
    
    /* Test UNGE */
    resd = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    checksum += _mm_movemask_pd(resd);
    
    /* Test UNGT */
    resd = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    checksum += _mm_movemask_pd(resd);
    
    /* Test UNLE */
    resd = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    checksum += _mm_movemask_pd(resd);
    
    /* Test UNLT */
    resd = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    checksum += _mm_movemask_pd(resd);
    
    /* Test LTGT (une) */
    resd = _mm_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    checksum += _mm_movemask_pd(resd);
    
    /* Float vector tests */
    resf = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    checksum += _mm_movemask_ps(resf);
    
    resf = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    checksum += _mm_movemask_ps(resf);
}

__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile int result;
    
    /* Inline assembly with explicit condition code mnemonics */
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result) : "x"(b), "0"(a) : "al", "cc"
    );
    checksum += result;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(result) : "x"(b), "0"(a) : "al", "cc"
    );
    checksum += result;
    
    /* Test with template substitution for condition codes */
    /* Using %{cond|...} pattern to test both AT&T and Intel syntax */
    __asm__ volatile (
        "cmppd %2, %1, %{%0|unord}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ord}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ueq}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nlt}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|nle}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ule}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|ult}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
    
    __asm__ volatile (
        "cmppd %2, %1, %{%0|une}\n\t"
        "movmskpd %1, %0"
        : "=r"(result) : "x"(a), "x"(b) : "cc"
    );
    checksum += result;
}

__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d v1, __m256d v2) {
    volatile __m256d res;
    
    /* AVX comparisons to test condition codes in wider vectors */
    res = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    checksum += _mm256_movemask_pd(res);
    
    res = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    checksum += _mm256_movemask_pd(res);
    
    res = _mm256_cmp_pd(v1, v2, _CMP_EQ_UQ);
    checksum += _mm256_movemask_pd(res);
    
    res = _mm256_cmp_pd(v1, v2, _CMP_NLT_UQ);
    checksum += _mm256_movemask_pd(res);
    
    res = _mm256_cmp_pd(v1, v2, _CMP_NLE_UQ);
    checksum += _mm256_movemask_pd(res);
    
    res = _mm256_cmp_pd(v1, v2, _CMP_LE_UQ);
    checksum += _mm256_movemask_pd(res);
    
    res = _mm256_cmp_pd(v1, v2, _CMP_LT_UQ);
    checksum += _mm256_movemask_pd(res);
    
    res = _mm256_cmp_pd(v1, v2, _CMP_NEQ_UQ);
    checksum += _mm256_movemask_pd(res);
}

void test_mixed_conditions(volatile double* darr, volatile float* farr, int n) {
    volatile double d1, d2;
    volatile float f1, f2;
    
    for (int i = 0; i < n - 1; i++) {
        d1 = darr[i];
        d2 = darr[i + 1];
        f1 = farr[i];
        f2 = farr[i + 1];
        
        /* Nested conditionals with uncommon floating-point conditions */
        if (isunordered(d1, d2)) {
            checksum += i;
            if (!isunordered(f1, f2) && f1 > f2) {
                checksum += i * 2;
            }
        } else if (d1 == d2) {
            checksum += i * 3;
        } else if (d1 > d2) {
            checksum += i * 4;
            /* Test LTGT condition */
            if (!isunordered(f1, f2) && f1 != f2) {
                checksum += i * 5;
            }
        } else {
            checksum += i * 6;
        }
        
        /* Ternary operator with floating-point conditions */
        int cond = (isunordered(d1, d2) || d1 >= d2) ? 1 : 0;
        checksum += cond;
        
        cond = (isunordered(d1, d2) || d1 > d2) ? 2 : 0;
        checksum += cond;
        
        cond = (isunordered(d1, d2) || d1 <= d2) ? 3 : 0;
        checksum += cond;
        
        cond = (isunordered(d1, d2) || d1 < d2) ? 4 : 0;
        checksum += cond;
    }
}

int main(int argc, char* argv[]) {
    /* Initialize with non-uniform values */
    unsigned int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create arrays with mixed values including NaNs and infinities */
    double darr[16];
    float farr[16];
    
    for (int i = 0; i < 16; i++) {
        double val = (double)(rand() % 1000) / 100.0;
        /* Introduce some special values */
        if (i % 5 == 0) val = 0.0 / 0.0;  /* NaN */
        if (i % 7 == 0) val = 1.0 / 0.0;  /* +Inf */
        if (i % 9 == 0) val = -1.0 / 0.0; /* -Inf */
        darr[i] = val;
        farr[i] = (float)val;
    }
    
    /* Initialize vector values */
    __m128d vd1 = _mm_set_pd(darr[0], darr[1]);
    __m128d vd2 = _mm_set_pd(darr[2], darr[3]);
    __m128 vf1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 vf2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    __m256d avx1 = _mm256_set_pd(darr[0], darr[1], darr[2], darr[3]);
    __m256d avx2 = _mm256_set_pd(darr[4], darr[5], darr[6], darr[7]);
    
    /* Run all test functions */
    test_scalar_conditions_O0(darr[0], darr[1], farr[0], farr[1]);
    test_vector_conditions_O2(vd1, vd2, vf1, vf2);
    test_inline_asm_conditions(darr[2], darr[3]);
    test_avx_conditions(avx1, avx2);
    test_mixed_conditions(darr, farr, 16);
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
