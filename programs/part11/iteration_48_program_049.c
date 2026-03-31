/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper functions marked to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int test_signed_fract_range(volatile int seed) {
    /* Use various signed fract types */
    signed short _Fract sf1, sf2, sf3;
    signed _Fract f1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Non-power-of-two fraction */
    sf2 = -0.125r;   /* Exact power-of-two fraction */
    f1 = 0.5r;       /* Another exact fraction */
    
    /* Introduce uncertainty through volatile seed */
    sf3 = (signed short _Fract)(seed % 256) * 0.00390625r; /* 1/256 */
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;          /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf3;          /* Squaring could reduce magnitude */
    
    /* CRITICAL: Comparisons that should trigger range analysis */
    int result = 0;
    if (sf1 > 0.5r) {         /* Likely false, but forces comparison */
        result |= 1;
    }
    if (sf1 == -0.025r) {     /* Exact comparison */
        result |= 2;
    }
    if (sf3 < 0.0r) {         /* Should be false for positive square */
        result |= 4;
    }
    
    /* Cast between types with different scaling */
    signed _Fract f2 = (signed _Fract)sf3;
    if (f2 > f1) {            /* Another comparison */
        result |= 8;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_accumulator_saturation(volatile int seed) {
    /* Use accumulator types with different sizes */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    signed long _Accum sla1;
    
    /* Initialize with volatile-dependent values */
    sa1 = (signed _Accum)(seed - 50) * 0.1rk;  /* Range ~ -5.0 to 5.0 */
    sa2 = -0.125rk;                            /* Exact value */
    ua1 = (unsigned _Accum)(seed % 100) * 0.01rk; /* Range 0.0 to 0.99 */
    
    /* Arithmetic that could saturate */
    sla1 = (signed long _Accum)sa1 * (signed long _Accum)sa1; /* Square */
    
    /* CRITICAL: More comparisons for range analysis */
    int result = 0;
    
    /* This comparison structure mimics the uncovered code's logic */
    if (sa1 > 0.5rk) {
        result |= 16;
    }
    
    if (sa1 == sa2) {         /* Equality comparison */
        result |= 32;
    }
    
    /* Cast with potential saturation */
    signed short _Fract sf_cast = (signed short _Fract)sa1;
    if (sf_cast < 0.0r) {
        result |= 64;
    }
    
    /* Multiplication that could overflow */
    sa1 = sa1 * 10.0rk;       /* Could push beyond range */
    
    /* Comparison after potential overflow */
    if (sa1 > 100.0rk || sa1 < -100.0rk) {
        result |= 128;
    }
    
    /* Unsigned accumulator comparison */
    if (ua1 > 0.5rk) {
        result |= 256;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_mixed_types_comparisons(volatile int seed) {
    /* Mix different fixed-point types */
    unsigned short _Fract usf1, usf2;
    signed _Accum sa;
    unsigned long _Accum ula;
    
    /* Initialize with various values */
    usf1 = 0.75ur;            /* Near upper bound */
    usf2 = (unsigned short _Fract)(seed % 64) * 0.015625ur; /* 1/64 */
    sa = (signed _Accum)(seed - 32) * 0.03125rk; /* ~ -1.0 to 1.0 */
    ula = (unsigned long _Accum)(seed % 128) * 0.0078125ulk; /* 1/128 */
    
    /* Arithmetic operations */
    usf1 = usf1 - usf2;       /* Could underflow to negative? No, unsigned */
    sa = sa + 0.5rk;          /* Shift range */
    
    /* CRITICAL: Multiple comparisons to trigger bounds checking */
    int result = 0;
    
    if (usf1 > 0.5ur) {
        result |= 512;
    }
    
    if (usf1 == 0.75ur) {     /* Equality test */
        result |= 1024;
    }
    
    /* Cross-type comparison after cast */
    if ((unsigned _Accum)usf1 < ula) {
        result |= 2048;
    }
    
    /* Complex condition similar to uncovered code pattern */
    if (sa > 0.0rk && sa < 1.0rk) {
        result |= 4096;
    }
    
    /* Another saturation-prone operation */
    ula = ula * 2.0ulk;
    if (ula > 1.0ulk) {       /* Could saturate */
        result |= 8192;
    }
    
    return result;
}

int main() {
    /* Volatile control variables to prevent constant propagation */
    volatile int iterations = 100;
    volatile int seed = 1;
    
    int total_result = 0;
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different value ranges */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions */
        total_result ^= test_signed_fract_range(seed);
        total_result ^= test_accumulator_saturation(seed);
        total_result ^= test_mixed_types_comparisons(seed);
        
        /* Use result to prevent dead code elimination */
        global_result += total_result;
    }
    
    /* Print final result to ensure all code is used */
    printf("Final result: %d (global: %d)\n", total_result, global_result);
    
    return total_result != 0 ? 0 : 1;
}
