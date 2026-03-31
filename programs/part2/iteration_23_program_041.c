/* Test program for x86 condition code mnemonics coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

/* Global volatile checksum to prevent optimization */
volatile unsigned long g_checksum = 0;

/* Function to mix bits into checksum */
static inline void mix(unsigned long *sum, unsigned long val) {
    *sum ^= val;
    *sum = (*sum << 13) | (*sum >> (64 - 13));
    *sum *= 0x9e3779b97f4a7c15UL;
}

/* Test scalar floating-point conditions with different optimizations */
__attribute__((optimize("O0"), noinline))
void test_scalar_conditions_O0(double a, double b, float fa, float fb) {
    volatile int result;
    
    /* Test UNORDERED (unord) */
    result = isunordered(a, b);
    mix(&g_checksum, result);
    
    /* Test ORDERED (ord) */
    result = !isunordered(a, b);
    mix(&g_checksum, result);
    
    /* Complex branching to force condition code generation */
    volatile double v = a;
    if (isunordered(v, b)) {
        mix(&g_checksum, 1);
    } else {
        mix(&g_checksum, 2);
    }
    
    /* Test UNEQ (ueq) - unordered or equal */
    if (!(fa < fb) && !(fa > fb)) {  /* Either unordered or equal */
        mix(&g_checksum, 3);
    }
    
    /* Test UNGE (nlt) - unordered or not less than */
    if (!(fa < fb)) {
        mix(&g_checksum, 4);
    }
    
    /* Test UNGT (nle) - unordered or not less than or equal */
    if (!(fa <= fb)) {
        mix(&g_checksum, 5);
    }
    
    /* Test UNLE (ule) - unordered or less than or equal */
    if (fa <= fb || isunordered(fa, fb)) {
        mix(&g_checksum, 6);
    }
    
    /* Test UNLT (ult) - unordered or less than */
    if (fa < fb || isunordered(fa, fb)) {
        mix(&g_checksum, 7);
    }
    
    /* Test LTGT (une) - less than or greater than (ordered and not equal) */
    if ((fa < fb) || (fa > fb)) {
        mix(&g_checksum, 8);
    }
}

__attribute__((optimize("O2"), target("sse2"), noinline))
void test_scalar_conditions_O2(double a, double b, float fa, float fb) {
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Force generation of all condition codes through ternary operations */
    r1 = isunordered(a, b) ? 1001 : 2001;  /* UNORDERED */
    r2 = !isunordered(a, b) ? 1002 : 2002; /* ORDERED */
    r3 = (!(fa < fb) && !(fa > fb)) ? 1003 : 2003; /* UNEQ */
    r4 = (!(fa < fb)) ? 1004 : 2004; /* UNGE */
    r5 = (!(fa <= fb)) ? 1005 : 2005; /* UNGT */
    r6 = (fa <= fb || isunordered(fa, fb)) ? 1006 : 2006; /* UNLE */
    r7 = (fa < fb || isunordered(fa, fb)) ? 1007 : 2007; /* UNLT */
    r8 = ((fa < fb) || (fa > fb)) ? 1008 : 2008; /* LTGT */
    
    mix(&g_checksum, r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8);
}

/* Test vector conditions using SSE intrinsics */
__attribute__((optimize("O3"), target("sse2"), noinline))
void test_vector_conditions(__m128d a, __m128d b, __m128 fa, __m128 fb) {
    __m128d cmp_result;
    __m128 cmp_result_f;
    volatile double vd[2];
    volatile float vf[4];
    
    /* Test UNORDERED - _CMP_UNORD_Q */
    cmp_result = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
    _mm_storeu_pd(vd, cmp_result);
    mix(&g_checksum, (unsigned long)vd[0]);
    
    /* Test ORDERED - _CMP_ORD_Q */
    cmp_result = _mm_cmp_pd(a, b, _CMP_ORD_Q);
    _mm_storeu_pd(vd, cmp_result);
    mix(&g_checksum, (unsigned long)vd[1]);
    
    /* Test UNEQ - _CMP_EQ_UQ */
    cmp_result = _mm_cmp_pd(a, b, _CMP_EQ_UQ);
    _mm_storeu_pd(vd, cmp_result);
    mix(&g_checksum, (unsigned long)(vd[0] + vd[1]));
    
    /* Test UNGE - _CMP_NLT_UQ */
    cmp_result_f = _mm_cmp_ps(fa, fb, _CMP_NLT_UQ);
    _mm_storeu_ps(vf, cmp_result_f);
    mix(&g_checksum, (unsigned long)vf[0]);
    
    /* Test UNGT - _CMP_NLE_UQ */
    cmp_result_f = _mm_cmp_ps(fa, fb, _CMP_NLE_UQ);
    _mm_storeu_ps(vf, cmp_result_f);
    mix(&g_checksum, (unsigned long)vf[1]);
    
    /* Test UNLE - _CMP_LE_OS */
    cmp_result = _mm_cmp_pd(a, b, _CMP_LE_OS);
    _mm_storeu_pd(vd, cmp_result);
    mix(&g_checksum, (unsigned long)vd[0]);
    
    /* Test UNLT - _CMP_LT_OS */
    cmp_result = _mm_cmp_pd(a, b, _CMP_LT_OS);
    _mm_storeu_pd(vd, cmp_result);
    mix(&g_checksum, (unsigned long)vd[1]);
    
    /* Test LTGT - _CMP_NEQ_OS */
    cmp_result_f = _mm_cmp_ps(fa, fb, _CMP_NEQ_OS);
    _mm_storeu_ps(vf, cmp_result_f);
    mix(&g_checksum, (unsigned long)(vf[2] + vf[3]));
}

