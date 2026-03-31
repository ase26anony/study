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
    *c = *c * 0x9e3779b97f4a7c15ULL;
}

/* Test scalar floating-point conditions with different optimizations */
__attribute__((optimize("O0")))
void test_scalar_conditions_O0(double a, double b, float c, float d) {
    volatile int res;
    
    /* Test UNORDERED (unord) - unordered comparison */
    res = isunordered(a, b);
    mix(&checksum, res);
    
    /* Test ORDERED (ord) - ordered comparison */
    res = !isunordered(c, d);
    mix(&checksum, res);
    
    /* Test UNEQ (ueq) - unordered or equal */
    /* Using explicit comparison with NaN handling */
    volatile double da = a;
    volatile double db = b;
    if ((da != db) || isunordered(da, db)) {
        res = 0;
    } else {
        res = 1;
    }
    mix(&checksum, res);
    
    /* Test UNGE (nlt) - unordered or greater than or equal */
    res = !isless(c, d);
    mix(&checksum, res);
    
    /* Test UNGT (nle) - unordered or greater than */
    res = !islessequal(a, b);
    mix(&checksum, res);
    
    /* Test UNLE (ule) - unordered or less than or equal */
    res = islessequal(c, d) || isunordered(c, d);
    mix(&checksum, res);
    
    /* Test UNLT (ult) - unordered or less than */
    res = isless(a, b) || isunordered(a, b);
    mix(&checksum, res);
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    res = (a < b) || (a > b);
    mix(&checksum, res);
}

__attribute__((optimize("O2"), target("sse2")))
void test_scalar_conditions_O2(double a, double b, float c, float d) {
    volatile int res;
    
    /* Different pattern to trigger different code generation */
    res = (isunordered(a, b) ? 1 : 0) +
          (!isunordered(c, d) ? 2 : 0) +
          (!isless(a, b) ? 4 : 0) +
          (!islessequal(b, a) ? 8 : 0);
    mix(&checksum, res);
    
    /* Complex conditional to force branching */
    if (isunordered(a, b)) {
        res = 1;
    } else if (!isless(a, b) && !isgreater(a, b)) {
        res = 2;
    } else {
        res = 3;
    }
    mix(&checksum, res);
}

/* Test vector conditions using SSE intrinsics */
__attribute__((optimize("O3"), target("sse2")))
void test_vector_conditions(__m128d v1, __m128d v2, __m128 f1, __m128 f2) {
    __m128d cmp_res;
    __m128 fcmp_res;
    volatile double dres[2];
    volatile float fres[4];
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_UNORD_Q);
    _mm_storeu_pd(dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    
    /* Test ORDERED - _CMP_ORD_Q */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);
    _mm_storeu_pd(dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNEQ - _CMP_EQ_UQ */
    fcmp_res = _mm_cmp_ps(f1, f2, _CMP_EQ_UQ);
    _mm_storeu_ps(fres, fcmp_res);
    mix(&checksum, *(unsigned int*)&fres[0]);
    
    /* Test UNGE - _CMP_NLT_UQ (not less than, unordered quiet) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLT_UQ);
    _mm_storeu_pd(dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0]);
    
    /* Test UNGT - _CMP_NLE_UQ (not less than or equal, unordered quiet) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NLE_UQ);
    _mm_storeu_pd(dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[1]);
    
    /* Test UNLE - _CMP_LE_UQ (less than or equal, unordered quiet) */
    fcmp_res = _mm_cmp_ps(f1, f2, _CMP_LE_UQ);
    _mm_storeu_ps(fres, fcmp_res);
    mix(&checksum, *(unsigned int*)&fres[1]);
    
    /* Test UNLT - _CMP_LT_UQ (less than, unordered quiet) */
    fcmp_res = _mm_cmp_ps(f1, f2, _CMP_LT_UQ);
    _mm_storeu_ps(fres, fcmp_res);
    mix(&checksum, *(unsigned int*)&fres[2]);
    
    /* Test LTGT - _CMP_NEQ_OQ (not equal, ordered quiet) */
    cmp_res = _mm_cmp_pd(v1, v2, _CMP_NEQ_OQ);
    _mm_storeu_pd(dres, cmp_res);
    mix(&checksum, *(unsigned long*)&dres[0] ^ *(unsigned long*)&dres[1]);
}

