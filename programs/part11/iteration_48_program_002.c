/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper functions marked to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int test_signed_fract_range(volatile int seed) {
    /* Use various signed fract types */
    signed short _Fract sf1, sf2, sf3;
    signed _Fract f1, f2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Non-power-of-two fraction */
    sf2 = -0.125r;   /* Exact power-of-two fraction */
    f1 = 0.5r;       /* Another exact fraction */
    
    /* Introduce uncertainty through volatile seed */
    f2 = (signed _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could approach saturation bounds */
    sf3 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    
    /* Multiple comparisons that should trigger range analysis */
    int result = 0;
    
    /* Comparison against positive constant */
    if (sf1 > 0.05r) {
        result += 1;
    }
    
    /* Comparison against negative constant */
    if (sf2 < -0.1r) {
        result += 2;
    }
    
    /* Equality comparison with non-zero */
    if (sf3 == -0.025r) {
        result += 4;
    }
    
    /* Chain of comparisons that may trigger the specific condition */
    signed _Fract f3 = f1 * f2;
    if (f3 > 0.25r || (f3 == 0.25r && f2 > 0.5r)) {
        result += 8;
    }
    
    /* Cast between types with different scaling */
    signed short _Fract sf4 = (signed short _Fract)f3;
    if (sf4 > 0.0r) {
        result += 16;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_accum_saturation(volatile int seed) {
    /* Use accum types which have larger ranges */
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1, ua2;
    
    /* Initialize with mixed constants and volatile-dependent values */
    sa1 = 0.1k;
    sa2 = -0.5k;
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.01k;
    
    /* Arithmetic operations that could overflow/saturate */
    sa3 = sa1 * 10.0k;  /* Could approach max for _Accum */
    
    /* Multiplication that could overflow */
    ua2 = ua1 * ua1;
    
    int result = 0;
    
    /* Comparisons that should trigger bounds checking */
    if (sa3 > 0.0k) {
        result += 1;
    }
    
    /* Comparison with negative constant */
    if (sa2 < -0.25k) {
        result += 2;
    }
    
    /* Complex comparison chain */
    if (ua2 > 0.5k || (ua2 == 0.5k && ua1 > 0.7k)) {
        result += 4;
    }
    
    /* Cast from accum to fract with potential saturation */
    signed _Fract f1 = (signed _Fract)sa3;
    if (f1 > 0.0r) {
        result += 8;
    }
    
    /* Another comparison after scaling */
    signed _Accum sa4 = sa2 / 2.0k;
    if (sa4 == -0.25k) {
        result += 16;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_mixed_types_comparisons(volatile int seed) {
    /* Mix different fixed-point types */
    unsigned short _Fract usf1, usf2;
    signed long _Accum sla1;
    
    /* Initialize with volatile-dependent values */
    usf1 = (unsigned short _Fract)((seed * 37) & 0xFF) / 256.0r;
    usf2 = 0.75r;
    sla1 = (signed long _Accum)seed * 0.001k;
    
    /* Arithmetic */
    unsigned short _Fract usf3 = usf1 + usf2;
    
    int result = 0;
    
    /* Comparisons that may trigger the high/low split logic */
    if (usf3 > 0.8r) {
        result += 1;
    }
    
    /* Equality comparison */
    if (usf1 == 0.0r) {
        result += 2;
    }
    
    /* Cast and compare */
    signed _Accum sa_temp = (signed _Accum)usf3;
    if (sa_temp > 0.5k && sa_temp < 1.0k) {
        result += 4;
    }
    
    /* Complex condition similar to the uncovered code */
    signed long _Accum sla2 = sla1 * 1000.0k;
    if (sla2 > 0.5lk || (sla2 == 0.5lk && sla1 > 0.0005lk)) {
        result += 8;
    }
    
    return result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int total_result = 0;
    
    /* Loop to force dynamic analysis with varying values */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions to exercise different paths */
        total_result += test_signed_fract_range(seed);
        total_result += test_accum_saturation(seed);
        total_result += test_mixed_types_comparisons(seed);
        
        /* Use volatile to prevent loop unrolling */
        asm volatile("" : "+r" (total_result));
    }
    
    /* Store to global to prevent dead code elimination */
    global_result = total_result;
    
    printf("Result: %d\n", total_result % 1000);
    return total_result % 256;
}
