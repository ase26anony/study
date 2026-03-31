/* Test fixed-point boundary value calculations */
/* Compile with: gcc -std=c99 -ffixed-point -O2 -c test_fixed.c */

#include <stdint.h>

/* Test various fixed-point types with different integer/fractional bit combinations */
void test_fixed_boundaries(void) {
    /* Short fract (7 fractional bits, 1 integer bit including sign) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max: 1 - 2^-7 */
    short _Fract sf_min = -1.0r;                 /* Min value */
    
    /* Unsigned short fract (7 fractional bits, 0 integer bits) */
    unsigned short _Fract usf_max = 0.99999999976716936r;
    unsigned short _Fract usf_min = 0.0r;
    
    /* Fract (15 fractional bits, 1 integer bit including sign) */
    _Fract f_max = 0.99999999999999645r;  /* 1 - 2^-15 */
    _Fract f_min = -1.0r;
    
    /* Long fract (31 fractional bits, 1 integer bit including sign) */
    long _Fract lf_max = 0.999999999999999999999999999999767169356r;
    long _Fract lf_min = -1.0r;
    
    /* Short accum (7 fractional bits, 8 integer bits including sign) */
    short _Accum sa_max = 127.99999999976716936r;  /* 127 + (1 - 2^-7) */
    short _Accum sa_min = -128.0r;
    
    /* Unsigned short accum (7 fractional bits, 8 integer bits) */
    unsigned short _Accum usa_max = 255.99999999976716936r;  /* 255 + (1 - 2^-7) */
    unsigned short _Accum usa_min = 0.0r;
    
    /* Accum (15 fractional bits, 16 integer bits including sign) */
    _Accum a_max = 32767.99999999999999645r;  /* 32767 + (1 - 2^-15) */
    _Accum a_min = -32768.0r;
    
    /* Long accum (31 fractional bits, 32 integer bits including sign) */
    long _Accum la_max = 2147483647.999999999999999999999999999999767169356r;
    long _Accum la_min = -2147483648.0r;
    
    /* Saturated variants */
    _Sat short _Fract ssf = 0.5r;
    _Sat unsigned short _Fract susf = 0.5r;
    _Sat short _Accum ssa = 100.0r;
    _Sat unsigned short _Accum susa = 100.0r;
    
    /* Test overflow/underflow with saturated arithmetic */
    ssf = ssf + ssf + ssf;  /* Should saturate to max */
    susf = susf + susf + susf;
    ssa = ssa + ssa + ssa + ssa + ssa;  /* Should saturate */
    susa = susa + susa + susa + susa + susa;
    
    /* Test boundary assignments with explicit constants */
    /* These should trigger the boundary comparison logic */
    short _Fract sf_boundary = 0.9921875r;  /* 127/128 = 1 - 2^-7 */
    _Fract f_boundary = 0.999969482421875r; /* 32767/32768 = 1 - 2^-15 */
    long _Fract lf_boundary = 0.9999999995343387r; /* Very close to 1 */
    
    /* Test accum boundaries */
    short _Accum sa_boundary = 127.9921875r;  /* 127 + 127/128 */
    _Accum a_boundary = 32767.999969482421875r;
    long _Accum la_boundary = 2147483647.9999999995343387r;
    
    /* Force computations that might exceed boundaries */
    volatile short _Fract vsf = sf_max;
    vsf = vsf + (_Fract)0.0001r;  /* Should wrap or saturate */
    
    volatile _Accum va = a_max;
    va = va + (_Accum)0.0001r;
    
    /* Test with different fractional bit counts through type conversions */
    long _Accum la_from_sa = sa_max;  /* Conversion with different i_f_bits */
    _Accum a_from_la = la_max;        /* Another conversion */
    
    /* Test unsigned boundaries */
    unsigned short _Accum usa_boundary = 255.9921875r;
    unsigned _Accum ua_boundary = 65535.999969482421875r;
    
    /* Mix operations to trigger various code paths */
    _Accum mixed = a_boundary + sa_boundary - f_boundary * 2.0r;
    
    /* Prevent dead code elimination */
    (void)sf_max; (void)sf_min;
    (void)usf_max; (void)usf_min;
    (void)f_max; (void)f_min;
    (void)lf_max; (void)lf_min;
    (void)sa_max; (void)sa_min;
    (void)usa_max; (void)usa_min;
    (void)a_max; (void)a_min;
    (void)la_max; (void)la_min;
    (void)ssf; (void)susf; (void)ssa; (void)susa;
    (void)sf_boundary; (void)f_boundary; (void)lf_boundary;
    (void)sa_boundary; (void)a_boundary; (void)la_boundary;
    (void)vsf; (void)va;
    (void)la_from_sa; (void)a_from_la;
    (void)usa_boundary; (void)ua_boundary;
    (void)mixed;
}

/* Additional test with compile-time constant expressions */
/* These should be evaluated at compile-time, hitting the uncovered lines */
struct FixedConstants {
    short _Fract sf_ct;
    _Fract f_ct;
    long _Fract lf_ct;
    short _Accum sa_ct;
    _Accum a_ct;
    long _Accum la_ct;
};

/* Constants near boundaries */
static const struct FixedConstants test_constants = {
    .sf_ct = 0.99999999976716936r,  /* Max short _Fract */
    .f_ct = 0.99999999999999645r,   /* Max _Fract */
    .lf_ct = 0.999999999999999999999999999999767169356r, /* Max long _Fract */
    .sa_ct = 127.99999999976716936r, /* Max short _Accum */
    .a_ct = 32767.99999999999999645r, /* Max _Accum */
    .la_ct = 2147483647.999999999999999999999999999999767169356r /* Max long _Accum */
};

/* Test initialization with minimum values */
static const struct FixedConstants min_constants = {
    .sf_ct = -1.0r,
    .f_ct = -1.0r,
    .lf_ct = -1.0r,
    .sa_ct = -128.0r,
    .a_ct = -32768.0r,
    .la_ct = -2147483648.0r
};

int main(void) {
    test_fixed_boundaries();
    return 0;
}
