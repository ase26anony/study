/* fixed-point-test.c */
#include <stdio.h>

/* Prevent constant folding and inlining */
volatile int iter_count = 100;
volatile int seed = 1;

/* Global side effects to prevent dead code elimination */
volatile int global_result = 0;

/* Core fixed-point operations - marked to prevent optimization */
__attribute__((noinline, noipa))
int fixed_point_range_test(int base) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1, uf2;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have non-trivial binary representations */
    sf1 = 0.1r;          /* Binary: 0.0001100110011... */
    sf2 = -0.125r;       /* Binary: -0.001 exactly */
    uf1 = 0.5r;          /* Binary: 0.1 exactly */
    
    /* Use volatile input to prevent constant propagation */
    int volatile_input = base;
    
    /* Initialize accumulators with values that may cause saturation */
    sa1 = (signed _Accum)volatile_input * 0.5rk;
    ua1 = (unsigned _Accum)volatile_input * 0.25rk;
    
    /* Create values near saturation boundaries */
    sf3 = 0.999r;        /* Near maximum for signed short _Fract */
    
    /* Critical arithmetic that may overflow/saturate */
    for (int i = 0; i < 3; i++) {
        /* Operations that could trigger range analysis */
        sa1 = sa1 + sa1;  /* Potential overflow for _Accum */
        ua1 = ua1 * 0.9rk; /* Multiplication that may saturate */
        
        /* Mix different fixed-point types */
        sf1 = sf1 + (signed short _Fract)(sa1 * 0.1rk);
        
        /* THE CRITICAL COMPARISONS - designed to trigger the uncovered code */
        /* These comparisons should exercise the double-int high/low logic */
        if (sa1 > 0.75rk) {
            /* This comparison may trigger a_high.sgt(max_r) path */
            sf2 = sf2 * 0.5r;
        }
        
        if (ua1 == 0.0rk) {
            /* Equality comparison with zero */
            sf3 = sf3 - 0.1r;
        }
        
        /* Complex comparison chain similar to the uncovered condition */
        if ((signed _Accum)sf1 > 0.5rk || 
            ((signed _Accum)sf1 == 0.5rk && (unsigned _Accum)uf1 > 0.25rk)) {
            uf2 = uf1 * 0.8r;
        }
        
        /* Comparisons with negative values */
        if (sf2 < -0.1r) {
            sa2 = sa1 * -0.5rk;
        }
    }
    
    /* More saturation-prone operations */
    /* Cast between different fixed-point types with different scaling */
    signed _Fract sf_large = (signed _Fract)sa1;
    unsigned _Accum ua_large = (unsigned _Accum)sf3;
    
    /* Final comparisons that may hit the specific condition */
    if (sf_large > 0.99r || (sf_large == 0.99r && ua_large > 0.5rk)) {
        return 1;
    }
    
    if (sa1 < -0.99rk || (sa1 == -0.99rk && uf1 < 0.1r)) {
        return -1;
    }
    
    return 0;
}

__attribute__((noinline, noipa))
int another_fixed_point_test(int val) {
    /* Test with different fixed-point types */
    long _Fract lf1, lf2;
    unsigned long _Accum ula1;
    
    lf1 = 0.33333333333333333333rL;
    lf2 = -0.66666666666666666666rL;
    
    /* Create values that may exercise saturation bounds */
    ula1 = (unsigned long _Accum)val * 0.0001rkL;
    
    /* Operations near boundaries */
    for (int i = 0; i < 2; i++) {
        lf1 = lf1 + lf2;
        ula1 = ula1 * 2.0rkL;
        
        /* Comparisons designed to trigger range analysis */
        if (lf1 > 0.9rL) {
            lf2 = lf2 * 0.5rL;
        }
        
        if (ula1 == 0.0rkL || ula1 > 0.99999999999999999999rkL) {
            return i;
        }
    }
    
    /* Cast with potential saturation */
    signed short _Fract sf_result = (signed short _Fract)lf1;
    
    if (sf_result > 0.5r || sf_result < -0.5r) {
        return 2;
    }
    
    return 3;
}

int main() {
    int total = 0;
    
    /* Main loop with volatile iteration count */
    for (int i = 0; i < iter_count; i++) {
        /* Vary the input to prevent constant propagation */
        int input = seed + i;
        
        /* Call the fixed-point functions */
        int result1 = fixed_point_range_test(input);
        int result2 = another_fixed_point_test(input);
        
        /* Use results to prevent dead code elimination */
        total += result1 + result2;
        global_result = total;
        
        /* Modify seed to create varying behavior */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Final result: %d (global: %d)\n", total, global_result);
    return total != 0 ? 0 : 1;
}
