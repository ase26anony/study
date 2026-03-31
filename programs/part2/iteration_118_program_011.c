/* Compile with: gcc -O2 -std=c23 -Wno-psabi fixed_test.c -o fixed_test */

#include <stdio.h>
#include <stdint.h>

/* Force compile-time evaluation with constexpr-style patterns */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Accum usa;
    signed long _Sat _Accum slsa;
    _Fract f;
};

/* Array initialization with fixed-point constants */
static const struct FixedData fixed_array[] = {
    { /* Push unsigned fract to max */
        .usf = 0.999999ur,
        .sf = -0.999999r,
        .usa = 255.999999uhk,
        .slsa = -32767.999999llk,
        .f = 0.5r
    },
    { /* Test boundary values */
        .usf = 0.000001ur,
        .sf = -0.000001r,
        .usa = 0.000001uhk,
        .slsa = 0.000001llk,
        .f = -0.5r
    }
};

/* Function to trigger range checks through conversions */
static int_fast32_t convert_with_check(_Accum val) {
    /* This conversion should trigger range checking */
    return (int_fast32_t)val;
}

int main(void) {
    volatile int result = 0; /* Prevent dead code elimination */
    
    /* 1. Fixed-point variables at representable limits */
    const unsigned _Fract max_uf = 0.999999ur;
    const signed _Fract min_sf = -0.999999r;
    const unsigned _Sat _Accum max_usa = 255.999999uhk; /* 8-bit accum */
    const signed long _Sat _Accum min_slsa = -32767.999999llk;
    
    /* 2. Force constant folding with ternary operators */
    const _Fract folded_const = 
        (max_uf > 0.5ur) ? 
        (0.999999r * 0.999999r) : /* Multiplication near upper bound */
        (0.000001r);
    
    /* 3. Saturation arithmetic that will overflow/underflow */
    unsigned _Sat _Fract sat_uf1 = 0.75ur;
    unsigned _Sat _Fract sat_uf2 = 0.75ur;
    unsigned _Sat _Fract sat_sum = sat_uf1 + sat_uf2; /* Should saturate to 0.999999ur */
    
    signed _Sat _Fract sat_sf1 = -0.75r;
    signed _Sat _Fract sat_sf2 = -0.75r;
    signed _Sat _Fract sat_diff = sat_sf1 - sat_sf2; /* Should be 0r */
    
    /* 4. Mixed-type conversions triggering range checks */
    float float_from_fixed = (float)max_usa;
    int int_from_fixed = (int)min_slsa;
    
    /* 5. Compile-time conditional with __builtin_constant_p */
#if defined(__GNUC__)
    if (__builtin_constant_p(max_uf)) {
        /* This array index uses fixed-point converted to integer */
        int idx = (int)(max_uf * 10);
        if (idx >= 0 && idx < 2) {
            result += fixed_array[idx].usf;
        }
    }
#endif
    
    /* 6. Loop with fixed iteration for unrolling and constant propagation */
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        _Accum temp = 0.5hk;
        
        if (i == 0) {
            temp = temp * max_uf; /* Near upper bound */
        } else if (i == 1) {
            temp = temp * min_sf; /* Near lower bound */
        } else {
            temp = temp * folded_const; /* Use folded constant */
        }
        
        /* Convert to integer, triggering range checks */
        result += convert_with_check(temp);
        
        /* Left shift simulation through multiplication */
        if (temp > 0.25hk) {
            /* This multiplication may overflow depending on temp */
            unsigned _Sat _Accum shifted = temp * 4.0uhk;
            result += (int)shifted;
        }
    }
    
    /* 7. Additional boundary tests */
    /* These should trigger the uncovered max_r/max_s/min_r/min_s logic */
    const signed _Fract boundary_test1 = -0.5r;
    const signed _Fract boundary_test2 = 0.5r;
    
    /* Operations that push against boundaries */
    signed _Sat _Fract boundary_op1 = boundary_test1 * 2.0r; /* Should be -0.999999r */
    signed _Sat _Fract boundary_op2 = boundary_test2 * 2.0r; /* Should be 0.999999r */
    
    /* 8. Explicit overflow cases */
    unsigned _Sat short _Fract max_short_fract = 0.999999uhr;
    unsigned _Sat short _Fract overflow_test = max_short_fract + 0.000001uhr;
    
    /* Print results to prevent optimization */
    printf("Results: %d %f %d %f\n", 
           result, 
           float_from_fixed,
           int_from_fixed,
           (float)overflow_test);
    
    /* Use volatile assignments to ensure all values are computed */
    volatile double check1 = sat_sum;
    volatile double check2 = sat_diff;
    volatile double check3 = boundary_op1;
    volatile double check4 = boundary_op2;
    
    return 0;
}
