/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *c, unsigned long v) {
    *c ^= v;
    *c = (*c << 13) | (*c >> (64 - 13));
    *c *= 0x9e3779b97f4a7c15ULL;
}

/* Test functions with different optimization attributes */

__attribute__((optimize("O0"), target("sse2")))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* Test UNORDERED (unord) - using isnan() */
    res = isnan(a) || isnan(b);
    mix(&checksum, res);
    
    /* Test ORDERED (ord) - using !isnan() */
    res = !isnan(a) && !isnan(b);
    mix(&checksum, res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    res = isnan(a) || isnan(b) || (a == b);
    mix(&checksum, res);
    
    /* Test UNGE (nlt) - unordered or not less than */
    res = isnan(a) || isnan(b) || !(a < b);
    mix(&checksum, res);
    
    /* Test UNGT (nle) - unordered or not less than or equal */
    res = isnan(a) || isnan(b) || !(a <= b);
    mix(&checksum, res);
    
    /* Test UNLE (ule) - unordered or less than or equal */
    res = isnan(a) || isnan(b) || (a <= b);
    mix(&checksum, res);
    
    /* Test UNLT (ult) - unordered or less than */
    res = isnan(a) || isnan(b) || (a < b);
    mix(&checksum, res);
    
    /* Test LTGT (une) - not equal and ordered */
    res = (!isnan(a) && !isnan(b)) && (a != b);
    mix(&checksum, res);
    
    /* Float versions to test different mode */
    res = isnan(fa) || isnan(fb);
    mix(&checksum, res);
    
    res = !isnan(fa) && !isnan(fb);
    mix(&checksum, res);
}

__attribute__((optimize("O2"), target("sse2")))
void test_scalar_conditions_O2(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* Different patterns to trigger different code generation */
    if (isunordered(a, b)) {
        res = 1;
    } else if (isgreater(a, b)) {
        res = 2;
    } else if (isless(a, b)) {
        res = 3;
    } else if (islessgreater(a, b)) {
        res = 4;
    } else {
        res = 5;
    }
    mix(&checksum, res);
    
    /* Ternary operations that might generate conditional moves */
    res = (isnan(a) || isnan(b)) ? 100 : 200;
    mix(&checksum, res);
    
    res = (!isnan(a) && !isnan(b)) ? 300 : 400;
    mix(&checksum, res);
    
    /* Complex expression */
    res = (isnan(a) || isnan(b) || a == b) ? 500 : 600;
    mix(&checksum, res);
}

__attribute__((optimize("O3"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    volatile __m128d resd;
    volatile __m128 resf;
    volatile int mask;
    
    /* Test UNORDERED for vectors */
    resd = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    mask = _mm_movemask_pd(resd);
    mix(&checksum, mask);
    
    /* Test ORDERED for vectors */
    resd = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    mask = _mm_movemask_pd(resd);
    mix(&checksum, mask);
    
    /* Test UNEQ for vectors */
    resd = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    mask = _mm_movemask_pd(resd);
    mix(&checksum, mask);
    
    /* Test UNGE for vectors */
    resd = _mm_cmp_pd(v1, v2, _CMP_NGE_UQ);
    mask = _mm_movemask_pd(resd);
    mix(&checksum, mask);
    
    /* Test UNGT for vectors */
    resd = _mm_cmp_pd(v1, v2, _CMP_NGT_UQ);
    mask = _mm_movemask_pd(resd);
    mix(&checksum, mask);
    
    /* Test UNLE for vectors */
    resd = _mm_cmp_pd(v1, v2, _CMP_LE_OS);
    mask = _mm_movemask_pd(resd);
    mix(&checksum, mask);
    
    /* Test UNLT for vectors */
    resd = _mm_cmp_pd(v1, v2, _CMP_LT_OS);
    mask = _mm_movemask_pd(resd);
    mix(&checksum, mask);
    
    /* Test LTGT for vectors */
    resd = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    mask = _mm_movemask_pd(resd);
    mix(&checksum, mask);
    
    /* Float vector tests */
    resf = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    mask = _mm_movemask_ps(resf);
    mix(&checksum, mask);
    
    resf = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    mask = _mm_movemask_ps(resf);
    mix(&checksum, mask);
}

__attribute__((optimize("O1"), target("sse2")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile int cc;
    
    /* Inline assembly with explicit condition code mnemonics */
    
    /* UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %b0"
        : "=q"(cc) : "x"(a), "x"(b) : "cc"
    );
    mix(&checksum, cc);
    
    /* ORDERED (ord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %b0"
        : "=q"(cc) : "x"(a), "x"(b) : "cc"
    );
    mix(&checksum, cc);
    
    /* UNEQ (ueq) - unordered or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %b0\n\t"
        "setp %%dl\n\t"
        "orb %%dl, %b0"
        : "=q"(cc) : "x"(a), "x"(b) : "cc", "dx"
    );
    mix(&checksum, cc);
    
    /* UNGE (nlt) - unordered or not less than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setae %b0"
        : "=q"(cc) : "x"(a), "x"(b) : "cc"
    );
    mix(&checksum, cc);
    
    /* UNGT (nle) - unordered or not less than or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "seta %b0"
        : "=q"(cc) : "x"(a), "x"(b) : "cc"
    );
    mix(&checksum, cc);
    
    /* UNLE (ule) - unordered or less than or equal */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %b0"
        : "=q"(cc) : "x"(a), "x"(b) : "cc"
    );
    mix(&checksum, cc);
    
    /* UNLT (ult) - unordered or less than */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %b0"
        : "=q"(cc) : "x"(a), "x"(b) : "cc"
    );
    mix(&checksum, cc);
    
    /* LTGT (une) - not equal and ordered */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %b0\n\t"
        "setnp %%dl\n\t"
        "andb %%dl, %b0"
        : "=q"(cc) : "x"(a), "x"(b) : "cc", "dx"
    );
    mix(&checksum, cc);
    
    /* Test with cmppd instruction and condition code in template */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|%0}"
        : "=x"(result) : "i"(0x3), "x"(a), "x"(b)  /* 0x3 = UNORD */
    );
    mix(&checksum, *(unsigned long long*)&result);
}

