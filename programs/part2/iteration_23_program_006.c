/* Test program for x86 condition code mnemonics (unord, ord, ueq, nlt, nle, ule, ult, une) */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Test function with O0 optimization to ensure all operations are emitted */
__attribute__((optimize("O0"), noinline))
void test_scalar_conditions(double a, double b, float c, float d) {
    volatile int res;
    
    /* Test UNORDERED (unord) */
    res = isunordered(a, b);
    checksum += res;
    
    /* Test ORDERED (ord) */
    res = !isunordered(a, b);
    checksum += res;
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = !(a > b) && !(a < b);
    checksum += res;
    
    /* Test UNGE (nlt) - not less than (greater or equal or unordered) */
    res = !(a < b);
    checksum += res;
    
    /* Test UNGT (nle) - not less or equal (greater or unordered) */
    res = !(a <= b);
    checksum += res;
    
    /* Test UNLE (ule) - less or equal or unordered */
    res = (a <= b) || isunordered(a, b);
    checksum += res;
    
    /* Test UNLT (ult) - less than or unordered */
    res = (a < b) || isunordered(a, b);
    checksum += res;
    
    /* Test LTGT (une) - less than or greater than (but not equal, not unordered) */
    res = (a < b) || (a > b);
    checksum += res;
    
    /* Repeat with float types */
    res = isunordered(c, d);
    checksum += res;
    
    res = !isunordered(c, d);
    checksum += res;
    
    /* Complex branching to force condition code generation */
    if (isunordered(a, b)) {
        checksum += 1;
    } else if (!(a < b)) {
        checksum += 2;
    } else if (!(a <= b)) {
        checksum += 3;
    } else if ((a <= b) || isunordered(a, b)) {
        checksum += 4;
    }
}

/* SSE2 vector tests */
__attribute__((target("sse2"), optimize("O2"), noinline))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED with _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1];
    
    /* Test ORDERED with _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1];
    
    /* Test UNEQ with _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1];
    
    /* Test UNGE with _CMP_NLT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1];
    
    /* Test UNGT with _CMP_NLE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1];
    
    /* Test UNLE with _CMP_LE_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_OS);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1];
    
    /* Test UNLT with _CMP_LT_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_OS);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1];
    
    /* Test LTGT with _CMP_NEQ_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    _mm_storeu_pd((double*)dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1];
    
    /* Repeat with float vectors */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    checksum += (int)fres[0] + (int)fres[1] + (int)fres[2] + (int)fres[3];
    
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    checksum += (int)fres[0] + (int)fres[1] + (int)fres[2] + (int)fres[3];
}

/* Inline assembly tests with explicit condition codes */
__attribute__((optimize("O1"), noinline))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile int flag;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += flag;
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flag)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    checksum += flag;
    
    /* Test UNEQ (ueq) using cmppd */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movq %2, %0"
        : "=m" (result)
        : "i" (0), "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += (int)result;
    
    /* Test UNGE (nlt) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movq %2, %0"
        : "=m" (result)
        : "i" (1), "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += (int)result;
    
    /* Test UNGT (nle) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nle}\n\t"
        "movq %2, %0"
        : "=m" (result)
        : "i" (6), "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += (int)result;
    
    /* Test UNLE (ule) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movq %2, %0"
        : "=m" (result)
        : "i" (2), "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += (int)result;
    
    /* Test UNLT (ult) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ult}\n\t"
        "movq %2, %0"
        : "=m" (result)
        : "i" (1), "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += (int)result;
    
    /* Test LTGT (une) */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movq %2, %0"
        : "=m" (result)
        : "i" (4), "x" (_mm_set1_pd(a)), "x" (_mm_set1_pd(b))
        : "memory"
    );
    checksum += (int)result;
}

/* High optimization level test */
__attribute__((optimize("O3"), target("avx"), noinline))
void test_avx_conditions(__m256d v1, __m256d v2) {
    __m256d cmp_res;
    volatile double dres[4];
    
    /* Mix of different comparison predicates */
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1] + (int)dres[2] + (int)dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1] + (int)dres[2] + (int)dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1] + (int)dres[2] + (int)dres[3];
    
    cmp_res = _mm256_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm256_storeu_pd(dres, cmp_res);
    checksum += (int)dres[0] + (int)dres[1] + (int)dres[2] + (int)dres[3];
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned seed = (argc > 1) ? (unsigned)atoi(argv[1]) : 12345;
    srand(seed);
    
    /* Create test data with NaN, normal, and special values */
    double d1 = 1.0 / (rand() % 100 + 1);
    double d2 = (rand() % 2) ? 0.0 / 0.0 : 2.0 / (rand() % 100 + 1); /* Possibly NaN */
    float f1 = (float)d1;
    float f2 = (float)d2;
    
    /* Vector data */
    __m128d vd1 = _mm_set_pd(d1, d2);
    __m128d vd2 = _mm_set_pd(d2, d1);
    __m128 vf1 = _mm_set_ps(f1, f2, f2, f1);
    __m128 vf2 = _mm_set_ps(f2, f1, f1, f2);
    __m256d avx1 = _mm256_set_pd(d1, d2, d1, d2);
    __m256d avx2 = _mm256_set_pd(d2, d1, d2, d1);
    
    printf("Testing condition code mnemonics with seed: %u\n", seed);
    
    /* Run all test functions */
    test_scalar_conditions(d1, d2, f1, f2);
    test_vector_conditions(vd1, vd2, vf1, vf2);
    test_inline_asm_conditions(d1, d2);
    test_avx_conditions(avx1, avx2);
    
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
