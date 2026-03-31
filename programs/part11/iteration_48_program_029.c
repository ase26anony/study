/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 1;

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
signed short _Fract test_fract_range(volatile int seed) {
    /* Initialize with constants that have non-trivial binary representations */
    signed short _Fract sf1 = 0.1r;
    signed short _Fract sf2 = -0.125r;
    signed short _Fract sf3 = 0.5r;
    signed short _Fract sf4 = 0.0r;
    
    /* Use volatile seed to prevent constant propagation */
    sf4 = (signed short _Fract)(seed * 0.01r);
    
    /* Arithmetic that could approach saturation bounds */
    sf1 = sf1 + sf2;          /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf4;          /* 0.5 * variable */
    
    /* Critical comparisons - these should trigger range analysis */
    if (sf1 > 0.5r) {
        /* Unlikely but forces range check */
        return sf1;
    }
    
    if (sf3 == 0.0r) {
        /* Equality comparison */
        return sf3;
    }
    
    /* Additional comparison with constant */
    if (sf1 < -0.125r) {
        return sf2;
    }
    
    /* Mixed comparisons */
    signed short _Fract result = sf1 + sf3;
    if (result > 0.75r) {
        return 0.75r;
    }
    
    return result;
}

__attribute__((noinline, noipa))
signed _Accum test_accum_saturation(volatile int iter) {
    signed _Accum sa1 = 0.5rk;
    signed _Accum sa2 = -0.125rk;
    signed _Accum sa3 = 0.0rk;
    
    /* Create value that varies per iteration */
    sa3 = (signed _Accum)(iter) * 0.01rk;
    
    /* Multiplication that could overflow */
    sa1 = sa1 * sa3;
    
    /* Operations approaching representable limits */
    sa2 = sa2 - sa3;
    
    /* Critical comparisons for _Accum type */
    if (sa1 > 0.8rk) {
        /* May trigger max bound calculation */
        return sa1;
    }
    
    if (sa2 == -0.5rk) {
        return sa2;
    }
    
    /* Cast between types to trigger range analysis */
    signed short _Fract temp = (signed short _Fract)sa1;
    if (temp < 0.1r) {
        return (signed _Accum)temp;
    }
    
    /* Compound condition similar to uncovered code */
    signed _Accum result = sa1 + sa2;
    if (result > 0.9rk || result < -0.9rk) {
        /* This should trigger saturation bound calculations */
        return (result > 0) ? 0.999rk : -0.999rk;
    }
    
    return result;
}

__attribute__((noinline, noipa))
unsigned _Accum test_unsigned_range(volatile int seed) {
    unsigned _Accum ua1 = 0.1urk;
    unsigned _Accum ua2 = 0.5urk;
    unsigned short _Fract uf1 = 0.25ur;
    
    /* Variable initialization */
    ua1 = (unsigned _Accum)(seed % 100) * 0.01urk;
    
    /* Multiplication that could saturate */
    ua2 = ua2 * ua1;
    
    /* Cast with potential range issues */
    uf1 = (unsigned short _Fract)ua2;
    
    /* Comparisons for unsigned types */
    if (ua2 > 0.8urk) {
        return ua2;
    }
    
    if (uf1 == 0.0ur) {
        return (unsigned _Accum)uf1;
    }
    
    /* Mixed-type comparison */
    if (ua1 < (unsigned _Accum)uf1) {
        return ua1;
    }
    
    return ua2;
}

__attribute__((noinline, noipa))
void test_saturation_cast(volatile int x) {
    signed _Accum sa = (signed _Accum)x * 0.1rk;
    signed short _Fract sf;
    
    /* Explicit cast that may require saturation */
    sf = (signed short _Fract)sa;
    
    /* Comparisons after cast */
    if (sf > 0.5r) {
        control = 2;
    }
    
    if (sa < -0.5rk) {
        control = 3;
    }
    
    /* Additional arithmetic that could overflow */
    sa = sa * sa;
    if (sa > 0.9rk) {
        control = 4;
    }
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    signed short _Fract total_fract = 0.0r;
    signed _Accum total_accum = 0.0rk;
    unsigned _Accum total_unsigned = 0.0urk;
    
    /* Loop to force dynamic analysis */
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant folding */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call all test functions */
        total_fract = total_fract + test_fract_range(seed % 100);
        total_accum = total_accum + test_accum_saturation(i);
        total_unsigned = total_unsigned + test_unsigned_range(seed % 50);
        
        /* Test saturation casts */
        test_saturation_cast(seed % 20 - 10); /* Range -10..9 */
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: fract=%r, accum=%rk, unsigned=%urk, control=%d\n",
           total_fract, total_accum, total_unsigned, control);
    
    return control;
}
