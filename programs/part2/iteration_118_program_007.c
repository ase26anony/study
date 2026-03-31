/* Target: fixed-value.cc uncovered lines 264-277 */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original test.c */

#include <stdio.h>

/* Fixed-point type declarations covering various modes */
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

/* Saturated types */
typedef _Sat short _Fract ssf;
typedef _Sat _Fract sfract;
typedef _Sat long _Fract slf;
typedef _Sat short _Accum ssa;
typedef _Sat _Accum saccum;
typedef _Sat long _Accum sla;
typedef unsigned _Sat short _Fract ussf;
typedef unsigned _Sat _Fract usfract;
typedef unsigned _Sat long _Accum usla;

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    sf short_fract;
    a accum;
    uf unsigned_fract;
    ua unsigned_accum;
    ssf sat_short_fract;
    saccum sat_accum;
};

/* Array initialized with fixed-point constants at boundaries */
static const struct FixedPointData fp_array[] = {
    { /* Max values for signed types */
        .short_fract = 0.999969r,  /* ~0.999969 (max for Q0.15) */
        .accum = 0.999999999r,     /* Max for _Accum */
        .unsigned_fract = 0.999999r, /* Max for unsigned _Fract */
        .unsigned_accum = 0.999999999r,
        .sat_short_fract = 0.999969r,
        .sat_accum = 0.999999999r
    },
    { /* Min values for signed types */
        .short_fract = -1.0r,      /* Min for signed Q1.15 */
        .accum = -1.0r,
        .unsigned_fract = 0.0r,    /* Min for unsigned */
        .unsigned_accum = 0.0r,
        .sat_short_fract = -1.0r,
        .sat_accum = -1.0r
    },
    { /* Boundary values */
        .short_fract = 0.5r,
        .accum = -0.5r,
        .unsigned_fract = 0.999999r,
        .unsigned_accum = 0.999999999r,
        .sat_short_fract = 0.999969r,
        .sat_accum = 0.999999999r
    }
};

/* Function to force constant folding with boundary checks */
static constexpr _Accum boundary_check(_Accum val) {
    /* This should trigger range calculation in constant folder */
    return (val > 0.999999999r) ? 0.999999999r : 
           (val < -1.0r) ? -1.0r : val;
}

/* Macro to create compile-time conditional blocks */
#define CHECK_CONSTANT(expr) \
    if (__builtin_constant_p(expr)) { \
        static const int idx = (int)((expr) * 100); \
        volatile int dummy = idx; \
        (void)dummy; \
    }

int main(void) {
    /* Declare and initialize fixed-point variables at boundaries */
    const _Fract max_fract = 0.999999r;      /* Max for _Fract */
    const _Fract min_fract = -1.0r;          /* Min for _Fract */
    const unsigned _Fract umax_fract = 0.999999r;
    
    const _Accum max_accum = 0.999999999r;   /* Max for _Accum */
    const _Accum min_accum = -1.0r;          /* Min for _Accum */
    const _Accum half_accum = 0.5r;
    
    const long _Accum max_laccum = 0.999999999999999999r;
    const long _Accum min_laccum = -1.0r;
    
    /* Saturated types with operations that should overflow */
    unsigned _Sat _Fract usat_f = 0.999999r;
    _Sat _Accum sat_a = 0.999999999r;
    _Sat long _Accum sat_la = 0.999999999999999999r;
    
    /* Force constant folding with ternary operators */
    static const _Accum folded1 = (max_accum > 0.5r) ? max_accum + 0.000000001r : 0.0r;
    static const _Accum folded2 = (min_accum < -0.5r) ? min_accum - 0.000000001r : 0.0r;
    
    /* Use __builtin_constant_p to create compile-time paths */
    CHECK_CONSTANT(max_fract);
    CHECK_CONSTANT(min_accum);
    
    /* Mixed-type expressions that require range checking */
    const int int_from_fract = (int)(max_fract * 1000);
    const float float_from_accum = (float)max_accum;
    const _Accum accum_from_int = (_Accum)255 / 256;
    
    /* Array indexing with fixed-point derived index */
    const int idx = (int)(max_fract * 2);
    volatile int result = fp_array[idx].short_fract;
    
    /* Loop with fixed iteration to allow unrolling and constant propagation */
    _Accum loop_accum = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Operations that approach boundaries */
        loop_accum = loop_accum + 0.25r;
        
        /* Conditional assignments based on fixed-point comparisons */
        if (loop_accum > 0.5r) {
            loop_accum = loop_accum * 1.5r;  /* May overflow */
        } else {
            loop_accum = loop_accum * 0.5r;
        }
        
        /* Mix with integer arithmetic */
        loop_accum = loop_accum + (_Accum)i * 0.1r;
    }
    
    /* Saturation arithmetic that should trigger boundary checks */
    usat_f = usat_f + 0.000001r;  /* Should saturate at max */
    sat_a = sat_a * 1.1r;         /* Should saturate at max */
    sat_la = sat_la + sat_la;     /* Should saturate at max */
    
    /* Negative saturation tests */
    _Sat _Accum neg_sat = -0.999999999r;
    neg_sat = neg_sat - 0.000000001r;  /* Should saturate at min */
    
    /* Complex expression with multiple conversions */
    const long _Accum complex_expr = 
        (long _Accum)max_fract * (long _Accum)max_accum / 
        (long _Accum)((int)min_fract + 2);
    
    /* Prevent dead code elimination */
    volatile _Accum v1 = loop_accum;
    volatile _Sat _Accum v2 = sat_a;
    volatile unsigned _Sat _Fract v3 = usat_f;
    volatile long _Accum v4 = complex_expr;
    
    /* Use results to prevent optimization */
    printf("Results: %d %f %f %f\n", 
           int_from_fract,
           (double)v1,
           (double)v2,
           (double)v4);
    
    return 0;
}

/* Additional compile-time tests */
#if defined(__OPTIMIZE__)
/* This struct initialization should be evaluated at compile time */
static const struct {
    _Accum a;
    _Fract f;
} compile_time_struct = {
    .a = boundary_check(1.5r),  /* Should clamp to max */
    .f = boundary_check(-1.5r)  /* Should clamp to min */
};
#endif

/* Edge case: shifting operations on fixed-point (through integer casts) */
static inline unsigned long long fract_to_bits(_Fract f) {
    union {
        _Fract f;
        unsigned long long bits;
    } u = { .f = f };
    return u.bits;
}

/* Test function that uses the uncovered range comparison logic */
void test_range_calculation(void) {
    /* These initializations should trigger the range calculation code */
    const _Accum at_max = 0.999999999r;
    const _Accum beyond_max = at_max + (_Accum)0.0000000005r;
    const _Accum at_min = -1.0r;
    const _Accum beyond_min = at_min - (_Accum)0.0000000005r;
    
    /* Casts that require range checking */
    const short _Fract sf_from_accum = (short _Fract)at_max;
    const _Accum accum_from_sf = (_Accum)sf_from_accum;
    
    /* Prevent optimization */
    volatile _Accum v_max = at_max;
    volatile _Accum v_beyond = beyond_max;
    volatile _Accum v_min = at_min;
    volatile _Accum v_beyond_min = beyond_min;
    (void)v_max; (void)v_beyond; (void)v_min; (void)v_beyond_min;
}
