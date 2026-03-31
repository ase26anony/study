#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *c, unsigned long v) {
    *c ^= v;
    *c = (*c << 13) | (*c >> (64 - 13));
    *c = *c * 0x9e3779b97f4a7c15ULL;
}

/* Test scalar floating-point conditions using math.h macros */
__attribute__((optimize("O0")))
void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* UNORDERED - using isunordered */
    res = isunordered(a, b);
    mix(&g_checksum, res);
    
    /* ORDERED - using !isunordered */
    res = !isunordered(a, b);
    mix(&g_checksum, res);
    
    /* Complex conditions that may generate UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT */
    volatile double da = a, db = b;
    
    /* Force branching on uncommon conditions */
    if (isunordered(da, db) || (da == db)) {  /* May generate UNEQ */
        g_checksum += 1;
    }
    
    if (!(da < db) && !isunordered(da, db)) {  /* May generate UNGE (nlt) */
        g_checksum += 2;
    }
    
    if (!(da <= db) && !isunordered(da, db)) {  /* May generate UNGT (nle) */
        g_checksum += 3;
    }
    
    if ((da <= db) || isunordered(da, db)) {  /* May generate UNLE (ule) */
        g_checksum += 4;
    }
    
    if ((da < db) || isunordered(da, db)) {  /* May generate UNLT (ult) */
        g_checksum += 5;
    }
    
    if ((da < db || db < da) && !isunordered(da, db)) {  /* May generate LTGT (une) */
        g_checksum += 6;
    }
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* UNORDERED - _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, *(unsigned long*)&dres[0]);
    
    /* ORDERED - _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, *(unsigned long*)&dres[1]);
    
    /* UNEQ - _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, *(unsigned long*)&dres[0] ^ *(unsigned long*)&dres[1]);
    
    /* UNGE - _CMP_NLT_UQ (nlt) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, *(unsigned long*)&dres[0]);
    
    /* UNGT - _CMP_NLE_UQ (nle) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, *(unsigned long*)&dres[1]);
    
    /* UNLE - _CMP_LE_OS (ule) - Note: different predicate but may generate ule */
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_LE_OS);
    _mm_storeu_ps((float*)fres, cmp_resf);
    mix(&g_checksum, *(unsigned int*)&fres[0]);
    
    /* UNLT - _CMP_LT_OS (ult) */
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_LT_OS);
    _mm_storeu_ps((float*)fres, cmp_resf);
    mix(&g_checksum, *(unsigned int*)&fres[1]);
    
    /* LTGT - _CMP_NEQ_OS (une) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&g_checksum, *(unsigned long*)&dres[0] ^ *(unsigned long*)&dres[1]);
}

/* Test with inline assembly using condition code mnemonics */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile unsigned long flags;
    
    /* Test each condition code mnemonic in inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %{%0|unord}\n\t"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setae %{%0|ord}\n\t"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
    
    /* UNEQ */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "sete %{%0|ueq}\n\t"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
    
    /* UNGE (nlt) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnb %{%0|nlt}\n\t"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
    
    /* UNGT (nle) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %{%0|nle}\n\t"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
    
    /* UNLE (ule) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setbe %{%0|ule}\n\t"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
    
    /* UNLT (ult) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setb %{%0|ult}\n\t"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
    
    /* LTGT (une) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setne %{%0|une}\n\t"
        : "=r"(flags)
        : "x"(a), "x"(b)
        : "cc"
    );
    mix(&g_checksum, flags);
    
    /* Test with vector comparisons */
    __m128d v1 = _mm_set_pd(a, b);
    __m128d v2 = _mm_set_pd(b, a);
    
    /* UNORDERED with vector */
    __asm__ volatile (
        "cmppd %{%2|unord}, %1, %0\n\t"
        : "=x"(result)
        : "x"(v1), "x"(v2)
    );
    mix(&g_checksum, *(unsigned long*)&result);
}

/* Test with optimization O3 and no inlining */
__attribute__((optimize("O3"), noinline))
void test_optimized_conditions(double a, double b, float fa, float fb) {
    volatile double temp;
    
    /* Generate various condition codes through complex expressions */
    for (int i = 0; i < 3; i++) {
        double x = a + i * 0.5;
        double y = b - i * 0.25;
        
        /* This complex condition may generate multiple condition codes */
        if ((isunordered(x, y) || x == y) && 
            (!(x < y) || !isunordered(x, y)) &&
            ((x <= y) || isunordered(x, y))) {
            temp = x * y;
            mix(&g_checksum, *(unsigned long*)&temp);
        }
        
        /* Another complex condition */
        if (!(x <= y) && !isunordered(x, y) && 
            (x < y || y < x) && !isunordered(x, y)) {
            temp = x / (y + 1.0);
            mix(&g_checksum, *(unsigned long*)&temp);
        }
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = 123456789;
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double dvals[8];
    float fvals[8];
    
    for (int i = 0; i < 8; i++) {
        switch (i % 4) {
            case 0: dvals[i] = (double)rand() / RAND_MAX * 100.0; break;
            case 1: dvals[i] = -((double)rand() / RAND_MAX * 100.0); break;
            case 2: dvals[i] = 0.0 / 0.0; /* NaN */ break;
            case 3: dvals[i] = 1.0 / 0.0; /* Inf */ break;
        }
        fvals[i] = (float)dvals[i];
    }
    
    /* Run all test functions with different combinations */
    for (int i = 0; i < 4; i++) {
        test_scalar_conditions(dvals[i], dvals[i+1], fvals[i], fvals[i+1]);
        test_inline_asm_conditions(dvals[i], dvals[i+2]);
        test_optimized_conditions(dvals[i+1], dvals[i+3], fvals[i+1], fvals[i+3]);
    }
    
    /* Test vector conditions */
    __m128d v1 = _mm_set_pd(dvals[0], dvals[1]);
    __m128d v2 = _mm_set_pd(dvals[2], dvals[3]);
    __m128 f1 = _mm_set_ps(fvals[0], fvals[1], fvals[2], fvals[3]);
    __m128 f2 = _mm_set_ps(fvals[4], fvals[5], fvals[6], fvals[7]);
    
    test_vector_conditions(v1, v2, f1, f2);
    
    /* Additional vector tests with different alignments */
    v1 = _mm_loadu_pd(&dvals[1]);
    v2 = _mm_loadu_pd(&dvals[3]);
    test_vector_conditions(v1, v2, f1, f2);
    
    printf("Final checksum: %lu\n", g_checksum);
    return 0;
}