__attribute__((optimize("Os"), target("sse2")))
void test_mixed_conditions(volatile double* darr, volatile float* farr, int n) {
    volatile int count_unord = 0;
    volatile int count_ord = 0;
    volatile int count_ueq = 0;
    volatile int count_nlt = 0;
    volatile int count_nle = 0;
    volatile int count_ule = 0;
    volatile int count_ult = 0;
    volatile int count_une = 0;
    
    /* Loop to prevent excessive optimization */
    for (int i = 0; i < n - 1; i++) {
        double a = darr[i];
        double b = darr[i + 1];
        float fa = farr[i];
        float fb = farr[i + 1];
        
        /* Count various conditions */
        if (isunordered(a, b)) count_unord++;
        if (!isunordered(a, b)) count_ord++;
        if (isunordered(a, b) || a == b) count_ueq++;
        if (isunordered(a, b) || !(a < b)) count_nlt++;
        if (isunordered(a, b) || !(a <= b)) count_nle++;
        if (isunordered(a, b) || a <= b) count_ule++;
        if (isunordered(a, b) || a < b) count_ult++;
        if (!isunordered(a, b) && a != b) count_une++;
        
        /* Float versions */
        if (isunordered(fa, fb)) count_unord++;
        if (!isunordered(fa, fb)) count_ord++;
    }
    
    mix(&checksum, count_unord);
    mix(&checksum, count_ord);
    mix(&checksum, count_ueq);
    mix(&checksum, count_nlt);
    mix(&checksum, count_nle);
    mix(&checksum, count_ule);
    mix(&checksum, count_ult);
    mix(&checksum, count_une);
}

int main(int argc, char* argv[]) {
    /* Initialize with non-uniform values including NaN, Inf, normal numbers */
    unsigned seed = (argc > 1) ? atoi(argv[1]) : 12345;
    srand(seed);
    
    const int N = 32;
    volatile double darr[N];
    volatile float farr[N];
    
    /* Fill arrays with mixed values: normal, NaN, Inf */
    for (int i = 0; i < N; i++) {
        int r = rand();
        switch (r % 7) {
            case 0: darr[i] = 0.0; break;
            case 1: darr[i] = 1.0; break;
            case 2: darr[i] = -1.0; break;
            case 3: darr[i] = (double)r / RAND_MAX; break;
            case 4: darr[i] = -(double)r / RAND_MAX; break;
            case 5: darr[i] = 0.0 / 0.0; /* NaN */ break;
            case 6: darr[i] = 1.0 / 0.0; /* Inf */ break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Initialize vector values */
    __m128d v1 = _mm_set_pd(darr[0], darr[1]);
    __m128d v2 = _mm_set_pd(darr[2], darr[3]);
    __m128 f1 = _mm_set_ps(farr[0], farr[1], farr[2], farr[3]);
    __m128 f2 = _mm_set_ps(farr[4], farr[5], farr[6], farr[7]);
    
    /* Run all test functions */
    test_scalar_conditions_O0(darr[0], darr[1], farr[0], farr[1]);
    test_scalar_conditions_O2(darr[2], darr[3], farr[2], farr[3]);
    test_vector_conditions(v1, v2, f1, f2);
    test_inline_asm_conditions(darr[4], darr[5]);
    test_mixed_conditions(darr, farr, N);
    
    /* Print final checksum */
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
