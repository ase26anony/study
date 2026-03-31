/* Test program to cover x86 condition code mnemonics in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *sum, unsigned long val) {
    *sum = (*sum * 31) ^ val;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    unsigned long local_sum = 0;
    
    /* Test UNORDERED (unord) */
    res = isunordered(d1, d2);
    mix(&local_sum, res);
    if (res) {
        mix(&local_sum, 0xDEAD);
    }
    
    /* Test ORDERED (ord) */
    res = !isunordered(d1, d2);
    mix(&local_sum, res);
    if (res) {
        mix(&local_sum, 0xBEEF);
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = (isunordered(f1, f2) || (f1 == f2));
    mix(&local_sum, res);
    
    /* Test UNGE (nlt) - unordered or greater or equal */
    res = (isunordered(d1, d2) || (d1 >= d2));
    mix(&local_sum, res);
    
    /* Test UNGT (nle) - unordered or greater */
    res = (isunordered(d1, d2) || (d1 > d2));
    mix(&local_sum, res);
    
    /* Test UNLE (ule) - unordered or less or equal */
    res = (isunordered(f1, f2) || (f1 <= f2));
    mix(&local_sum, res);
    
    /* Test UNLT (ult) - unordered or less */
    res = (isunordered(f1, f2) || (f1 < f2));
    mix(&local_sum, res);
    
    /* Test LTGT (une) - less or greater (ordered and not equal) */
    res = (!isunordered(d1, d2) && (d1 != d2));
    mix(&local_sum, res);
    
    /* Complex branching to force condition code generation */
    volatile double vd = d1;
    for (int i = 0; i < 3; i++) {
        if (isunordered(vd, d2)) {
            mix(&local_sum, i * 2);
            vd += 1.0;
        } else if (!isunordered(vd, d2) && (vd != d2)) {
            mix(&local_sum, i * 3);
            vd -= 1.0;
        }
    }
    
    g_checksum += local_sum;
}

/* Test vector conditions with SSE */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    unsigned long local_sum = 0;
    
    /* Test UNORDERED (unord) */
    __m128d cmp_unord = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    int res_unord = _mm_movemask_pd(cmp_unord);
    mix(&local_sum, res_unord);
    
    /* Test ORDERED (ord) */
    __m128d cmp_ord = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    int res_ord = _mm_movemask_pd(cmp_ord);
    mix(&local_sum, res_ord);
    
    /* Test UNEQ (ueq) */
    __m128d cmp_ueq = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    int res_ueq = _mm_movemask_pd(cmp_ueq);
    mix(&local_sum, res_ueq);
    
    /* Test UNGE (nlt) */
    __m128d cmp_nlt = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    int res_nlt = _mm_movemask_pd(cmp_nlt);
    mix(&local_sum, res_nlt);
    
    /* Test UNGT (nle) */
    __m128d cmp_nle = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    int res_nle = _mm_movemask_pd(cmp_nle);
    mix(&local_sum, res_nle);
    
    /* Test UNLE (ule) */
    __m128 cmp_ule = _mm_cmp_ps(v3, v4, _CMP_LE_UQ);
    int res_ule = _mm_movemask_ps(cmp_ule);
    mix(&local_sum, res_ule);
    
    /* Test UNLT (ult) */
    __m128 cmp_ult = _mm_cmp_ps(v3, v4, _CMP_LT_UQ);
    int res_ult = _mm_movemask_ps(cmp_ult);
    mix(&local_sum, res_ult);
    
    /* Test LTGT (une) */
    __m128d cmp_une = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    int res_une = _mm_movemask_pd(cmp_une);
    mix(&local_sum, res_une);
    
    /* Conditional operations based on vector comparisons */
    volatile __m128d mask = _mm_and_pd(v1, cmp_unord);
    double d[2];
    _mm_store_pd(d, mask);
    mix(&local_sum, (unsigned long)(d[0] * 1000));
    mix(&local_sum, (unsigned long)(d[1] * 1000));
    
    g_checksum += local_sum;
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    unsigned long local_sum = 0;
    volatile double result;
    volatile float fresult;
    int cc;
    
    /* Test UNORDERED (unord) with inline asm */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc) : "x"(a), "x"(b) : "cc"
    );
    mix(&local_sum, cc);
    
    /* Test ORDERED (ord) with inline asm */
    __asm__ volatile (
        "ucomiss %2, %1\n\t"
        "setnp %0"
        : "=r"(cc) : "x"(fa), "x"(fb) : "cc"
    );
    mix(&local_sum, cc);
    
    /* Test various condition codes in cmppd with template substitution */
    #ifdef __x86_64__
    __m128d va = _mm_set_pd(a, b);
    __m128d vb = _mm_set_pd(b, a);
    
    /* Using extended asm with condition code in template */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|unord}\n\t"
        "movmskpd %1, %0"
        : "=r"(cc) : "x"(va), "x"(vb), "i"(0) : "cc"
    );
    mix(&local_sum, cc);
    
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ord}\n\t"
        "movmskpd %1, %0"
        : "=r"(cc) : "x"(va), "x"(vb), "i"(7) : "cc"
    );
    mix(&local_sum, cc);
    
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movmskpd %1, %0"
        : "=r"(cc) : "x"(va), "x"(vb), "i"(8) : "cc"
    );
    mix(&local_sum, cc);
    
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nlt}\n\t"
        "movmskpd %1, %0"
        : "=r"(cc) : "x"(va), "x"(vb), "i"(13) : "cc"
    );
    mix(&local_sum, cc);
    
    __asm__ volatile (
        "cmppd %3, %2, %{%1|nle}\n\t"
        "movmskpd %1, %0"
        : "=r"(cc) : "x"(va), "x"(vb), "i"(14) : "cc"
    );
    mix(&local_sum, cc);
    
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ule}\n\t"
        "movmskpd %1, %0"
        : "=r"(cc) : "x"(va), "x"(vb), "i"(18) : "cc"
    );
    mix(&local_sum, cc);
    
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ult}\n\t"
        "movmskpd %1, %0"
        : "=r"(cc) : "x"(va), "x"(vb), "i"(17) : "cc"
    );
    mix(&local_sum, cc);
    
    __asm__ volatile (
        "cmppd %3, %2, %{%1|une}\n\t"
        "movmskpd %1, %0"
        : "=r"(cc) : "x"(va), "x"(vb), "i"(12) : "cc"
    );
    mix(&local_sum, cc);
    #endif
    
    g_checksum += local_sum;
}

