/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 0;
volatile int iter_count = 100;
volatile int seed = 1;

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
signed short _Fract fixed_compare_1(int base) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: 0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    uf1 = 0.5ur;     /* Binary: 0.1 exactly */
    
    /* Introduce uncertainty using volatile input */
    sf3 = (signed short _Fract)((base % 100) * 0.01r);
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;           /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * 0.75r;         /* Potential overflow */
    
    /* CRITICAL: Comparisons that should trigger range analysis */
    /* These comparisons should generate the a_high.sgt(max_r) logic */
    if (sf1 > 0.0r) {
        /* This branch may be taken depending on range analysis */
        sf3 = sf3 + 0.1r;
    }
    
    if (sf2 == -0.125r) {
        /* Exact match comparison */
        sf1 = sf1 - 0.01r;
    }
    
    /* Comparison with different type */
    if ((signed short _Fract)uf1 < sf3) {
        sf2 = sf2 * 0.9r;
    }
    
    /* Return value prevents dead code elimination */
    return sf1 + sf2 + sf3 + (signed short _Fract)uf1;
}

__attribute__((noinline, noipa))
signed _Accum fixed_compare_2(int base) {
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with mixed constants */
    sa1 = 0.5rk;           /* 0.5 in accum format */
    sa2 = -0.25rk;         /* -0.25 */
    ua1 = 0.75urk;         /* 0.75 unsigned */
    
    /* Introduce uncertainty */
    sa1 = sa1 + (signed _Accum)(base % 50) * 0.01rk;
    
    /* Arithmetic that could saturate */
    sa2 = sa2 * sa2;       /* 0.0625 - should not overflow */
    ua1 = ua1 * 1.5urk;    /* 1.125 - may saturate for unsigned */
    
    /* CRITICAL: More comparisons for range analysis */
    /* These should trigger the a_high == max_r && a_low.ugt(max_s) path */
    if (sa1 > 0.0rk) {
        ua1 = ua1 - 0.1urk;
    }
    
    if (sa2 == 0.0625rk) {
        /* Exact equality test */
        sa1 = sa1 / 2.0rk;
    }
    
    /* Cross-type comparison after cast */
    if ((signed _Accum)ua1 > sa1) {
        sa2 = sa2 + 0.01rk;
    }
    
    /* Force saturation context */
    sa1 = sa1 * 100.0rk;   /* Could overflow signed _Accum range */
    
    return sa1 + sa2 + (signed _Accum)ua1;
}

__attribute__((noinline, noipa))
long _Fract fixed_compare_3(int base) {
    long _Fract lf1, lf2;
    unsigned long _Fract ulf1;
    
    /* Use wider types for more complex range calculations */
    lf1 = 0.33333333333333333333lr;  /* Approx 1/3 */
    lf2 = -0.66666666666666666666lr; /* Approx -2/3 */
    ulf1 = 0.99999999999999999999ulr; /* Nearly 1.0 */
    
    /* Volatile-dependent initialization */
    lf1 = lf1 + (long _Fract)(base % 10) * 0.1lr;
    
    /* Operations that require careful range analysis */
    lf2 = lf2 / 2.0lr;               /* -0.333... */
    ulf1 = ulf1 * ulf1;              /* 0.999... * 0.999... */
    
    /* CRITICAL: Complex comparisons */
    /* Should trigger saturation bound calculations */
    if (lf1 > 0.5lr && lf1 < 1.0lr) {
        lf2 = lf2 + 0.25lr;
    }
    
    if (ulf1 == 0.99999999999999999999ulr) {
        /* Near-equality test */
        lf1 = lf1 - 0.01lr;
    }
    
    /* Cast and compare */
    if ((long _Fract)ulf1 > lf2) {
        ulf1 = ulf1 - 0.00000000000000000001ulr;
    }
    
    return lf1 + lf2 + (long _Fract)ulf1;
}

int main() {
    signed short _Fract result1 = 0.0r;
    signed _Accum result2 = 0.0rk;
    long _Fract result3 = 0.0lr;
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iter_count; i++) {
        /* Use volatile to prevent loop unrolling */
        control = i;
        
        /* Call all comparison functions */
        result1 = result1 + fixed_compare_1(seed + i);
        result2 = result2 + fixed_compare_2(seed + i * 2);
        result3 = result3 + fixed_compare_3(seed + i * 3);
        
        /* Modify seed to create varying patterns */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Use results to prevent elimination */
    printf("Results: %f, %f, %f\n", 
           (double)result1, 
           (double)result2, 
           (double)result3);
    
    return (result1 > 0.0r) ? 0 : 1;
}
