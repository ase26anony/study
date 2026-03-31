/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
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
    sf1 = 0.1r;      /* Binary: ~0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    f1 = 0.5r;       /* Binary: 0.1 exactly */
    f2 = -0.75r;     /* Binary: -0.11 exactly */
    
    /* Introduce uncertainty using volatile seed */
    sf3 = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could approach saturation bounds */
    sf1 = sf1 + sf2;           /* 0.1 - 0.125 = -0.025 */
    f1 = f1 * f1;              /* 0.25 */
    
    /* Critical comparisons - these should trigger range analysis */
    if (sf1 > 0.0r) {
        global_result += 1;
    }
    
    if (f1 == 0.25r) {
        global_result += 2;
    }
    
    /* Comparison with constant that may trigger the specific condition */
    if (sf3 < -0.5r) {
        global_result += 4;
    }
    
    /* Additional arithmetic that could overflow */
    signed short _Fract sf4 = sf1 * sf3;
    if (sf4 > 0.1r) {
        global_result += 8;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int test_accumulator_types(volatile int seed) {
    /* Use accumulator types with different sizes */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1, ua2;
    long _Accum la1;
    
    /* Initialize with volatile-dependent values */
    sa1 = (signed _Accum)(seed - 50) * 0.1rk;
    ua1 = (unsigned _Accum)(seed % 100) * 0.01rk;
    la1 = (long _Accum)(seed * 2) * 0.05rk;
    
    /* Constants that may trigger specific bounds calculations */
    sa2 = 0.999999999rk;      /* Near upper bound for signed accum */
    ua2 = 0.000000001rk;      /* Near lower bound for unsigned accum */
    
    /* Arithmetic operations that could saturate */
    sa1 = sa1 + sa2;
    ua1 = ua1 * ua2;
    
    /* Critical comparisons for accum types */
    if (sa1 > 0.5rk) {
        global_result += 16;
    }
    
    if (ua1 == 0.0rk) {
        global_result += 32;
    }
    
    /* Cross-type comparison after cast */
    if ((signed _Fract)sa1 < 0.1r) {
        global_result += 64;
    }
    
    /* Multiplication that could overflow */
    long _Accum la2 = la1 * la1;
    if (la2 > 10.0rk) {
        global_result += 128;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int test_mixed_types_saturation(volatile int seed) {
    /* Mix different fixed-point types to trigger range calculations */
    signed short _Fract sf;
    unsigned _Fract uf;
    signed _Accum sa;
    
    /* Initialize with values that may approach bounds */
    sf = (signed short _Fract)(seed % 128 - 64) / 128.0r;
    uf = (unsigned _Fract)(seed % 256) / 256.0r;
    sa = (signed _Accum)(seed - 100) * 0.02rk;
    
    /* Operations that could saturate */
    sf = sf + 0.9r;           /* Could saturate for short fract */
    uf = uf * 2.0r;           /* Could saturate for unsigned fract */
    sa = sa * 50.0rk;         /* Could saturate for signed accum */
    
    /* Multiple comparisons to trigger the uncovered condition */
    if (sf > 0.5r) {
        global_result += 256;
    }
    
    if (uf < 0.1r) {
        global_result += 512;
    }
    
    if (sa == 0.0rk) {
        global_result += 1024;
    }
    
    /* Complex condition similar to the uncovered code */
    signed _Accum sa2 = sa + 1.0rk;
    if (sa2 > 2.0rk || (sa2 == 2.0rk && sa > 0.0rk)) {
        global_result += 2048;
    }
    
    return global_result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    
    printf("Starting fixed-point range analysis test...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions to exercise different paths */
        test_signed_fract_range(seed);
        test_accumulator_types(seed);
        test_mixed_types_saturation(seed);
        
        /* Additional direct comparisons in main */
        signed _Fract f = (signed _Fract)(seed % 100) / 100.0r;
        if (f > 0.5r) {
            global_result += 4096;
        }
        
        if (f < -0.5r) {
            global_result += 8192;
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed. Check if fixed-value.cc lines 264-277 were executed.\n");
    
    return global_result != 0 ? 0 : 1;
}
