/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper function with attributes to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int fixed_point_operations_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned _Accum ua1, ua2;
    signed _Accum sa1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Will have precise fractional representation */
    sf2 = -0.125r;   /* Exact power of two fraction */
    sf3 = 0.5r;      /* Another exact fraction */
    
    /* Introduce uncertainty using volatile seed */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.5rk;
    sa1 = (signed _Accum)((seed - 128) * 2) * 0.25rk;
    
    /* Arithmetic that could overflow/saturate */
    ua2 = ua1 + ua1 * 0.75rk;  /* Potential overflow for unsigned accum */
    sf1 = sf1 + sf2;           /* Signed fract addition */
    
    /* Critical comparisons - these should trigger range analysis */
    if (sf1 > 0.5r) {
        global_result += 1;
    }
    
    if (ua1 == 0.0rk) {
        global_result += 2;
    }
    
    /* Comparison after cast - may trigger different range logic */
    if ((signed _Fract)ua1 < sf2) {
        global_result += 4;
    }
    
    /* More complex comparison chain */
    if (sa1 > 0.0rk || (sa1 == 0.0rk && ua2 > 0.5rk)) {
        global_result += 8;
    }
    
    /* Multiplication that could saturate */
    ua1 = ua1 * ua1;
    if (ua1 > 0.9rk) {
        global_result += 16;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int fixed_point_operations_2(volatile int iter) {
    unsigned short _Fract usf1, usf2;
    long _Accum la1;
    
    /* Initialize with volatile-dependent values */
    usf1 = (unsigned short _Fract)(iter % 256) / 256.0r;
    usf2 = 0.75r;
    la1 = (long _Accum)(iter - 50) * 0.1rlk;
    
    /* Operations that may saturate */
    usf1 = usf1 + usf2;  /* Could saturate for unsigned fract */
    
    /* Comparisons that should trigger bound calculations */
    if (usf1 > 0.5r) {
        global_result += 32;
    }
    
    if (la1 < -0.5rlk) {
        global_result += 64;
    }
    
    /* Cast between different fixed-point types */
    if ((unsigned _Accum)la1 > 0.25rk) {
        global_result += 128;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int fixed_point_operations_3(volatile int seed) {
    _Fract f1, f2, f3;
    _Accum a1;
    
    /* Use exact fractions and volatile-dependent values */
    f1 = 0.333r;      /* Non-exact binary fraction */
    f2 = -0.25r;      /* Exact fraction */
    f3 = (seed & 1) ? 0.1r : 0.9r;
    
    a1 = (_Accum)(seed * 10) * 0.05rk;
    
    /* Chain of operations that could overflow */
    f1 = f1 + f2;
    f1 = f1 * f3;
    
    /* Multiple comparisons in sequence */
    if (f1 > 0.0r) {
        global_result += 256;
    }
    
    if (a1 == 0.0rk) {
        global_result += 512;
    }
    
    /* Complex condition similar to uncovered code logic */
    if (f1 > 0.5r || (f1 == 0.5r && a1 > 0.25rk)) {
        global_result += 1024;
    }
    
    /* Saturation context with explicit cast */
    f2 = (_Fract)(a1 * 2.0rk);
    if (f2 < -0.5r) {
        global_result += 2048;
    }
    
    return global_result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int i, result = 0;
    
    printf("Starting fixed-point coverage test...\n");
    
    for (i = 0; i < iterations; i++) {
        /* Vary the seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all fixed-point operation functions */
        result += fixed_point_operations_1(seed);
        result += fixed_point_operations_2(i);
        result += fixed_point_operations_3(seed);
        
        /* Prevent loop unrolling */
        asm volatile("" : : "r"(result) : "memory");
    }
    
    printf("Final result: %d\n", result);
    printf("Global side effect: %d\n", global_result);
    
    return result != 0 ? 0 : 1;
}
