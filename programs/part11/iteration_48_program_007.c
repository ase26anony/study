/* fixed-point-test.c */
#include <stdio.h>

/* Prevent constant folding and inlining */
volatile int iter = 100;
volatile int seed = 1;

/* Global side effect to prevent dead code elimination */
volatile int global_result = 0;

/* Helper function 1: Focus on _Fract types with comparisons */
__attribute__((noinline, noipa))
int test_fract_operations(int base) {
    /* Use various _Fract types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1, uf2;
    signed _Fract f1, f2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;          /* Non-trivial binary fraction */
    sf2 = -0.125r;       /* Exact binary fraction: -1/8 */
    uf1 = 0.5r;          /* Exact binary fraction: 1/2 */
    f1 = 0.33333333r;    /* Approximation of 1/3 */
    
    /* Initialize with volatile-dependent values */
    sf3 = (signed short _Fract)((base & 0xFF) * 0.00392156862745098r); /* ~1/255 */
    uf2 = (unsigned short _Fract)((base % 64) * 0.015625r); /* 1/64 increments */
    f2 = (signed _Fract)(base * 0.01r);
    
    /* Arithmetic operations that could saturate */
    sf1 = sf1 + sf2;                    /* 0.1 - 0.125 = -0.025 */
    uf1 = uf1 + uf2;                    /* Could overflow for unsigned */
    f1 = f1 * f2;                       /* Multiplication could overflow */
    
    /* CRITICAL: Comparisons that should trigger range analysis */
    int result = 0;
    
    /* Comparison 1: Direct comparison with constant */
    if (sf1 > 0.5r) {                   /* Likely false, but forces analysis */
        result |= 1;
    }
    
    /* Comparison 2: Equality comparison */
    if (uf1 == 0.0r) {                  /* Forces equality check */
        result |= 2;
    }
    
    /* Comparison 3: Less-than with negative constant */
    if (f1 < -0.25r) {                  /* Could be true depending on base */
        result |= 4;
    }
    
    /* Comparison 4: Complex comparison chain */
    if (sf3 > 0.1r || (sf3 == 0.1r && uf2 > 0.5r)) {
        result |= 8;
    }
    
    /* Cast between types with different scaling */
    signed _Fract f3 = (signed _Fract)sf1;
    if (f3 > 0.75r) {
        result |= 16;
    }
    
    return result;
}

/* Helper function 2: Focus on _Accum types with saturation */
__attribute__((noinline, noipa))
int test_accum_operations(int base) {
    /* Use various _Accum types */
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1, ua2;
    signed long _Accum sla1;
    
    /* Initialize with constants */
    sa1 = 0.5k;                         /* Exact: 1/2 */
    sa2 = -0.125k;                      /* Exact: -1/8 */
    ua1 = 0.33333333k;                  /* Approximation of 1/3 */
    
    /* Initialize with volatile-dependent values */
    sa3 = (signed _Accum)(base) * 0.01k;
    ua2 = (unsigned _Accum)((base % 100) * 0.02k);
    sla1 = (signed long _Accum)(base * 0.005k);
    
    /* Arithmetic that could overflow/saturate */
    sa1 = sa1 + sa2;                    /* 0.5 - 0.125 = 0.375 */
    ua1 = ua1 * ua2;                    /* Multiplication could overflow */
    sa3 = sa3 * sa3;                    /* Square could overflow */
    
    /* CRITICAL: More comparisons for range analysis */
    int result = 0;
    
    /* Comparison with maximum representable values */
    if (sa1 > 0.9999999999999999k) {    /* Near maximum */
        result |= 32;
    }
    
    /* Equality with zero after operations */
    if (ua1 == 0.0k) {
        result |= 64;
    }
    
    /* Comparison chain similar to uncovered code pattern */
    signed _Accum temp = sa3 * 2.0k;
    if (temp > 1.0k || (temp == 1.0k && ua2 > 0.5k)) {
        result |= 128;
    }
    
    /* Cast from _Accum to _Fract (different scaling) */
    signed _Fract sf_from_acc = (signed _Fract)sa1;
    if (sf_from_acc < -0.5r) {
        result |= 256;
    }
    
    /* Operations that specifically test saturation bounds */
    signed _Accum sat_test = sa2 * 100.0k;  /* Could overflow negative range */
    if (sat_test < -0.9k) {
        result |= 512;
    }
    
    return result;
}

/* Helper function 3: Mixed types and explicit saturation context */
__attribute__((noinline, noipa))
int test_mixed_saturation(int base) {
    signed short _Fract sf = 0.8r;
    unsigned _Accum ua = 0.9k;
    signed _Accum sa;
    
    /* Force saturation context through explicit casts */
    sa = (signed _Accum)sf * 2.0k;      /* 0.8 * 2 = 1.6, may saturate */
    
    /* More comparisons */
    int result = 0;
    
    if (sa > 1.0k) {                    /* Check if saturation occurred */
        result |= 1024;
    }
    
    /* Complex condition with AND/OR */
    if (ua > 0.5k && sf < 0.5r) {
        result |= 2048;
    }
    
    /* Another comparison chain */
    signed _Fract sf2 = (signed _Fract)(base * 0.001r);
    if (sf2 > 0.1r || (sf2 == 0.1r && ua > 0.75k)) {
        result |= 4096;
    }
    
    return result;
}

int main() {
    int total_result = 0;
    
    printf("Starting fixed-point range analysis test...\n");
    
    /* Loop with volatile iteration counter */
    for (int i = 0; i < iter; i++) {
        /* Use volatile seed to prevent constant propagation */
        int current_base = seed + i;
        
        /* Call all test functions */
        total_result ^= test_fract_operations(current_base);
        total_result ^= test_accum_operations(current_base);
        total_result ^= test_mixed_saturation(current_base);
        
        /* Update global side effect */
        global_result += total_result & 0xFF;
    }
    
    printf("Final result: %d (global: %d)\n", total_result, global_result);
    
    /* Return non-zero to indicate execution */
    return (total_result != 0) ? 0 : 1;
}
