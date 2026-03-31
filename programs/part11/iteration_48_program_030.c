/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent constant folding and inlining */
#define NOOPT __attribute__((noinline, noipa))

/* Global side effect to prevent dead code elimination */
volatile int global_result = 0;

/* Function 1: Tests with signed _Fract types */
NOOPT int test_signed_fract_range(volatile int seed) {
    signed short _Fract sf1, sf2, sf3;
    signed _Fract f1, f2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;      /* Binary: 0.0001100110011... */
    sf2 = -0.125r;   /* Binary: -0.001 exactly */
    f1 = 0.5r;       /* Binary: 0.1 */
    f2 = -0.75r;     /* Binary: -0.11 */
    
    /* Introduce uncertainty using volatile seed */
    sf3 = (signed short _Fract)(seed % 256) * 0.00390625r; /* 1/256 */
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;          /* 0.1 - 0.125 = -0.025 */
    f1 = f1 * f2;             /* 0.5 * -0.75 = -0.375 */
    
    /* Critical comparisons - these should trigger range analysis */
    if (sf1 > 0.0r) {
        global_result += 1;
    }
    
    if (sf3 == 0.0r) {
        global_result += 2;
    }
    
    if (f1 < -0.5r) {
        global_result += 4;
    }
    
    /* Comparison after cast - forces range analysis */
    if ((signed _Fract)sf3 > f2) {
        global_result += 8;
    }
    
    /* Additional comparison that might hit the uncovered condition */
    signed short _Fract temp = sf1 * 2.0r;
    if (temp > 0.1r || temp == 0.1r) {
        global_result += 16;
    }
    
    return global_result;
}

/* Function 2: Tests with _Accum types and saturation */
NOOPT int test_accum_saturation(volatile int iter) {
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1, ua2;
    
    /* Initialize with various constants */
    sa1 = 0.1rk;           /* 0.1 in accum format */
    sa2 = -0.125rk;        /* -0.125 */
    ua1 = 0.5urk;          /* 0.5 unsigned */
    
    /* Use loop iteration to create varying values */
    sa3 = (signed _Accum)(iter) * 0.01rk;
    ua2 = (unsigned _Accum)(iter % 100) * 0.02urk;
    
    /* Operations that could saturate */
    sa1 = sa1 + sa2;       /* Might underflow */
    sa3 = sa3 * sa3;       /* Could overflow */
    
    /* Cast between types with different scaling */
    signed short _Fract sf_from_acc = (signed short _Fract)sa3;
    
    /* Critical comparisons for accum types */
    if (sa1 > 0.0rk) {
        global_result += 32;
    }
    
    if (ua1 == 0.5urk) {
        global_result += 64;
    }
    
    /* Complex comparison that might trigger the specific uncovered condition */
    if (sa3 > 1.0rk || (sa3 == 1.0rk && ua2 > 0.5urk)) {
        global_result += 128;
    }
    
    /* Comparison after arithmetic */
    signed _Accum sa_product = sa1 * sa2;
    if (sa_product < -0.01rk) {
        global_result += 256;
    }
    
    return sf_from_acc > 0.0r ? global_result + 512 : global_result;
}

/* Function 3: Mixed types and explicit saturation contexts */
NOOPT int test_mixed_fixed_point(volatile int seed) {
    unsigned short _Fract usf1, usf2;
    long _Fract lf1, lf2;
    
    /* Initialize with volatile-dependent values */
    usf1 = (unsigned short _Fract)(seed & 0xFF) * 0.003921568627r; /* ~1/255 */
    usf2 = 0.8r;
    lf1 = 0.33333333333333333333r;  /* 1/3 approximation */
    lf2 = -0.66666666666666666666r; /* -2/3 approximation */
    
    /* Arithmetic with potential overflow */
    usf1 = usf1 + usf2;    /* Could saturate for unsigned */
    lf1 = lf1 * lf2;       /* 1/3 * -2/3 = -2/9 */
    
    /* Comparisons that should trigger range analysis */
    if (usf1 > 0.9r) {
        global_result += 1024;
    }
    
    if (lf1 == -0.22222222222222222222r) {
        global_result += 2048;
    }
    
    /* Nested comparisons */
    if (usf1 < 1.0r && usf1 > 0.5r) {
        global_result += 4096;
    }
    
    /* Cast and compare */
    signed _Fract casted = (signed _Fract)lf1;
    if (casted > -0.3r || (casted == -0.3r && usf2 > 0.7r)) {
        global_result += 8192;
    }
    
    return global_result;
}

/* Main function with loop to force dynamic analysis */
int main(void) {
    volatile int iterations = 100;
    volatile int seed = 1;
    int final_result = 0;
    
    printf("Starting fixed-point range analysis test...\n");
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary the seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions with varying inputs */
        final_result += test_signed_fract_range(seed % 1000);
        final_result += test_accum_saturation(i);
        final_result += test_mixed_fixed_point(seed);
        
        /* Prevent loop unrolling */
        if (i % 10 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Final result: %d\n", final_result);
    printf("Global side effect: %d\n", global_result);
    
    return final_result != 0 ? 0 : 1;
}
