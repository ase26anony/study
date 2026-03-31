/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

volatile int control = 1;
volatile int iterations = 100;

/* Prevent constant folding and inlining */
__attribute__((noinline, noipa))
signed int test_sat_comparisons(volatile int seed) {
    /* Use various fixed-point types to trigger different range calculations */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Will have precise fractional representation */
    sf2 = -0.125r;   /* Exact power of two fraction */
    sf3 = 0.5r;      /* Another exact fraction */
    
    /* Use volatile seed to prevent constant propagation */
    uf1 = (unsigned short _Fract)(seed & 0xFF) / 256.0r;
    
    /* Accumulator types with different scaling */
    sa1 = (signed _Accum)(seed) * 0.5rk;
    sa2 = -0.3333333333333333333333rk;  /* Non-exact fraction */
    
    /* This multiplication could overflow/saturate */
    ua1 = (unsigned _Accum)(seed * seed) * 0.01rk;
    
    /* Critical arithmetic operations that may require saturation analysis */
    sf1 = sf1 + sf2;  /* Could underflow for negative results */
    
    /* Multiplications that could overflow the representable range */
    sa1 = sa1 * sa2;
    
    /* Explicit casts between types with different scaling - may trigger saturation */
    signed _Fract sf_cast = (signed _Fract)sa1;
    
    /* THE CRITICAL COMPARISONS - designed to trigger the uncovered condition */
    int result = 0;
    
    /* Comparison 1: Direct comparison with constants */
    if (sf1 > 0.5r) {
        result += 1;
    }
    
    /* Comparison 2: Equality check with zero */
    if (sa1 == 0.0rk) {
        result += 2;
    }
    
    /* Comparison 3: Comparison after cast */
    if ((signed _Fract)ua1 < sf2) {
        result += 4;
    }
    
    /* Comparison 4: Complex comparison chain */
    if (sf_cast > 0.25r || (sf_cast == 0.25r && uf1 > 0.125r)) {
        result += 8;
    }
    
    /* Comparison 5: Negative value comparisons */
    if (sa2 < -0.5rk) {
        result += 16;
    }
    
    /* Additional arithmetic to force range analysis */
    for (int i = 0; i < 3; i++) {
        sf3 = sf3 * 0.75r;
        if (sf3 < 0.1r) {
            result ^= (1 << i);
        }
    }
    
    return result;
}

__attribute__((noinline, noipa))
unsigned int test_accumulator_range(volatile int base) {
    signed long _Accum sla1, sla2;
    unsigned _Accum ua1, ua2;
    
    /* Initialize with values that may approach saturation bounds */
    sla1 = (signed long _Accum)base * 0.0001rlk;
    sla2 = 0.999999999999999999rk;  /* Very close to 1.0 */
    
    ua1 = (unsigned _Accum)(base % 1000) * 0.001rk;
    ua2 = 0.5rk;
    
    /* Operations that could saturate */
    for (int i = 0; i < 5; i++) {
        ua1 = ua1 * 2.0rk;  /* Could overflow for large values */
        sla1 = sla1 + 0.1rk;
        
        /* Comparisons inside loop - force dynamic range analysis */
        if (ua1 > 0.75rk) {
            ua2 = ua2 * 0.9rk;
        }
        
        if (sla1 > 0.5rk && sla1 < 1.0rk) {
            sla2 = sla2 - 0.1rk;
        }
    }
    
    /* Final comparisons */
    unsigned int result = 0;
    if (ua1 == 1.0rk) {
        result |= 0x1;
    }
    
    if (sla2 < 0.0rk) {
        result |= 0x2;
    }
    
    if (ua2 > ua1) {
        result |= 0x4;
    }
    
    return result;
}

int main() {
    int total_result = 0;
    volatile int seed = 1;
    
    /* Loop with volatile iteration count to prevent optimization */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different value ranges */
        seed = (seed * 1103515245 + 12345) & 0x7FFF;
        
        /* Call functions that perform fixed-point operations */
        total_result ^= test_sat_comparisons(seed);
        total_result += test_accumulator_range(seed + i);
        
        /* Modify control to prevent loop unrolling */
        if (control) {
            seed += i % 7;
        }
    }
    
    printf("Result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