/* Test inline assembly with explicit condition code mnemonics */
__attribute__((optimize("O1"), target("sse2"), noinline))
void test_inline_asm_conditions(double a, double b) {
    double result1, result2;
    __m128d va = _mm_set1_pd(a);
    __m128d vb = _mm_set1_pd(b);
    __m128d vresult;
    
    /* Test unord */
    __asm__ volatile (
        "cmppd %[unord], %[a], %[b]\n\t"
        "movapd %[a], %[result]"
        : [result] "=x" (vresult)
        : [a] "x" (va), [b] "x" (vb), [unord] "i" (_CMP_UNORD_Q)
        : "cc"
    );
    _mm_store_sd(&result1, vresult);
    mix(&g_checksum, (unsigned long)result1);
    
    /* Test ord */
    __asm__ volatile (
        "cmppd %[ord], %[a], %[b]\n\t"
        "movapd %[a], %[result]"
        : [result] "=x" (vresult)
        : [a] "x" (va), [b] "x" (vb), [ord] "i" (_CMP_ORD_Q)
        : "cc"
    );
    _mm_store_sd(&result2, vresult);
    mix(&g_checksum, (unsigned long)result2);
    
    /* Test ueq with AT&T/Intel syntax variation */
    unsigned long r3;
    __asm__ volatile (
        "%{%[ueq]|ueq}"
        : "=@ccae" (r3)
        : [ueq] "i" (0)
        : "cc"
    );
    mix(&g_checksum, r3);
    
    /* Test nlt */
    unsigned long r4;
    __asm__ volatile (
        "%{%[nlt]|nlt}"
        : "=@ccae" (r4)
        : [nlt] "i" (0)
        : "cc"
    );
    mix(&g_checksum, r4);
    
    /* Test nle */
    unsigned long r5;
    __asm__ volatile (
        "%{%[nle]|nle}"
        : "=@ccae" (r5)
        : [nle] "i" (0)
        : "cc"
    );
    mix(&g_checksum, r5);
    
    /* Test ule */
    unsigned long r6;
    __asm__ volatile (
        "%{%[ule]|ule}"
        : "=@ccae" (r6)
        : [ule] "i" (0)
        : "cc"
    );
    mix(&g_checksum, r6);
    
    /* Test ult */
    unsigned long r7;
    __asm__ volatile (
        "%{%[ult]|ult}"
        : "=@ccae" (r7)
        : [ult] "i" (0)
        : "cc"
    );
    mix(&g_checksum, r7);
    
    /* Test une */
    unsigned long r8;
    __asm__ volatile (
        "%{%[une]|une}"
        : "=@ccae" (r8)
        : [une] "i" (0)
        : "cc"
    );
    mix(&g_checksum, r8);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize with non-uniform values including NaN, infinity, normal numbers */
    unsigned long seed = (argc > 1) ? strtoul(argv[1], NULL, 0) : 0x12345678;
    srand(seed);
    
    /* Create test data with various floating-point values */
    double dvals[] = {
        1.0, -1.0, 0.0, -0.0,
        __builtin_nan(""), __builtin_inf(), -__builtin_inf(),
        3.14159, -2.71828, 1.0e-30, 1.0e30
    };
    
    float fvals[] = {
        1.0f, -1.0f, 0.0f, -0.0f,
        __builtin_nanf(""), __builtin_inff(), -__builtin_inff(),
        2.5f, -3.7f, 1.0e-20f, 1.0e20f
    };
    
    /* Run tests with different combinations */
    for (int i = 0; i < 8; i++) {
        double a = dvals[i % 11];
        double b = dvals[(i + 3) % 11];
        float fa = fvals[i % 11];
        float fb = fvals[(i + 5) % 11];
        
        __m128d va = _mm_set_pd(a, b);
        __m128d vb = _mm_set_pd(b, a);
        __m128 vfa = _mm_set_ps(fa, fb, fa, fb);
        __m128 vfb = _mm_set_ps(fb, fa, fb, fa);
        
        test_scalar_conditions_O0(a, b, fa, fb);
        test_scalar_conditions_O2(a, b, fa, fb);
        test_vector_conditions(va, vb, vfa, vfb);
        test_inline_asm_conditions(a, b);
    }
    
    printf("Final checksum: %lu\n", g_checksum);
    return (int)(g_checksum & 0x7FFFFFFF);
}
