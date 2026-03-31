/* Test case to trigger fixed-point arithmetic range analysis in GCC's fixed-value.cc */
/* Specifically targeting lines 264-277 in the uncovered lines report */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper function with attributes to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int fixed_point_operations_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned _Accum ua1, ua2;
    signed _Accum sa1, sa2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;          /* Non-trivial binary fraction */
    sf2 = -0.125r;       /* Exact binary fraction: -1/8 */
    sf3 = 0.5r;          /* Exact binary fraction: 1/2 */
    
    /* Initialize with volatile-dependent values to prevent constant folding */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.5rk;  /* Scale by 0.5 */
    sa1 = (signed _Accum)((seed - 128) * 2) * 0.25rk;
    
    /* Arithmetic operations that could overflow/saturate */
    sf1 = sf1 + sf2;     /* 0.1 + (-0.125) = -0.025 */
    ua2 = ua1 * ua1;     /* Square - could overflow */
    sa2 = sa1 + sa1;     /* Double - could overflow */
    
    /* Critical comparisons - these should trigger the range analysis */
    int result = 0;
    
    /* Comparison 1: fixed-point variable vs constant */
    if (sf1 > 0.5r) {    /* Likely false, but forces range analysis */
        result |= 1;
    }
    
    /* Comparison 2: equality check with non-zero constant */
    if (ua1 == 0.0rk) {  /* Forces comparison logic */
        result |= 2;
    }
    
    /* Comparison 3: inequality with negative constant */
    if (sa1 < -0.5rk) {  /* Could be true depending on seed */
        result |= 4;
    }
    
    /* Comparison 4: after type cast - different scaling */
    if ((signed _Fract)ua1 < sf2) {  /* Cast between types */
        result |= 8;
    }
    
    /* Comparison 5: complex expression */
    if (sf3 > sf1 && sf1 != 0.0r) {  /* Multiple conditions */
        result |= 16;
    }
    
    /* Additional arithmetic that might saturate */
    signed _Accum sa3 = sa2 * 2.0rk;  /* Could overflow */
    if (sa3 > 1.0rk || sa3 < -1.0rk) {
        result |= 32;
    }
    
    return result;
}

/* Second helper function with different fixed-point types */
__attribute__((noinline, noipa))
int fixed_point_operations_2(volatile int seed) {
    unsigned short _Fract usf1, usf2;
    signed long _Accum sla1;
    
    /* Initialize with volatile-dependent values */
    usf1 = (unsigned short _Fract)(seed & 0xF) / 16.0r;
    usf2 = 0.75r;  /* 3/4 */
    sla1 = (signed long _Accum)seed * 0.1rlk;
    
    /* Operations that might saturate */
    usf1 = usf1 + usf2;  /* Could saturate for large values */
    
    /* Comparisons to trigger range analysis */
    int result = 0;
    
    /* Compare against bounds */
    if (usf1 > 0.9r) {    /* Near upper bound */
        result |= 64;
    }
    
    if (sla1 == 0.0rlk) { /* Equality check */
        result |= 128;
    }
    
    /* Cast and compare */
    if ((signed _Accum)usf1 < 0.0rk) {
        result |= 256;
    }
    
    return result;
}

/* Third function focusing on saturation contexts */
__attribute__((noinline, noipa))
int saturation_operations(volatile int seed) {
    signed _Accum sa1, sa2, sa3;
    unsigned _Fract uf1, uf2;
    
    /* Values that could cause overflow */
    sa1 = (signed _Accum)(seed - 50) * 0.8rk;
    sa2 = (signed _Accum)(seed + 50) * 0.8rk;
    
    /* Multiplication that could overflow */
    sa3 = sa1 * sa2;
    
    /* Fixed-point with full range */
    uf1 = 0.999999r;  /* Nearly 1.0 */
    uf2 = 0.000001r;  /* Nearly 0.0 */
    
    /* Operations near boundaries */
    uf1 = uf1 + uf2;  /* Could saturate to 1.0 */
    
    int result = 0;
    
    /* Comparisons that should use saturation bounds */
    if (sa3 > 0.5rk) {
        result |= 512;
    }
    
    if (uf1 == 1.0r) {  /* Exact equality at boundary */
        result |= 1024;
    }
    
    /* Check if negative */
    if (sa1 < 0.0rk && sa2 > 0.0rk) {
        result |= 2048;
    }
    
    return result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int total_result = 0;
    
    /* Loop to force dynamic analysis with varying values */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different value ranges */
        seed = (seed * 1103515245 + 12345) & 0x7FFF;
        
        /* Call all fixed-point operation functions */
        total_result ^= fixed_point_operations_1(seed);
        total_result ^= fixed_point_operations_2(seed);
        total_result ^= saturation_operations(seed);
        
        /* Use volatile to prevent loop unrolling */
        asm volatile("" : "+r" (total_result));
    }
    
    /* Store to global to prevent dead code elimination */
    global_result = total_result;
    
    printf("Result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
