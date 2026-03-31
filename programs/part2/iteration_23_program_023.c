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
    *c = *c * 0x9e3779b97f4a7c15UL;
}

/* Test scalar floating-point conditions with macros */
__attribute__((optimize("O0")))
void test_scalar_conditions(double a, double b, float fa, float fb) {
    volatile int res;
    
    /* UNORDERED: isunordered() */
    res = isunordered(a, b);
    mix(&checksum, res);
    
    /* ORDERED: !isunordered() */
    res = !isunordered(a, b);
    mix(&checksum, res);
    
    /* UNEQ: unordered or equal - simulate with macros */
    res = (isunordered(a, b) || (a == b));
    mix(&checksum, res);
    
    /* UNGE: !isless() */
    res = !isless(a, b);
    mix(&checksum, res);
    
    /* UNGT: !islessequal() */
    res = !islessequal(a, b);
    mix(&checksum, res);
    
    /* UNLE: islessequal() with unordered handling */
    res = (isunordered(a, b) || islessequal(a, b));
    mix(&checksum, res);
    
    /* UNLT: isless() with unordered handling */
    res = (isunordered(a, b) || isless(a, b));
    mix(&checksum, res);
    
    /* LTGT: not equal and ordered */
    res = (!isunordered(a, b) && (a != b));
    mix(&checksum, res);
    
    /* Additional float tests */
    res = isunordered(fa, fb);
    mix(&checksum, res);
    
    res = !isunordered(fa, fb);
    mix(&checksum, res);
}

/* Test with SSE2 vector comparisons */
__attribute__((optimize("O2"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 cmp_resf;
    volatile double dres[2];
    volatile float fres[4];
    
    /* UNORDERED: _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* ORDERED: _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNEQ: _CMP_EQ_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_EQ_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNGE: _CMP_NLT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNGT: _CMP_NLE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNLE: _CMP_LE_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LE_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* UNLT: _CMP_LT_UQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_LT_UQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* LTGT: _CMP_NEQ_OQ */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd((double*)dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Float vector tests */
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_UNORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(&checksum, *(unsigned*)&fres[i]);
    
    cmp_resf = _mm_cmp_ps(f1, f2, _CMP_ORD_Q);
    _mm_storeu_ps((float*)fres, cmp_resf);
    for (int i = 0; i < 4; i++) mix(&checksum, *(unsigned*)&fres[i]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile unsigned long flags;
    
    /* Test each condition code mnemonic in inline assembly */
    
    /* UNORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzx %%al, %0"
        : "=r"(flags) : "x"(a), "x"(b) : "eax", "cc"
    );
    mix(&checksum, flags);
    
    /* ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzx %%al, %0"
        : "=r"(flags) : "x"(a), "x"(b) : "eax", "cc"
    );
    mix(&checksum, flags);
    
    /* UNEQ - using cmppd with unord predicate */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        __asm__ volatile (
            "cmppd %3, %2, %{%1|ueq}\n\t"
            "movq %2, %0"
            : "=m"(result) : "i"(0), "x"(va), "x"(vb) : "memory"
        );
        mix(&checksum, *(unsigned long*)&result);
    }
    
    /* UNGE - using nlt */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        __asm__ volatile (
            "cmppd %3, %2, %{%1|nlt}\n\t"
            "movq %2, %0"
            : "=m"(result) : "i"(0), "x"(va), "x"(vb) : "memory"
        );
        mix(&checksum, *(unsigned long*)&result);
    }
    
    /* UNGT - using nle */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        __asm__ volatile (
            "cmppd %3, %2, %{%1|nle}\n\t"
            "movq %2, %0"
            : "=m"(result) : "i"(0), "x"(va), "x"(vb) : "memory"
        );
        mix(&checksum, *(unsigned long*)&result);
    }
    
    /* UNLE - using ule */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        __asm__ volatile (
            "cmppd %3, %2, %{%1|ule}\n\t"
            "movq %2, %0"
            : "=m"(result) : "i"(0), "x"(va), "x"(vb) : "memory"
        );
        mix(&checksum, *(unsigned long*)&result);
    }
    
    /* UNLT - using ult */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        __asm__ volatile (
            "cmppd %3, %2, %{%1|ult}\n\t"
            "movq %2, %0"
            : "=m"(result) : "i"(0), "x"(va), "x"(vb) : "memory"
        );
        mix(&checksum, *(unsigned long*)&result);
    }
    
    /* LTGT - using une */
    {
        __m128d va = _mm_set1_pd(a);
        __m128d vb = _mm_set1_pd(b);
        __asm__ volatile (
            "cmppd %3, %2, %{%1|une}\n\t"
            "movq %2, %0"
            : "=m"(result) : "i"(0), "x"(va), "x"(vb) : "memory"
        );
        mix(&checksum, *(unsigned long*)&result);
    }
}

/* Complex branching to force condition code generation */
__attribute__((optimize("O3"), noinline))
void test_complex_branching(double *arr, int n) {
    volatile int count = 0;
    
    for (int i = 0; i < n - 1; i++) {
        double a = arr[i];
        double b = arr[i + 1];
        
        /* Complex nested conditions using all the macros */
        if (isunordered(a, b)) {
            count++;
        } else if (!isunordered(a, b) && (a != b)) {
            count += 2;
        }
        
        if (!isless(a, b)) {
            count += 3;
        }
        
        if (!islessequal(a, b)) {
            count += 5;
        }
        
        if (isunordered(a, b) || islessequal(a, b)) {
            count += 7;
        }
        
        if (isunordered(a, b) || isless(a, b)) {
            count += 11;
        }
        
        if (isunordered(a, b) || (a == b)) {
            count += 13;
        }
    }
    
    mix(&checksum, count);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = 0x12345678;
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[16];
    float farr[16];
    
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: darr[i] = (double)rand() / RAND_MAX * 100.0; break;
            case 1: darr[i] = -((double)rand() / RAND_MAX * 100.0); break;
            case 2: darr[i] = 0.0; break;
            case 3: darr[i] = __builtin_nan(""); break;
            case 4: darr[i] = __builtin_inf(); break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Test scalar conditions */
    for (int i = 0; i < 15; i++) {
        test_scalar_conditions(darr[i], darr[i+1], farr[i], farr[i+1]);
    }
    
    /* Test vector conditions */
    for (int i = 0; i < 15; i += 2) {
        __m128d v1 = _mm_set_pd(darr[i], darr[i+1]);
        __m128d v2 = _mm_set_pd(darr[i+1], darr[i+2]);
        __m128 f1 = _mm_set_ps(farr[i], farr[i+1], farr[i+2], farr[i+3]);
        __m128 f2 = _mm_set_ps(farr[i+1], farr[i+2], farr[i+3], farr[i+4]);
        test_vector_conditions(v1, v2, f1, f2);
    }
    
    /* Test inline assembly */
    for (int i = 0; i < 15; i++) {
        test_inline_asm_conditions(darr[i], darr[i+1]);
    }
    
    /* Test complex branching */
    test_complex_branching(darr, 16);
    
    printf("Final checksum: 0x%016lx\n", checksum);
    
    return 0;
}
