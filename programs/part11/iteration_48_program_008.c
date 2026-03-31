/* fixed-point-test.c */
#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and constant folding */
volatile int iter_limit = 100;
volatile int seed = 1;
volatile int dummy = 0;

/* Global side effects to prevent elimination */
int global_result = 0;

/* Core fixed-point function - designed to trigger range analysis */
__attribute__((noinline, noipa))
int fixed_point_range_test(int base) {
    /* Use various fixed-point types */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1, uf2;
    signed _Accum sa1, sa2;
    unsigned _Accum ua1;
    
    /* Initialize with constants that have interesting binary representations */
    sf1 = 0.1r;          /* Non-trivial binary fraction */
    sf2 = -0.125r;       /* Exact power of two fraction */
    sf3 = 0.5r;          /* Common boundary value */
    
    uf1 = 0.75r;
    uf2 = 0.0r;
    
    /* Initialize accumulators with values that may overflow */
    sa1 = 0.0rk;
    sa2 = -0.5rk;
    
    /* Use volatile input to prevent constant propagation */
    ua1 = (unsigned _Accum)(seed + base) * 0.25rk;
    
    /* Force range analysis through arithmetic that may saturate */
    for (int i = 0; i < 3; i++) {
        /* Operations that could overflow/saturate */
        sf1 = sf1 + sf2;  /* 0.1 + (-0.125) = -0.025 */
        sf3 = sf3 * sf1;  /* Could underflow */
        
        /* Accumulator operations with wider range */
        sa1 = sa1 + 0.1rk;
        sa2 = sa2 * 0.9rk;
        
        /* Mixed-type operations requiring conversion */
        uf1 = uf1 - (unsigned short _Fract)0.1r;
        
        /* Critical comparisons that should trigger the uncovered condition */
        /* These comparisons involve high/low part splitting */
        if (sf1 > 0.5r) {  /* Comparison against boundary */
            ua1 = ua1 + 0.1rk;
        }
        
        if (sf2 == -0.125r) {  /* Exact equality test */
            sa1 = sa1 - 0.05rk;
        }
        
        /* Complex comparison chain similar to the uncovered code */
        if (sa1 > 0.0rk || (sa1 == 0.0rk && ua1 > 0.0rk)) {
            uf2 = uf2 + 0.01r;
        }
        
        /* Another comparison that may trigger high/low logic */
        if (sf3 < -0.25r) {
            sf3 = 0.0r;
        }
    }
    
    /* Cast between types with different scaling - may trigger saturation logic */
    signed _Fract sf_large = (signed _Fract)sa1;
    unsigned short _Fract uf_small = (unsigned short _Fract)ua1;
    
    /* Final comparisons that should reach the target lines */
    /* These mimic the a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) pattern */
    if (sa1 > 100.0rk) {
        return 1;
    }
    
    if (sa2 < -100.0rk) {
        return 2;
    }
    
    /* Comparison with intermediate value */
    if (ua1 == 0.5rk) {
        return 3;
    }
    
    /* Mixed comparison after conversion */
    if ((signed _Fract)sa2 > sf1) {
        return 4;
    }
    
    return 0;
}

/* Second test function focusing on saturation boundaries */
__attribute__((noinline, noipa))
int saturation_boundary_test(int offset) {
    signed _Accum sa_max, sa_min;
    unsigned _Accum ua_max;
    signed short _Fract sf_bound;
    
    /* Initialize near boundaries */
    sa_max = 0.999999999rk;  /* Very close to 1.0 */
    sa_min = -0.999999999rk; /* Very close to -1.0 */
    ua_max = 0.999999999rk;  /* Close to 1.0 for unsigned */
    
    /* Use volatile to prevent constant folding */
    sf_bound = (signed short _Fract)((seed + offset) * 0.01r);
    
    /* Operations that should trigger saturation checks */
    for (int i = 0; i < 2; i++) {
        /* These may overflow the fixed-point range */
        sa_max = sa_max + 0.000000001rk;
        sa_min = sa_min - 0.000000001rk;
        ua_max = ua_max + 0.000000001rk;
        
        /* Comparisons at boundary conditions */
        if (sa_max >= 1.0rk) {
            sa_max = 0.5rk;
        }
        
        if (sa_min <= -1.0rk) {
            sa_min = -0.5rk;
        }
        
        if (ua_max >= 1.0rk) {
            ua_max = 0.25rk;
        }
        
        /* Comparison that may trigger the specific uncovered logic */
        if (sf_bound > 0.5r || (sf_bound == 0.5r && sa_max > 0.0rk)) {
            sf_bound = sf_bound * 0.5r;
        }
    }
    
    /* Final boundary check */
    if (sa_max > 0.9rk && sa_min < -0.9rk) {
        return 10;
    }
    
    return 20;
}

int main() {
    int result = 0;
    
    printf("Starting fixed-point range analysis test...\n");
    
    /* Main loop with volatile control to prevent optimization */
    for (int i = 0; i < iter_limit; i++) {
        /* Call both test functions with varying inputs */
        int r1 = fixed_point_range_test(i);
        int r2 = saturation_boundary_test(i * 2);
        
        /* Use results to prevent dead code elimination */
        result += r1 + r2;
        dummy = i;  /* Volatile write */
    }
    
    global_result = result;
    printf("Test completed. Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
