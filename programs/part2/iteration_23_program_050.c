/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *sum, unsigned long val) {
    *sum ^= val;
    *sum = (*sum << 13) | (*sum >> (64 - 13));
    *sum *= 0x9e3779b97f4a7c15UL;
}

/* Test scalar floating-point conditions */
__attribute__((optimize("O0")))
void test_scalar_conditions(double d1, double d2, float f1, float f2) {
    volatile int res;
    unsigned long local_sum = 0;
    
    /* UNORDERED/ORDERED tests using isnan/isunordered */
    res = isnan(d1) || isnan(d2);
    mix(&local_sum, res);
    
    res = !isnan(d1) && !isnan(d2);
    mix(&local_sum, res);
    
    /* Complex branching to force condition code generation */
    if (isunordered(f1, f2)) {
        local_sum += 0x1234;
    } else if (isgreater(f1, f2)) {
        local_sum += 0x5678;
    } else if (isless(f1, f2)) {
        local_sum += 0x9abc;
    }
    
    /* UNEQ: unordered or equal */
    if (!(f1 < f2) && !(f1 > f2)) {
        local_sum += 0xdef0;
    }
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(d1 < d2)) {
        local_sum += 0x1111;
    }
    
    /* UNGT: not less than or equal (unordered or greater) */
    if (!(d1 <= d2)) {
        local_sum += 0x2222;
    }
    
    /* UNLE: unordered or less or equal */
    if (islessequal(f1, f2) || isnan(f1) || isnan(f2)) {
        local_sum += 0x3333;
    }
    
    /* UNLT: unordered or less than */
    if (isless(f1, f2) || isnan(f1) || isnan(f2)) {
        local_sum += 0x4444;
    }
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((f1 < f2) || (f1 > f2)) {
        local_sum += 0x5555;
    }
    
    g_checksum ^= local_sum;
}

/* Test with SSE2 vector operations */
__attribute__((target("sse2"), optimize("O2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 v3, __m128 v4) {
    unsigned long local_sum = 0;
    volatile __m128d cmp_res;
    volatile __m128 cmp_resf;
    
    /* UNORDERED: _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    mix(&local_sum, _mm_movemask_pd(cmp_res));
    
    /* ORDERED: _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    mix(&local_sum, _mm_movemask_pd(cmp_res));
    
    /* UNEQ: _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    mix(&local_sum, _mm_movemask_pd(cmp_res));
    
    /* UNGE: _CMP_NLT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    mix(&local_sum, _mm_movemask_pd(cmp_res));
    
    /* UNGT: _CMP_NLE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    mix(&local_sum, _mm_movemask_pd(cmp_res));
    
    /* UNLE: _CMP_LE_OS (will generate ule in some contexts) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_OS);
    mix(&local_sum, _mm_movemask_pd(cmp_res));
    
    /* UNLT: _CMP_LT_OS (will generate ult in some contexts) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_OS);
    mix(&local_sum, _mm_movemask_pd(cmp_res));
    
    /* LTGT: _CMP_NEQ_OS */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OS);
    mix(&local_sum, _mm_movemask_pd(cmp_res));
    
    /* Float vector tests */
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_UNORD_Q);
    mix(&local_sum, _mm_movemask_ps(cmp_resf));
    
    cmp_resf = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    mix(&local_sum, _mm_movemask_ps(cmp_resf));
    
    g_checksum ^= local_sum;
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b, float fa, float fb) {
    unsigned long local_sum = 0;
    volatile int res;
    
    /* Test each condition code mnemonic in inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "+r"(res) : "x"(a), "x"(b) : "al", "cc"
    );
    mix(&local_sum, res);
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %1, %0\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "+r"(res) : "x"(a), "x"(b) : "al", "cc"
    );
    mix(&local_sum, res);
    
    /* UNEQ - using cmppd with unord predicate */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        int mask;
        __asm__ volatile (
            "cmppd %2, %1, %{%0|ueq}\n\t"
            "movmskpd %1, %0"
            : "=r"(mask) : "x"(va), "x"(vb) : "cc"
        );
        mix(&local_sum, mask);
    }
    
    /* UNGE - using nlt */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        int mask;
        __asm__ volatile (
            "cmppd %2, %1, %{%0|nlt}\n\t"
            "movmskpd %1, %0"
            : "=r"(mask) : "x"(va), "x"(vb) : "cc"
        );
        mix(&local_sum, mask);
    }
    
    /* UNGT - using nle */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        int mask;
        __asm__ volatile (
            "cmppd %2, %1, %{%0|nle}\n\t"
            "movmskpd %1, %0"
            : "=r"(mask) : "x"(va), "x"(vb) : "cc"
        );
        mix(&local_sum, mask);
    }
    
    /* UNLE - using ule */
    {
        float f1 = fa, f2 = fb;
        int cmp;
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setbe %0\n\t"
            "setp %%al\n\t"
            "orb %%al, %0"
            : "=r"(cmp) : "x"(f1), "x"(f2) : "al", "cc"
        );
        mix(&local_sum, cmp);
    }
    
    /* UNLT - using ult */
    {
        float f1 = fa, f2 = fb;
        int cmp;
        __asm__ volatile (
            "ucomiss %2, %1\n\t"
            "setb %0\n\t"
            "setp %%al\n\t"
            "orb %%al, %0"
            : "=r"(cmp) : "x"(f1), "x"(f2) : "al", "cc"
        );
        mix(&local_sum, cmp);
    }
    
    /* LTGT - using une */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        int mask;
        __asm__ volatile (
            "cmppd %2, %1, %{%0|une}\n\t"
            "movmskpd %1, %0"
            : "=r"(mask) : "x"(va), "x"(vb) : "cc"
        );
        mix(&local_sum, mask);
    }
    
    g_checksum ^= local_sum;
}

