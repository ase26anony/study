/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 0;

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
signed short _Fract test_fract_range(volatile int seed) {
    /* Initialize with constants that have non-trivial binary representations */
    signed short _Fract sf1 = 0.1r;
    signed short _Fract sf2 = -0.125r;
    signed short _Fract sf3 = 0.5r;
    signed short _Fract sf4 = 0.0r;
    
    /* Introduce uncertainty through volatile seed */
    sf4 = (signed short _Fract)(seed % 100) * 0.01r;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;          /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf3;          /* 0.25 */
    
    /* Critical comparisons that should trigger range analysis */
    if (sf1 > 0.0r) {
        /* This branch unlikely but forces comparison analysis */
        sf4 = sf4 + 0.1r;
    }
    
    /* Another comparison with different constant */
    if (sf3 == 0.25r) {
        sf4 = sf4 - 0.05r;
    }
    
    /* Comparison after arithmetic */
    signed short _Fract sf5 = sf1 + sf3;  /* -0.025 + 0.25 = 0.225 */
    if (sf5 < 0.5r) {
        return sf5;
    }
    
    return sf4;
}

__attribute__((noinline, noipa))
signed _Accum test_accum_range(volatile int seed) {
    signed _Accum sa1 = 0.5rk;
    signed _Accum sa2 = -0.125rk;
    signed _Accum sa3 = 0.0rk;
    
    /* Create value with uncertainty */
    sa3 = (signed _Accum)(seed) * 0.01rk;
    
    /* Potentially overflowing multiplication */
    sa1 = sa1 * sa1;          /* 0.25 */
    
    /* More complex arithmetic chain */
    sa2 = sa2 + sa3;
    
    /* Multiple comparisons to trigger bounds checking */
    if (sa1 > 0.0rk) {
        sa2 = sa2 + 0.1rk;
    }
    
    if (sa2 == 0.0rk) {
        sa1 = sa1 - 0.1rk;
    }
    
    /* Cast between types to trigger range conversion logic */
    signed short _Fract sf_cast = (signed short _Fract)sa1;
    if (sf_cast > 0.1r) {
        sa3 = sa3 + 0.05rk;
    }
    
    /* Final comparison that might hit the uncovered condition */
    signed _Accum result = sa1 + sa2 + sa3;
    if (result > 1.0rk || result < -1.0rk) {
        /* This could trigger saturation bounds calculation */
        return (result > 0.0rk) ? 0.999rk : -0.999rk;
    }
    
    return result;
}

__attribute__((noinline, noipa))
unsigned _Accum test_unsigned_range(volatile int seed) {
    unsigned _Accum ua1 = 0.1urk;
    unsigned _Accum ua2 = 0.5urk;
    unsigned short _Fract uf1 = 0.25ur;
    
    /* Create varying values */
    ua1 = (unsigned _Accum)(seed % 50) * 0.02urk;
    
    /* Arithmetic that could saturate at high end */
    ua2 = ua2 + ua1;
    
    /* Comparisons with unsigned fixed-point */
    if (ua2 > 0.75urk) {
        ua2 = ua2 - 0.1urk;
    }
    
    /* Cast and compare */
    uf1 = (unsigned short _Fract)ua1;
    if (uf1 == 0.0ur) {
        ua2 = ua2 + 0.05urk;
    }
    
    return ua2;
}

__attribute__((noinline, noipa))
void test_mixed_types(volatile int seed) {
    /* Mix signed and unsigned, fract and accum */
    signed _Accum sa = (seed % 3) ? 0.3rk : -0.3rk;
    unsigned short _Fract uf = 0.7ur;
    signed long _Fract slf = 0.1234567lr;
    
    /* Cross-type operations with explicit casts */
    slf = (signed long _Fract)sa * 0.5lr;
    
    /* Multiple comparisons in sequence */
    if (sa > 0.0rk) {
        uf = uf - 0.1ur;
    }
    
    if ((signed _Fract)sa < 0.1r) {
        slf = slf + 0.01lr;
    }
    
    /* Complex condition that might trigger the specific uncovered code */
    signed _Accum temp = sa * 2.0rk;
    if (temp > 0.5rk || temp < -0.5rk) {
        control = (control + 1) % 100;
    }
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    
    signed short _Fract fract_result = 0.0r;
    signed _Accum accum_result = 0.0rk;
    unsigned _Accum unsigned_result = 0.0urk;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions to exercise different paths */
        fract_result = fract_result + test_fract_range(seed);
        accum_result = accum_result + test_accum_range(seed);
        unsigned_result = unsigned_result + test_unsigned_range(seed);
        test_mixed_types(seed);
        
        /* Prevent loop unrolling */
        if (i % 10 == 0) {
            control = i;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: fract=%r, accum=%rk, unsigned=%urk, control=%d\n",
           fract_result, accum_result, unsigned_result, control);
    
    return (fract_result > 0.0r) ? 0 : 1;
}
