/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper function with attributes to prevent constant folding and inlining */
__attribute__((noinline, noipa))
int fixed_point_operations_1(volatile int seed) {
    /* Declare various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    uf1 = 0.5r;      /* Binary: 0.1 exactly */
    
    /* Use volatile seed to introduce uncertainty and prevent constant propagation */
    sa1 = (signed _Accum)(seed) * 0.5rk;  /* Multiplication that may saturate */
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.25rk;
    
    /* Arithmetic operations that could overflow/saturate */
    sf3 = sf1 + sf2;  /* 0.1 + (-0.125) = -0.025 */
    sa2 = sa1 * sa1;  /* Square - could overflow for large seed values */
    
    /* Critical comparisons - these should trigger the range analysis logic */
    if (sf3 > 0.0r) {
        /* This branch may be taken depending on the actual value */
        global_result += 1;
    }
    
    if (sa1 == 0.0rk) {
        global_result += 2;
    }
    
    /* Comparison after cast - may trigger saturation bounds calculation */
    if ((signed _Fract)sa2 < sf2) {
        global_result += 4;
    }
    
    /* More comparisons with constants */
    if (uf1 >= 0.75r) {
        global_result += 8;
    }
    
    /* Complex comparison chain that might trigger the specific condition */
    signed _Accum temp = sa1 + 0.25rk;
    if (temp > 1.0rk || (temp == 1.0rk && ua1 > 0.5rk)) {
        global_result += 16;
    }
    
    return global_result;
}

/* Second helper function with different fixed-point types */
__attribute__((noinline, noipa))
int fixed_point_operations_2(int iteration) {
    long _Fract lf1, lf2;
    unsigned long _Fract ulf1;
    signed long _Accum sla1;
    
    /* Initialize with values that depend on iteration */
    lf1 = (iteration % 100) * 0.01r;  /* Range: 0.00 to 0.99 */
    lf2 = -0.333r;  /* Approximate -1/3 */
    ulf1 = 0.666r;  /* Approximate 2/3 */
    
    /* Arithmetic that could saturate */
    sla1 = (signed long _Accum)lf1 * 2.0rk;
    
    /* Comparisons that should trigger range analysis */
    if (lf1 > 0.5r) {
        global_result += 32;
    }
    
    if (lf2 < -0.25r && ulf1 > 0.5r) {
        global_result += 64;
    }
    
    /* Cast between different fixed-point types with different scaling */
    if ((signed _Fract)sla1 == 0.0r) {
        global_result += 128;
    }
    
    return global_result;
}

/* Third helper focusing on saturation contexts */
__attribute__((noinline, noipa))
int saturation_operations(volatile int seed) {
    signed _Accum sa_max = 0.999999999rk;  /* Near maximum for signed _Accum */
    signed _Accum sa_min = -1.0rk;         /* Minimum for signed _Accum */
    unsigned _Accum ua_max = 1.999999999rk; /* Near maximum for unsigned _Accum */
    
    /* Operations that should trigger saturation logic */
    signed _Accum sa_result = sa_max + 0.1rk;  /* Should saturate */
    unsigned _Accum ua_result = ua_max * 1.1rk; /* Should saturate */
    
    /* Comparisons involving saturated values */
    if (sa_result > 1.0rk) {
        global_result += 256;
    }
    
    if (ua_result == 2.0rk) {
        global_result += 512;
    }
    
    /* Cast that requires saturation */
    signed short _Fract sf_from_acc = (signed short _Fract)sa_min;
    if (sf_from_acc < -0.5r) {
        global_result += 1024;
    }
    
    return global_result;
}

int main() {
    /* Volatile variables to prevent constant propagation */
    volatile int iterations = 100;
    volatile int seed = 1;
    
    printf("Starting fixed-point coverage test...\n");
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < iterations; i++) {
        /* Update seed to create varying input values */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all fixed-point operation functions */
        fixed_point_operations_1(seed % 256);
        fixed_point_operations_2(i);
        saturation_operations(seed % 512);
        
        /* Prevent loop unrolling */
        asm volatile("" : : "r"(global_result));
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed.\n");
    
    return global_result != 0 ? 0 : 1;
}