/* Test with mixed optimization levels */
__attribute__((optimize("O3"), target("avx")))
void test_avx_conditions(double d1, double d2) {
    unsigned long local_sum = 0;
    volatile double res;
    
    /* Force generation of condition codes through conditional moves */
    res = (isunordered(d1, d2) || d1 > d2) ? d1 * 2.0 : d2 / 2.0;
    mix(&local_sum, *(unsigned long*)&res);
    
    res = (!isnan(d1) && !isnan(d2) && d1 != d2) ? d1 + d2 : d1 - d2;
    mix(&local_sum, *(unsigned long*)&res);
    
    /* Nested conditions */
    if (isunordered(d1, d2)) {
        if (!(d1 < d2)) {
            res = d1;
        } else {
            res = d2;
        }
    } else if (d1 == d2) {
        res = d1 * d2;
    } else {
        res = d1 / d2;
    }
    mix(&local_sum, *(unsigned long*)&res);
    
    g_checksum ^= local_sum;
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = (argc > 1) ? strtoul(argv[1], NULL, 0) : 0x12345678;
    srand(seed);
    
    /* Create test data with NaNs, infinities, and normal numbers */
    double darray[8];
    float farray[8];
    
    for (int i = 0; i < 8; i++) {
        double val = (double)rand() / RAND_MAX * 100.0 - 50.0;
        /* Introduce some special values */
        if (i == 2) val = 0.0 / 0.0;  /* NaN */
        if (i == 3) val = 1.0 / 0.0;  /* +Inf */
        if (i == 4) val = -1.0 / 0.0; /* -Inf */
        darray[i] = val;
        farray[i] = (float)val;
    }
    
    /* Call test functions with various combinations */
    for (int i = 0; i < 4; i++) {
        test_scalar_conditions(darray[i], darray[i+1], 
                              farray[i], farray[i+1]);
        
        __m128d v1 = _mm_set_pd(darray[i*2], darray[i*2+1]);
        __m128d v2 = _mm_set_pd(darray[i*2+2], darray[i*2+3]);
        __m128 f1 = _mm_set_ps(farray[i*2], farray[i*2+1],
                              farray[i*2+2], farray[i*2+3]);
        __m128 f2 = _mm_set_ps(farray[i*2+4], farray[i*2+5],
                              farray[i*2+6], farray[i*2+7]);
        
        test_vector_conditions(v1, v2, f1, f2);
        test_inline_asm_conditions(darray[i], darray[i+2],
                                  farray[i], farray[i+2]);
        test_avx_conditions(darray[i], darray[i+3]);
    }
    
    printf("Final checksum: 0x%016lx\n", g_checksum);
    return (int)(g_checksum & 0x7FFFFFFF);
}
