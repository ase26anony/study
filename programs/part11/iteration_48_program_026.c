/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 1;
volatile int iterations = 100;

/* Helper functions marked to prevent optimization */
__attribute__((noinline, noipa))
signed short _Fract fixed_range_test1(int seed) {
    /* Use various fixed-point types and operations */
    signed short _Fract sf1 = 0.1r;
    signed short _Fract sf2 = -0.125r;
    unsigned short _Fract usf = 0.5r;
    
    /* Introduce uncertainty through volatile seed */
    sf1 = sf1 + (signed short _Fract)(seed * 0.01r);
    
    /* Operations that could saturate */
    sf2 = sf2 + 0.9r;  /* Potential overflow for signed fract */
    
    /* Critical comparisons that should trigger range analysis */
    if (sf1 > 0.5r) {
        usf = usf - 0.25r;
    }
    
    if (sf2 == -0.125r) {
        sf1 = sf1 * 0.75r;
    }
    
    /* Another comparison with different constant */
    if (sf1 < 0.0r) {
        return sf2;
    }
    
    return sf1 + usf;
}

__attribute__((noinline, noipa))
signed _Accum accum_range_test(int seed) {
    signed _Accum sa1 = 0.5rk;
    signed _Accum sa2 = -0.125rk;
    
    /* Create value with uncertainty */
    sa1 = sa1 * (signed _Accum)(seed * 0.01rk);
    
    /* Multiplication that could overflow */
    sa2 = sa2 * sa2 * 4.0rk;
    
    /* Cast between types - may trigger saturation logic */
    signed short _Fract sf_cast = (signed short _Fract)sa1;
    
    /* Multiple comparisons */
    if (sa1 > 0.75rk) {
        sa2 = sa2 + 0.25rk;
    }
    
    if (sa2 == 0.0rk) {
        sa1 = sa1 - 0.5rk;
    }
    
    /* Complex condition similar to uncovered code */
    if (sa1 > 1.0rk || (sa1 == 1.0rk && sa2 > 0.0rk)) {
        return sa2;
    }
    
    return sa1;
}

__attribute__((noinline, noipa))
unsigned _Accum unsigned_accum_test(int seed) {
    unsigned _Accum ua1 = 0.1urk;
    unsigned _Accum ua2 = 0.9urk;
    
    /* Create varying values */
    ua1 = ua1 * (unsigned _Accum)(seed * 0.01urk);
    
    /* Operation near saturation bounds */
    ua2 = ua2 + 0.2urk;
    
    /* Comparisons with different constants */
    if (ua1 > 0.5urk) {
        ua2 = ua2 - 0.3urk;
    }
    
    if (ua2 == 1.0urk) {
        ua1 = ua1 * 0.5urk;
    }
    
    return ua1 + ua2;
}

__attribute__((noinline, noipa))
void mixed_type_operations(int seed) {
    signed _Accum sa = 0.25rk;
    unsigned short _Fract usf = 0.75r;
    signed short _Fract sf = -0.5r;
    
    /* Cross-type operations */
    sa = sa + (signed _Accum)usf;
    sf = sf * (signed short _Fract)(sa * 0.5rk);
    
    /* Multiple comparisons in sequence */
    if (sa > 0.8rk) {
        usf = usf - 0.1r;
    }
    
    if (sf < -0.25r) {
        sa = sa - 0.3rk;
    }
    
    /* Compound condition */
    if (usf > 0.5r || (usf == 0.5r && sf > 0.0r)) {
        sf = 0.0r;
    }
    
    /* Use results to prevent elimination */
    control = (int)(sa * 100rk) + (int)(usf * 100r) + (int)(sf * 100r);
}

int main() {
    int result = 0;
    
    /* Loop with volatile control to prevent constant propagation */
    for (int i = 0; i < iterations; i++) {
        /* Call all test functions with varying inputs */
        signed short _Fract f1 = fixed_range_test1(i + control);
        signed _Accum a1 = accum_range_test(i * control);
        unsigned _Accum ua1 = unsigned_accum_test(i + control * 2);
        
        mixed_type_operations(i);
        
        /* Accumulate results to prevent dead code elimination */
        result += (int)(f1 * 100r) + (int)(a1 * 100rk) + (int)(ua1 * 100urk);
    }
    
    printf("Result: %d\n", result);
    return result;
}
