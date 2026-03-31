/* fixed-point-coverage.c
 * Targets uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O3 -std=c23 -Wno-psabi fixed-point-coverage.c -o fixed-point-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Force compile-time evaluation with constexpr-style constants */
#define EVAL_CONSTANT(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    long _Accum la;
    unsigned long _Sat _Accum ulsata;
};

/* Array initialized with fixed-point constants at boundaries */
static const struct FixedPointData fp_array[] = {
    {0.999999hr, -0.999999r, 0.999999ur, 9223372036854775.807lk, 18446744073709551.615ulk},
    {0.000000hr, -0.000001r, 0.000000ur, -9223372036854775.808lk, 0.000000ulk},
    {0.5hr, -0.5r, 0.5ur, 0.0lk, 0.0ulk}
};

/* Function to trigger range checks through conversions */
static inline int check_range_conversion(long _Accum val) {
    /* This conversion should trigger range checking */
    return (int)val;
}

/* Compile-time conditional using preprocessor */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define USE_FIXED_POINT 1
#else
#define USE_FIXED_POINT 0
#endif

int main(void) {
    /* Declare fixed-point variables at representable limits */
    const unsigned _Fract max_uf = 0.999999r;
    const unsigned _Fract min_uf = 0.000000r;
    const signed _Fract max_sf = 0.999999r;
    const signed _Fract min_sf = -1.000000r;
    
    /* Saturation types that will overflow/underflow */
    unsigned _Sat _Fract sat_uf = 0.999999ur;
    signed _Sat _Fract sat_sf = 0.999999r;
    unsigned _Sat long _Accum sat_ula = 18446744073709551.615ulk;
    
    /* Accum types with more integer bits */
    long _Accum max_la = 9223372036854775.807lk;
    long _Accum min_la = -9223372036854775.808lk;
    short _Accum sa = 0.0hk;
    
    /* Force constant folding with ternary operator */
    const unsigned _Fract folded_uf = 
        EVAL_CONSTANT(max_uf > 0.5r) ? 
        EVAL_CONSTANT(max_uf * max_uf) :  /* Will overflow for some types */
        EVAL_CONSTANT(min_uf);
    
    /* Compile-time conditional block */
    #if USE_FIXED_POINT
    /* Array indexing with fixed-point converted to integer */
    int idx = (int)(max_sf * 2.0r);  /* Conversion triggers range check */
    if (idx < 0) idx = 0;
    if (idx > 2) idx = 2;
    volatile struct FixedPointData data = fp_array[idx];
    #endif
    
    /* Loop with fixed iteration for unrolling and constant propagation */
    for (int i = 0; i < 3; i++) {
        /* Operations that may overflow/underflow */
        sat_uf += sat_uf;  /* Will saturate for unsigned */
        sat_sf -= 0.5r;    /* May underflow for signed */
        
        /* Conditional based on fixed-point comparison */
        if (sa > 0.0hk) {
            sa *= 1.5hk;  /* Multiplication near limits */
        } else {
            sa = (short _Accum)(max_la / 1000000.0lk);  /* Conversion */
        }
        
        /* Mix with integer arithmetic */
        int converted = check_range_conversion(max_la - i * 1000000.0lk);
        
        /* Use volatile to prevent elimination */
        volatile int prevent_elim = converted + (int)sa;
        (void)prevent_elim;
    }
    
    /* Trigger saturation arithmetic at boundaries */
    sat_uf = sat_uf + sat_uf;  /* Should saturate to max */
    sat_sf = sat_sf - 1.5r;    /* Should saturate to min */
    sat_ula = sat_ula * 2.0ulk; /* Should saturate to max */
    
    /* Complex expression with multiple conversions */
    float float_from_fixed = (float)max_uf + (float)min_sf;
    double double_from_accum = (double)max_la;
    int int_from_sat = (int)sat_uf;
    
    /* Cast to volatile and print to prevent dead code elimination */
    volatile float v1 = float_from_fixed;
    volatile double v2 = double_from_accum;
    volatile int v3 = int_from_sat;
    volatile unsigned _Fract v4 = folded_uf;
    
    printf("Results: %f %f %d\n", (float)v1, (float)v2, v3);
    
    /* Additional edge cases */
    
    /* 1. Left shift simulation through multiplication */
    short _Accum shifted = sa * 4.0hk;  /* Like left shift for fixed-point */
    
    /* 2. Extreme values in conditional expressions */
    long _Accum test_val = (max_la > 0.0lk) ? 
                          max_la * 0.999999lk :  /* Near upper bound */
                          min_la * 0.999999lk;   /* Near lower bound */
    
    /* 3. Fixed-point in switch statement */
    switch ((int)(sat_uf * 8.0ur)) {
        case 0: test_val = 0.0lk; break;
        case 7: test_val = max_la; break;  /* Max index for 3 bits */
        default: test_val = (max_la + min_la) / 2.0lk; break;
    }
    
    /* 4. Fixed-point array */
    signed _Fract sf_array[4] = {
        -1.000000r, -0.5r, 0.5r, 0.999999r
    };
    
    /* 5. Pointer arithmetic with fixed-point */
    signed _Fract *ptr = &sf_array[0];
    for (int i = 0; i < 4; i++) {
        ptr[i] = ptr[i] * ptr[i];  /* Squares - may overflow */
    }
    
    return 0;
}
