/* Test program to exercise fixed-point range calculation logic in GCC */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original test_fixed.c -o test_fixed */

#include <stdio.h>
#include <stdint.h>

/* Force compile-time evaluation with constexpr-like behavior */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
};

/* Array initialized with boundary fixed-point values */
static const struct FixedPointData test_data[] = {
    {0.999999hr, -0.999999r, 0.999999ur, 0.999999999ak, 0.99999999999999999lk},
    {0.000000hr, -0.000001r, 0.000000ur, -0.999999999ak, -0.99999999999999999lk},
    {0.5hr, -0.5r, 0.5ur, 0.0ak, 0.0lk}
};

/* Function to trigger range checks through conversions */
static int check_range_conversions(void) {
    /* Test extreme values that should trigger range boundary checks */
    const unsigned _Fract max_uf = 0.99999999999999999r;
    const unsigned _Fract min_uf = 0.00000000000000000r;
    const signed _Fract max_sf = 0.99999999999999999r;
    const signed _Fract min_sf = -1.00000000000000000r;
    
    /* These conversions should invoke range checking logic */
    int imax = (int)(max_uf * 1000);
    int imin = (int)(min_uf * 1000);
    float fmax = (float)max_sf;
    float fmin = (float)min_sf;
    
    /* Use volatile to prevent optimization */
    volatile int result = 0;
    result += imax;
    result -= imin;
    result += (int)(fmax * 1000);
    result -= (int)(fmin * 1000);
    
    return result;
}

/* Function using ternary operator with constant conditions */
static signed _Sat _Accum ternary_saturation(void) {
    /* Force constant folding with ternary operator */
    const signed _Accum a = 0.75ak;
    const signed _Accum b = 0.85ak;
    
    /* This should be evaluated at compile-time, triggering range checks */
    static const signed _Sat _Accum result = 
        EVAL_CONST(a > 0.5ak ? a * 2.0ak : b * 2.0ak);
    
    return result;
}

/* Test saturation arithmetic with boundary values */
static void test_saturation_overflow(void) {
    unsigned _Sat _Fract us1 = 0.9ur;
    unsigned _Sat _Fract us2 = 0.8ur;
    
    /* These should saturate at 1.0 */
    unsigned _Sat _Fract sum = us1 + us2;  /* 1.7 -> saturated to 1.0 */
    unsigned _Sat _Fract prod = us1 * 2.0ur; /* 1.8 -> saturated to 1.0 */
    
    signed _Sat _Fract ss1 = 0.9r;
    signed _Sat _Fract ss2 = -0.9r;
    
    /* These should trigger range checks */
    signed _Sat _Fract diff = ss1 - ss2;  /* 1.8 -> saturated to 1.0 */
    signed _Sat _Fract neg_prod = ss2 * 2.0r; /* -1.8 -> saturated to -1.0 */
    
    /* Use volatile to prevent dead code elimination */
    volatile unsigned _Sat _Fract vsum = sum;
    volatile signed _Sat _Fract vdiff = diff;
    (void)vsum;
    (void)vdiff;
}

/* Main test function with loops and conditional assignments */
int main(void) {
    /* Initialize with boundary values */
    unsigned short _Fract usf_bound = 0.999999hr;
    signed _Fract sf_bound = -0.999999r;
    long _Accum la_bound = 0.99999999999999999lk;
    
    /* Array for loop results */
    signed _Fract results[4] = {0};
    
    /* Small fixed loop for unrolling and constant propagation */
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        if (usf_bound > 0.5hr) {
            results[i] = (signed _Fract)(usf_bound * 2.0hr);
        } else {
            results[i] = (signed _Fract)(usf_bound / 2.0hr);
        }
        
        /* Mix with other types */
        if (sf_bound < -0.5r) {
            la_bound = la_bound * 0.5lk;
        } else {
            la_bound = la_bound + 0.25lk;
        }
        
        /* Shift-like behavior through multiplication */
        usf_bound = usf_bound * 0.75hr;
        sf_bound = sf_bound * 1.25r;
    }
    
    /* Test compile-time constant expressions */
    #if 1
    /* This block should trigger constant evaluation */
    const signed _Accum compile_time_acc = 0.999999999ak;
    const int array_index = (int)(compile_time_acc * 3.0ak);
    
    /* Use in array access (bounds checked at compile time) */
    if (array_index >= 0 && array_index < 4) {
        results[3] = results[array_index];
    }
    #endif
    
    /* Test conversions that should trigger range checking */
    int int_from_fract = (int)(la_bound * 100);
    float float_from_fract = (float)(sf_bound);
    
    /* Test saturation functions */
    test_saturation_overflow();
    
    /* Get ternary result (compile-time evaluated) */
    signed _Sat _Accum ternary_result = ternary_saturation();
    
    /* Range conversion checks */
    int range_check_result = check_range_conversions();
    
    /* Print results to prevent optimization */
    printf("Results: %d %f %d %d\n", 
           int_from_fract, 
           float_from_fract,
           (int)(ternary_result * 1000),
           range_check_result);
    
    /* Use volatile to ensure all results are computed */
    volatile signed _Fract vr0 = results[0];
    volatile signed _Fract vr1 = results[1];
    volatile signed _Fract vr2 = results[2];
    volatile signed _Fract vr3 = results[3];
    (void)vr0; (void)vr1; (void)vr2; (void)vr3;
    
    return 0;
}

/* Additional compile-time tests using preprocessor */
#ifdef __OPTIMIZE__
/* This section only compiled with optimization */
static const unsigned _Fract compile_time_max = 0.99999999999999999r;
static const unsigned _Fract compile_time_min = 0.00000000000000000r;

/* Force evaluation of boundary comparisons */
static const int is_max_valid = __builtin_constant_p(compile_time_max) && 
                                (compile_time_max <= 1.0r);
static const int is_min_valid = __builtin_constant_p(compile_time_min) && 
                                (compile_time_min >= 0.0r);
#endif
