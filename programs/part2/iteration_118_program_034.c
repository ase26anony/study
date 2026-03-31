/* Target: fixed-value.cc lines 264-277 - fixed-point range calculations */
#include <stdio.h>

/* Force compile-time evaluation with static constants */
static const short _Fract sf_min = -0.5hr;
static const short _Fract sf_max = 0.5hr;
static const unsigned short _Fract usf_max = 0.999999ur;
static const _Sat long _Accum sat_la_max = 0.999999999999999999lk;
static const _Sat long _Accum sat_la_min = -0.999999999999999999lk;
static const _Sat unsigned long _Accum sat_ula_max = 0.999999999999999999ulk;

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    short _Fract sf;
    _Fract f;
    long _Accum la;
    unsigned _Sat _Fract usf_sat;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData fp_array[] = {
    { .sf = -0.5hr, .f = 0.99r, .la = 0.999999999lk, .usf_sat = 0.999999ur },
    { .sf = 0.499hr, .f = -0.99r, .la = -0.999999999lk, .usf_sat = 0.0ur },
    { .sf = 0.0hr, .f = 0.0r, .la = 0.0lk, .usf_sat = 0.5ur }
};

/* Compile-time conditional using fixed-point constants */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define USE_FIXED_POINT 1
#else
#define USE_FIXED_POINT 0
#endif

int main(void) {
    /* Declare variables with different fixed-point types */
    volatile _Fract f1 = 0.75r;  /* volatile to prevent elimination */
    volatile _Accum a1 = 0.999999k;
    volatile unsigned _Sat _Fract usf1 = 0.999999ur;
    volatile _Sat _Accum sat_a1 = 0.999999k;
    volatile long _Accum la1 = 0.999999999lk;
    
    /* Force constant folding with ternary operator */
    const _Fract const_fold_test = 
        (sf_max > 0.4hr) ? (0.999999r * 0.999999r) : 0.0r;
    
    /* This should trigger range checking during constant evaluation */
    const _Sat _Fract sat_fold_test = 
        (usf_max == 0.999999ur) ? (0.999999ur + 0.000001ur) : 0.0ur;
    
    /* Mixed-type conversions that may trigger boundary checks */
    int int_from_fract = (int)(sf_max * 100);
    float float_from_accum = (float)(sat_la_max);
    
    /* Array indexing with fixed-point derived index */
    int idx = (int)(const_fold_test * 2);
    if (idx < 0) idx = 0;
    if (idx > 2) idx = 2;
    
    /* Loop with fixed iterations to allow unrolling and constant propagation */
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        if (f1 > 0.5r) {
            a1 = a1 * 1.1k;  /* Potential overflow */
        } else {
            a1 = a1 * 0.9k;
        }
        
        /* Saturation arithmetic that should hit boundaries */
        usf1 = usf1 + 0.5ur;  /* Will saturate for large values */
        
        /* Left shift simulation through multiplication */
        if (la1 > 0.5lk) {
            la1 = la1 * 2.0lk;  /* May overflow */
        }
        
        /* Use array element */
        f1 = f1 + fp_array[i].f;
    }
    
    /* Extreme boundary tests that should trigger the uncovered range checks */
    
    /* 1. Test maximum representable values */
    const long _Accum la_boundary_test = 0.999999999999999999lk;
    const _Sat long _Accum sat_la_boundary = la_boundary_test + 0.000000000000000001lk;
    
    /* 2. Test minimum representable values */
    const long _Accum la_neg_boundary = -0.999999999999999999lk;
    const _Sat long _Accum sat_la_neg_boundary = la_neg_boundary - 0.000000000000000001lk;
    
    /* 3. Test unsigned saturation at 1.0 */
    const unsigned _Sat _Fract usf_boundary = 0.999999ur + 0.000001ur;
    
    /* 4. Mixed operations that may overflow */
    const _Accum mixed_boundary = (_Accum)(0.999999k * 1.000001k);
    
    /* 5. Use __builtin_constant_p to create constant-only paths */
    if (__builtin_constant_p(sf_max)) {
        /* This code only runs if sf_max is a compile-time constant */
        const short _Fract builtin_test = sf_max * sf_max;
        int_from_fract += (int)(builtin_test * 1000);
    }
    
    /* Prevent dead code elimination */
    volatile int result = int_from_fract + idx;
    result += (int)(float_from_accum * 100);
    
    /* Print to prevent optimization */
    printf("Results: %d %f\n", result, (double)float_from_accum);
    
    return 0;
}
