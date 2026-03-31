/* fixed-value-coverage.c
 * Designed to trigger uncovered lines 264-277 in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-value-coverage.c -o fixed-value-coverage
 */

#include <stdio.h>

/* Global side effect to prevent dead code elimination */
volatile int global_result = 0;

/* Helper functions marked to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int fixed_point_operations_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned _Accum ua1, ua2;
    signed _Accum sa1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;          /* Binary: 0.0001100110011... */
    sf2 = -0.125r;       /* Binary: -0.001 exactly */
    sf3 = 0.5r;          /* Binary: 0.1 exactly */
    
    /* Use volatile seed to prevent constant folding */
    ua1 = (unsigned _Accum)(seed) * 0.5rk;
    sa1 = (signed _Accum)(seed - 50) * 0.25rk;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;     /* 0.1 - 0.125 = -0.025 */
    ua2 = ua1 * ua1;     /* Could overflow for large seed values */
    
    /* Critical comparisons - these should trigger range analysis */
    if (sf1 > 0.5r) {
        global_result += 1;
    }
    
    if (ua1 == 0.0rk) {
        global_result += 2;
    }
    
    /* Comparison after cast - forces range analysis across types */
    if ((signed _Fract)ua1 < sf2) {
        global_result += 4;
    }
    
    /* More complex comparison chain */
    if (sa1 > 0.75rk || (sa1 == 0.75rk && ua1 > 0.25rk)) {
        global_result += 8;
    }
    
    /* Saturation context through explicit cast */
    signed _Fract sf4 = (signed _Fract)sa1;
    if (sf4 > 0.9r) {
        global_result += 16;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int fixed_point_operations_2(volatile int iter) {
    unsigned short _Fract usf1, usf2;
    long _Accum la1;
    
    /* Initialize with loop-dependent values */
    usf1 = 0.0ur;
    usf2 = 0.5ur;
    
    /* Create value that changes per iteration */
    la1 = (long _Accum)(iter) * 0.1lk;
    
    /* Operations that may saturate */
    for (int i = 0; i < 3; i++) {
        usf1 = usf1 + usf2;  /* Could saturate at 1.0 */
    }
    
    /* Comparisons that should trigger the uncovered condition */
    if (la1 > 0.5lk) {
        global_result += 32;
    }
    
    if (usf1 == 1.0ur) {
        global_result += 64;
    }
    
    /* Cast between different fixed-point types */
    if ((unsigned _Accum)usf1 < 0.75rk) {
        global_result += 128;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int fixed_point_operations_3(volatile int seed) {
    signed _Fract sf1, sf2;
    unsigned long _Accum ula1;
    
    /* Use negative values to test signed range analysis */
    sf1 = -0.1r;
    sf2 = -0.9r;
    
    /* Create value that could be near bounds */
    ula1 = (unsigned long _Accum)(seed * 2) * 0.125lrk;
    
    /* Arithmetic near saturation points */
    sf1 = sf1 * 2.0r;    /* -0.2 */
    sf2 = sf2 + 0.5r;    /* -0.4 */
    
    /* Comparisons that should exercise the max_r/min_r logic */
    if (sf1 > 0.0r) {
        global_result += 256;
    }
    
    if (sf2 < -0.5r) {
        global_result += 512;
    }
    
    /* Complex condition similar to the uncovered code */
    if (ula1 > 0.25lrk || (ula1 == 0.25lrk && sf1 > -0.1r)) {
        global_result += 1024;
    }
    
    return global_result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    
    printf("Starting fixed-point coverage test...\n");
    
    /* Loop to force dynamic range analysis */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call all fixed-point operation functions */
        fixed_point_operations_1(seed % 100);
        fixed_point_operations_2(i);
        fixed_point_operations_3(seed % 50);
        
        /* Vary the seed to create different value ranges */
        if (i % 10 == 0) {
            seed = seed ^ 0x55555555;
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed. Check if lines 264-277 in fixed-value.cc were executed.\n");
    
    return global_result != 0 ? 0 : 1;
}
