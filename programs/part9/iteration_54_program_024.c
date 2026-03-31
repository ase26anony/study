/* Test program to exercise fixed-point type boundary calculations in GCC */
/* Compile with: gcc -std=c99 -ffixed-point -O2 -ftest-coverage -fprofile-arcs */

#include <stdio.h>

/* Test various fixed-point types with different integer/fractional bit combinations */
int main(void) {
    /* Short fract types (7 fractional bits) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    short _Fract sf_mid = 0.5r;
    
    /* Fract types (15 fractional bits) */
    _Fract f_max = 0.9999999999999999r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;
    _Fract f_mid = 0.0r;
    
    /* Long fract types (31 fractional bits) */
    long _Fract lf_max = 0.9999999999999999999999999999999r;
    long _Fract lf_min = -1.0r;
    long _Fract lf_mid = -0.5r;
    
    /* Short accum types (7 integer bits, 8 fractional bits) */
    short _Accum sa_max = 127.99999999976716936rk;
    short _Accum sa_min = -128.0rk;
    short _Accum sa_mid = 0.0rk;
    
    /* Accum types (15 integer bits, 16 fractional bits) */
    _Accum a_max = 32767.9999999999999999rk;
    _Accum a_min = -32768.0rk;
    _Accum a_mid = 100.5rk;
    
    /* Long accum types (31 integer bits, 32 fractional bits) */
    long _Accum la_max = 2147483647.9999999999999999999999999999999rk;
    long _Accum la_min = -2147483648.0rk;
    long _Accum la_mid = -1000.25rk;
    
    /* Unsigned variants */
    unsigned short _Fract usf_max = 0.99999999976716936ur;
    unsigned short _Fract usf_min = 0.0ur;
    
    unsigned _Fract uf_max = 0.9999999999999999ur;
    unsigned _Fract uf_min = 0.0ur;
    
    unsigned short _Accum usa_max = 255.99999999976716936urk;
    unsigned short _Accum usa_min = 0.0urk;
    
    unsigned _Accum ua_max = 65535.9999999999999999urk;
    unsigned _Accum ua_min = 0.0urk;
    
    /* Saturated variants */
    _Sat short _Fract ssf = 0.75r;
    _Sat _Fract sf = -0.25r;
    _Sat short _Accum ssa = 50.5rk;
    _Sat _Accum sa = -100.75rk;
    
    /* Test operations that might trigger boundary checks */
    
    /* 1. Assignment with overflow potential */
    short _Accum sa_overflow_test = 128.0rk;  /* Should be clamped for saturating */
    _Accum a_overflow_test = 32768.0rk;
    
    /* 2. Arithmetic operations */
    short _Fract sf_sum = sf_max + sf_mid;
    _Accum a_diff = a_max - a_mid;
    long _Accum la_prod = la_mid * 2.0rk;
    
    /* 3. Type conversions with potential overflow */
    _Accum from_short = sa_max;
    short _Accum to_short = a_mid;
    
    /* 4. Comparisons that might use the boundary checking logic */
    int cmp1 = (sf_max > 0.9r);
    int cmp2 = (a_min < -32767.5rk);
    int cmp3 = (la_max == la_max);
    
    /* 5. Edge case: minimum values */
    _Fract f_neg_min = -0.9999999999999999r;
    long _Accum la_neg_near_min = -2147483647.9999999999999999999999999999999rk;
    
    /* 6. Test with fractional part only (no integer bits) */
    /* This should trigger the specific uncovered code path where i_f_bits = 0 */
    /* Use _Fract types which typically have 0 integer bits */
    _Fract pure_fract_max = 0.9999999999999999r;
    _Fract pure_fract_min = -1.0r;
    
    /* Force computation of boundary values by using in expressions */
    volatile _Fract vf1 = pure_fract_max * 1.0r;
    volatile _Fract vf2 = pure_fract_min / 1.0r;
    
    /* 7. Test with different i_f_bits configurations */
    /* short _Accum has i=7, f=8 bits */
    /* _Accum has i=15, f=16 bits */
    /* long _Accum has i=31, f=32 bits */
    
    /* Create expressions that require boundary value computation */
    short _Accum sa_boundary = 127.99999999976716936rk + 0.0000000001rk;
    _Accum a_boundary = 32767.9999999999999999rk - 0.0000000000000001rk;
    long _Accum la_boundary = -2147483648.0rk * 0.9999999999999999999999999999999rk;
    
    /* 8. Test the specific comparison pattern from uncovered lines */
    /* The uncovered code checks: if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))) */
    /* We need values that might trigger both parts of this condition */
    
    /* Use values very close to maximum to potentially trigger the second part */
    unsigned short _Fract usf_near_max = 0.99999999976716935ur;  /* Slightly less than max */
    unsigned _Accum ua_near_max = 65535.9999999999999998urk;
    
    /* 9. Test with zero and negative values for signed types */
    short _Accum sa_zero = 0.0rk;
    _Accum a_neg_small = -0.0000000000000001rk;
    long _Accum la_neg_large = -1000000000.5rk;
    
    /* 10. Complex expressions that might trigger multiple code paths */
    long _Accum complex_expr = (la_max + la_min) / 2.0rk + la_mid * 3.0rk - la_max / 4.0rk;
    
    /* Prevent unused variable warnings */
    (void)sf_max; (void)sf_min; (void)sf_mid;
    (void)f_max; (void)f_min; (void)f_mid;
    (void)lf_max; (void)lf_min; (void)lf_mid;
    (void)sa_max; (void)sa_min; (void)sa_mid;
    (void)a_max; (void)a_min; (void)a_mid;
    (void)la_max; (void)la_min; (void)la_mid;
    (void)usf_max; (void)usf_min;
    (void)uf_max; (void)uf_min;
    (void)usa_max; (void)usa_min;
    (void)ua_max; (void)ua_min;
    (void)ssf; (void)sf; (void)ssa; (void)sa;
    (void)sa_overflow_test; (void)a_overflow_test;
    (void)sf_sum; (void)a_diff; (void)la_prod;
    (void)from_short; (void)to_short;
    (void)cmp1; (void)cmp2; (void)cmp3;
    (void)f_neg_min; (void)la_neg_near_min;
    (void)pure_fract_max; (void)pure_fract_min;
    (void)vf1; (void)vf2;
    (void)sa_boundary; (void)a_boundary; (void)la_boundary;
    (void)usf_near_max; (void)ua_near_max;
    (void)sa_zero; (void)a_neg_small; (void)la_neg_large;
    (void)complex_expr;
    
    printf("Fixed-point boundary test completed\n");
    
    return 0;
}
