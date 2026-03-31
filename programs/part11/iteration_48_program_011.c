/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

/* Global to prevent elimination */
volatile int global_result = 0;

/* Helper functions marked to prevent optimization */
__attribute__((noinline, noipa))
int test_signed_fract_range(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: 0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    uf1 = 0.5ur;     /* Binary: 0.1 exactly */
    
    /* Introduce uncertainty through volatile seed */
    sf3 = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could saturate */
    sf1 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf3;  /* Squaring could cause overflow */
    
    /* CRITICAL: Comparisons that should trigger range analysis */
    int result = 0;
    
    /* First comparison type: variable > constant */
    if (sf1 > 0.0r) {
        result |= 1;
    }
    
    /* Second: variable == constant */
    if (sf2 == -0.125r) {
        result |= 2;
    }
    
    /* Third: variable < constant */
    if (sf3 < 0.25r) {
        result |= 4;
    }
    
    /* Mixed-type comparison after cast */
    if ((signed _Fract)uf1 > sf1) {
        result |= 8;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_accum_saturation(volatile int iter) {
    /* Use accumulator types with wider range */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with values that may approach bounds */
    sa1 = 0.5k;           /* 0.5 in accum format */
    sa2 = -0.25k;         /* -0.25 */
    ua1 = (unsigned _Accum)(iter % 1000) / 1000.0uk;
    
    /* Arithmetic that could overflow the accum range */
    sa1 = sa1 * 1.5k;     /* 0.75 - still in range */
    sa2 = sa2 - 0.8k;     /* -1.05 - may trigger saturation check */
    
    /* Multiplication that could overflow */
    ua1 = ua1 * ua1;      /* Squaring a value near 1.0 */
    
    /* CRITICAL: More comparisons for range analysis */
    int result = 0;
    
    /* Compare against positive bound */
    if (sa1 > 0.99k) {
        result |= 16;
    }
    
    /* Compare against negative bound */
    if (sa2 < -0.99k) {
        result |= 32;
    }
    
    /* Equality comparison with non-zero */
    if (ua1 == 0.25uk) {
        result |= 64;
    }
    
    /* Complex condition similar to uncovered code */
    if (sa1 > 0.8k || (sa1 == 0.8k && ua1 > 0.1uk)) {
        result |= 128;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_mixed_precision(volatile int seed) {
    /* Mix different fixed-point types */
    long _Fract lf1, lf2;
    short _Accum sa1;
    
    /* Initialize with volatile-dependent values */
    lf1 = (long _Fract)(seed % 10000) / 10000.0lr;
    lf2 = 0.123456789lr;  /* High precision constant */
    sa1 = 0.0001hk;       /* Small accum value */
    
    /* Operations that may require range analysis */
    lf1 = lf1 + lf2;
    sa1 = sa1 * 10000.0hk;  /* Scale up - could overflow */
    
    /* Cast between types with different scaling */
    long _Fract lf3 = (long _Fract)sa1;
    
    /* Comparisons after casting */
    int result = 0;
    if (lf1 > 0.5lr) {
        result |= 256;
    }
    
    if (lf3 == lf2) {
        result |= 512;
    }
    
    /* Comparison similar to the uncovered condition structure */
    if (lf1 > 0.9lr || (lf1 == 0.9lr && lf3 > 0.1lr)) {
        result |= 1024;
    }
    
    return result;
}

int main() {
    /* Volatile to prevent constant propagation */
    volatile int iterations = 100;
    volatile int seed = 1;
    
    int total_result = 0;
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different value ranges */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call all test functions */
        total_result ^= test_signed_fract_range(seed);
        total_result ^= test_accum_saturation(i);
        total_result ^= test_mixed_precision(seed);
        
        /* Use result to prevent dead code elimination */
        global_result += total_result;
    }
    
    printf("Final result: %d (global: %d)\n", total_result, global_result);
    return total_result != 0 ? 0 : 1;
}
