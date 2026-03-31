/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Global to prevent dead code elimination */
volatile int global_result = 0;

/* Helper function with attributes to prevent constant folding and inlining */
__attribute__((noinline, noipa))
int fixed_point_operations_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    uf1 = 0.5r;      /* Binary: 0.1 exactly */
    
    /* Use volatile seed to introduce uncertainty */
    sa1 = (signed _Accum)(seed) * 0.5rk;
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.25rk;
    
    /* Arithmetic that could overflow/saturate */
    sf3 = sf1 + sf2;  /* 0.1 - 0.125 = -0.025 */
    sa2 = sa1 * sa1;  /* Square - could overflow */
    
    /* Critical comparisons that should trigger range analysis */
    int result = 0;
    
    /* Comparison 1: sf3 > 0.0r */
    if (sf3 > 0.0r) {
        result += 1;
    }
    
    /* Comparison 2: sa1 == 0.0rk */
    if (sa1 == 0.0rk) {
        result += 2;
    }
    
    /* Comparison 3: ua1 < 0.75rk */
    if (ua1 < 0.75rk) {
        result += 4;
    }
    
    /* Comparison after cast - different scaling */
    if ((signed _Fract)sa1 < sf2) {
        result += 8;
    }
    
    /* More complex comparison chain */
    if (sf1 > 0.5r || (sf1 == 0.5r && uf1 > 0.25r)) {
        result += 16;
    }
    
    /* Saturation context: explicit cast with potential overflow */
    signed _Fract sf4 = (signed _Fract)sa2;
    if (sf4 > 0.9r) {
        result += 32;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int fixed_point_operations_2(volatile int iter) {
    long _Accum la1, la2;
    unsigned long _Accum ula1;
    signed _Fract sf1, sf2;
    
    /* Initialize with values that may approach bounds */
    la1 = 0.999999999999999999lrk;  /* Very close to 1.0 */
    ula1 = (unsigned long _Accum)(iter) * 0.0009765625lrk;  /* 1/1024 */
    
    /* Operations that could trigger saturation bounds calculation */
    la2 = la1 + la1;  /* Could overflow for _Accum type */
    
    /* Comparisons with constants near bounds */
    int result = 0;
    
    if (la1 > 0.5lrk) {
        result += 1;
    }
    
    if (ula1 == 0.0lrk) {
        result += 2;
    }
    
    /* Cast between different fixed-point types */
    sf1 = (signed _Fract)la2;
    sf2 = -0.5r;
    
    if (sf1 > sf2) {
        result += 4;
    }
    
    /* Complex condition similar to the uncovered code pattern */
    if (la1 > 0.999lrk || (la1 == 0.999lrk && ula1 > 0.0005lrk)) {
        result += 8;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int fixed_point_operations_3(volatile int seed) {
    /* Test with negative values and saturation */
    signed _Accum sa1, sa2, sa3;
    signed short _Fract sf1;
    
    /* Initialize with negative values */
    sa1 = (signed _Accum)(seed - 50) * 0.02rk;
    sa2 = -0.9rk;
    
    /* Multiplication that could underflow */
    sa3 = sa1 * sa2;
    
    /* Comparisons with negative bounds */
    int result = 0;
    
    if (sa1 < -0.5rk) {
        result += 1;
    }
    
    if (sa3 > 0.0rk) {
        result += 2;
    }
    
    /* Cast to _Fract with potential saturation */
    sf1 = (signed short _Fract)sa3;
    
    if (sf1 == -1.0r) {  /* Minimum value for signed short _Fract */
        result += 4;
    }
    
    /* Comparison chain */
    if (sa1 < -0.8rk || (sa1 == -0.8rk && sa3 > -0.1rk)) {
        result += 8;
    }
    
    return result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int total_result = 0;
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7FFF;
        
        /* Call different fixed-point operations */
        total_result += fixed_point_operations_1(seed);
        total_result += fixed_point_operations_2(i);
        total_result += fixed_point_operations_3(seed);
        
        /* Use volatile to prevent optimization */
        global_result = total_result;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    printf("Global result: %d\n", global_result);
    
    return total_result != 0 ? 0 : 1;
}
