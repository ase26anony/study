/* Test program to cover fixed-value.cc boundary calculations */
/* Compile with: gcc -std=c99 -ffixed-point -O2 test_fixed.c -o test_fixed */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with different integer/fractional bit combinations */
    
    /* Short fract types (7 fractional bits) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    short _Fract sf_mid = 0.5r;
    
    /* Unsigned short fract */
    unsigned short _Fract usf_max = 0.99999999976716936ur;
    unsigned short _Fract usf_min = 0.0ur;
    
    /* Fract types (15 fractional bits) */
    _Fract f_max = 0.9999999999999999r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;
    
    /* Long fract types (31 fractional bits) */
    long _Fract lf_max = 0.9999999999999999999999999999999lr;
    long _Fract lf_min = -1.0lr;
    
    /* Accum types (8 integer, 7 fractional bits) */
    _Accum a_max = 255.99999999976716936k;       /* Near max for _Accum */
    _Accum a_min = -256.0k;
    
    /* Long accum types (24 integer, 7 fractional bits) */
    long _Accum la_max = 16777215.99999999976716936lk;  /* Near max for long _Accum */
    long _Accum la_min = -16777216.0lk;
    
    /* Unsigned accum */
    unsigned _Accum ua_max = 255.99999999976716936uk;
    unsigned _Accum ua_min = 0.0uk;
    
    /* Saturated types - these should trigger boundary checks */
    _Sat short _Fract ssf_max = 0.99999999976716936r;
    _Sat short _Fract ssf_min = -1.0r;
    
    _Sat _Accum sa_max = 255.99999999976716936k;
    _Sat _Accum sa_min = -256.0k;
    
    /* Test overflow/underflow with arithmetic operations */
    short _Fract sf_overflow = sf_max + sf_mid;  /* Should saturate or wrap */
    short _Fract sf_underflow = sf_min - sf_mid;
    
    _Accum a_overflow = a_max + 0.5k;
    _Accum a_underflow = a_min - 0.5k;
    
    /* Test with compile-time constant expressions */
    const short _Fract const_sf = 0.75r;
    const _Accum const_a = 128.5k;
    
    /* Mixed precision operations */
    _Accum mixed = const_sf + const_a;
    
    /* Array initializations with fixed-point constants */
    short _Fract sf_array[3] = {0.0r, 0.5r, 0.99999999976716936r};
    _Accum a_array[3] = {-256.0k, 0.0k, 255.99999999976716936k};
    
    /* Struct with fixed-point members */
    struct {
        short _Fract sf;
        _Accum a;
        long _Accum la;
    } fixed_struct = {
        0.99999999976716936r,
        255.99999999976716936k,
        16777215.99999999976716936lk
    };
    
    /* Test boundary comparisons */
    int cmp1 = (sf_max > 0.9r);
    int cmp2 = (a_min < -255.0k);
    int cmp3 = (la_max == 16777215.99999999976716936lk);
    
    /* Force evaluation of extreme values through type conversions */
    double d_from_sf = (double)sf_max;
    float f_from_a = (float)a_max;
    
    /* Test with conditional expressions */
    short _Fract sf_cond = (cmp1 ? sf_max : sf_min);
    _Accum a_cond = (cmp2 ? a_min : a_max);
    
    /* Test shift operations (if supported in constant expressions) */
    /* Note: Fixed-point shift operations might not be standard C */
    
    /* Print values to prevent optimization */
    printf("sf_max = %f\n", (double)sf_max);
    printf("sf_min = %f\n", (double)sf_min);
    printf("a_max = %f\n", (double)a_max);
    printf("a_min = %f\n", (double)a_min);
    printf("la_max = %f\n", (double)la_max);
    printf("la_min = %f\n", (double)la_min);
    
    printf("sf_overflow = %f\n", (double)sf_overflow);
    printf("a_overflow = %f\n", (double)a_overflow);
    
    printf("Comparison results: %d %d %d\n", cmp1, cmp2, cmp3);
    
    return 0;
}
