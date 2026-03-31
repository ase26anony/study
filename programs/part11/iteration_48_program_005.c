/* Test program to trigger fixed-point arithmetic range analysis in GCC's fixed-value.cc */
/* Specifically targeting lines 264-277 in the uncovered lines report */

#include <stdio.h>

/* Global to prevent elimination */
volatile int g_result = 0;

/* Helper function with attributes to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int fixed_point_comparisons_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned _Accum ua1, ua2;
    signed _Accum sa1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;         /* Binary: 0.0001100110011... */
    sf2 = -0.125r;      /* Binary: -0.001 exactly */
    sf3 = 0.5r;         /* Binary: 0.1 exactly */
    
    /* Use volatile seed to introduce uncertainty */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.00390625rk; /* 1/256 */
    sa1 = (signed _Accum)((seed - 128)) * 0.0078125rk;   /* 1/128 */
    
    /* Arithmetic that could overflow/saturate */
    ua2 = ua1 * ua1;  /* Square - could overflow for larger values */
    
    /* Critical comparisons - these should trigger range analysis */
    int result = 0;
    
    /* Comparison 1: fixed-point variable vs constant */
    if (sf1 > 0.5r) {
        result += 1;
    }
    
    /* Comparison 2: after arithmetic operation */
    sf1 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    if (sf1 == -0.025r) {
        result += 2;
    }
    
    /* Comparison 3: accumulator comparison */
    if (ua1 > 0.5rk) {
        result += 4;
    }
    
    /* Comparison 4: signed accumulator with negative values */
    if (sa1 < -0.5rk) {
        result += 8;
    }
    
    /* Comparison 5: cross-type comparison after cast */
    if ((signed _Fract)ua1 < sf2) {
        result += 16;
    }
    
    /* Comparison 6: equality test with non-zero */
    if (ua2 == 0.0rk) {
        result += 32;
    }
    
    /* Additional arithmetic that might saturate */
    signed short _Fract sf4 = 0.9r;
    sf4 = sf4 + 0.2r;  /* Could saturate to 1.0r for short _Fract */
    
    if (sf4 >= 1.0r) {
        result += 64;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int fixed_point_comparisons_2(volatile int iter) {
    unsigned short _Fract usf1, usf2;
    signed _Accum sa1, sa2;
    
    /* Initialize with loop-dependent values */
    usf1 = (unsigned short _Fract)(iter % 256) / 256.0r;
    sa1 = (signed _Accum)(iter - 128) * 0.015625rk; /* 1/64 */
    
    /* Multiple arithmetic operations */
    usf2 = usf1 * 0.5r;
    sa2 = sa1 + sa1;  /* Double the value */
    
    int result = 0;
    
    /* Comparisons that should trigger the specific uncovered code */
    /* These mimic the a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) pattern */
    
    /* Comparison with upper bound */
    if (usf1 > 0.75r) {
        result += 1;
    }
    
    /* Equality comparison followed by another comparison */
    if (usf1 == 0.5r && usf2 > 0.25r) {
        result += 2;
    }
    
    /* Signed accumulator comparisons */
    if (sa1 > 1.0rk) {
        result += 4;
    }
    
    if (sa1 == 0.0rk && sa2 > 0.0rk) {
        result += 8;
    }
    
    /* Negative value comparisons */
    if (sa1 < -1.0rk) {
        result += 16;
    }
    
    /* Complex expression that might trigger saturation bounds calculation */
    signed short _Fract sf_temp = 0.8r;
    for (int i = 0; i < 3; i++) {
        sf_temp = sf_temp + 0.1r;
    }
    
    if (sf_temp > 1.0r) {
        result += 32;  /* Should be true due to saturation */
    }
    
    return result;
}

__attribute__((noinline, noipa))
int fixed_point_with_saturation(volatile int base) {
    /* This function specifically tests saturation contexts */
    signed _Accum sa1, sa2, sa3;
    unsigned short _Fract usf1, usf2;
    
    /* Values that might overflow when multiplied */
    sa1 = (signed _Accum)(base) * 0.5rk;
    sa2 = sa1 * 2.0rk;  /* Should get back to original * 1.0 */
    
    /* Force potential overflow */
    sa3 = sa1 * sa1;
    
    usf1 = 0.9r;
    usf2 = usf1 + 0.2r;  /* Should saturate to 1.0r */
    
    int result = 0;
    
    /* Comparisons in saturation context */
    if (sa3 > sa2) {
        result += 1;
    }
    
    if (usf2 == 1.0r) {
        result += 2;
    }
    
    /* Cross-type comparison */
    if ((signed _Accum)usf1 < sa1) {
        result += 4;
    }
    
    /* Multiple comparisons chained */
    if (sa1 > 0.0rk && sa2 > 0.0rk && sa3 > 0.0rk) {
        result += 8;
    }
    
    return result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    
    printf("Starting fixed-point range analysis test...\n");
    
    int total_result = 0;
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFF;
        
        /* Call different fixed-point functions */
        total_result ^= fixed_point_comparisons_1(seed);
        total_result ^= fixed_point_comparisons_2(i);
        total_result ^= fixed_point_with_saturation(seed % 64);
        
        /* Modify global to prevent dead code elimination */
        g_result += total_result & 1;
    }
    
    printf("Final result: %d (g_result: %d)\n", total_result, g_result);
    
    return total_result != 0 ? 0 : 1;
}
