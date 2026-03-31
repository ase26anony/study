/* fixed-point-test.c */
#include <stdio.h>

/* Prevent optimization and constant folding */
volatile int iter = 100;
volatile int seed = 1;

/* Global side effects to prevent elimination */
volatile int global_result = 0;

/* Core fixed-point function with no optimization */
__attribute__((noinline, noipa))
int fixed_point_operations(int base) {
    /* Mixed fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract usf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;           /* Binary: 0.0001100110011... */
    sf2 = -0.125r;        /* Binary: -0.001 exactly */
    usf1 = 0.5r;          /* Binary: 0.1 exactly */
    
    /* Use volatile parameter to prevent constant propagation */
    sa1 = (signed _Accum)base * 0.25rk;
    ua1 = (unsigned _Accum)(base + 1) * 0.1rk;
    
    /* Arithmetic that could overflow/saturate */
    sf3 = sf1 + sf2;      /* 0.1 - 0.125 = -0.025 */
    
    /* Multiplication that may exceed range */
    sa2 = sa1 * sa1;
    
    /* Critical comparisons - these should trigger range analysis */
    if (sf3 > 0.0r) {
        /* This branch unlikely but forces comparison */
        global_result += 1;
    }
    
    /* Comparison with constant that may hit the uncovered condition */
    if (sa1 > 0.5rk) {
        global_result += 2;
    }
    
    /* Equality comparison */
    if (ua1 == 0.0rk) {
        global_result += 4;
    }
    
    /* Complex comparison chain */
    if (sf1 < 0.2r && sf2 > -0.2r) {
        global_result += 8;
    }
    
    /* Cast between types with different scaling */
    signed _Fract cast_result = (signed _Fract)sa1;
    if (cast_result < sf2) {
        global_result += 16;
    }
    
    /* Additional saturation-prone operations */
    signed _Accum sa3 = sa1 + 0.9rk;
    if (sa3 > 1.0rk) {
        /* Potential saturation point */
        global_result += 32;
    }
    
    return global_result;
}

/* Second function with different fixed-point types */
__attribute__((noinline, noipa))
int more_fixed_point_ops(int counter) {
    long _Fract lf1, lf2;
    unsigned long _Fract ulf1;
    signed long _Accum sla1;
    
    /* Initialize with various constants */
    lf1 = 0.333r;          /* Approximate 1/3 */
    lf2 = -0.666r;         /* Approximate -2/3 */
    ulf1 = 0.75r;          /* Binary: 0.11 exactly */
    
    /* Use counter to create varying values */
    sla1 = (signed long _Accum)counter * 0.125rlk;
    
    /* Operations that may trigger saturation logic */
    long _Fract lf3 = lf1 + lf2;
    
    /* Comparisons that should hit range analysis */
    if (lf3 > -0.5r) {
        global_result += 64;
    }
    
    if (sla1 == 0.0rlk) {
        global_result += 128;
    }
    
    /* Multiplication that could overflow */
    signed long _Accum sla2 = sla1 * 2.0rlk;
    if (sla2 > 1.0rlk) {
        global_result += 256;
    }
    
    return global_result;
}

int main() {
    int result = 0;
    
    /* Loop to force dynamic analysis */
    for (int i = 0; i < iter; i++) {
        /* Vary the input to prevent constant folding */
        int input = seed + i;
        
        /* Call fixed-point operations */
        result = fixed_point_operations(input);
        
        /* Call second set of operations */
        result = more_fixed_point_ops(input % 10);
        
        /* Use volatile to prevent loop unrolling */
        asm volatile("" : "+r" (result));
    }
    
    printf("Final result: %d\n", result);
    printf("Global side effect: %d\n", global_result);
    
    return result != 0 ? 0 : 1;
}
