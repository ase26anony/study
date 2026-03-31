/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

volatile int global_counter = 0;

/* Helper function with attributes to prevent constant folding and inlining */
__attribute__((noinline, noipa))
static int fixed_point_comparisons_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2;
    unsigned short _Fract uf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Will have precise fractional representation */
    sf2 = -0.125r;   /* Exact power of two fraction */
    uf1 = 0.5r;      /* Another exact fraction */
    
    /* Introduce uncertainty using volatile seed */
    sa1 = (signed _Accum)(seed) * 0.5rk;
    ua1 = (unsigned _Accum)(seed % 256) * 0.25rk;
    
    /* Perform arithmetic that could overflow/saturate */
    sa2 = sa1 + (signed _Accum)sf1;
    sf1 = sf1 + sf2;  /* Potential saturation for short _Fract */
    
    /* Multiplication that could overflow */
    sa1 = sa1 * sa2;
    
    /* Critical comparisons - these should trigger the range analysis */
    int result = 0;
    
    /* Comparison 1: Fixed-point variable against constant */
    if (sf1 > 0.5r) {
        result += 1;
    }
    
    /* Comparison 2: Equality check with zero */
    if (sa2 == 0.0rk) {
        result += 2;
    }
    
    /* Comparison 3: Complex comparison chain */
    if (ua1 < 0.75rk) {
        result += 4;
    }
    
    /* Comparison 4: After type cast - different scaling */
    if ((signed _Fract)sa1 < sf2) {
        result += 8;
    }
    
    /* Comparison 5: Against negative constant */
    if (sa2 > -0.25rk) {
        result += 16;
    }
    
    /* Additional arithmetic that might saturate */
    ua1 = ua1 * ua1;  /* Square could overflow unsigned _Accum */
    
    /* More comparisons */
    if (uf1 == 0.5r) {
        result += 32;
    }
    
    /* Use result to prevent dead code elimination */
    global_counter += result;
    return result;
}

/* Second helper function with different fixed-point types */
__attribute__((noinline, noipa))
static long fixed_point_comparisons_2(volatile int iter) {
    long _Fract lf1, lf2;
    signed _Accum sa1, sa2;
    
    /* Initialize with values that may trigger saturation bounds */
    lf1 = 0.9999999r;  /* Close to maximum for long _Fract */
    lf2 = -0.9999999r; /* Close to minimum */
    
    /* Use iteration count to create varying values */
    sa1 = (signed _Accum)(iter) * 0.1rk;
    sa2 = (signed _Accum)(iter % 100) * 0.01rk;
    
    /* Arithmetic that could saturate */
    lf1 = lf1 + 0.0000001r;  /* Might saturate at 1.0 */
    lf2 = lf2 - 0.0000001r;  /* Might saturate at -1.0 */
    
    /* Critical comparisons */
    long result = 0;
    
    /* Comparison near saturation boundaries */
    if (lf1 > 0.9r) {
        result += 1000;
    }
    
    if (lf2 < -0.9r) {
        result += 2000;
    }
    
    /* Complex comparison that might trigger the specific uncovered condition */
    if (sa1 > 5.0rk || (sa1 == 5.0rk && sa2 > 0.0rk)) {
        result += 4000;
    }
    
    /* Additional overflow-prone operation */
    sa1 = sa1 * sa1;
    
    /* Comparison after multiplication */
    if (sa1 > 25.0rk) {
        result += 8000;
    }
    
    /* Cast and compare */
    if ((long _Fract)sa2 > 0.5r) {
        result += 16000;
    }
    
    global_counter += (int)result;
    return result;
}

/* Third helper focusing on unsigned types */
__attribute__((noinline, noipa))
static unsigned int unsigned_fixed_point_ops(volatile int seed) {
    unsigned short _Fract usf1, usf2;
    unsigned _Accum ua1, ua2;
    
    /* Initialize with constants */
    usf1 = 0.75r;
    usf2 = 0.25r;
    
    /* Create varying values */
    ua1 = (unsigned _Accum)(seed * 2) * 0.125rk;
    ua2 = (unsigned _Accum)(seed % 50) * 0.02rk;
    
    /* Arithmetic operations */
    usf1 = usf1 + usf2;  /* Could saturate at 1.0 */
    ua1 = ua1 * ua2;     /* Could overflow */
    
    /* Comparisons */
    unsigned int result = 0;
    
    if (usf1 > 0.9r) {
        result += 100;
    }
    
    if (ua1 == 0.0rk) {
        result += 200;
    }
    
    if (ua2 < 0.5rk) {
        result += 400;
    }
    
    /* Complex condition */
    if (ua1 > 2.0rk || (ua1 == 2.0rk && ua2 > 0.1rk)) {
        result += 800;
    }
    
    global_counter += result;
    return result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int total_result = 0;
    
    printf("Starting fixed-point range analysis test...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call all helper functions to exercise different code paths */
        total_result += fixed_point_comparisons_1(seed % 100);
        total_result += (int)fixed_point_comparisons_2(i);
        total_result += (int)unsigned_fixed_point_ops(seed % 200);
        
        /* Additional direct operations in main */
        signed _Accum sa = (signed _Accum)(i) * 0.01rk;
        signed short _Fract sf = 0.5r;
        
        if (sa > 0.5rk) {
            total_result += 1;
        }
        
        if (sf == 0.5r) {
            total_result += 2;
        }
        
        /* Cast and compare */
        if ((signed _Fract)sa < sf) {
            total_result += 4;
        }
    }
    
    printf("Test completed. Total result: %d (global_counter: %d)\n", 
           total_result, global_counter);
    
    /* Return non-zero to indicate success if we got past all comparisons */
    return total_result != 0 ? 0 : 1;
}
