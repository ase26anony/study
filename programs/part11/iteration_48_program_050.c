/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 1;
volatile int iterations = 100;

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
signed short _Fract fixed_comparison_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1 = 0.1r;
    signed short _Fract sf2 = -0.125r;
    unsigned short _Fract uf1 = 0.5ur;
    
    /* Introduce uncertainty through volatile seed */
    sf1 = sf1 + (signed short _Fract)(seed * 0.01r);
    sf2 = sf2 - (signed short _Fract)(seed * 0.001r);
    
    /* Critical comparisons that may trigger range analysis */
    if (sf1 > 0.5r) {
        /* Potential overflow in addition */
        sf1 = sf1 + 0.9r;
    }
    
    if (sf2 == -0.125r) {
        sf2 = sf2 * 2.0r;  /* Could saturate */
    }
    
    /* Comparison with different representation */
    if (uf1 < 0.75ur) {
        uf1 = uf1 + 0.3ur;
    }
    
    /* Return value prevents dead code elimination */
    return sf1 + (signed short _Fract)uf1 + sf2;
}

__attribute__((noinline, noipa))
signed _Accum fixed_comparison_2(volatile int seed) {
    signed _Accum sa1 = 0.5rk;
    signed _Accum sa2 = -0.125rk;
    unsigned _Accum ua1 = 0.1urk;
    
    /* Operations that could overflow/saturate */
    sa1 = sa1 * (signed _Accum)(seed * 0.1rk);
    sa2 = sa2 - 0.9rk;
    
    /* Critical comparisons for double-int range analysis */
    if (sa1 > 0.75rk) {
        sa1 = sa1 * 1.5rk;  /* Potential overflow */
    }
    
    if (sa2 == -1.0rk) {
        sa2 = sa2 / 2.0rk;
    }
    
    /* Cross-type comparison */
    if ((signed short _Fract)ua1 < 0.05r) {
        ua1 = ua1 + 0.2urk;
    }
    
    /* Complex comparison chain */
    if (sa1 > 0.0rk && sa2 < 0.0rk) {
        sa1 = sa1 + (signed _Accum)ua1;
    }
    
    return sa1 + (signed _Accum)sa2;
}

__attribute__((noinline, noipa))
long _Accum fixed_comparison_3(volatile int seed) {
    long _Accum la1 = 0.1rlk;
    long _Accum la2 = -0.5rlk;
    
    /* Large multiplication that could overflow */
    la1 = la1 * (long _Accum)(seed * 10.0rlk);
    la2 = la2 - 0.9rlk;
    
    /* Comparisons with constants having non-trivial binary representations */
    if (la1 > 0.33333333333333333333rlk) {
        la1 = la1 / 3.0rlk;
    }
    
    if (la2 == -0.125rlk) {
        la2 = la2 * 4.0rlk;  /* Could reach saturation bounds */
    }
    
    /* Nested comparisons */
    if (la1 > 0.0rlk || la2 < 0.0rlk) {
        if (la1 > la2) {
            la1 = la1 - la2;
        }
    }
    
    return la1 + la2;
}

__attribute__((noinline, noipa))
void fixed_point_operations(volatile int seed) {
    /* Mix of different fixed-point types */
    signed _Fract f1 = 0.7r;
    unsigned _Fract f2 = 0.3ur;
    signed long _Fract f3 = -0.8rlr;
    
    /* Initialize with volatile to prevent constant propagation */
    f1 = f1 * (signed _Fract)(control * 0.01r);
    f2 = f2 + (unsigned _Fract)(seed * 0.001ur);
    f3 = f3 - (signed long _Fract)(control * 0.1rlr);
    
    /* Saturation-prone operations */
    f1 = f1 + 0.5r;  /* Could saturate for signed _Fract */
    f2 = f2 * 2.0ur; /* Could saturate for unsigned _Fract */
    
    /* Critical comparisons that may trigger the uncovered code */
    if (f1 > 0.8r) {
        f1 = 0.9r;
    }
    
    if (f2 < 0.1ur) {
        f2 = 0.0ur;
    }
    
    if (f3 == -0.8rlr) {
        f3 = f3 * 1.25rlr;
    }
    
    /* Use results to prevent elimination */
    control = (int)(f1 * 100.0r) + (int)(f2 * 100.0ur);
}

int main() {
    volatile int seed = 1;
    signed short _Fract result1 = 0.0r;
    signed _Accum result2 = 0.0rk;
    long _Accum result3 = 0.0rlk;
    
    /* Loop to force dynamic range analysis */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call all fixed-point functions */
        result1 = result1 + fixed_comparison_1(seed);
        result2 = result2 + fixed_comparison_2(seed);
        result3 = result3 + fixed_comparison_3(seed);
        fixed_point_operations(seed);
        
        /* Modify control to vary execution paths */
        control = (control + seed) % 100;
    }
    
    /* Print results to prevent optimization */
    printf("Results: %f %f %f\n", 
           (double)result1, 
           (double)result2, 
           (double)result3);
    
    return (int)(result1 * 100.0r) + (int)(result2 * 100.0rk);
}
