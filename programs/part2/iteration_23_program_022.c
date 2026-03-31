/* Test program to cover x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long val) {
    checksum = checksum * 31 + val;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* Test UNORDERED (unord) - unordered comparison */
    res = isunordered(a, b);
    mix(res);
    
    /* Test ORDERED (ord) - ordered comparison */
    res = isordered(a, b);
    mix(res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    /* Use complex expression to force condition code generation */
    if ((a != a) || (b != b) || (a == b)) {
        mix(1);
    } else {
        mix(0);
    }
    
    /* Test UNGE (nlt) - not less than (greater than or equal or unordered) */
    if (!(a < b) || (a != a) || (b != b)) {
        mix(2);
    }
    
    /* Test UNGT (nle) - not less than or equal (greater or unordered) */
    if (!(a <= b) || (a != a) || (b != b)) {
        mix(3);
    }
    
    /* Test UNLE (ule) - unordered or less than or equal */
    if ((a <= b) || (a != a) || (b != b)) {
        mix(4);
    }
    
    /* Test UNLT (ult) - unordered or less than */
    if ((a < b) || (a != a) || (b != b)) {
        mix(5);
    }
    
    /* Test LTGT (une) - less than or greater than (not equal and ordered) */
    if ((a < b) || (a > b)) {
        mix(6);
    }
    
    /* Float versions to test different modes */
    if (isunordered(fa, fb)) mix(7);
    if (isordered(fa, fb)) mix(8);
}

/* Test with SSE2 vector conditions */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test ORDERED for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNEQ for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNGE (nlt) for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NGE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNGT (nle) for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NGT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test UNLE (ule) for vectors - using float version */
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_LE_OS);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(*(unsigned*)&fres[i]);
    
    /* Test UNLT (ult) for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NGE_US);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
    
    /* Test LTGT (une) for vectors */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(*(unsigned long*)&dres[0]);
    mix(*(unsigned long*)&dres[1]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile int flag;
    
    /* Test UNORDERED in inline asm */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    mix(flag);
    
    /* Test ORDERED in inline asm */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    mix(flag);
    
    /* Test various conditions using cmppd with condition codes */
    __m128d va = _mm_set1_pd(a);
    __m128d vb = _mm_set1_pd(b);
    __m128d vres;
    
    /* Test UNEQ (ueq) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (8), "x" (va), "x" (vb)
        : 
    );
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (5), "x" (va), "x" (vb)
        : 
    );
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nle}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (6), "x" (va), "x" (vb)
        : 
    );
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (2), "x" (va), "x" (vb)
        : 
    );
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ult}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (1), "x" (va), "x" (vb)
        : 
    );
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movapd %2, %0"
        : "=x" (vres)
        : "i" (12), "x" (va), "x" (vb)
        : 
    );
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d v1, __m256d v2) {
    __m256d cmp_res;
    volatile double dres[4];
    
    /* Test various conditions with AVX */
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) mix(*(unsigned long*)&dres[i]);
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) mix(*(unsigned long*)&dres[i]);
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    for (int i = 0; i < 4; i++) mix(*(unsigned long*)&dres[i]);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with NaNs, normals, and special values */
    double test_doubles[] = {
        1.0, 2.0, 0.0, -1.0,
        __builtin_nan(""), __builtin_inf(),
        -__builtin_inf(), 3.14159
    };
    
    float test_floats[] = {
        1.0f, 2.0f, 0.0f, -1.0f,
        __builtin_nanf(""), __builtin_inff(),
        -__builtin_inff(), 2.71828f
    };
    
    /* Test scalar conditions with various value pairs */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            test_scalar_conditions(
                test_doubles[i], 
                test_doubles[j],
                test_floats[i],
                test_floats[j]
            );
        }
    }
    
    /* Test vector conditions */
    __m128d v1 = _mm_set_pd(test_doubles[0], test_doubles[1]);
    __m128d v2 = _mm_set_pd(test_doubles[2], test_doubles[3]);
    __m128 f1 = _mm_set_ps(test_floats[0], test_floats[1], test_floats[2], test_floats[3]);
    __m128 f2 = _mm_set_ps(test_floats[4], test_floats[5], test_floats[6], test_floats[7]);
    
    test_vector_conditions(v1, v2, f1, f2);
    
    /* Test inline assembly conditions */
    for (int i = 0; i < 4; i++) {
        test_inline_asm_conditions(test_doubles[i], test_doubles[i+4]);
    }
    
    /* Test AVX conditions if available */
    #ifdef __AVX__
    __m256d avx1 = _mm256_set_pd(test_doubles[0], test_doubles[1], 
                                 test_doubles[2], test_doubles[3]);
    __m256d avx2 = _mm256_set_pd(test_doubles[4], test_doubles[5],
                                 test_doubles[6], test_doubles[7]);
    test_avx_conditions(avx1, avx2);
    #endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
