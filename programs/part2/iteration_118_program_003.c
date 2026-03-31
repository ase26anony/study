/* Test program for GCC fixed-point arithmetic coverage */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-test.c */

#include <stdio.h>

/* Fixed-point type definitions */
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
typedef _Sat _Accum sacc;
typedef _Sat long _Accum sla;
typedef _Sat unsigned short _Fract susf;
typedef _Sat unsigned _Fract suf;
typedef _Sat unsigned long _Fract sulf;
typedef _Sat unsigned short _Accum susa;
typedef _Sat unsigned _Accum sua;
typedef _Sat unsigned long _Accum sula;

/* Struct with mixed fixed-point types */
struct FixedPointStruct {
    sf short_fract;
    a accum;
    uf unsigned_fract;
    sua sat_unsigned_accum;
    float float_member;
    int int_member;
};

/* Array with fixed-point initializers */
static const struct FixedPointStruct fp_array[] = {
    { /* Push to max representable values */
        .short_fract = 0.999999hr,  /* Max for short _Fract */
        .accum = 32767.999999k,     /* Near max for _Accum */
        .unsigned_fract = 0.999999r, /* Max for unsigned _Fract */
        .sat_unsigned_accum = 65535.999999uk, /* Max for unsigned short _Accum */
        .float_member = 1.0f,
        .int_member = 32767
    },
    { /* Push to min representable values */
        .short_fract = -1.0hr,      /* Min for short _Fract */
        .accum = -32768.0k,         /* Min for _Accum */
        .unsigned_fract = 0.0r,     /* Min for unsigned _Fract */
        .sat_unsigned_accum = 0.0uk, /* Min for unsigned short _Accum */
        .float_member = -1.0f,
        .int_member = -32768
    },
    { /* Boundary values that trigger range checks */
        .short_fract = 0.5hr,
        .accum = 0.0k,
        .unsigned_fract = 0.5r,
        .sat_unsigned_accum = 32767.5uk,
        .float_member = 0.5f,
        .int_member = 0
    }
};

/* Compile-time constant expressions that force range calculations */
#if defined(__GNUC__)
/* These will be evaluated at compile-time and trigger the uncovered logic */
static const sacc SAT_MAX = 32767.999999k;
static const sacc SAT_MIN = -32768.0k;
static const sua UNSAT_MAX = 65535.999999uk;
static const sua UNSAT_MIN = 0.0uk;

/* Force constant folding with ternary operator */
static const a CONST_FOLDED = (__builtin_constant_p(1) ? 32767.999999k : 0.0k);
static const sua CONST_FOLDED_SAT = (__builtin_constant_p(1) ? 65535.999999uk : 0.0uk);
#endif

