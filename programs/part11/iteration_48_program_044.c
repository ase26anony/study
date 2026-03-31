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
    signed _Fract f1, f2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Non-power-of-two fraction */
    sf2 = -0.125r;   /* Exact power-of-two fraction */
    f1 = 0.5r;       /* Common boundary value */
    f2 = -0.75r;     /* Negative value */
    
    /* Introduce uncertainty using volatile seed */
    sf3 = (signed short _Fract)((seed % 100) * 0.01r);
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;          /* 0.1 + (-0.125) = -0.025 */
    f1 = f1 * f1;             /* 0.5 * 0.5 = 0.25 */
    
    /* Critical comparisons - these should trigger range analysis */
    int result = 0;
    
    /* Comparison against positive constant */
    if (sf1 > 0.0r) {
        result += 1;
    }
    
    /* Comparison against negative constant */
    if (f2 < -0.5r) {
        result += 2;
    }
    
    /* Equality comparison */
    if (sf3 == 0.0r) {
        result += 4;
    }
    
    /* Complex comparison chain */
    if (f1 > 0.1r || (f1 == 0.1r && sf2 < -0.1r)) {
        result += 8;
    }
    
    /* Cast and compare - forces range analysis across types */
    signed _Fract casted = (signed _Fract)sf3;
    if (casted > 0.25r) {
        result += 16;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_accum_saturation(volatile int seed) {
    /* Use accum types with different sizes */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    long _Accum la1;
    
    /* Initialize with values that may saturate */
    sa1 = 0.5rk;
    sa2 = -0.25rk;
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.01rk;
    la1 = 0.9999999999lk;  /* Very close to 1.0 */
    
    /* Operations that could overflow */
    sa1 = sa1 + sa1;        /* 0.5 + 0.5 = 1.0 (may saturate depending on type) */
    sa2 = sa2 - 0.8rk;      /* -0.25 - 0.8 = -1.05 (may saturate) */
    
    /* Multiplication that could overflow */
    ua1 = ua1 * ua1;
    
    /* Cast between different fixed-point types */
    signed short _Fract sf_from_acc = (signed short _Fract)sa1;
    signed _Accum sa_from_long = (signed _Accum)la1;
    
    int result = 0;
    
    /* Comparisons that should trigger the specific uncovered condition */
    /* These compare against bounds that may be computed as double-int */
    if (sa1 > 0.75rk) {
        result += 1;
    }
    
    if (sa2 < -0.9rk) {
        result += 2;
    }
    
    /* Equality comparison with non-zero */
    if (ua1 == 0.0rk) {
        result += 4;
    }
    
    /* Complex condition similar to the uncovered code pattern */
    if (sa1 > 0.8rk || (sa1 == 0.8rk && ua1 > 0.1rk)) {
        result += 8;
    }
    
    /* Comparison after cast */
    if (sf_from_acc > 0.0r) {
        result += 16;
    }
    
    /* Near-boundary comparison */
    if (sa_from_long > 0.999999999lk) {
        result += 32;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_mixed_types_operations(volatile int seed) {
    /* Mix different fixed-point types */
    unsigned short _Fract usf1, usf2;
    signed long _Accum sla1;
    unsigned _Fract uf1;
    
    /* Initialize with various constants */
    usf1 = 0.33333ur;      /* Approximation of 1/3 */
    usf2 = 0.0ur;
    sla1 = 0.000000001lk;  /* Very small value */
    uf1 = 0.999999ur;      /* Very close to 1.0 */
    
    /* Use seed to create uncertainty */
    usf2 = (unsigned short _Fract)((seed % 50) * 0.02ur);
    
    /* Operations that might saturate */
    usf1 = usf1 + usf1 + usf1;  /* Approximately 1.0, may saturate */
    uf1 = uf1 + 0.000001ur;     /* May saturate to 1.0 */
    
    /* Multiplication with potential overflow */
    sla1 = sla1 * 1000000000.0lk;
    
    int result = 0;
    
    /* Multiple comparisons to trigger range analysis */
    if (usf1 > 0.9ur) {
        result += 1;
    }
    
    if (usf2 < 0.5ur) {
        result += 2;
    }
    
    /* Equality with non-trivial constant */
    if (uf1 == 1.0ur) {
        result += 4;
    }
    
    /* Complex condition */
    if (sla1 > 0.5lk || (sla1 == 0.5lk && usf1 > 0.5ur)) {
        result += 8;
    }
    
    /* Boundary comparisons */
    if (usf1 > 0.99ur && usf2 > 0.01ur) {
        result += 16;
    }
    
    return result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int total_result = 0;
    
    /* Loop to force dynamic analysis with varying values */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions */
        total_result ^= test_signed_fract_range(seed);
        total_result ^= test_accum_saturation(seed);
        total_result ^= test_mixed_types_operations(seed);
        
        /* Use result to prevent dead code elimination */
        global_result += total_result;
    }
    
    /* Print result to ensure code isn't optimized away */
    printf("Final result: %d (global: %d)\n", total_result, global_result);
    
    return total_result != 0 ? 0 : 1;
}