/* Additional test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(__m256d v1, __m256d v2) {
    unsigned long local_sum = 0;
    
    /* AVX comparisons that should generate condition codes */
    __m256d cmp = _mm256_cmp_pd(v1, v2, _CMP_UNORD_Q);
    double d[4];
    _mm256_store_pd(d, cmp);
    
    for (int i = 0; i < 4; i++) {
        mix(&local_sum, (unsigned long)(d[i] * 1000));
    }
    
    cmp = _mm256_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm256_store_pd(d, cmp);
    for (int i = 0; i < 4; i++) {
        mix(&local_sum, (unsigned long)(d[i] * 1000));
    }
    
    g_checksum += local_sum;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    /* Create test data with NaNs, infinities, and normal numbers */
    double d1 = (rand() % 100) / 10.0;
    double d2 = (rand() % 100) / 10.0;
    float f1 = (rand() % 100) / 10.0f;
    float f2 = (rand() % 100) / 10.0f;
    
    /* Introduce some special values */
    if (rand() % 3 == 0) d1 = 0.0 / 0.0;  /* NaN */
    if (rand() % 3 == 1) d2 = 1.0 / 0.0;  /* Inf */
    if (rand() % 3 == 2) f1 = -0.0 / 0.0; /* -NaN */
    
    /* Vector test data */
    __m128d vd1 = _mm_set_pd(d1, d2);
    __m128d vd2 = _mm_set_pd(d2, d1);
    __m128 vf1 = _mm_set_ps(f1, f2, f2, f1);
    __m128 vf2 = _mm_set_ps(f2, f1, f1, f2);
    
    /* AVX test data if available */
    __m256d avx1 = _mm256_set_pd(d1, d2, d1, d2);
    __m256d avx2 = _mm256_set_pd(d2, d1, d2, d1);
    
    printf("Testing condition code mnemonics...\n");
    printf("Seed: %lu\n", seed);
    
    /* Run all tests */
    test_scalar_conditions(d1, d2, f1, f2);
    test_vector_conditions(vd1, vd2, vf1, vf2);
    test_inline_asm_conditions(d1, d2, f1, f2);
    
    #ifdef __AVX__
    test_avx_conditions(avx1, avx2);
    #endif
    
    printf("Final checksum: %lu\n", g_checksum);
    
    return (int)(g_checksum % 256);
}
