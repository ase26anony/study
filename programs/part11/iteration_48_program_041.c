/* Test program to trigger fixed-point arithmetic range analysis in fixed-value.cc */
/* Specifically targets lines 264-277 involving max_r, max_s, min_r, min_s comparisons */

#include <stdio.h>

/* Prevent constant folding and inlining */
static volatile int control = 1;
static volatile int seed = 12345;

/* Global side effects to prevent dead code elimination */
static int global_result = 0;

/* Helper function with critical fixed-point operations */
__attribute__((noinline, noipa))
static int fixed_point_comparisons(int iter) {
    /* Declare various fixed-point types */
    signed short _Fract sf1, sf2;
    unsigned short _Fract usf1;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;          /* Non-trivial binary fraction */
    sf2 = -0.125r;       /* Exact binary fraction: -1/8 */
    usf1 = 0.5ur;        /* Exact binary fraction: 1/2 */
    
    /* Use volatile seed to prevent constant propagation */
    int local_seed = seed;
    
    /* Initialize accumulators with values that depend on iteration */
    sa1 = (signed _Accum)local_seed * 0.5rk;
    ua1 = (unsigned _Accum)(iter % 256) * 0.25urk;
    
    /* Perform arithmetic that could overflow/saturate */
    for (int i = 0; i < iter % 8; i++) {
        /* Operations that may trigger saturation logic */
        sf1 = sf1 + sf2;
        sa1 = sa1 * 0.9rk;
        
        /* Force range analysis by varying values */
        if (i % 2 == 0) {
            ua1 = ua1 + 0.125urk;
        } else {
            ua1 = ua1 - 0.0625urk;
        }
    }
    
    /* CRITICAL COMPARISONS - Aim to trigger the uncovered lines */
    /* These comparisons should involve the double-int split high/low logic */
    
    /* Comparison 1: Compare against positive bound */
    if (sf1 > 0.5r) {
        global_result += 1;
    }
    
    /* Comparison 2: Compare against negative bound */
    if (sf2 < -0.25r) {
        global_result += 2;
    }
    
    /* Comparison 3: Equality comparison with non-zero */
    if (usf1 == 0.5ur) {
        global_result += 4;
    }
    
    /* Comparison 4: Accumulator comparisons that may trigger the specific condition */
    /* a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) */
    sa2 = (signed _Accum)iter * 0.01rk;
    if (sa1 > 50.0rk) {
        global_result += 8;
    }
    
    /* Comparison 5: Another accumulator comparison */
    if (ua1 < 10.0urk) {
        global_result += 16;
    }
    
    /* Comparison 6: Cast and compare - may trigger different code paths */
    if ((signed _Fract)sa1 < sf2) {
        global_result += 32;
    }
    
    /* Comparison 7: Near-boundary comparison */
    if (sf1 == 0.0r) {
        global_result += 64;
    }
    
    /* Additional arithmetic that could saturate */
    signed _Accum sa3 = sa1 * sa2;
    if (sa3 > 100.0rk) {
        global_result += 128;
    }
    
    return global_result;
}

/* Second helper function with different fixed-point types */
__attribute__((noinline, noipa))
static int more_fixed_point_ops(int base) {
    long _Fract lf1, lf2;
    unsigned long _Fract ulf1;
    signed long _Accum sla1;
    
    /* Initialize with values that may approach bounds */
    lf1 = 0.9999999lr;  /* Close to upper bound */
    lf2 = -0.9999999lr; /* Close to lower bound */
    ulf1 = 0.0000001ulr; /* Close to zero but positive */
    
    /* Use base to create variation */
    sla1 = (signed long _Accum)base * 0.0001lrk;
    
    /* Operations that may saturate */
    for (int i = 0; i < 3; i++) {
        lf1 = lf1 * 1.1lr;
        lf2 = lf2 * 1.1lr;
        sla1 = sla1 + (signed long _Accum)(i * 0.5lrk);
    }
    
    /* More comparisons targeting the uncovered logic */
    int result = 0;
    
    /* These comparisons should exercise the range analysis */
    if (lf1 > 0.5lr) {
        result += 256;
    }
    
    if (lf2 < -0.5lr) {
        result += 512;
    }
    
    if (ulf1 == 0.0ulr) {
        result += 1024;
    }
    
    /* Comparison that might trigger a_high.sgt(max_r) path */
    if (sla1 > 0.0lrk) {
        result += 2048;
    }
    
    /* Cast between different fixed-point types */
    if ((signed _Fract)sla1 > 0.1r) {
        result += 4096;
    }
    
    return result;
}

int main() {
    /* Volatile iteration control to prevent loop unrolling */
    volatile int iterations = 100;
    int total = 0;
    
    /* Main loop calling fixed-point functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary the input to prevent constant propagation */
        int val = (i * seed) % 100;
        
        /* Call first fixed-point function */
        total += fixed_point_comparisons(val);
        
        /* Call second fixed-point function */
        total += more_fixed_point_ops(val + i);
        
        /* Modify volatile to prevent optimization */
        if (control) {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Fixed-point test result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
