/* fixed-value-test.c */
/* Compile with: gcc -O2 -fsat-conversion -ffixed-point -frounding-math fixed-value-test.c -o fixed-test */

#include <stdio.h>
#include <stdint.h>

/* Global side effect to prevent optimization */
volatile int global_result = 0;

/* Helper functions marked to prevent inlining and IPA */
__attribute__((noinline, noipa))
int test_signed_fract_range(volatile int seed) {
    /* Use various signed fract types */
    signed short _Fract sf1, sf2, sf3;
    signed _Fract f1, f2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;
    sf2 = -0.125r;
    f1 = 0.5r;
    f2 = -0.75r;
    
    /* Use volatile seed to prevent constant folding */
    signed short _Fract sf_vol = (signed short _Fract)(seed * 0.01r);
    
    /* Arithmetic that could overflow/saturate */
    sf3 = sf1 + sf2 + sf_vol;
    f1 = f1 * f2;
    
    /* Critical comparisons - these should trigger range analysis */
    int result = 0;
    if (sf3 > 0.5r) {
        result += 1;
    }
    if (sf3 < -0.5r) {
        result += 2;
    }
    if (f1 == 0.0r) {
        result += 4;
    }
    if ((signed _Fract)sf3 < f2) {
        result += 8;
    }
    
    /* Additional comparison with constant that might hit the uncovered condition */
    signed short _Fract temp = sf1 * 2.0r;
    if (temp > 0.9r) {
        result += 16;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_accum_range(volatile int seed) {
    /* Use accum types with different sizes */
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    long _Accum la1;
    
    /* Initialize with various constants */
    sa1 = 0.1k;
    sa2 = -0.125k;
    ua1 = 0.5uk;
    
    /* Use seed to create non-constant values */
    la1 = (long _Accum)(seed) * 0.01k;
    
    /* Arithmetic operations that could overflow */
    sa1 = sa1 + sa2 + (signed _Accum)la1;
    ua1 = ua1 * ua1;
    la1 = la1 * la1;
    
    /* Critical comparisons for accum types */
    int result = 0;
    if (sa1 > 0.5k) {
        result += 1;
    }
    if (sa1 < -0.5k) {
        result += 32;
    }
    if (ua1 == 0.0uk) {
        result += 64;
    }
    if (la1 > 100.0k) {
        result += 128;
    }
    
    /* Cast between types and compare */
    signed _Fract casted = (signed _Fract)sa1;
    if (casted > 0.1r) {
        result += 256;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_mixed_types_saturation(volatile int seed) {
    /* Mixed fixed-point types to trigger complex range analysis */
    unsigned short _Fract usf1, usf2;
    signed _Accum sa1;
    unsigned long _Accum ula1;
    
    /* Initialize with constants */
    usf1 = 0.75ur;
    usf2 = 0.25ur;
    sa1 = 0.333k;
    ula1 = 0.666ulk;
    
    /* Use seed for variability */
    signed _Accum sa_vol = (signed _Accum)(seed % 100) * 0.01k;
    
    /* Operations that might saturate */
    usf1 = usf1 + usf2 + (unsigned short _Fract)sa_vol;
    sa1 = sa1 * sa_vol;
    ula1 = ula1 / (unsigned long _Accum)2.0ulk;
    
    /* Multiple comparisons to trigger the uncovered condition */
    int result = 0;
    
    /* This comparison structure might trigger a_high.sgt(max_r) logic */
    if (usf1 > 0.9ur) {
        result += 512;
    }
    
    if (sa1 < -0.1k) {
        result += 1024;
    }
    
    if (ula1 == 0.0ulk) {
        result += 2048;
    }
    
    /* Nested comparisons similar to the uncovered code pattern */
    signed _Accum temp1 = sa1 * 2.0k;
    signed _Accum temp2 = -0.5k;
    
    if (temp1 > 1.0k || (temp1 == 1.0k && sa1 > 0.0k)) {
        result += 4096;
    }
    
    if (temp2 < -1.0k || (temp2 == -1.0k && sa1 < 0.0k)) {
        result += 8192;
    }
    
    return result;
}

__attribute__((noinline, noipa))
int test_edge_cases(volatile int seed) {
    /* Test edge cases near saturation boundaries */
    signed short _Fract sf_min = -1.0r;
    signed short _Fract sf_max = 0.9999r;  /* Close to max */
    signed _Accum sa_near_max = 127.999k;  /* Near max for _Accum */
    
    /* Operations that push boundaries */
    sf_min = sf_min * (signed short _Fract)0.5r;
    sf_max = sf_max + (signed short _Fract)(seed * 0.0001r);
    sa_near_max = sa_near_max * (signed _Accum)1.01k;  /* Might overflow */
    
    int result = 0;
    
    /* Comparisons that should trigger saturation bound calculations */
    if (sf_min < -0.5r) {
        result += 16384;
    }
    
    if (sf_max > 0.9r) {
        result += 32768;
    }
    
    if (sa_near_max > 100.0k) {
        result += 65536;
    }
    
    return result;
}

int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    
    int total_result = 0;
    
    /* Loop to force dynamic analysis */
    for (volatile int i = 0; i < iterations; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call all test functions */
        total_result ^= test_signed_fract_range(seed);
        total_result ^= test_accum_range(seed);
        total_result ^= test_mixed_types_saturation(seed);
        total_result ^= test_edge_cases(seed);
        
        /* Prevent loop unrolling */
        global_result += i;
    }
    
    printf("Result: %d\n", total_result);
    printf("Global side effect: %d\n", global_result);
    
    return total_result != 0 ? 0 : 1;
}
