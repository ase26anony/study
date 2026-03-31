/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 1;
volatile int iterations = 100;

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
signed short _Fract fixed_compare_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1 = 0.1r;      /* Non-trivial binary representation */
    signed short _Fract sf2 = -0.125r;   /* Exact power of two */
    unsigned short _Fract uf1;
    
    /* Introduce uncertainty through volatile */
    uf1 = (unsigned short _Fract)(seed * 0.01r);
    
    /* Arithmetic that could saturate */
    sf1 = sf1 + sf2;                     /* 0.1 - 0.125 = -0.025 */
    
    /* Critical comparisons */
    if (sf1 > 0.5r) {                    /* Comparison with constant */
        /* This path unlikely but forces range analysis */
        sf1 = 0.9r;
    }
    
    if (uf1 == 0.0r) {                   /* Equality comparison */
        sf1 = sf1 * 0.5r;
    }
    
    /* Additional comparison to trigger the specific condition */
    if (sf1 < -0.5r) {
        sf1 = -0.5r;
    }
    
    return sf1;
}

__attribute__((noinline, noipa))
signed _Accum fixed_compare_2(volatile int seed) {
    signed _Accum sa1 = 0.5rk;
    signed _Accum sa2 = -0.125rk;
    unsigned _Accum ua1;
    
    /* Volatile-based initialization prevents constant propagation */
    ua1 = (unsigned _Accum)(seed) * 0.01rk;
    
    /* Multiplication that could overflow */
    sa1 = sa1 * sa2;                     /* 0.5 * -0.125 = -0.0625 */
    
    /* More comparisons */
    if (sa1 > 0.0rk) {
        sa1 = sa1 + 0.1rk;
    }
    
    /* Cast and compare - may trigger range analysis */
    if ((signed short _Fract)sa1 < 0.0r) {
        sa1 = sa1 - 0.05rk;
    }
    
    /* Explicit comparison that could hit the uncovered condition */
    if (ua1 == 0.0rk || ua1 > 1.0rk) {
        ua1 = 0.5rk;
    }
    
    return (signed _Accum)ua1 + sa1;
}

__attribute__((noinline, noipa))
long _Fract fixed_compare_3(volatile int seed) {
    long _Fract lf1 = 0.333333333333333333rL;  /* Complex binary representation */
    long _Fract lf2 = -0.666666666666666666rL;
    
    /* Loop-like behavior with volatile */
    lf1 = lf1 + (long _Fract)(seed * 0.0001rL);
    
    /* Operations that require saturation analysis */
    lf2 = lf2 * lf1;
    
    /* Multiple comparisons */
    if (lf1 > 0.5rL && lf2 < -0.5rL) {
        lf1 = lf1 - 0.25rL;
    }
    
    /* Boundary comparisons */
    if (lf1 == 0.0rL || lf1 == 1.0rL || lf1 == -1.0rL) {
        lf1 = 0.75rL;
    }
    
    return lf1 + lf2;
}

int main() {
    volatile int seed = 1;
    signed short _Fract result1 = 0.0r;
    signed _Accum result2 = 0.0rk;
    long _Fract result3 = 0.0rL;
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call all comparison functions */
        result1 = result1 + fixed_compare_1(seed % 100);
        result2 = result2 + fixed_compare_2(seed % 200);
        result3 = result3 + fixed_compare_3(seed % 300);
        
        /* Modify control to prevent optimization */
        control = (control + i) & 1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %f %f %Lf\n", 
           (double)result1, 
           (double)result2, 
           (double)result3);
    
    return (result1 > 0.0r) ? 0 : 1;
}
