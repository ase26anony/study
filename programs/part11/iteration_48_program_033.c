/* fixed-point-test.c */
#include <stdio.h>
#include <stdint.h>

/* Prevent constant folding and inlining */
#define NOOPT __attribute__((noinline, noipa))

/* Global side effect to prevent dead code elimination */
volatile int global_result = 0;

/* Function 1: Tests with short _Fract types */
NOOPT int test_short_fract(volatile int seed) {
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1, uf2;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;          /* Binary: 0.0001100110011... */
    sf2 = -0.125r;       /* Binary: -0.001 exactly */
    uf1 = 0.5r;          /* Binary: 0.1 exactly */
    uf2 = 0.333r;        /* Binary: 0.0101010101... */
    
    /* Introduce uncertainty using volatile seed */
    sf3 = (signed short _Fract)(seed % 256) / 256.0r;
    
    /* Arithmetic that could overflow/saturate */
    sf1 = sf1 + sf2;     /* 0.1 - 0.125 = -0.025 */
    uf1 = uf1 * uf2;     /* 0.5 * 0.333... = 0.166... */
    
    /* Critical comparisons that should trigger range analysis */
    int result = 0;
    
    /* Comparison 1: sf1 > 0.5r (likely false) */
    if (sf1 > 0.5r) {
        result |= 1;
    }
    
    /* Comparison 2: sf1 == -0.125r */
    if (sf1 == -0.125r) {
        result |= 2;
    }
    
    /* Comparison 3: uf1 < 0.2r */
    if (uf1 < 0.2r) {
        result |= 4;
    }
    
    /* Comparison 4: sf3 >= 0.0r */
    if (sf3 >= 0.0r) {
        result |= 8;
    }
    
    /* Saturation context: cast with potential overflow */
    signed _Fract sf_large = (signed _Fract)sf3 * 2.0r;
    if (sf_large > 0.9r) {
        result |= 16;
    }
    
    return result;
}

/* Function 2: Tests with _Accum types */
NOOPT int test_accum(volatile int seed) {
    signed _Accum sa1, sa2, sa3;
    unsigned _Accum ua1, ua2;
    
    /* Initialize with various constants */
    sa1 = 0.1k;          /* Binary fractional part */
    sa2 = -0.5k;         /* Binary: -0.1 exactly */
    ua1 = 0.75k;         /* Binary: 0.11 exactly */
    
    /* Use seed to create uncertainty */
    sa3 = (signed _Accum)(seed % 1000) / 1000.0k;
    ua2 = (unsigned _Accum)(seed % 500) / 1000.0k;
    
    /* Arithmetic operations that could overflow */
    sa1 = sa1 * sa3;     /* Multiplication */
    ua1 = ua1 + ua2;     /* Addition */
    
    /* More complex expression */
    sa2 = sa2 - sa1 * 0.25k;
    
    int result = 0;
    
    /* Comparison 1: sa1 > 0.0k */
    if (sa1 > 0.0k) {
        result |= 1;
    }
    
    /* Comparison 2: sa2 == -0.5k */
    if (sa2 == -0.5k) {
        result |= 2;
    }
    
    /* Comparison 3: ua1 <= 1.0k */
    if (ua1 <= 1.0k) {
        result |= 4;
    }
    
    /* Comparison 4: sa3 < sa1 */
    if (sa3 < sa1) {
        result |= 8;
    }
    
    /* Cast between different fixed-point types */
    signed short _Fract sf_from_acc = (signed short _Fract)sa1;
    if (sf_from_acc > 0.0r) {
        result |= 16;
    }
    
    /* Saturation test with explicit cast */
    unsigned _Accum ua_sat = (unsigned _Accum)(sa1 * 10.0k);
    if (ua_sat > 0.9k) {
        result |= 32;
    }
    
    return result;
}

/* Function 3: Mixed types and saturation contexts */
NOOPT int test_mixed_saturation(volatile int seed) {
    signed _Accum sa;
    unsigned short _Fract uf;
    long _Fract lf;
    
    /* Initialize with seed-dependent values */
    sa = (signed _Accum)((seed - 50) * 2) / 100.0k;  /* Range: -1.0 to 1.0 */
    uf = (unsigned short _Fract)(seed % 100) / 100.0r;
    lf = 0.9r;  /* Constant */
    
    int result = 0;
    
    /* Critical comparison that may trigger the uncovered code */
    /* This should create the a_high.sgt(max_r) condition */
    if (sa > 0.9k) {
        result |= 1;
    }
    
    /* Another comparison: uf < 0.1r */
    if (uf < 0.1r) {
        result |= 2;
    }
    
    /* Cast with potential saturation */
    signed short _Fract sf_sat = (signed short _Fract)sa;
    if (sf_sat == 0.0r) {
        result |= 4;
    }
    
    /* Multiplication that could overflow */
    lf = lf * (long _Fract)uf;
    if (lf > 0.5r) {
        result |= 8;
    }
    
    /* Comparison after arithmetic */
    if (sa * 2.0k > 1.5k) {
        result |= 16;
    }
    
    return result;
}

/* Main function with loop to force dynamic analysis */
int main() {
    volatile int iterations = 100;
    volatile int seed = 1;
    int final_result = 0;
    
    printf("Starting fixed-point range analysis test...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the seed to create different value ranges */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call all test functions */
        final_result ^= test_short_fract(seed % 256);
        final_result ^= test_accum(seed % 1000);
        final_result ^= test_mixed_saturation(seed % 200);
        
        /* Prevent loop unrolling */
        asm volatile("" : : "r"(seed) : "memory");
    }
    
    printf("Final result: %d\n", final_result);
    global_result = final_result;
    
    return final_result != 0 ? 0 : 1;
}
