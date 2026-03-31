/* fixed-value-coverage.c
 * 
 * This program is designed to trigger the specific uncovered lines in
 * GCC's fixed-value.cc (lines 264-277) by exercising fixed-point
 * arithmetic range analysis with saturation and comparisons.
 *
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math fixed-value-coverage.c -o fixed-value-coverage
 */

#include <stdio.h>

/* Global side effect to prevent dead code elimination */
volatile int global_side_effect = 0;

/* Helper functions marked to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int fixed_point_comparisons_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2;
    unsigned short _Fract uf1;
    signed _Accum sa1;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;     /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;  /* Binary: -0.001 exactly */
    uf1 = 0.5r;     /* Binary: 0.1 exactly */
    
    /* Initialize with volatile-dependent values to prevent constant folding */
    sa1 = (signed _Accum)(seed) * 0.5rk;  /* Multiplication that may saturate */
    ua1 = (unsigned _Accum)(seed % 256) * 0.25rk;
    
    /* Perform arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;  /* 0.1 + (-0.125) = -0.025 */
    uf1 = uf1 * uf1;  /* 0.5 * 0.5 = 0.25 */
    
    /* Critical comparisons that should trigger range analysis */
    /* These comparisons involve high/low part splitting in fixed-value.cc */
    if (sf1 > 0.5r) {  /* Comparison with constant */
        global_side_effect += 1;
    }
    
    if (ua1 == 0.0rk) {  /* Equality comparison */
        global_side_effect += 2;
    }
    
    /* More complex comparison after cast */
    if ((signed _Fract)sa1 < sf2) {
        global_side_effect += 4;
    }
    
    /* Saturation-prone operation */
    sa1 = sa1 * sa1;  /* Could overflow for larger seed values */
    
    /* Another comparison that may trigger the uncovered condition */
    if (sa1 > 1.0rk) {
        global_side_effect += 8;
    }
    
    return global_side_effect;
}

__attribute__((noinline, noipa))
int fixed_point_comparisons_2(volatile int seed) {
    /* Different set of fixed-point types */
    long _Fract lf1, lf2;
    long _Accum la1;
    
    /* Initialize with values that may approach saturation bounds */
    lf1 = 0.9999999r;  /* Close to maximum */
    lf2 = -0.9999999r; /* Close to minimum */
    
    /* Volatile-dependent initialization */
    la1 = (long _Accum)(seed) * 0.1rk;
    
    /* Operations that may saturate */
    lf1 = lf1 + 0.0000001r;  /* Could saturate to 1.0 */
    lf2 = lf2 - 0.0000001r;  /* Could saturate to -1.0 */
    
    /* Comparisons that should trigger the specific uncovered condition */
    /* The condition a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) */
    if (lf1 > 0.5r) {
        global_side_effect += 16;
    }
    
    if (lf2 < -0.5r) {
        global_side_effect += 32;
    }
    
    /* Cast with potential saturation */
    if ((signed short _Fract)la1 == 0.0r) {
        global_side_effect += 64;
    }
    
    /* Multiplication that could overflow */
    la1 = la1 * la1;
    
    /* Final comparison */
    if (la1 > 0.25rk) {
        global_side_effect += 128;
    }
    
    return global_side_effect;
}

__attribute__((noinline, noipa))
int fixed_point_saturation_test(volatile int seed) {
    /* Test explicit saturation contexts */
    unsigned _Accum ua1, ua2;
    signed _Accum sa1, sa2;
    
    /* Initialize near bounds */
    ua1 = 0.999999999999999rk;  /* Very close to 1.0 */
    sa1 = 0.999999999999999rk;
    
    /* Operations that should trigger saturation logic */
    ua2 = ua1 + (unsigned _Accum)(seed) * 0.0000000001rk;
    sa2 = sa1 + (signed _Accum)(seed) * 0.0000000001rk;
    
    /* Comparisons that exercise the range analysis */
    if (ua2 > 0.5rk) {
        global_side_effect += 256;
    }
    
    if (sa2 < -0.5rk) {
        global_side_effect += 512;
    }
    
    /* Equality comparison with saturated value */
    if (ua2 == 1.0rk) {
        global_side_effect += 1024;
    }
    
    return global_side_effect;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int result = 0;
    
    printf("Starting fixed-point coverage test...\n");
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all fixed-point functions with varying seeds */
        result += fixed_point_comparisons_1(seed % 100);
        result += fixed_point_comparisons_2(seed % 50);
        result += fixed_point_saturation_test(seed % 10);
        
        /* Modify seed to create different value patterns */
        if (i % 10 == 0) {
            seed = -seed;
        }
    }
    
    printf("Final result: %d (global side effect: %d)\n", 
           result, global_side_effect);
    
    return result != 0 ? 0 : 1;
}
