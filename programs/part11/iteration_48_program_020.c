/* fixed-value-coverage.c
 * 
 * This program is designed to trigger the specific uncovered lines 264-277
 * in GCC's fixed-value.cc file, which handles fixed-point arithmetic range
 * analysis. It uses GCC's _Fract and _Accum types with comparisons and
 * saturation contexts to exercise the double-int bound checking logic.
 */

#include <stdio.h>

/* Global side effect to prevent dead code elimination */
volatile int global_side_effect = 0;

/* Helper function 1: Focus on signed _Fract operations with comparisons */
__attribute__((noinline, noipa))
int test_signed_fract_range(volatile int seed) {
    /* Use different fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: 0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    uf1 = 0.5ur;     /* Binary: 0.1 exactly */
    
    /* Introduce uncertainty through volatile seed */
    sf3 = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could approach saturation bounds */
    sf1 = sf1 + sf2;          /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * 0.75r;        /* Scale down */
    
    /* CRITICAL COMPARISONS: These should trigger the range analysis */
    if (sf1 > 0.0r) {
        global_side_effect += 1;
    }
    
    if (sf3 == -0.125r) {
        global_side_effect += 2;
    }
    
    /* Comparison with different type constant */
    if (sf1 < 0.5r) {
        global_side_effect += 4;
    }
    
    /* Chain of comparisons that may trigger the uncovered condition */
    signed short _Fract temp = sf2 + uf1;  /* -0.125 + 0.5 = 0.375 */
    if (temp > 0.25r && temp < 0.5r) {
        global_side_effect += 8;
    }
    
    return global_side_effect;
}

/* Helper function 2: Focus on _Accum types with potential overflow */
__attribute__((noinline, noipa))
int test_accum_saturation(volatile int seed) {
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with mixed constants */
    sa1 = 0.1k;           /* _Accum constant */
    sa2 = -0.125k;
    ua1 = 0.5uk;
    
    /* Use seed to create non-constant value */
    sa1 = sa1 * (signed _Accum)(seed % 10);
    sa2 = sa2 + (signed _Accum)(seed % 5) * 0.1k;
    
    /* Multiplication that could overflow the representable range */
    sa1 = sa1 * sa1;
    
    /* CRITICAL COMPARISONS for _Accum types */
    if (sa1 > 0.0k) {
        global_side_effect += 16;
    }
    
    if (sa2 == -0.125k) {
        global_side_effect += 32;
    }
    
    /* Comparison after cast to different type */
    if ((signed short _Fract)sa1 < 0.5r) {
        global_side_effect += 64;
    }
    
    /* Complex comparison that may trigger high/low part analysis */
    if (sa1 > 100.0k || (sa1 == 100.0k && ua1 > 0.0uk)) {
        global_side_effect += 128;
    }
    
    return global_side_effect;
}

/* Helper function 3: Mixed types and explicit casts with saturation */
__attribute__((noinline, noipa))
int test_mixed_casts(volatile int seed) {
    signed _Accum sa;
    signed short _Fract sf;
    unsigned _Accum ua;
    
    /* Initialize with volatile-dependent values */
    sa = (signed _Accum)(seed % 100) * 0.01k;
    sf = (signed short _Fract)(seed % 50) / 100.0r;
    ua = 0.75uk;
    
    /* Operations that may require saturation handling */
    sa = sa + 100.0k;           /* Could overflow for large seed */
    sf = sf - 0.5r;             /* Could underflow */
    
    /* Cast between types with different scaling - may trigger saturation */
    signed short _Fract cast_result = (signed short _Fract)sa;
    
    /* CRITICAL COMPARISONS after casts */
    if (cast_result > 0.0r) {
        global_side_effect += 256;
    }
    
    if (sa > 50.0k && sf < 0.0r) {
        global_side_effect += 512;
    }
    
    /* Comparison that directly mirrors the uncovered condition structure */
    if (sa > 127.0k || (sa == 127.0k && ua > 0.0uk)) {
        global_side_effect += 1024;
    }
    
    return global_side_effect;
}

/* Main function with volatile loop control */
int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int result = 0;
    
    printf("Starting fixed-point coverage test...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions to exercise different paths */
        result += test_signed_fract_range(seed);
        result += test_accum_saturation(seed);
        result += test_mixed_casts(seed);
        
        /* Additional inline fixed-point operations */
        {
            volatile signed long _Fract slf = 0.999999lr;
            volatile signed _Accum sa = 0.999999k;
            
            /* Comparisons with values near bounds */
            if (slf > 0.5lr) {
                result += 1;
            }
            
            if (sa == 1.0k) {
                result += 2;
            }
        }
    }
    
    printf("Final result: %d (side effect: %d)\n", result, global_side_effect);
    
    /* Return non-zero if any comparisons triggered */
    return (result != 0) ? 0 : 1;
}
