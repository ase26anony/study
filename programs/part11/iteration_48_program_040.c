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
    signed short _Fract sf1 = 0.1r;
    signed short _Fract sf2 = -0.125r;
    unsigned short _Fract usf = 0.5r;
    
    /* Introduce uncertainty through volatile */
    signed short _Fract sf3 = (signed short _Fract)(seed * 0.01r);
    
    /* Arithmetic that could saturate */
    sf1 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf3;  /* Square - could overflow for some values */
    
    /* Critical comparisons with constants */
    if (sf1 > 0.5r) {
        /* This branch unlikely but forces range analysis */
        return 0.9r;
    }
    
    if (sf3 == 0.0r) {
        return 0.0r;
    }
    
    /* More comparisons */
    if ((signed short _Fract)(sf1 * 10.0r) < sf2) {
        return -0.25r;
    }
    
    return sf3;
}

__attribute__((noinline, noipa))
signed _Accum fixed_compare_2(volatile int seed) {
    signed _Accum sa1 = 0.5rk;
    signed _Accum sa2 = -0.125rk;
    unsigned _Accum usa = 0.75rk;
    
    /* Volatile-based initialization */
    signed _Accum sa3 = (signed _Accum)(seed) * 0.01rk;
    
    /* Potentially overflowing operations */
    sa1 = sa1 + sa2;  /* 0.5 - 0.125 = 0.375 */
    sa3 = sa3 * sa3;  /* Square - may overflow */
    
    /* Cast between different fixed-point types */
    signed short _Fract sf_from_acc = (signed short _Fract)sa3;
    
    /* Multiple comparisons to trigger range analysis */
    if (sa1 > 0.8rk) {
        return 1.0rk;
    }
    
    if (sa3 == 0.0rk) {
        return 0.0rk;
    }
    
    /* Comparison after cast */
    if (sf_from_acc < 0.1r) {
        return -0.5rk;
    }
    
    /* Complex condition similar to uncovered lines */
    if (sa1 > 0.9rk || (sa1 == 0.9rk && usa > 0.5rk)) {
        return 0.9rk;
    }
    
    return sa3;
}

__attribute__((noinline, noipa))
unsigned _Accum fixed_compare_3(volatile int seed) {
    unsigned _Accum ua1 = 0.1rk;
    unsigned _Accum ua2 = 0.9rk;
    signed _Accum sa = -0.5rk;
    
    /* Volatile-based value */
    ua1 = (unsigned _Accum)(seed % 100) * 0.01rk;
    
    /* Operations that might saturate */
    ua2 = ua2 + ua1;  /* Could overflow past 1.0 */
    
    /* Cast to different type */
    unsigned short _Fract usf = (unsigned short _Fract)ua2;
    
    /* Comparisons with various constants */
    if (ua2 > 0.95rk) {
        return 1.0rk;
    }
    
    if (usf == 0.0r) {
        return 0.0rk;
    }
    
    /* Mixed-type comparison */
    if ((signed _Accum)ua1 < sa) {
        return 0.25rk;
    }
    
    return ua2;
}

int main() {
    volatile int seed = 1;
    signed short _Fract result_fract = 0.0r;
    signed _Accum result_accum = 0.0rk;
    unsigned _Accum result_uaccum = 0.0rk;
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call all comparison functions */
        result_fract = fixed_compare_1(seed);
        result_accum = fixed_compare_2(seed);
        result_uaccum = fixed_compare_3(seed);
        
        /* Use results to prevent elimination */
        control += (int)(result_fract * 1000.0r);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Results: %f, %f, %f\n", 
           (double)result_fract, 
           (double)result_accum, 
           (double)result_uaccum);
    
    return control > 0 ? 0 : 1;
}
