/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Fixed-point type definitions covering various combinations */
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
typedef _Sat unsigned short _Fract susf;
typedef _Sat unsigned _Fract suf;
typedef _Sat unsigned long _Fract sulf;
typedef _Sat unsigned short _Accum susa;
typedef _Sat unsigned _Accum sua;
typedef _Sat unsigned long _Accum sula;

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointStruct {
    sf short_fract;
    a accum;
    uf unsigned_fract;
    ua unsigned_accum;
    sfx sat_fract;
    sua sat_unsigned_accum;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointStruct fp_array[] = {
    { /* Push to maximum representable values */
        .short_fract = 0.999999hr,  /* Max for short _Fract */
        .accum = 32767.999999k,     /* Near max for _Accum */
        .unsigned_fract = 0.999999r, /* Max for unsigned _Fract */
        .unsigned_accum = 65535.999999uk, /* Near max for unsigned _Accum */
        .sat_fract = 0.999999r,
        .sat_unsigned_accum = 65535.999999uk
    },
    { /* Push to minimum representable values */
        .short_fract = -0.999999hr,
        .accum = -32768.999999k,
        .unsigned_fract = 0.0r,
        .unsigned_accum = 0.0uk,
        .sat_fract = -0.999999r,
        .sat_unsigned_accum = 0.0uk
    }
};

/* Compile-time constant expressions that force range calculations */
#define FORCE_RANGE_CHECK(expr) \
    (__builtin_constant_p(expr) ? (expr) : (expr))

/* Test function that performs operations likely to trigger the uncovered code */
static void test_fixed_point_range(void) {
    /* Variables at representable limits */
    const sf max_sf = 0.999999hr;
    const sf min_sf = -0.999999hr;
    const a max_a = 32767.999999k;
    const a min_a = -32768.999999k;
    const uf max_uf = 0.999999r;
    const ua max_ua = 65535.999999uk;
    
    /* Saturated types with operations that will overflow/underflow */
    sfx sat1 = 0.999999r;
    sfx sat2 = 0.999999r;
    sua sat_ua1 = 65535.999999uk;
    sua sat_ua2 = 65535.999999uk;
    
    /* Force constant folding with ternary operator */
    static const int use_max = 1;
    const a test_accum = use_max ? max_a : min_a;
    const uf test_ufract = use_max ? max_uf : 0.0r;
    
    /* Operations that may overflow and trigger range checks */
    sat1 = sat1 + 0.1r;  /* Will saturate for _Sat _Fract */
    sat_ua1 = sat_ua1 + 1000.0uk;  /* Will saturate for _Sat unsigned _Accum */
    
    /* Multiplication near limits */
    a mult_result = test_accum * 1.0001k;
    uf mult_ufract = test_ufract * 1.1r;
    
    /* Conversions that require range checking */
    int int_from_accum = (int)test_accum;
    float float_from_fract = (float)test_ufract;
    
    /* Shift operations (simulated with multiplication by power of 2) */
    a shifted = test_accum * 2.0k;  /* Equivalent to left shift */
    
    /* Complex expression with multiple operations */
    a complex_expr = (test_accum + 1000.0k) * 0.5k - 500.0k;
    
    /* Use __builtin_constant_p to create conditional compilation */
#if defined(__GNUC__)
    if (__builtin_constant_p(max_sf)) {
        /* This block only compiled if max_sf is constant */
        const int idx = (int)(max_sf * 1000);
        volatile int dummy = idx;  /* Prevent optimization */
    }
#endif
    
    /* Loop with fixed iterations to allow unrolling */
    a loop_accum = 0.0k;
    for (int i = 0; i < 4; i++) {
        /* Conditional based on fixed-point comparison */
        if (loop_accum > 10000.0k) {
            loop_accum = loop_accum * 0.9k;
        } else {
            loop_accum = loop_accum + 10000.0k;
        }
        
        /* Mix with integer arithmetic */
        loop_accum = loop_accum + (a)(i * 1000);
    }
    
    /* Final conversions to prevent dead code elimination */
    volatile int v1 = int_from_accum;
    volatile float v2 = float_from_fract;
    volatile a v3 = mult_result;
    volatile uf v4 = mult_ufract;
    volatile a v5 = shifted;
    volatile a v6 = complex_expr;
    volatile a v7 = loop_accum;
    volatile sfx v8 = sat1;
    volatile sua v9 = sat_ua1;
    
    (void)v1; (void)v2; (void)v3; (void)v4;
    (void)v5; (void)v6; (void)v7; (void)v8; (void)v9;
}

/* Additional test with explicit boundary values */
static void test_boundary_values(void) {
    /* These should trigger the max_r/min_r comparisons */
    const la max_la = 9223372036854775807.999999lk;
    const la min_la = -9223372036854775808.999999lk;
    
    /* Operations that push beyond boundaries */
    sla sat_la = max_la;
    sat_la = sat_la + 1.0lk;  /* Should saturate */
    
    /* Conversion that requires precise range checking */
    long long int_from_la = (long long)max_la;
    
    /* Use in array indexing (converted to integer) */
    volatile long long idx = int_from_la & 0xFF;
    
    /* Force evaluation of boundary conditions */
    const int is_max = (max_la > 9223372036854775800.0lk) ? 1 : 0;
    const int is_min = (min_la < -9223372036854775800.0lk) ? 1 : 0;
    
    volatile int v1 = is_max;
    volatile int v2 = is_min;
    volatile sla v3 = sat_la;
    
    (void)v1; (void)v2; (void)v3;
}

/* Test with mixed-type expressions */
static void test_mixed_expressions(void) {
    /* Mix different fixed-point types */
    sf sf_val = 0.5hr;
    a a_val = 1000.5k;
    uf uf_val = 0.75r;
    
    /* Convert and mix types */
    a mixed1 = (a)sf_val + a_val;
    float mixed2 = (float)a_val + (float)uf_val;
    
    /* Conditional with fixed-point comparison */
    a result = (sf_val > 0.3hr) ? (a_val * 2.0k) : (a_val * 0.5k);
    
    /* Chain of operations */
    a chain = 1.0k;
    chain = chain * 1.1k;
    chain = chain + 100.0k;
    chain = chain - 50.0k;
    chain = chain * 0.9k;
    
    volatile a v1 = mixed1;
    volatile float v2 = mixed2;
    volatile a v3 = result;
    volatile a v4 = chain;
    
    (void)v1; (void)v2; (void)v3; (void)v4;
}

int main(void) {
    printf("Testing fixed-point range calculations...\n");
    
    /* Access array to ensure initialization is evaluated */
    volatile const struct FixedPointStruct *fp_ptr = fp_array;
    (void)fp_ptr;
    
    /* Run tests */
    test_fixed_point_range();
    test_boundary_values();
    test_mixed_expressions();
    
    printf("Tests completed.\n");
    
    return 0;
}
