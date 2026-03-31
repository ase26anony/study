/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

volatile int global_counter = 0;

/* Helper function with attributes to prevent constant folding and inlining */
__attribute__((noinline, noipa))
int fixed_point_operations_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Non-trivial binary fraction */
    sf2 = -0.125r;   /* Exact binary fraction: -1/8 */
    uf1 = 0.5r;      /* Exact binary fraction: 1/2 */
    
    /* Use volatile seed to introduce uncertainty */
    sa1 = (signed _Accum)(seed) * 0.5rk;
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.25rk;
    
    /* Arithmetic operations that could overflow/saturate */
    sf3 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    
    /* Multiplication that could overflow */
    sa2 = sa1 * sa1;
    
    /* Critical comparisons - these should trigger the range analysis */
    if (sf3 > 0.0r) {
        /* This branch may be taken depending on the actual value */
        global_counter++;
    }
    
    if (sa1 == 0.0rk) {
        /* Comparison against zero */
        global_counter += 2;
    }
    
    /* Comparison with non-zero constant */
    if (ua1 > 0.5rk) {
        global_counter += 3;
    }
    
    /* Cast between types with different scaling */
    signed _Fract cast_result = (signed _Fract)sa1;
    if (cast_result < sf2) {
        global_counter += 4;
    }
    
    /* Complex comparison chain */
    if (sa2 > 0.25rk || (sa2 == 0.25rk && ua1 > 0.75rk)) {
        global_counter += 5;
    }
    
    /* Return a value based on the comparisons */
    return (sf3 > -0.1r) ? 1 : 0;
}

__attribute__((noinline, noipa))
int fixed_point_operations_2(volatile int iter) {
    long _Fract lf1, lf2;
    unsigned long _Fract ulf1;
    signed long _Accum sla1;
    
    /* Initialize with loop-dependent values */
    lf1 = (long _Fract)(iter % 100) / 100.0r;
    lf2 = -0.33333333333333333333r;  /* Approx -1/3 */
    
    /* Arithmetic with potential overflow */
    sla1 = (signed long _Accum)lf1 * 2.0rk;
    
    /* Multiple comparisons */
    if (lf1 > 0.5r) {
        global_counter += 10;
    }
    
    if (sla1 < -1.0rk) {
        global_counter += 20;
    }
    
    /* Comparison that might trigger the specific uncovered condition */
    ulf1 = (unsigned long _Fract)(iter & 0x7F) / 128.0r;
    if (ulf1 > 0.9r || (ulf1 == 0.9r && lf1 > 0.0r)) {
        global_counter += 30;
    }
    
    return (lf1 > lf2) ? 1 : 0;
}

__attribute__((noinline, noipa))
int saturation_tests(volatile int val) {
    /* Tests specifically designed for saturation contexts */
    signed _Accum sa_max = 0.99999999999999999999rk;  /* Near maximum */
    signed _Accum sa_min = -0.99999999999999999999rk; /* Near minimum */
    unsigned _Accum ua_max = 0.99999999999999999999rk;
    
    /* Operations that should trigger saturation logic */
    signed _Accum sa_sum = sa_max + 0.1rk;  /* Should saturate */
    signed _Accum sa_diff = sa_min - 0.1rk; /* Should saturate */
    unsigned _Accum ua_prod = ua_max * 1.1rk; /* Should saturate */
    
    /* Comparisons involving saturated values */
    if (sa_sum > 0.5rk) {
        global_counter += 100;
    }
    
    if (sa_diff < -0.5rk) {
        global_counter += 200;
    }
    
    if (ua_prod == 0.99999999999999999999rk) {
        global_counter += 300;
    }
    
    /* Cast with potential saturation */
    signed short _Fract saturated_cast = (signed short _Fract)sa_max;
    if (saturated_cast > 0.9r) {
        global_counter += 400;
    }
    
    return (sa_sum > sa_diff) ? 1 : 0;
}

int main() {
    volatile int iterations = 1000;
    volatile int seed = 12345;
    int result = 0;
    
    printf("Starting fixed-point coverage test...\n");
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary the seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all fixed-point operation functions */
        result += fixed_point_operations_1(seed % 100);
        result += fixed_point_operations_2(i);
        result += saturation_tests(seed % 50);
        
        /* Additional direct comparisons in main */
        unsigned _Accum ua_temp = (unsigned _Accum)(i % 256) / 256.0rk;
        if (ua_temp > 0.5rk && ua_temp < 0.75rk) {
            result++;
        }
    }
    
    printf("Final result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    return result > 0 ? 0 : 1;
}
