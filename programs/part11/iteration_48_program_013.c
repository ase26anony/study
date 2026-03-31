/* fixed-value-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-value-test.c -o fixed-value-test
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper functions marked to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int test_signed_fract_range(volatile int seed) {
    /* Use signed short _Fract types */
    signed short _Fract sf1, sf2, sf3;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    sf3 = 0.5r;      /* Binary: 0.1 exactly */
    
    /* Introduce uncertainty through volatile seed */
    signed short _Fract sf_vol = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could approach saturation bounds */
    sf1 = sf1 + sf2;           /* 0.1 - 0.125 = -0.025 */
    sf3 = sf3 * sf_vol;        /* Could be positive or negative depending on seed */
    
    /* Critical comparisons - these should trigger the range analysis */
    int result = 0;
    if (sf1 > 0.5r) {          /* Comparison with constant */
        result |= 1;
    }
    if (sf3 == 0.0r) {         /* Equality comparison */
        result |= 2;
    }
    if ((signed short _Fract)(sf1 + sf3) < -0.5r) {  /* Comparison after arithmetic */
        result |= 4;
    }
    
    /* Additional comparison to trigger a_high.sgt(max_r) logic */
    signed short _Fract sf4 = 0.75r;
    if (sf4 > 0.8r) {          /* Should be false, but forces range check */
        result |= 8;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_accum_saturation(volatile int seed) {
    /* Use _Accum types with different sizes */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with mixed constants */
    sa1 = 0.1k;                /* _Accum constant */
    sa2 = -0.125k;
    
    /* Use volatile seed to create uncertainty */
    ua1 = (unsigned _Accum)(seed & 0xFFF) * 0.01k;
    
    /* Operations that could overflow/saturate */
    sa1 = sa1 * sa1;           /* 0.01 - small but positive */
    sa2 = sa2 + sa2 + sa2;     /* -0.375 */
    
    /* Cast between types - may trigger saturation logic */
    signed short _Fract sf_from_acc = (signed short _Fract)sa1;
    
    /* Critical comparisons for _Accum types */
    int result = 0;
    if (sa1 > 0.5k) {
        result |= 16;
    }
    if (sa2 == -0.375k) {      /* Exact equality with non-trivial constant */
        result |= 32;
    }
    if (ua1 > 1.0uk) {         /* Unsigned comparison */
        result |= 64;
    }
    if (sf_from_acc < 0.0r) {  /* Comparison after cast */
        result |= 128;
    }
    
    /* Multiplication that could overflow */
    signed _Accum sa3 = sa1 * 10.0k;
    if (sa3 > 2.0k) {          /* Range check after potential overflow */
        result |= 256;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_mixed_types_comparisons(volatile int seed) {
    /* Mix different fixed-point types */
    signed short _Fract sf = 0.3r;
    signed _Accum sa = 0.7k;
    unsigned short _Fract usf = 0.8ur;
    
    /* Initialize with seed-dependent values */
    sf = (signed short _Fract)((seed % 100) / 100.0r);
    sa = (signed _Accum)(seed % 1000) / 1000.0k;
    
    /* Arithmetic that approaches bounds */
    sf = sf + 0.5r;            /* Could exceed 1.0 for some seeds */
    sa = sa - 0.5k;            /* Could go negative */
    
    /* Multiple comparisons to trigger range analysis */
    int result = 0;
    
    /* These comparisons should exercise the uncovered condition:
     * a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
     */
    if (sf > 0.9r) {           /* Near upper bound of short _Fract */
        result |= 512;
    }
    if (sa < -0.9k) {          /* Near lower bound of signed _Accum */
        result |= 1024;
    }
    if (usf == 0.8ur) {        /* Exact equality */
        result |= 2048;
    }
    
    /* Cast and compare */
    signed _Accum sa_from_sf = (signed _Accum)sf;
    if (sa_from_sf > 0.5k) {
        result |= 4096;
    }
    
    /* Complex expression with multiple operations */
    signed short _Fract sf_complex = sf - 0.2r + 0.1r;
    if (sf_complex > 0.75r) {
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
        total_result ^= test_accum_saturation(seed);
        total_result ^= test_mixed_types_comparisons(seed);
        
        /* Use global side effect */
        global_result += total_result & 1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (global side effect: %d)\n", total_result, global_result);
    
    return total_result != 0 ? 0 : 1;
}
