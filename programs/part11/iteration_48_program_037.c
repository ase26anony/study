/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math -dA fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Global to prevent optimization */
volatile int g_result = 0;

/* Helper function with attributes to prevent inlining and constant propagation */
__attribute__((noinline, noipa))
int fixed_point_comparisons_1(volatile int seed) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2;
    unsigned short _Fract uf1;
    signed _Accum sa1;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;        /* Will be approximated */
    sf2 = -0.125r;     /* Exact binary: -1/8 */
    uf1 = 0.5r;        /* Exact binary: 1/2 */
    
    /* Initialize with volatile seed to prevent constant folding */
    sa1 = (signed _Accum)(seed) * 0.5rk;
    ua1 = (unsigned _Accum)(seed & 0xFF) * 0.25rk;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;   /* 0.1 - 0.125 = -0.025 */
    sa1 = sa1 * sa1;   /* Could overflow */
    ua1 = ua1 + ua1;   /* Could saturate for unsigned */
    
    /* CRITICAL: Comparisons that should trigger the uncovered range analysis */
    /* These comparisons involve fixed-point constants and variables */
    if (sf1 > 0.5r) {
        g_result += 1;
    }
    
    if (ua1 == 0.0rk) {
        g_result += 2;
    }
    
    /* Comparison after cast - may trigger saturation bounds calculation */
    if ((signed _Fract)ua1 < sf2) {
        g_result += 4;
    }
    
    /* More comparisons with different constants */
    if (sa1 > 0.75rk) {
        g_result += 8;
    }
    
    if (uf1 <= 0.25r) {
        g_result += 16;
    }
    
    /* Complex comparison chain */
    if (sf1 > 0.0r || (sf1 == 0.0r && uf1 > 0.75r)) {
        g_result += 32;
    }
    
    return g_result;
}

/* Second helper function with different fixed-point types */
__attribute__((noinline, noipa))
int fixed_point_comparisons_2(volatile int iter) {
    long _Fract lf1, lf2;
    signed _Accum sa2;
    unsigned short _Fract uf2;
    
    /* Initialize with values that may approach bounds */
    lf1 = 0.999999r;   /* Close to maximum */
    lf2 = -0.999999r;  /* Close to minimum */
    sa2 = (signed _Accum)(iter) * 0.1rk;
    uf2 = 0.0r;
    
    /* Loop to create varying values */
    for (int i = 0; i < (iter & 3); i++) {
        uf2 = uf2 + 0.333r;  /* 1/3 - repeating binary */
    }
    
    /* Arithmetic that could saturate */
    lf1 = lf1 + 0.0001r;
    lf2 = lf2 - 0.0001r;
    sa2 = sa2 / 2.0rk;
    
    /* CRITICAL: More comparisons to trigger range analysis */
    if (lf1 > 0.5r) {
        g_result += 64;
    }
    
    if (lf2 < -0.5r) {
        g_result += 128;
    }
    
    /* Comparison that may trigger the specific uncovered condition */
    if (sa2 > 0.0rk || (sa2 == 0.0rk && uf2 > 0.0r)) {
        g_result += 256;
    }
    
    /* Cast between different fixed-point types */
    if ((unsigned short _Fract)sa2 > uf2) {
        g_result += 512;
    }
    
    return g_result;
}

/* Third helper with saturation-prone operations */
__attribute__((noinline, noipa))
int fixed_point_saturation_tests(volatile int seed) {
    signed _Accum sa3, sa4;
    unsigned _Accum ua3;
    signed short _Fract sf3;
    
    /* Initialize near bounds */
    sa3 = 0.99rk;      /* Near maximum for some representations */
    sa4 = -0.99rk;     /* Near minimum */
    ua3 = 0.999999rk;  /* Near maximum for unsigned */
    sf3 = 0.0r;
    
    /* Operations likely to cause saturation */
    sa3 = sa3 * 1.1rk;     /* Likely overflow */
    sa4 = sa4 * 1.1rk;     /* Likely underflow */
    ua3 = ua3 + 0.1rk;     /* Likely saturate */
    
    /* Vary sf3 based on seed */
    for (int i = 0; i < (seed & 7); i++) {
        sf3 = sf3 + 0.142857r;  /* 1/7 - repeating binary */
    }
    
    /* CRITICAL: Comparisons after saturation-prone operations */
    if (sa3 > 0.5rk) {
        g_result += 1024;
    }
    
    if (sa4 < -0.5rk) {
        g_result += 2048;
    }
    
    if (ua3 == 1.0rk) {
        g_result += 4096;
    }
    
    /* Complex condition similar to uncovered code */
    if (sf3 > 0.75r || (sf3 == 0.75r && sa3 > 0.0rk)) {
        g_result += 8192;
    }
    
    return g_result;
}

int main() {
    /* Volatile to prevent constant propagation */
    volatile int iterations = 100;
    volatile int seed = 1;
    int total = 0;
    
    printf("Starting fixed-point range analysis tests...\n");
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all helper functions */
        total += fixed_point_comparisons_1(seed);
        total += fixed_point_comparisons_2(i);
        total += fixed_point_saturation_tests(seed);
        
        /* Prevent loop unrolling */
        if (i % 10 == 0) {
            printf("Iteration %d, current total: %d\n", i, total);
        }
    }
    
    printf("Final result: %d\n", total);
    printf("Global side effect: %d\n", g_result);
    
    return total != 0 ? 0 : 1;
}
