/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper function with attributes to prevent constant folding and inlining */
__attribute__((noinline, noipa))
int fixed_point_range_test(volatile int seed) {
    /* Declare various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;        /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;     /* Binary: -0.001 exactly */
    uf1 = 0.5r;        /* Binary: 0.1 exactly */
    
    /* Use volatile seed to introduce uncertainty */
    sa1 = (signed _Accum)(seed) * 0.5rk;  /* 0.5rk is _Accum constant */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.25rk;
    
    /* Arithmetic operations that could overflow/saturate */
    sf3 = sf1 + sf2;   /* 0.1 - 0.125 = -0.025 */
    sa2 = sa1 * sa1;   /* Square - could overflow */
    
    /* Critical comparisons that should trigger range analysis */
    /* These comparisons should generate the double-int high/low comparisons */
    if (sf3 > 0.0r) {
        global_result += 1;
    }
    
    if (sa1 == 0.0rk) {
        global_result += 2;
    }
    
    /* More complex comparisons */
    if (sf1 > 0.5r) {
        global_result += 4;
    }
    
    if (ua1 < 0.75rk) {
        global_result += 8;
    }
    
    /* Cast between types with different scaling - may trigger saturation bounds */
    signed _Fract cast_result = (signed _Fract)sa1;
    if (cast_result < sf2) {
        global_result += 16;
    }
    
    /* Additional arithmetic that could overflow */
    sa1 = sa1 + 100.0rk;  /* Could overflow signed _Accum range */
    
    /* Final comparison that might use the saturation bounds logic */
    if (sa1 > 50.0rk) {
        global_result += 32;
    }
    
    /* Chain of comparisons to increase analysis complexity */
    if (sf1 > 0.0r && sf1 < 1.0r) {
        if (sf2 > -1.0r && sf2 < 0.0r) {
            global_result += 64;
        }
    }
    
    return global_result;
}

/* Second helper function with different fixed-point types */
__attribute__((noinline, noipa))
int accum_range_test(volatile int iter) {
    long _Accum la1, la2;
    unsigned long _Accum ula1;
    
    /* Initialize with values that may require range analysis */
    la1 = 0.333333333333333333rl;  /* Long _Accum constant */
    ula1 = (unsigned long _Accum)(iter) * 0.1rl;
    
    /* Operations that could saturate */
    la2 = la1 * 10.0rl;  /* Could overflow */
    
    /* Comparisons that should trigger the uncovered code */
    if (la2 > 5.0rl) {
        global_result += 128;
    }
    
    if (ula1 == 0.0rl) {
        global_result += 256;
    }
    
    /* Complex condition similar to the uncovered lines logic */
    if (la1 > 0.0rl || (la1 == 0.0rl && ula1 > 0.0rl)) {
        global_result += 512;
    }
    
    return global_result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int result = 0;
    
    /* Loop to force dynamic range analysis */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFF;
        
        /* Call both test functions */
        result = fixed_point_range_test(seed);
        result = accum_range_test(i);
        
        /* Use volatile to prevent loop optimization */
        asm volatile("" : "+r" (result));
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Global side effect: %d\n", global_result);
    
    return result != 0 ? 0 : 1;
}
