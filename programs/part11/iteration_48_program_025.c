/* fixed-point-coverage.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * fixed-value.cc (lines 264-277) by exercising fixed-point arithmetic
 * range analysis with saturation contexts, comparisons, and loops.
 */

#include <stdio.h>

/* Global side effect to prevent dead code elimination */
volatile int global_side_effect = 0;

/* Helper function 1: Focus on signed _Fract operations with comparisons */
__attribute__((noinline, noipa))
int test_signed_fract_range(volatile int seed) {
    /* Use different fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;        /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;     /* Binary: -0.001 exactly */
    uf1 = 0.5ur;       /* Binary: 0.1 exactly */
    
    /* Introduce uncertainty through volatile seed */
    sf3 = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;   /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf3;   /* Squaring could saturate */
    
    /* CRITICAL COMPARISONS - targeting a_high.sgt(max_r) logic */
    if (sf1 > 0.0r) {
        global_side_effect += 1;
    }
    
    if (sf3 == -0.125r) {
        global_side_effect += 2;
    }
    
    /* Comparison after cast */
    if ((signed _Fract)sf3 < sf2) {
        global_side_effect += 4;
    }
    
    /* Additional comparison with constant */
    if (uf1 > 0.25ur) {
        global_side_effect += 8;
    }
    
    return global_side_effect;
}

/* Helper function 2: Focus on _Accum types with saturation */
__attribute__((noinline, noipa))
int test_accum_saturation(volatile int iter) {
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants */
    sa1 = 0.5k;        /* Binary: 0.1... */
    sa2 = -0.25k;      /* Binary: -0.01... */
    
    /* Use iteration to create varying values */
    ua1 = (unsigned _Accum)(iter) * 0.1uk;
    
    /* Arithmetic that could overflow */
    sa1 = sa1 + sa2;   /* 0.5 - 0.25 = 0.25 */
    
    /* Multiplication that could saturate */
    for (int i = 0; i < 3; i++) {
        sa1 = sa1 * sa1;
    }
    
    /* CRITICAL COMPARISONS - targeting the uncovered condition */
    if (sa1 > 0.0k) {
        global_side_effect += 16;
    }
    
    if (ua1 == 0.0uk) {
        global_side_effect += 32;
    }
    
    /* Complex comparison chain */
    if (sa1 > 0.1k || (sa1 == 0.1k && ua1 > 0.5uk)) {
        global_side_effect += 64;
    }
    
    /* Cast and compare */
    signed short _Fract temp = (signed short _Fract)sa1;
    if (temp < 0.0r) {
        global_side_effect += 128;
    }
    
    return global_side_effect;
}

/* Helper function 3: Mixed types and explicit saturation contexts */
__attribute__((noinline, noipa))
int test_mixed_saturation(volatile int seed) {
    signed long _Fract slf1, slf2;
    signed _Accum sa1;
    
    /* Initialize with volatile-dependent values */
    slf1 = (signed long _Fract)(seed & 0xFF) / 256.0lr;
    slf2 = -0.0625lr;  /* Binary: -0.0001 exactly */
    
    /* Arithmetic with potential saturation */
    slf1 = slf1 + slf2;
    slf1 = slf1 * 2.0lr;  /* Could overflow */
    
    /* Convert between types - may trigger saturation logic */
    sa1 = (signed _Accum)slf1;
    
    /* CRITICAL COMPARISONS */
    if (slf1 > 0.5lr) {
        global_side_effect += 256;
    }
    
    if (sa1 < -0.5k) {
        global_side_effect += 512;
    }
    
    /* Comparison with zero */
    if (slf1 == 0.0lr) {
        global_side_effect += 1024;
    }
    
    return global_side_effect;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int result = 0;
    
    printf("Starting fixed-point coverage test...\n");
    
    /* Loop to force dynamic range analysis */
    for (volatile int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions to exercise different paths */
        result += test_signed_fract_range(seed);
        result += test_accum_saturation(i);
        result += test_mixed_saturation(seed);
        
        /* Prevent loop unrolling */
        asm volatile("" : : "r"(result));
    }
    
    printf("Final result: %d\n", result);
    printf("Global side effect: %d\n", global_side_effect);
    
    return result != 0 ? 0 : 1;
}
