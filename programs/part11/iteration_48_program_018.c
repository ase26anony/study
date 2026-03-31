/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper function 1: Focus on _Fract types with comparisons */
__attribute__((noinline, noipa))
int test_fract_range(volatile int seed) {
    /* Use different _Fract types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1, uf2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    uf1 = 0.5r;      /* Binary: 0.1 exactly */
    
    /* Introduce uncertainty through volatile seed */
    uf2 = (unsigned short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could saturate */
    sf3 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    
    /* Critical comparisons that should trigger range analysis */
    int result = 0;
    
    /* First comparison type: variable > constant */
    if (sf3 > 0.0r) {
        result |= 1;
    }
    
    /* Second comparison: variable < constant */
    if (uf2 < 0.75r) {
        result |= 2;
    }
    
    /* Third comparison: variable == constant (edge case) */
    if (sf2 == -0.125r) {
        result |= 4;
    }
    
    /* Arithmetic that could overflow */
    signed short _Fract sf4 = sf1 * 2.0r;  /* Could saturate for some values */
    
    /* Comparison after arithmetic */
    if (sf4 > 0.5r) {
        result |= 8;
    }
    
    /* Cast between types with comparison */
    if ((signed _Fract)uf1 > sf2) {
        result |= 16;
    }
    
    return result;
}

/* Helper function 2: Focus on _Accum types with saturation contexts */
__attribute__((noinline, noipa))
int test_accum_range(volatile int seed) {
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1;
    
    /* Initialize with constants */
    sa1 = 0.1k;        /* Binary fractional part */
    sa2 = -0.125k;
    
    /* Use volatile seed to create uncertain value */
    ua1 = (unsigned _Accum)(seed & 0xFFF) * 0.001k;
    
    /* Multiplication that could overflow */
    sa3 = sa1 * sa1;   /* 0.01 - well within range */
    
    /* More aggressive multiplication */
    signed _Accum sa4 = sa1 * 10.0k;  /* Could approach limits */
    
    int result = 0;
    
    /* Comparisons with constants that may trigger the specific bounds check */
    if (sa3 > 0.005k) {
        result |= 32;
    }
    
    /* Equality comparison */
    if (sa2 == -0.125k) {
        result |= 64;
    }
    
    /* Comparison after cast */
    if ((signed short _Fract)sa4 > 0.5r) {
        result |= 128;
    }
    
    /* Create potential overflow scenario */
    signed _Accum sa5 = sa1;
    for (int i = 0; i < 5; i++) {
        sa5 = sa5 + sa5;  /* Doubling each time */
    }
    
    /* Comparison that might trigger max_r/max_s bounds */
    if (sa5 > 1.0k) {
        result |= 256;
    }
    
    /* Negative side comparison */
    if (sa2 < -0.1k) {
        result |= 512;
    }
    
    return result;
}

/* Helper function 3: Mixed types and explicit casts */
__attribute__((noinline, noipa))
int test_mixed_conversions(volatile int seed) {
    signed _Accum sa;
    signed short _Fract sf;
    unsigned _Accum ua;
    
    /* Initialize with volatile-dependent values */
    sa = (signed _Accum)(seed - 50) * 0.02k;
    sf = (signed short _Fract)(seed % 100) / 100.0r;
    ua = (unsigned _Accum)(seed % 1000) * 0.001k;
    
    int result = 0;
    
    /* Explicit casts between different fixed-point types */
    signed short _Fract sf_from_acc = (signed short _Fract)sa;
    
    /* Comparisons after casts - these often trigger range analysis */
    if (sf_from_acc > 0.25r) {
        result |= 1024;
    }
    
    /* Cast from unsigned to signed with comparison */
    if ((signed _Accum)ua > sa) {
        result |= 2048;
    }
    
    /* Complex expression with comparison */
    signed _Accum sa_mixed = sa + (signed _Accum)sf;
    if (sa_mixed > 0.5k) {
        result |= 4096;
    }
    
    /* Boundary comparisons */
    if (sf == 0.0r) {
        result |= 8192;
    }
    
    if (ua == 0.0uk) {
        result |= 16384;
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
        total_result ^= test_fract_range(seed);
        total_result ^= test_accum_range(seed);
        total_result ^= test_mixed_conversions(seed);
        
        /* Use global side effect */
        global_result += total_result & 1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d (global side effect: %d)\n", 
           total_result, global_result);
    
    return total_result != 0 ? 0 : 1;
}
