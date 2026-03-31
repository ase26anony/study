/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Global side-effect to prevent optimization */
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
    f1 = 0.5r;       /* Binary: 0.1 */
    f2 = -0.75r;     /* Binary: -0.11 */
    
    /* Introduce uncertainty using volatile seed */
    sf3 = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;           /* 0.1 - 0.125 = -0.025 */
    f1 = f1 * f2;              /* 0.5 * -0.75 = -0.375 */
    
    /* Critical comparisons - these should trigger range analysis */
    if (sf1 > 0.0r) {
        global_result += 1;
    }
    
    if (sf3 == -0.125r) {
        global_result += 2;
    }
    
    /* Comparison after cast */
    if ((signed _Fract)sf1 < f2) {
        global_result += 4;
    }
    
    /* Additional comparison with constant */
    if (f1 > -0.5r) {
        global_result += 8;
    }
    
    /* Complex comparison chain */
    signed short _Fract temp = sf1 + sf3;
    if (temp > 0.25r || temp < -0.25r) {
        global_result += 16;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int test_accum_saturation(volatile int seed) {
    /* Use accum types with different sizes */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    long _Accum la1;
    
    /* Initialize with constants */
    sa1 = 0.1k;           /* Binary representation matters */
    sa2 = -0.125k;
    ua1 = 0.5uk;
    
    /* Use volatile seed to prevent constant folding */
    la1 = (long _Accum)(seed) * 0.01lk;
    
    /* Operations that may saturate */
    sa1 = sa1 + sa2;      /* Potential underflow */
    ua1 = ua1 * ua1;      /* 0.5 * 0.5 = 0.25, safe */
    
    /* Cast between types with different scaling */
    signed _Fract sf_from_acc = (signed _Fract)sa1;
    
    /* Critical comparisons for accum types */
    if (sa1 > 0.0k) {
        global_result += 32;
    }
    
    if (ua1 == 0.25uk) {
        global_result += 64;
    }
    
    /* Comparison that might trigger the specific uncovered condition */
    if (la1 > 0.5lk || la1 < -0.5lk) {
        global_result += 128;
    }
    
    /* Mixed-type comparison */
    if ((signed _Accum)sf_from_acc < sa2) {
        global_result += 256;
    }
    
    /* Additional saturation-prone operation */
    sa1 = sa1 * 10.0k;    /* Could overflow */
    
    if (sa1 > 1.0k || sa1 < -1.0k) {
        global_result += 512;
    }
    
    return global_result;
}

__attribute__((noinline, noipa))
int test_unsigned_fract_operations(volatile int seed) {
    unsigned short _Fract usf1, usf2;
    unsigned _Fract uf1;
    
    /* Initialize with constants */
    usf1 = 0.75ur;
    usf2 = 0.125ur;
    uf1 = 0.99ur;
    
    /* Use seed for variability */
    usf1 = (unsigned short _Fract)(seed % 128) / 128.0ur;
    
    /* Arithmetic */
    usf2 = usf1 + usf2;    /* Could saturate near 1.0 */
    uf1 = uf1 - usf1;      /* Could underflow near 0.0 */
    
    /* Comparisons */
    if (usf2 > 0.5ur) {
        global_result += 1024;
    }
    
    if (uf1 == 0.0ur) {
        global_result += 2048;
    }
    
    /* Cast to signed and compare */
    if ((signed _Fract)usf1 < 0.0r) {
        global_result += 4096;  /* Should never happen for unsigned */
    }
    
    return global_result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int i;
    
    printf("Starting fixed-point range analysis tests...\n");
    
    for (i = 0; i < iterations; i++) {
        /* Vary the seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions to exercise different fixed-point types */
        test_signed_fract_range(seed);
        test_accum_saturation(seed);
        test_unsigned_fract_operations(seed);
        
        /* Additional direct operations in main to increase coverage */
        {
            signed _Accum sa = (signed _Accum)(seed % 100) / 100.0k;
            signed short _Fract sf = 0.5r;
            
            /* Comparison that might trigger the target condition */
            if (sa > 0.5k) {
                global_result += 8192;
            }
            
            /* Cast and compare */
            if ((signed _Accum)sf == 0.5k) {
                global_result += 16384;
            }
        }
    }
    
    printf("Final result: %d\n", global_result);
    printf("Test completed. Check if the uncovered lines in fixed-value.cc were executed.\n");
    
    return global_result != 0 ? 0 : 1;
}
