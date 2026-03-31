/* fixed-point-test.c */
#include <stdio.h>

/* Test program targeting uncovered lines in fixed-value.cc
 * Compile with: gcc -O2 -std=c23 -Wno-psabi fixed-point-test.c -o fixed-test
 * For more coverage: gcc -O3 -fsanitize=undefined -frounding-math -std=c23 fixed-point-test.c
 */

/* Force compile-time evaluation with constexpr-style */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.999999hr, -0.999999r, 0.999999ur, 0.999999k, 0.999999lk},
    {0.5hr, -0.5r, 0.5ur, -0.5k, -0.5lk},
    {0.0hr, 0.0r, 0.0ur, 0.0k, 0.0lk}
};

/* Function to trigger range calculations through conversions */
static int convert_and_check(unsigned _Sat _Fract val) {
    /* This should trigger range checking during conversion */
    int int_val = (int)val;
    float float_val = (float)val;
    
    /* Use in conditional to prevent optimization */
    return (int_val > 0) ? (int)(float_val * 100) : 0;
}

int main(void) {
    /* Declare variables with extreme values */
    const unsigned _Fract max_uf = 0.999999r;
    const unsigned _Fract min_uf = 0.0r;
    const signed _Fract max_sf = 0.999999r;
    const signed _Fract min_sf = -1.0r;
    
    /* Saturation types that will trigger bounds checking */
    unsigned _Sat _Fract sat_uf = 0.999999ur;
    signed _Sat _Accum sat_sa = 0.999999k;
    
    /* Long accum with fractional part */
    long _Accum long_acc = 0.999999lk;
    long _Accum neg_long_acc = -0.999999lk;
    
    /* Force compile-time evaluation of extreme expressions */
    #if __STDC_VERSION__ >= 202311L
    /* These should be evaluated at compile-time, triggering range checks */
    static const unsigned _Sat _Fract compile_time_sat = 
        EVAL_CONST(0.999999ur + 0.000001ur);  /* Should saturate to max */
    
    static const signed _Sat _Accum compile_time_acc = 
        EVAL_CONST(0.999999k * 2.0k);  /* Should saturate */
    #endif
    
    /* Array indexing with fixed-point derived values */
    int idx = (int)(sat_uf * 3);
    if (idx >= 0 && idx < 3) {
        /* Access with bounds-checked index */
        sat_uf = init_data[idx].usatf;
    }
    
    /* Loop with fixed iterations for unrolling */
    volatile unsigned _Fract result = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Operations that may overflow/saturate */
        sat_uf = sat_uf + 0.5ur;
        sat_sa = sat_sa - 0.25k;
        
        /* Conditional based on fixed-point comparison */
        if (long_acc > 0.5lk) {
            long_acc = long_acc * 0.75lk;
        } else {
            long_acc = long_acc + 0.25lk;
        }
        
        /* Mix with integer arithmetic */
        int temp = (int)(sat_uf * 100);
        if (temp > 50) {
            sat_uf = sat_uf - 0.6ur;
        }
        
        /* Accumulate result */
        result = result + sat_uf;
    }
    
    /* Conversions that trigger range checking */
    int int_result = convert_and_check(sat_uf);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d\n", int_result);
    
    /* More complex compile-time expressions */
    const signed _Fract sf1 = 0.75r;
    const signed _Fract sf2 = -0.25r;
    
    /* This ternary with constant condition forces compile-time eval */
    const signed _Fract sf3 = (sf1 > sf2) ? 
        EVAL_CONST(sf1 * sf1 - sf2 * sf2) : 
        EVAL_CONST(sf1 + sf2);
    
    /* Use in array size calculation (compile-time) */
    char buffer[(int)(sf3 * 10) + 10];
    
    /* Final mixed-type expression */
    long _Accum final_acc = long_acc + (long _Accum)sat_sa;
    int final_int = (int)final_acc;
    
    printf("Final: %d\n", final_int);
    
    return 0;
}

/* Additional compile-time tests using preprocessor */
#ifdef __CHECK_FIXED_CONSTANTS__
/* These macros test constant folding with fixed-point */
#define FIXED_CONST_TEST(expr, expected) \
    static_assert(__builtin_constant_p(expr), "Not constant"); \
    static_assert((expr) == (expected), "Wrong value")

/* Test cases that should trigger the uncovered range code */
FIXED_CONST_TEST((unsigned _Fract)1.5r, 0.999999r);  /* Should clamp */
FIXED_CONST_TEST((signed _Fract)-1.5r, -1.0r);       /* Should clamp */
FIXED_CONST_TEST((unsigned _Sat _Fract)0.999999ur + 0.000001ur, 0.999999ur);
#endif
