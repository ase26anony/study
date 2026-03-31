/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Lines 264-277: range analysis for fixed-point arithmetic with saturation
 */

#include <stdio.h>

/* Prevent constant folding and inlining */
#define NOOPT __attribute__((noinline, noipa))

/* Global to prevent elimination */
volatile int g_result = 0;

/* Function 1: Tests signed _Fract operations with saturation */
NOOPT int test_signed_fract_range(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: 0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    uf1 = 0.5r;      /* Binary: 0.1 exactly */
    
    /* Introduce uncertainty through volatile seed */
    sf3 = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;          /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf3;          /* Square - could approach limits */
    
    /* CRITICAL: Comparisons that should trigger range analysis */
    if (sf1 > 0.5r) {
        g_result += 1;
    }
    
    if (sf3 == 0.0r) {
        g_result += 2;
    }
    
    /* Cast between types with different scaling */
    signed _Fract f1 = (signed _Fract)sf3;
    if (f1 < sf2) {
        g_result += 4;
    }
    
    /* More comparisons with constants */
    if (uf1 > 0.75r) {
        g_result += 8;
    }
    
    return g_result;
}

/* Function 2: Tests _Accum types with saturation */
NOOPT int test_accum_saturation(volatile int iter) {
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1;
    
    /* Initialize with mixed constants */
    sa1 = 0.1k;           /* _Accum constant */
    sa2 = -0.5k;
    ua1 = 0.25uk;         /* unsigned _Accum constant */
    
    /* Use iteration to create varying values */
    sa3 = (signed _Accum)iter * 0.01k;
    
    /* Operations that could saturate */
    sa1 = sa1 + sa2;      /* -0.4 */
    sa3 = sa3 * sa3;      /* Square - could overflow */
    
    /* CRITICAL: More comparisons for range analysis */
    if (sa1 > 0.0k) {
        g_result += 16;
    }
    
    if (sa3 == 0.0k) {
        g_result += 32;
    }
    
    /* Cross-type comparison */
    if ((signed short _Fract)sa1 < 0.0r) {
        g_result += 64;
    }
    
    /* Test near boundary values */
    signed _Accum boundary = 0.999999999k;  /* Near maximum */
    if (ua1 > boundary) {
        g_result += 128;
    }
    
    return g_result;
}

/* Function 3: Mixed types with explicit casts causing range analysis */
NOOPT int test_mixed_casts(volatile int seed) {
    long _Fract lf1, lf2;
    signed _Accum sa1;
    
    /* Initialize with volatile-dependent values */
    lf1 = (long _Fract)(seed & 0xFF) / 256.0r;
    lf2 = -0.333r;        /* Non-power-of-two fraction */
    sa1 = 100.0k;         /* Large value for _Accum */
    
    /* Cast to different precision - may trigger saturation logic */
    signed short _Fract sf_cast = (signed short _Fract)sa1;
    
    /* Arithmetic with potential overflow */
    lf1 = lf1 * lf2;
    
    /* CRITICAL: Complex comparison chain */
    if (lf1 > 0.5r || (lf1 == 0.5r && lf2 < -0.25r)) {
        g_result += 256;
    }
    
    /* Comparison after cast */
    if (sf_cast > 0.1r) {
        g_result += 512;
    }
    
    return g_result;
}

int main(void) {
    volatile int iterations = 100;
    volatile int seed = 1;
    int final_result = 0;
    
    /* Loop to force dynamic analysis */
    for (volatile int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions */
        final_result += test_signed_fract_range(seed % 100);
        final_result += test_accum_saturation(i);
        final_result += test_mixed_casts(seed);
        
        /* Prevent loop unrolling */
        asm volatile("" : : "r"(final_result));
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