/* Test inline assembly with condition codes */
__attribute__((optimize("O1")))
void test_inline_asm_conditions(double a, double b) {
    volatile double result;
    volatile int flags;
    
    /* Test UNORDERED (unord) */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flags)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    mix(&checksum, flags);
    
    /* Test ORDERED (ord) */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (flags)
        : "x" (a), "x" (b)
        : "al", "cc"
    );
    mix(&checksum, flags);
    
    /* Test using condition codes in cmov */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "mov $1.0, %%xmm0\n\t"
        "mov $2.0, %%xmm1\n\t"
        "cmovnbe %3, %0\n\t"  /* Using condition code implicitly */
        : "=r" (flags)
        : "x" (a), "x" (b), "r" (flags)
        : "xmm0", "xmm1", "cc"
    );
    mix(&checksum, flags);
    
    /* Test with explicit template substitution - AT&T syntax */
    result = 0.0;
    __asm__ volatile (
        "cmppd %3, %2, %{%1|unord}\n\t"
        "movapd %2, %0"
        : "=x" (result)
        : "i" (0), "x" (*(__m128d*)&a), "x" (*(__m128d*)&b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
    
    /* Test ueq condition code */
    __asm__ volatile (
        "cmppd %3, %2, %{%1|ueq}\n\t"
        "movapd %2, %0"
        : "=x" (result)
        : "i" (8), "x" (*(__m128d*)&a), "x" (*(__m128d*)&b)
        : "cc"
    );
    mix(&checksum, *(unsigned long*)&result);
}

/* Test with mixed types and control flow */
__attribute__((noinline, optimize("O2")))
void test_mixed_conditions(volatile double* darr, volatile float* farr, int n) {
    int i;
    volatile int branch_taken = 0;
    
    for (i = 0; i < n; i++) {
        double d1 = darr[i];
        double d2 = darr[(i + 1) % n];
        float f1 = farr[i];
        float f2 = farr[(i + 1) % n];
        
        /* Complex branching to force condition code generation */
        if (isunordered(d1, d2)) {
            branch_taken |= 1;
        } else if (!isless(f1, f2) && isunordered(f1, f2)) {
            branch_taken |= 2;
        } else if ((d1 <= d2) || isunordered(d1, d2)) {
            branch_taken |= 4;
        } else if (!(f1 > f2) && !isunordered(f1, f2)) {
            branch_taken |= 8;
        }
        
        /* Ternary with uncommon conditions */
        int res = (isunordered(d1, d2) ? 1 : 
                  (!isless(d1, d2) && !isgreater(d1, d2)) ? 2 : 3);
        mix(&checksum, res);
    }
    
    mix(&checksum, branch_taken);
}

int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values */
    unsigned long seed = 0x12345678;
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    
    srand(seed);
    
    /* Create arrays with mixed values including NaN, infinity, normal numbers */
    double darr[8];
    float farr[8];
    
    /* Fill with varied floating-point values */
    for (int i = 0; i < 8; i++) {
        switch (i % 4) {
            case 0: darr[i] = (double)rand() / RAND_MAX; break;
            case 1: darr[i] = -(double)rand() / RAND_MAX; break;
            case 2: darr[i] = 0.0 / 0.0; /* NaN */ break;
            case 3: darr[i] = 1.0 / 0.0; /* Inf */ break;
        }
        farr[i] = (float)darr[i];
    }
    
    /* Call test functions with different value combinations */
    for (int i = 0; i < 4; i++) {
        test_scalar_conditions_O0(darr[i], darr[i+1], farr[i], farr[i+1]);
        test_scalar_conditions_O2(darr[i+2], darr[i+3], farr[i+2], farr[i+3]);
        
        __m128d v1 = _mm_set_pd(darr[i], darr[i+1]);
        __m128d v2 = _mm_set_pd(darr[i+2], darr[i+3]);
        __m128 f1 = _mm_set_ps(farr[i], farr[i+1], farr[i+2], farr[i+3]);
        __m128 f2 = _mm_set_ps(farr[i+3], farr[i+2], farr[i+1], farr[i]);
        
        test_vector_conditions(v1, v2, f1, f2);
        test_inline_asm_conditions(darr[i], darr[(i+1)%8]);
    }
    
    test_mixed_conditions(darr, farr, 8);
    
    printf("Final checksum: 0x%016lx\n", checksum);
    return 0;
}
