/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper functions marked to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int test_saturating_accum_comparisons(volatile int seed) {
    /* Use various fixed-point types */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    signed short _Fract sf1, sf2;
    unsigned _Fract uf1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sa1 = 0.1rk;        /* Will have precise fractional representation */
    sf1 = -0.125r;      /* Exact power of two fraction */
    uf1 = 0.5ur;        /* Exact 0.5 */
    
    /* Introduce uncertainty using volatile seed */
    sa2 = (signed _Accum)(seed) * 0.25rk;
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.125urk;
    sf2 = (signed short _Fract)((seed % 100) * 0.01r);
    
    /* Perform arithmetic that could overflow/saturate */
    sa1 = sa1 + sa2;    /* Possible overflow */
    ua1 = ua1 * ua1;    /* Possible overflow for larger values */
    sf1 = sf1 - sf2;    /* Possible underflow */
    
    /* CRITICAL: Comparisons that should trigger the uncovered range analysis */
    /* These comparisons involve high/low part splitting in fixed-value.cc */
    if (sa1 > 0.75rk) {
        global_result += 1;
    }
    
    if (ua1 == 0.0urk) {
        global_result += 2;
    }
    
    /* Comparison after cast - may trigger saturation bounds calculation */
    if ((signed _Fract)ua1 < sf2) {
        global_result += 4;
    }
    
    /* Complex comparison chain similar to the uncovered condition */
    if (sa1 > 0.5rk || (sa1 == 0.5rk && ua1 > 0.25urk)) {
        global_result += 8;
    }
    
    /* Additional saturation-inducing operations */
    signed _Accum sa3 = sa1 * 2.0rk;  /* Likely overflow for some values */
    if (sa3 > 1.0rk) {
        /* This should trigger saturation bound calculations */
        sa3 = 1.0rk;  /* Explicit saturation */
        global_result += 16;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int test_fract_range_analysis(volatile int iter) {
    unsigned long _Accum ula1, ula2;
    signed _Fract sf3, sf4;
    
    /* Initialize with values that may hit edge cases */
    ula1 = 0.999999999999999999ulrk;  /* Very close to 1.0 */
    sf3 = 0.99r;                      /* Close to 1.0 for short _Fract */
    sf4 = -0.99r;                     /* Close to -1.0 */
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < iter; i++) {
        /* Volatile read prevents constant propagation */
        volatile int temp = i;
        ula2 = (unsigned long _Accum)temp * 0.01ulrk;
        
        /* Operations that may saturate */
        ula1 = ula1 + ula2;
        sf3 = sf3 - 0.01r;
        sf4 = sf4 + 0.01r;
        
        /* Comparisons that should trigger the specific uncovered code */
        /* The condition structure matches a_high.sgt(max_r) pattern */
        if (ula1 > 0.5ulrk || (ula1 == 0.5ulrk && sf3 > 0.0r)) {
            global_result += 32;
        }
        
        /* Another comparison with negative values */
        if (sf4 < -0.5r || (sf4 == -0.5r && sf3 < 0.0r)) {
            global_result += 64;
        }
    }
    
    /* Final comparison that may use computed saturation bounds */
    if (ula1 > 1.0ulrk) {
        /* Should trigger max_r/max_s initialization in uncovered code */
        ula1 = 1.0ulrk;
        global_result += 128;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int test_mixed_type_conversions(volatile int seed) {
    /* Test conversions between different fixed-point types */
    signed _Accum sa;
    unsigned short _Fract usf;
    long _Fract lf;
    
    /* Initialize with values that may cause overflow on conversion */
    sa = (signed _Accum)(seed - 50) * 0.1rk;
    usf = 0.9ur;  /* Near upper bound for unsigned short _Fract */
    lf = -0.9lr;  /* Near lower bound for long _Fract */
    
    /* Conversions that may saturate */
    signed _Fract sf_from_sa = (signed _Fract)sa;
    unsigned _Accum ua_from_usf = (unsigned _Accum)usf;
    
    /* Comparisons after conversions - likely to trigger range analysis */
    if (sf_from_sa > 0.5r) {
        global_result += 256;
    }
    
    if (ua_from_usf < 0.25urk) {
        global_result += 512;
    }
    
    /* Complex comparison involving converted values */
    if (sa > (signed _Accum)lf || (sa == (signed _Accum)lf && usf > 0.5ur)) {
        global_result += 1024;
    }
    
    return global_result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int final_result = 0;
    
    /* Call test functions in loop to ensure multiple executions */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        final_result += test_saturating_accum_comparisons(seed);
        final_result += test_fract_range_analysis(i % 10 + 1);
        final_result += test_mixed_type_conversions(seed % 100);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
