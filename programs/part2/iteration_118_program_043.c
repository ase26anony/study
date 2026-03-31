/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Fixed-point type definitions covering various modes */
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
typedef _Sat _Fract sfract;
typedef _Sat long _Fract slf;
typedef _Sat short _Accum ssa;
typedef _Sat _Accum saccum;
typedef _Sat long _Accum sla;
typedef unsigned _Sat short _Fract ussf;
typedef unsigned _Sat _Fract usfract;
typedef unsigned _Sat long _Fract uslf;
typedef unsigned _Sat short _Accum ussa;
typedef unsigned _Sat _Accum usaccum;
typedef unsigned _Sat long _Accum usla;

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointStruct {
    sf short_fract;
    a accum;
    uf unsigned_fract;
    ua unsigned_accum;
    ssf sat_short_fract;
    usaccum sat_unsigned_accum;
};

/* Array initialized with fixed-point constants at boundaries */
static const struct FixedPointStruct fp_array[] = {
    { /* Max values for signed types */
        .short_fract = 0.999999r,  /* ~0.999969 (Q0.15) */
        .accum = 0.9999999999k,    /* Q15.16 max */
        .unsigned_fract = 0.999999r,
        .unsigned_accum = 0.9999999999k,
        .sat_short_fract = 0.999999r,
        .sat_unsigned_accum = 0.9999999999k
    },
    { /* Min values for signed types */
        .short_fract = -0.999999r,
        .accum = -0.9999999999k,
        .unsigned_fract = 0.0r,
        .unsigned_accum = 0.0k,
        .sat_short_fract = -0.999999r,
        .sat_unsigned_accum = 0.0k
    },
    { /* Boundary values that may trigger overflow checks */
        .short_fract = 0.5r,
        .accum = 0.5k,
        .unsigned_fract = 0.5r,
        .unsigned_accum = 0.5k,
        .sat_short_fract = 0.5r,
        .sat_unsigned_accum = 0.5k
    }
};

/* Compile-time constant expressions using ternary operators */
#define BOUNDARY_TEST(x) ((x) > 0.999r ? 0.999r : (x))
#define SATURATION_TEST(x) ((x) * 2.0r)

/* Function to force constant folding with boundary checks */
static const f compile_time_boundary_check(void) {
    /* These will be evaluated at compile time, triggering range calculations */
    const f max_fract = 0.999999r;
    const f min_fract = -0.999999r;
    const f half = 0.5r;
    
    /* Complex expression that forces range analysis */
    return (max_fract + half) > 1.0r ? 
           (min_fract * half) < -0.5r ? 0.25r : 0.75r : 
           0.5r;
}

/* Use __builtin_constant_p to create conditional compilation paths */
#ifdef __GNUC__
#define IS_CONSTANT_EXPR(expr) __builtin_constant_p(expr)
#else
#define IS_CONSTANT_EXPR(expr) 0
#endif

int main(void) {
    volatile int result = 0; /* volatile to prevent optimization */
    
    /* Test 1: Basic fixed-point variables at boundaries */
    sf sf_max = 0.999999r;
    sf sf_min = -0.999999r;
    usf usf_max = 0.999999r;
    
    a accum_max = 0.9999999999k;
    a accum_min = -0.9999999999k;
    ua ua_max = 0.9999999999k;
    
    /* Test 2: Saturation arithmetic that will overflow/underflow */
    ssf sat_sf = 0.999999r;
    usfract sat_uf = 0.999999r;
    
    /* These operations should trigger saturation logic */
    ssf sat_result1 = sat_sf + 0.1r;  /* Should saturate to max */
    usfract sat_result2 = sat_uf + 0.1r;  /* Should saturate to max */
    ssf sat_result3 = sat_sf - 1.5r;  /* Should saturate to min */
    
    /* Test 3: Mixed-type conversions */
    int int_from_fract = (int)(sf_max * 1000);
    float float_from_accum = (float)accum_max;
    a accum_from_int = (a)32767;
    f fract_from_float = (f)0.999;
    
    /* Test 4: Loop with fixed-point arithmetic (will be unrolled) */
    f loop_accum = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Conditional based on fixed-point comparison */
        if (loop_accum > 0.5r) {
            loop_accum = loop_accum * 0.5r;
        } else {
            loop_accum = loop_accum + 0.25r;
        }
        
        /* Mix with integer arithmetic */
        int_from_fract += (int)(loop_accum * 100);
    }
    
    /* Test 5: Compile-time constant expression evaluation */
    const f const_fract = compile_time_boundary_check();
    
    /* Test 6: Use preprocessor to conditionally include boundary tests */
#if IS_CONSTANT_EXPR(0.999999r + 0.000001r)
    /* This path tests compile-time overflow detection */
    const f overflow_test = 0.999999r + 0.000001r;
    result += (int)(overflow_test * 1000);
#endif
    
    /* Test 7: Array indexing with fixed-point derived index */
    int index = (int)(sf_max * 2);  /* Should be 1 or 2 */
    if (index >= 0 && index < 3) {
        result += (int)(fp_array[index].short_fract * 1000);
    }
    
    /* Test 8: Shift operations (converted to multiplication) */
    la long_accum = 0.5lk;
    /* Simulate left shift by multiplying by power of 2 */
    for (int i = 0; i < 3; i++) {
        long_accum = long_accum * 2.0lk;  /* May overflow for large values */
    }
    
    /* Test 9: Boundary comparisons that trigger the uncovered code */
    f test_value = 0.999999r;
    /* This comparison should trigger max/min range checks */
    if (test_value > 0.999998r && test_value < 1.0r) {
        result += 1000;
    }
    
    /* Test 10: Direct saturation boundary tests */
    usaccum sat_boundary_test = 0.9999999999k;
    /* Force saturation by exceeding bounds */
    usaccum saturated = sat_boundary_test + 0.0000000001k;
    
    /* Prevent dead code elimination */
    result += (int)(sf_max * 1000);
    result += (int)(accum_max * 1000);
    result += int_from_fract;
    result += (int)(const_fract * 1000);
    result += (int)(long_accum * 1000);
    result += (int)(saturated * 1000);
    
    printf("Result: %d\n", result);
    
    return 0;
}