/* Function to perform operations that trigger range checks */
static void perform_fixed_point_operations(void) {
    /* Declare variables with extreme values */
    volatile sf v_sf = 0.999999hr;  /* Will be optimized but volatile prevents removal */
    volatile a v_acc = 32767.999999k;
    volatile uf v_uf = 0.999999r;
    volatile sua v_sua = 65535.999999uk;
    
    /* Saturated types that will trigger saturation logic */
    ssf sat_sf = 0.999999hr;
    sacc sat_acc = 32767.999999k;
    suf sat_uf = 0.999999r;
    sua sat_ua = 65535.999999uk;
    
    /* Operations that may overflow/underflow */
    for (int i = 0; i < 3; i++) {  /* Small loop for unrolling */
        /* Multiplication near limits */
        sat_sf = sat_sf * 0.999999hr;
        sat_acc = sat_acc * 0.999999k;
        sat_uf = sat_uf * 0.999999r;
        sat_ua = sat_ua * 0.999999uk;
        
        /* Addition that may saturate */
        if (sat_sf > 0.5hr) {
            sat_sf = sat_sf + 0.5hr;
        }
        if (sat_acc > 16384.0k) {
            sat_acc = sat_acc + 16384.0k;  /* May overflow */
        }
        if (sat_uf > 0.5r) {
            sat_uf = sat_uf + 0.5r;  /* May saturate for unsigned */
        }
        if (sat_ua > 32768.0uk) {
            sat_ua = sat_ua + 32768.0uk;  /* Will saturate */
        }
        
        /* Conversions that trigger range checks */
        int int_from_fixed = (int)sat_acc;
        float float_from_fixed = (float)sat_uf;
        
        /* Use results to prevent dead code elimination */
        v_sf = sat_sf;
        v_acc = sat_acc;
        v_uf = sat_uf;
        v_sua = sat_ua;
    }
    
    /* Explicit overflow operations */
    sacc overflow_test = 32767.999999k;
    overflow_test = overflow_test * 2.0k;  /* Will saturate */
    
    sua uoverflow_test = 65535.999999uk;
    uoverflow_test = uoverflow_test + 1.0uk;  /* Will saturate */
    
    /* Underflow test for signed */
    sacc underflow_test = -32768.0k;
    underflow_test = underflow_test - 1.0k;  /* Will saturate */
    
    /* Mixed-type expressions */
    a mixed_expr = v_acc + (a)v_uf * 0.5k;
    
    /* Cast to volatile to ensure computation */
    volatile sacc volatile_sat = overflow_test;
    volatile sua volatile_usat = uoverflow_test;
    (void)volatile_sat;
    (void)volatile_usat;
}

/* Another function with compile-time evaluable expressions */
static int compile_time_checks(void) {
    /* These should be evaluated at compile-time */
    const sf max_sf = 0.999999hr;
    const sf min_sf = -1.0hr;
    const uf max_uf = 0.999999r;
    
    /* Operations that might be constant-folded */
    const sf sf_product = max_sf * max_sf;
    const uf uf_sum = max_uf + max_uf;  /* Will wrap/saturate at compile-time */
    
    /* Conditional based on fixed-point comparison */
    const int result = (max_sf > 0.9hr) ? 1 : 0;
    const int result2 = (min_sf < -0.9hr) ? 1 : 0;
    
    /* Array indexing with fixed-point conversion */
    const int idx = (int)(max_sf * 10);
    const char test_array[10] = {0};
    char elem = test_array[idx < 10 ? idx : 0];
    
    return result + result2 + elem;
}

int main(void) {
    printf("Testing fixed-point arithmetic coverage...\n");
    
    /* Trigger operations */
    perform_fixed_point_operations();
    
    /* Compile-time checks */
    int ct_result = compile_time_checks();
    
    /* Use array initializers */
    volatile struct FixedPointStruct test_struct = fp_array[0];
    (void)test_struct;
    
    /* More complex compile-time expression */
#if defined(__GNUC__) && __GNUC__ >= 5
    /* Use __builtin_constant_p to ensure compile-time evaluation */
    if (__builtin_constant_p(CONST_FOLDED)) {
        volatile a forced_eval = CONST_FOLDED * 2.0k;
        (void)forced_eval;
    }
#endif
    
    /* Print something to prevent optimization */
    printf("Result: %d\n", ct_result);
    
    return 0;
}

/* Additional compile-time tests in global scope */
/* These will be evaluated during compilation */
static const a GLOBAL_MAX = 32767.999999k;
static const a GLOBAL_MIN = -32768.0k;
static const a GLOBAL_PROD = GLOBAL_MAX * 0.5k;
static const a GLOBAL_SUM = GLOBAL_MAX + GLOBAL_MIN;

/* Saturation tests at global scope */
static const sua GLOBAL_SAT_MAX = 65535.999999uk;
static const sua GLOBAL_SAT_ADD = GLOBAL_SAT_MAX + 1.0uk;  /* Should saturate */
static const sacc GLOBAL_SAT_SUB = GLOBAL_MIN - 1.0k;      /* Should saturate */

/* Mixed-type conversion at global scope */
static const int INT_FROM_FIXED = (int)GLOBAL_MAX;
static const float FLOAT_FROM_FIXED = (float)GLOBAL_SAT_MAX;
