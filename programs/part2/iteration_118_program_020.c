/* Test program for fixed-point range calculation coverage */
#include <stdio.h>

/* Force compile-time evaluation with constexpr-like behavior */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    long _Accum la;
    unsigned long _Sat _Accum ulsata;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.99999hr, -0.99999r, 0.99999ur, 255.99999lk, 65535.99999ulk},
    {0.5hr, -0.5r, 0.5ur, 127.5lk, 32767.5ulk},
    {0.0hr, 0.0r, 0.0ur, -128.0lk, 0.0ulk}
};

/* Function to trigger range checks through conversions */
static int convert_and_check(unsigned _Sat _Fract val) {
    /* This conversion should trigger range checking */
    int int_val = (int)val;
    float float_val = (float)val;
    
    /* Use in conditional to force evaluation */
    return (int_val > 0) ? int_val : (int)(float_val * 100);
}

int main(void) {
    /* Declare fixed-point variables at representable limits */
    const unsigned short _Fract max_ushort_fract = 0.99999hr;
    const signed _Fract min_signed_fract = -0.99999r;
    const unsigned _Sat _Fract sat_fract = 0.99999ur;
    const long _Accum max_long_accum = 255.99999lk;
    const unsigned long _Sat _Accum max_ulong_sat_accum = 65535.99999ulk;
    
    /* Variables for loop computations */
    unsigned _Sat _Fract accum_sat = 0.5ur;
    long _Accum accum = 0.0lk;
    
    /* Compile-time conditional using preprocessor */
#if 1
    /* This block will trigger constant folding */
    const signed _Fract compile_time_fract = 
        (max_ushort_fract > 0.5hr) ? 0.75r : -0.25r;
    
    /* Force range check through array indexing */
    int idx = (int)(compile_time_fract * 4 + 2);
    if (idx >= 0 && idx < 3) {
        accum = init_data[idx].la;
    }
#endif
    
    /* Loop with fixed iteration for unrolling */
    for (int i = 0; i < 3; i++) {
        /* Operations that may overflow/underflow */
        accum_sat = accum_sat + sat_fract;  /* Should saturate */
        accum = accum * 2.0lk;              /* May overflow */
        
        /* Conditional based on fixed-point comparison */
        if (accum > max_long_accum) {
            accum = max_long_accum;
        }
        
        /* Mix with integer arithmetic */
        accum = accum + (long _Accum)(i * 10);
        
        /* Trigger conversions that need range checks */
        int temp = (int)accum_sat;
        if (temp > 100) {
            accum_sat = 1.0ur;  /* Max saturated value */
        }
    }
    
    /* Complex expression forcing constant evaluation */
    const long _Accum complex_expr = EVAL_CONST(
        (max_long_accum * 1.5lk) / 2.0lk + min_signed_fract
    );
    
    /* Use __builtin_constant_p to create conditional compilation */
    if (__builtin_constant_p(complex_expr > 0.0lk)) {
        /* This path should be taken during constant folding */
        accum = complex_expr;
    }
    
    /* Final conversions that trigger range checking */
    volatile int result1 = convert_and_check(accum_sat);
    volatile long result2 = (long)accum;
    volatile float result3 = (float)max_ulong_sat_accum;
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %ld, %f\n", result1, result2, result3);
    
    /* Additional extreme value tests */
    struct FixedPointData test_data = {
        .usf = 0.99999hr,
        .sf = -0.99999r,
        .usatf = 0.99999ur + 0.00001ur,  /* Should saturate to 1.0 */
        .la = 255.99999lk * 1.1lk,       /* Should overflow */
        .ulsata = 65535.99999ulk * 2.0ulk /* Should saturate */
    };
    
    /* Shift operations that may trigger range checks */
    long _Accum shifted = accum;
    for (int i = 0; i < 4; i++) {
        shifted = shifted * 2.0lk;  /* Left shift equivalent */
        
        /* This comparison should trigger the uncovered sgt/ugt checks */
        if (shifted > 511.99999lk || shifted < -512.0lk) {
            shifted = (shifted > 0) ? 511.99999lk : -512.0lk;
        }
    }
    
    volatile long _Accum final_shifted = shifted;
    
    return 0;
}
