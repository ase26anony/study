/* fixed-point-test.c */
#include <stdio.h>

/* Test program targeting uncovered lines in fixed-value.cc (lines 264-277)
 * Compile with: gcc -O2 -std=c23 -Wno-psabi fixed-point-test.c -o fixed-test
 * For debugging: gcc -O1 -fdump-tree-original -std=c23 fixed-point-test.c
 */

/* Force constant folding with static const */
static const unsigned short _Fract max_ufract = 0.999999ur;
static const signed _Fract min_sfract = -0.999999r;
static const unsigned _Sat _Fract sat_ufract = 0.75ur;
static const signed long _Accum max_laccum = 0.999999999999999999lk;
static const _Sat signed _Accum sat_saccum = 0.5k;

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed long _Accum sla;
    _Sat signed _Accum ssata;
};

/* Array initialized with fixed-point constants at extremes */
static const struct FixedPointData fp_array[] = {
    {0.99999ur, -0.99999r, 0.99999ur, 0.99999999999999999lk, 0.99999k},
    {0.00001ur, -0.00001r, 0.00001ur, -0.99999999999999999lk, -0.99999k},
    {0.5ur, -0.5r, 0.5ur, 0.0lk, 0.0k}
};

/* Compile-time conditional using __builtin_constant_p */
#define CHECK_CONSTANT(expr) \
    (__builtin_constant_p(expr) ? (expr) : 0)

int main(void) {
    /* Declare variables with different fixed-point types */
    unsigned _Fract uf1 = 0.75ur;
    signed _Fract sf1 = -0.25r;
    unsigned _Sat _Fract usf1 = 0.8ur;
    signed _Accum sa1 = 0.123456k;
    long _Accum la1 = -0.987654321lk;
    
    /* Force range boundary checks with extreme values */
    const unsigned _Fract boundary_test = CHECK_CONSTANT(0.999999999ur);
    const signed _Fract neg_boundary_test = CHECK_CONSTANT(-0.999999999r);
    
    /* Use preprocessor to create conditional compilation paths */
#if 1
    /* This block will trigger constant folding with boundary values */
    const unsigned _Fract compile_time_max = 0.999999ur;
    const signed _Fract compile_time_min = -0.999999r;
    
    /* Operations that may overflow/underflow */
    unsigned _Sat _Fract sat_add = usf1 + compile_time_max;
    signed _Sat _Accum sat_mul = sat_saccum * 2.0k;
#endif
    
    /* Loop with fixed iteration for unrolling and constant propagation */
    volatile unsigned _Fract result = 0.0ur;
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        if (uf1 > 0.5ur) {
            /* Multiplication that may approach max value */
            uf1 = uf1 * 0.999ur;
        } else {
            uf1 = uf1 + 0.25ur;
        }
        
        /* Mix with saturation arithmetic - guaranteed to saturate */
        usf1 = usf1 + 0.9ur;
        
        /* Signed operations with negative values */
        if (sf1 < -0.5r) {
            sf1 = sf1 * 0.9r;
        } else {
            sf1 = sf1 - 0.3r;
        }
        
        /* Accumulator operations */
        sa1 = sa1 * 1.1k;
        la1 = la1 - 0.123456789lk;
        
        /* Cast to integer - triggers conversion with range checking */
        int int_from_fixed = (int)(sa1 * 1000);
        
        /* Use array indexing with fixed-point derived index */
        int idx = (int)(uf1 * 2.0);
        if (idx >= 0 && idx < 3) {
            result = fp_array[idx].usf;
        }
    }
    
    /* Force evaluation by assigning to volatile */
    volatile unsigned _Fract vol_uf = uf1;
    volatile signed _Fract vol_sf = sf1;
    volatile unsigned _Sat _Fract vol_usf = usf1;
    volatile signed _Accum vol_sa = sa1;
    volatile long _Accum vol_la = la1;
    
    /* Mixed-type conversions that trigger range calculations */
    float float_from_fract = (float)uf1;
    double double_from_accum = (double)la1;
    int int_from_sat = (int)usf1;
    
    /* Complex expression with multiple conversions */
    double complex_expr = (double)uf1 + (float)sf1 * (double)sa1;
    
    /* Print results to prevent dead code elimination */
    printf("Results: %f %f %f %f %f\n", 
           (double)vol_uf, (double)vol_sf, (double)vol_usf,
           (double)vol_sa, (double)vol_la);
    
    /* Additional boundary-pushing operations */
    
    /* 1. Operations that should trigger max_r/max_s comparisons */
    const unsigned _Fract near_max = 0.9999999ur;
    unsigned _Sat _Fract test_sat_max = near_max + 0.1ur;
    
    /* 2. Operations that should trigger min_r/min_s comparisons */
    const signed _Fract near_min = -0.9999999r;
    signed _Sat _Fract test_sat_min = near_min - 0.1r;
    
    /* 3. Long accum operations at boundaries */
    const signed long _Accum long_near_max = 0.9999999999999999lk;
    signed long _Accum long_test = long_near_max * 1.0000000000000001lk;
    
    /* 4. Shift-like behavior through multiplication by powers of 2 */
    signed _Accum shift_test = 0.5k;
    for (int i = 0; i < 4; i++) {
        shift_test = shift_test * 2.0k;  /* Should saturate */
    }
    
    /* 5. Extreme conversion cases */
    unsigned _Fract from_float = (unsigned _Fract)0.999999;
    signed _Accum from_double = (signed _Accum)0.999999999;
    
    return 0;
}

/* Additional compile-time tests */
#ifdef __OPTIMIZE__
/* This function uses only compile-time constants */
static unsigned _Fract compile_time_only(void) {
    /* These operations will be evaluated at compile time */
    const unsigned _Fract a = 0.999999ur;
    const unsigned _Fract b = 0.000001ur;
    const unsigned _Sat _Fract c = 0.999999ur;
    
    /* Should trigger saturation at compile time */
    unsigned _Sat _Fract sat_result = c + 0.1ur;
    
    /* Should trigger max_r/max_s comparison */
    unsigned _Fract regular_result = a + b;
    
    /* Ternary with constant condition */
    return (a > 0.5ur) ? sat_result : regular_result;
}
#endif
