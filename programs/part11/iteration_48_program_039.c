/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Global to prevent elimination */
volatile int global_result = 0;

/* Helper function with attributes to prevent optimization */
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
    
    /* Use volatile seed to prevent constant folding */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.5rk;
    sa1 = (signed _Accum)((seed - 128) * 2) * 0.25rk;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;  /* 0.1 + (-0.125) = -0.025 */
    ua2 = ua1 * ua1;  /* Square - could overflow */
    
    /* Critical comparisons - these should trigger the range analysis */
    if (sf1 > 0.5r) {
        global_result += 1;
    }
    
    if (ua1 == 0.0rk) {
        global_result += 2;
    }
    
    /* Comparison after cast - different scaling factors */
    if ((signed _Fract)ua1 < sf2) {
        global_result += 4;
    }
    
    /* More complex comparison chain */
    if (sa1 > 0.75rk || (sa1 == 0.75rk && ua1 > 0.25rk)) {
        global_result += 8;
    }
    
    /* Multiplication that could saturate */
    signed _Accum sa2 = sa1 * sa1;
    if (sa2 > 0.9rk) {
        global_result += 16;
    }
    
    /* Return something based on the operations */
    return (int)(sf1 * 1000r) + (int)(ua1 * 100rk);
}

__attribute__((noinline, noipa))
int fixed_point_operations_2(volatile int iter) {
    unsigned short _Fract usf1, usf2;
    long _Accum la1;
    
    /* Initialize with values that may approach bounds */
    usf1 = 0.999r;  /* Close to maximum */
    usf2 = 0.001r;  /* Close to minimum */
    
    /* Use iteration count to create varying values */
    la1 = (long _Accum)(iter % 256) * 0.0078125rl;  /* 1/128 */
    
    /* Operations that could trigger saturation bounds calculation */
    usf1 = usf1 + usf2;  /* Could saturate to 1.0 */
    
    /* Comparisons that should use the uncovered logic */
    if (usf1 > 0.5r) {
        global_result += 32;
    }
    
    if (la1 < -0.5rl || la1 > 0.5rl) {
        global_result += 64;
    }
    
    /* Cast between different fixed-point types */
    signed _Fract sf_from_la = (signed _Fract)la1;
    if (sf_from_la == 0.0r) {
        global_result += 128;
    }
    
    return (int)(usf1 * 1000r) + (int)(la1 * 100rl);
}

__attribute__((noinline, noipa))
int fixed_point_operations_3(volatile int seed) {
    /* Test with negative values to trigger signed comparisons */
    signed _Accum sa_neg = -0.9rk;
    signed _Accum sa_pos = 0.9rk;
    signed short _Fract sf_mid = 0.0r;
    
    /* Create value that varies with seed */
    sf_mid = (signed short _Fract)((seed % 100) - 50) * 0.01r;
    
    /* Arithmetic that could overflow in negative direction */
    sa_neg = sa_neg - 0.2rk;  /* -0.9 - 0.2 = -1.1 (could saturate) */
    
    /* Complex comparison that should trigger the specific uncovered condition */
    /* This mimics: if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))) */
    if (sa_pos > 0.8rk || (sa_pos == 0.8rk && sa_neg > -0.5rk)) {
        global_result += 256;
    }
    
    /* Another comparison with mixed types */
    if ((signed _Fract)sa_neg < sf_mid) {
        global_result += 512;
    }
    
    return (int)(sa_neg * 100rk) + (int)(sa_pos * 100rk);
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int total = 0;
    
    printf("Starting fixed-point range analysis test...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all fixed-point operation functions */
        total += fixed_point_operations_1(seed);
        total += fixed_point_operations_2(i);
        total += fixed_point_operations_3(seed);
        
        /* Use volatile to prevent loop optimization */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    printf("Final result: %d (global: %d)\n", total, global_result);
    
    return total != 0 ? 0 : 1;
}
