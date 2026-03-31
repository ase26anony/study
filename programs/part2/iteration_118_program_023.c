/* Test program for GCC fixed-point arithmetic range calculations */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-test.c */

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
    { /* Push unsigned fract to max */
        .short_fract = 0.999999hr,
        .accum = 0.999999k,
        .unsigned_fract = 0.999999r,
        .unsigned_accum = 0.999999uk,
        .sat_short_fract = 0.999999hr,
        .sat_unsigned_accum = 0.999999uk
    },
    { /* Push signed fract to min */
        .short_fract = -0.999999hr,
        .accum = -0.999999k,
        .unsigned_fract = 0.0r,
        .unsigned_accum = 0.0uk,
        .sat_short_fract = -0.999999hr,
        .sat_unsigned_accum = 0.0uk
    },
    { /* Middle values */
        .short_fract = 0.5hr,
        .accum = 0.5k,
        .unsigned_fract = 0.5r,
        .unsigned_accum = 0.5uk,
        .sat_short_fract = 0.5hr,
        .sat_unsigned_accum = 0.5uk
    }
};

/* Compile-time constant expressions that force range calculations */
#if defined(__GNUC__)
/* Use __builtin_constant_p to ensure compile-time evaluation */
#define CONST_EXPR(expr) (__builtin_constant_p(expr) ? (expr) : (expr))
#else
#define CONST_EXPR(expr) (expr)
#endif

/* Function to perform operations that trigger range checks */
static void test_fixed_point_operations(void) {
    /* Declare variables at representable limits */
    const usf max_uf = 0.999999r;      /* Max unsigned fract */
    const usf min_uf = 0.0r;           /* Min unsigned fract */
    const f max_f = 0.999999r;         /* Max signed fract */
    const f min_f = -0.999999r;        /* Min signed fract */
    const ua max_ua = 0.999999uk;      /* Max unsigned accum */
    const a max_a = 0.999999k;         /* Max signed accum */
    const a min_a = -0.999999k;        /* Min signed accum */
    
    /* Saturated types that will trigger overflow/underflow checks */
    usfract sat_uf1 = 0.999999r;
    usfract sat_uf2 = 0.999999r;
    sfract sat_f1 = 0.999999r;
    sfract sat_f2 = -0.999999r;
    usaccum sat_ua1 = 0.999999uk;
    usaccum sat_ua2 = 0.999999uk;
    saccum sat_a1 = 0.999999k;
    saccum sat_a2 = -0.999999k;
    
    /* Force constant folding with ternary operator */
    volatile int result1 = CONST_EXPR((max_uf > 0.5r) ? 1 : 0);
    volatile int result2 = CONST_EXPR((min_f < -0.5r) ? 1 : 0);
    
    /* Operations that may overflow - triggers range calculations */
    usfract sat_sum_uf = sat_uf1 + sat_uf2;  /* Should saturate to max */
    sfract sat_sum_f = sat_f1 + sat_f2;      /* Should stay in range */
    usaccum sat_sum_ua = sat_ua1 + sat_ua2;  /* Should saturate to max */
    saccum sat_sum_a = sat_a1 + sat_a2;      /* Should stay in range */
    
    /* Multiplication at boundaries - likely to overflow */
    usfract sat_mul_uf = sat_uf1 * sat_uf2;  /* 0.999999 * 0.999999 */
    sfract sat_mul_f = sat_f1 * sat_f2;      /* 0.999999 * -0.999999 */
    
    /* Conversions that require range checking */
    volatile int int_from_uf = (int)max_uf;          /* Should be 0 */
    volatile int int_from_f = (int)max_f;            /* Should be 0 */
    volatile float float_from_ua = (float)max_ua;    /* Should be ~1.0 */
    volatile float float_from_a = (float)min_a;      /* Should be ~-1.0 */
    
    /* Complex expression with mixed types */
    volatile float mixed_expr = (float)max_uf + (float)min_f + (float)(max_ua * 0.5uk);
    
    /* Prevent dead code elimination */
    (void)result1;
    (void)result2;
    (void)sat_sum_uf;
    (void)sat_sum_f;
    (void)sat_sum_ua;
    (void)sat_sum_a;
    (void)sat_mul_uf;
    (void)sat_mul_f;
    (void)int_from_uf;
    (void)int_from_f;
    (void)float_from_ua;
    (void)float_from_a;
    (void)mixed_expr;
}

/* Main function with loop to allow unrolling and constant propagation */
int main(void) {
    /* Initialize variables with boundary values */
    usfract sat_uf = 0.999999r;
    sfract sat_f = 0.999999r;
    usaccum sat_ua = 0.999999uk;
    saccum sat_a = 0.999999k;
    
    /* Small fixed loop for unrolling */
    for (int i = 0; i < 3; i++) {
        /* Conditional assignments based on fixed-point comparisons */
        if (sat_uf > 0.5r) {
            sat_uf = sat_uf * 0.999999r;  /* Push toward max */
        } else {
            sat_uf = sat_uf + 0.25r;      /* Push toward max */
        }
        
        if (sat_f > 0.0r) {
            sat_f = sat_f - 0.000001r;    /* Small decrement */
        } else {
            sat_f = sat_f + 0.000001r;    /* Small increment */
        }
        
        /* Operations that may trigger overflow checks */
        sat_ua = sat_ua + 0.000001uk;
        sat_a = sat_a - 0.000001k;
        
        /* Use array element based on fixed-point comparison */
        volatile int idx = (sat_uf > 0.75r) ? 0 : 1;
        volatile float val = (float)fp_array[idx].unsigned_fract;
        (void)val;
    }
    
    /* Test the operations function */
    test_fixed_point_operations();
    
    /* Final conversions to prevent elimination */
    volatile int final_int = (int)sat_uf;
    volatile float final_float = (float)sat_f;
    
    /* Use the results */
    printf("Final int: %d\n", final_int);
    printf("Final float: %f\n", final_float);
    
    return 0;
}

/* Additional compile-time tests using preprocessor */
#if defined(__GNUC__) && __GNUC__ >= 7
/* This section only compiled if GCC can evaluate as constant */
static const long _Accum compile_time_acc = 0.999999lk;
static const unsigned long _Accum compile_time_uacc = 0.999999ulk;

/* Array indexing with fixed-point derived index */
#define FIXED_INDEX (int)(compile_time_acc * 10lk)
static const int fixed_array[10] = {0};
static const int fixed_idx_value = fixed_array[FIXED_INDEX];
#endif
