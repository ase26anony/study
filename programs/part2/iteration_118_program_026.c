/* Test program for fixed-point range calculation coverage */
#include <stdio.h>

/* Use C23 fixed-point types with GCC extensions */
#if __STDC_VERSION__ >= 202311L
#define USE_C23_TYPES 1
#else
#define USE_C23_TYPES 0
#endif

/* Force compile-time evaluation */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct fixed_data {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
};

/* Array initialized with fixed-point constants */
static const struct fixed_data init_array[] = {
    {0.999999hr, -0.999999r, 0.999999ur, 255.999999k, 32767.999999lk},
    {0.5hr, -0.5r, 0.5ur, 127.999999k, -32768.0lk},
    {0.0hr, 0.0r, 0.0ur, 0.0k, 0.0lk}
};

/* Function to trigger range checks through conversions */
static int convert_and_check(long _Accum value) {
    /* These conversions will trigger range checking */
    int as_int = (int)value;
    float as_float = (float)value;
    signed _Accum as_acc = (signed _Accum)value;
    
    /* Use results to prevent optimization */
    volatile int dummy = as_int;
    dummy += (int)(as_float * 1000);
    return dummy + (int)(as_acc * 100);
}

int main(void) {
    /* Declare variables with extreme values */
    const unsigned _Fract max_uf = 0.999999r;
    const signed _Fract min_sf = -1.0r;
    const unsigned _Sat _Fract sat_uf = 0.999999ur;
    const signed _Sat _Accum sat_sa = 255.999999k;
    const long _Accum max_la = 32767.999999lk;
    const long _Accum min_la = -32768.0lk;
    
    /* Compile-time conditional using fixed-point constants */
    #if USE_C23_TYPES
    const _Sat _Fract c23_sat = 0.75r;
    #endif
    
    /* Force constant folding with ternary operator */
    static const unsigned _Fract folded_const = 
        EVAL_CONST(max_uf > 0.5r) ? 0.999999r : 0.0r;
    
    /* Complex expression that requires range analysis */
    const signed _Accum complex_expr = 
        (signed _Accum)((max_la + min_la) * 0.5k);
    
    /* Array indexing with fixed-point derived index */
    int idx = (int)(folded_const * 3);
    if (idx >= 0 && idx < 3) {
        volatile struct fixed_data data = init_array[idx];
        (void)data;
    }
    
    /* Loop with fixed-point operations */
    signed _Fract accum = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Operations that may overflow */
        accum = accum + 0.333333r;
        
        /* Conditional based on fixed-point comparison */
        if (accum > 0.5r) {
            accum = accum * 0.75r;
        } else {
            accum = accum * 1.25r;
        }
        
        /* Saturation arithmetic */
        unsigned _Sat _Fract sat_val = sat_uf + 0.5ur;
        volatile unsigned _Sat _Fract vsat = sat_val;
        (void)vsat;
    }
    
    /* Trigger conversions with extreme values */
    int result1 = convert_and_check(max_la);
    int result2 = convert_and_check(min_la);
    
    /* Mixed-type expressions */
    long _Accum mixed = max_la * (long _Accum)complex_expr;
    mixed = mixed + (long _Accum)sat_sa;
    
    /* Final conversion to integer for output */
    volatile int output = (int)(accum * 100) + result1 + result2 + (int)mixed;
    
    printf("Test completed (output: %d)\n", output);
    
    /* Compile-time check using __builtin_constant_p */
    if (__builtin_constant_p(max_uf + min_sf)) {
        /* This path is taken at compile-time if expression is constant */
        volatile int const_check = 1;
        (void)const_check;
    }
    
    return 0;
}
