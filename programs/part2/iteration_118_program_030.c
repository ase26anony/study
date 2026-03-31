/* Compile with: gcc -O2 -std=c23 -Wno-psabi fixed-test.c -o fixed-test */

#include <stdio.h>

/* Fixed-point type declarations covering various combinations */
typedef short _Fract sf;
typedef _Fract f;
typedef long _Fract lf;
typedef short _Accum sa;
typedef _Accum a;
typedef long _Accum la;
typedef unsigned short _Fract usf;
typedef unsigned _Fract uf;
typedef unsigned long _Fract ulf;
typedef unsigned short _Accum usa;
typedef unsigned _Accum ua;
typedef unsigned long _Accum ula;

/* Saturated versions */
typedef _Sat short _Fract ssf;
typedef _Sat _Fract sfx;
typedef _Sat long _Fract slf;
typedef _Sat short _Accum ssa;
typedef _Sat _Accum sax;
typedef _Sat long _Accum sla;
typedef unsigned _Sat short _Fract ussf;
typedef unsigned _Sat _Fract usfx;
typedef unsigned _Sat long _Accum usla;

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    sf short_fract;
    a accum;
    uf unsigned_fract;
    ua unsigned_accum;
    sfx sat_fract;
    sla sat_long_accum;
};

/* Array initialized with fixed-point constants at boundaries */
static const struct FixedPointData fp_array[] = {
    {0.999hr, 255.999k, 0.999r, 255.999k, 0.999hr, 9223372036854775.807llk},
    {-0.999hr, -255.999k, 0.0r, 0.0k, -0.999hr, -9223372036854775.807llk},
    {0.5hr, 127.5k, 0.5r, 127.5k, 0.5hr, 4611686018427387.903llk},
    {0.0hr, 0.0k, 0.0r, 0.0k, 0.0hr, 0.0llk}
};

/* Compile-time constant expressions that should trigger range checks */
#define EXTREME_VALUE 0.999999999999999r  /* Pushing _Fract limits */
#define NEG_EXTREME (-0.999999999999999r)
#define ACCUM_MAX 9223372036854775.807llk  /* Long accum near max */
#define ACCUM_MIN (-9223372036854775.807llk)

/* Use __builtin_constant_p to create constant-only code paths */
#ifdef __builtin_constant_p
#define CONSTANT_EXPR(expr) (__builtin_constant_p(expr) ? (expr) : 0)
#else
#define CONSTANT_EXPR(expr) (expr)
#endif

/* Function that performs operations likely to trigger overflow checks */
static void test_fixed_point_operations(void) {
    /* Declare and initialize variables at representable limits */
    const usf max_ushort_fract = 0.999hr;
    const sf min_short_fract = -0.999hr;
    const la max_long_accum = 9223372036854775.807llk;
    const la min_long_accum = -9223372036854775.807llk;
    
    /* Saturated types with operations that should saturate */
    ussf sat_uf = 0.999hr;
    sla sat_la = 9223372036854775.807llk;
    
    /* Force constant folding with ternary operator */
    const int use_max = 1;
    const f folded_fract = use_max ? 0.999r : 0.5r;
    const a folded_accum = use_max ? 255.999k : 127.5k;
    
    /* Mixed-type expressions that require conversion and range checking */
    volatile int result1 = (int)(folded_fract * 256.0k);  /* Conversion to int */
    volatile float result2 = (float)(folded_accum / 2.0k); /* Conversion to float */
    
    /* Array indexing with fixed-point derived index */
    int idx = (int)(folded_fract * 3.0k);
    if (idx >= 0 && idx < 4) {
        volatile sf val = fp_array[idx].short_fract;
        (void)val;
    }
    
    /* Loop with fixed iteration count for unrolling */
    for (int i = 0; i < 3; i++) {
        /* Operations that may overflow */
        sat_uf = sat_uf + 0.5hr;  /* Should saturate for saturated unsigned */
        sat_la = sat_la * 1.1llk; /* Should saturate for long accum */
        
        /* Conditional based on fixed-point comparison */
        if (max_long_accum > 0.0llk) {
            sat_la = sat_la - 1000000.0llk;
        }
        
        /* Shift-like operation using multiplication */
        a shifted = folded_accum * (1 << i)k;
        
        /* Cast to prevent dead code elimination */
        volatile long long temp = (long long)shifted;
        (void)temp;
    }
    
    /* Test extreme values through conversions */
    volatile long long ll_result = (long long)ACCUM_MAX;
    volatile int int_result = (int)EXTREME_VALUE;
    
    /* Use constant expressions in conditional compilation */
#if (EXTREME_VALUE > 0.9r)
    volatile int compile_time_check = 1;
#else
    volatile int compile_time_check = 0;
#endif
    
    /* Complex expression that should trigger range calculation */
    const f test_val = CONSTANT_EXPR(0.999999r * 1.000001r);
    volatile double dbl_result = (double)test_val;
    
    /* Print results to prevent optimization */
    printf("Results: %d %f %lld %d %f\n", 
           result1, result2, ll_result, int_result, dbl_result);
}

/* Additional test with compile-time evaluation */
static const f compile_time_fract = 0.999r * 0.999r;  /* Should be constant folded */
static const a compile_time_accum = 255.999k + 0.001k;  /* Near overflow boundary */

int main(void) {
    /* Test conversions at boundaries */
    uf boundary_test = 0.999999r;
    volatile int as_int = (int)(boundary_test * 1000);
    
    /* Test saturation arithmetic */
    ussf sat_test1 = 0.9hr;
    ussf sat_test2 = 0.8hr;
    ussf sat_sum = sat_test1 + sat_test2;  /* Should saturate to 0.999hr */
    
    /* Test negative saturation */
    ssf neg_sat_test1 = -0.9hr;
    ssf neg_sat_test2 = -0.8hr;
    ssf neg_sat_sum = neg_sat_test1 + neg_sat_test2;  /* Should saturate to -0.999hr */
    
    /* Mixed precision operations */
    la large_accum = 1000000.0llk;
    f small_fract = 0.5r;
    la mixed_result = large_accum * (la)small_fract;
    
    /* Call the operation test function */
    test_fixed_point_operations();
    
    /* Use all variables to prevent dead code elimination */
    volatile ussf print_sat = sat_sum;
    volatile ssf print_neg_sat = neg_sat_sum;
    volatile la print_mixed = mixed_result;
    
    printf("Saturation test: %d %d\n", (int)(sat_sum * 1000), (int)(neg_sat_sum * 1000));
    printf("Compile-time constants: %d %d\n", 
           (int)(compile_time_fract * 1000), (int)(compile_time_accum));
    
    return 0;
}
