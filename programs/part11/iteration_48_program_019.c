/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 1;
volatile int iterations = 100;

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
signed int test_signed_fract_range(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Non-trivial binary fraction */
    sf2 = -0.125r;   /* Exact power of two */
    uf1 = 0.5ur;     /* Mid-range value */
    
    /* Introduce uncertainty through volatile seed */
    sf3 = (signed short _Fract)(seed * 0.01r);
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;          /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * 0.9r;         /* Potential saturation */
    
    /* CRITICAL: Comparisons that should trigger range analysis */
    /* These should generate the a_high.sgt(max_r) logic */
    if (sf1 > 0.5r) {
        /* This branch unlikely but forces comparison */
        return 1;
    }
    
    if (sf2 == -0.125r) {
        /* Exact comparison */
        sf3 = sf3 + 0.1r;
    }
    
    /* Another comparison with different constant */
    if (sf3 < -0.8r) {
        return 2;
    }
    
    /* Complex comparison chain */
    signed short _Fract temp = sf1 + sf3;
    if (temp > 0.75r || temp < -0.75r) {
        return 3;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
signed int test_accum_saturation(volatile int seed) {
    /* Test _Accum types which have more bits */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with mixed constants */
    sa1 = 0.1k;           /* Non-trivial _Accum constant */
    sa2 = -0.5k;          /* Negative mid-range */
    
    /* Use volatile seed to prevent constant propagation */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.01uk;
    
    /* Operations that could overflow */
    sa1 = sa1 * 10.0k;    /* Could overflow for some seeds */
    sa2 = sa2 - 0.3k;
    
    /* CRITICAL: More comparisons for range analysis */
    /* Should trigger the a_high == max_r && a_low.ugt(max_s) path */
    if (sa1 > 0.8k) {
        ua1 = ua1 / 2.0uk;
    }
    
    if (ua1 == 0.0uk) {
        sa1 = sa1 + 0.1k;
    }
    
    /* Cross-type comparison after cast */
    if ((signed short _Fract)sa2 < -0.25r) {
        return 4;
    }
    
    /* Multiplication that could saturate */
    signed _Accum product = sa1 * sa2;
    if (product > 1.0k || product < -1.0k) {
        return 5;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
signed int test_mixed_types_loop(volatile int counter) {
    /* Mix different fixed-point types in loop context */
    long _Fract lf1, lf2;
    signed _Accum sa;
    unsigned short _Fract usf;
    
    /* Initialize with values that depend on counter */
    lf1 = (long _Fract)(counter % 100) * 0.01lr;
    lf2 = -0.99lr;
    sa = 0.0k;
    usf = 0.5ur;
    
    /* Loop-like behavior with counter */
    for (int i = 0; i < (counter & 3); i++) {
        /* Operations that change ranges */
        lf1 = lf1 + 0.25lr;
        lf2 = lf2 * 0.9lr;
        sa = sa + (signed _Accum)lf1;
        
        /* CRITICAL: Comparisons inside "loop" */
        if (lf1 > 0.5lr) {
            usf = usf - 0.1ur;
        }
        
        if (lf2 < -0.5lr && sa > 0.0k) {
            return 6 + i;
        }
    }
    
    /* Final comparison that might trigger the uncovered code */
    if (sa > 1.0k || (sa == 1.0k && usf > 0.0ur)) {
        return 7;
    }
    
    return 0;
}

int main() {
    volatile int seed = 1;
    int result = 0;
    
    /* Main loop to execute fixed-point operations multiple times */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7FFF;
        
        /* Call all test functions */
        result ^= test_signed_fract_range(seed);
        result ^= test_accum_saturation(seed);
        result ^= test_mixed_types_loop(seed);
        
        /* Modify control to affect flow */
        control = (control + seed) % 10;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0;
}
