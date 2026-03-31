/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper functions marked to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int test_saturating_accum() {
    volatile int seed = 7; /* Prevent constant folding */
    
    /* Use various fixed-point types */
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1;
    signed short _Fract sf1, sf2;
    unsigned short _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sa1 = 0.1rk;      /* Will have precise fractional representation */
    sa2 = -0.125rk;   /* Power of two fraction */
    sf1 = 0.5r;       /* Common boundary value */
    sf2 = -0.75r;
    uf1 = 0.333r;     /* Non-power-of-two fraction */
    
    /* Introduce uncertainty through volatile read */
    ua1 = (unsigned _Accum)seed * 0.142857rk; /* 1/7 approximation */
    
    /* Critical arithmetic that may overflow/saturate */
    sa3 = sa1 + sa2;  /* 0.1 + (-0.125) = -0.025 */
    
    /* Multiplication that could overflow */
    sa1 = sa1 * sa3;  /* 0.1 * (-0.025) = -0.0025 */
    
    /* Cast between types with different scaling - may trigger saturation logic */
    sf1 = (signed short _Fract)sa3;
    
    /* ===== CRITICAL COMPARISONS ===== */
    /* These should trigger the range analysis with high/low part comparisons */
    
    /* Comparison 1: Variable against constant (may trigger a_high.sgt(max_r)) */
    if (sa3 > 0.0rk) {
        global_result += 1;
    }
    
    /* Comparison 2: Variable against another variable */
    if (sa1 == sa2) {
        global_result += 2;
    }
    
    /* Comparison 3: After type conversion */
    if ((signed _Accum)sf2 < sa3) {
        global_result += 4;
    }
    
    /* Comparison 4: Against boundary values */
    if (ua1 > 0.5uk) {
        global_result += 8;
    }
    
    /* Comparison 5: Complex expression */
    if (sf1 > 0.25r && sa3 < -0.01rk) {
        global_result += 16;
    }
    
    /* Additional saturation-prone operations */
    sa2 = sa2 * 10.0rk;  /* -0.125 * 10 = -1.25, may saturate depending on type */
    ua1 = ua1 + ua1;     /* Could overflow for unsigned accum */
    
    /* More comparisons with the modified values */
    if (sa2 < -1.0rk) {
        global_result += 32;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int test_fract_range() {
    volatile int counter = 3;
    
    signed _Fract f1, f2, f3;
    unsigned _Fract uf;
    
    /* Initialize with volatile-dependent values */
    f1 = (signed _Fract)(counter) * 0.25r;
    f2 = -0.8r;
    uf = 0.9ur;
    
    /* Operations that may saturate */
    f3 = f1 + f2;  /* Could go below -1.0 */
    
    /* Critical comparisons */
    if (f3 > 0.0r) {
        global_result += 64;
    }
    
    if (f1 == 0.75r) {
        global_result += 128;
    }
    
    /* Cast to different fixed-point type */
    signed _Accum sa_from_fract = (signed _Accum)f3;
    if (sa_from_fract < -0.5rk) {
        global_result += 256;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int test_mixed_width() {
    /* Test different bit-width fixed-point types */
    signed long _Fract lf1 = 0.999999999999999999lr;  /* Near upper bound */
    signed _Fract f1 = 0.5r;
    signed short _Accum sa1 = -0.9rk;
    
    /* Operations that stress range analysis */
    signed _Fract f_result = (signed _Fract)lf1 + f1;  /* Likely saturates */
    
    /* Comparisons that should trigger the uncovered condition */
    if (f_result > 0.99r) {
        global_result += 512;
    }
    
    if (sa1 < -0.8rk) {
        global_result += 1024;
    }
    
    /* Explicit cast that may require saturation */
    signed short _Fract sf_from_acc = (signed short _Fract)sa1;
    if (sf_from_acc == -0.9r) {
        global_result += 2048;
    }
    
    return global_result;
}

int main() {
    volatile int iterations = 100;
    int i;
    
    printf("Starting fixed-point range analysis test...\n");
    
    /* Loop to force dynamic analysis */
    for (i = 0; i < iterations; i++) {
        /* Vary the operations slightly each iteration */
        if (i % 3 == 0) {
            test_saturating_accum();
        } else if (i % 3 == 1) {
            test_fract_range();
        } else {
            test_mixed_width();
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed. Check if fixed-value.cc lines 264-277 were executed.\n");
    
    return global_result != 0 ? 0 : 1;
}
