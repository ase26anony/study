/* Test program to cover fixed-value.cc boundary calculations */
/* Compile with: gcc -std=c99 -ffixed-point -O2 test_fixed.c -o test_fixed */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with boundary values */
    
    /* Short fract types - 7 fractional bits */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    short _Fract sf_mid = 0.5r;
    
    /* Fract types - 15 fractional bits */
    _Fract f_max = 0.9999999999999964r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;
    _Fract f_mid = 0.0r;
    
    /* Long fract types - 31 fractional bits */
    long _Fract lf_max = 0.9999999999999999999999999999999r;
    long _Fract lf_min = -1.0r;
    long _Fract lf_mid = -0.5r;
    
    /* Short accum types - 7 integer bits, 8 fractional bits */
    short _Accum sa_max = 127.99999999976716936rk;
    short _Accum sa_min = -128.0rk;
    short _Accum sa_mid = 0.0rk;
    
    /* Accum types - 15 integer bits, 16 fractional bits */
    _Accum a_max = 32767.9999999999999964rk;
    _Accum a_min = -32768.0rk;
    _Accum a_mid = 100.5rk;
    
    /* Long accum types - 31 integer bits, 32 fractional bits */
    long _Accum la_max = 2147483647.9999999999999999999999999999999rk;
    long _Accum la_min = -2147483648.0rk;
    long _Accum la_mid = -1000.25rk;
    
    /* Unsigned variants */
    unsigned short _Fract usf_max = 0.99999999976716936ur;
    unsigned short _Fract usf_min = 0.0ur;
    
    unsigned _Fract uf_max = 0.9999999999999964ur;
    unsigned _Fract uf_min = 0.0ur;
    
    unsigned short _Accum usa_max = 255.99999999976716936urk;
    unsigned short _Accum usa_min = 0.0urk;
    
    unsigned _Accum ua_max = 65535.9999999999999964urk;
    unsigned _Accum ua_min = 0.0urk;
    
    /* Saturated variants */
    _Sat short _Fract ssf = 0.75r;
    _Sat _Fract sf = -0.25r;
    _Sat short _Accum ssa = 50.5rk;
    _Sat _Accum sa = -100.75rk;
    
    /* Test operations that might trigger boundary checks */
    
    /* 1. Assignment near boundaries */
    short _Fract sf_test1 = sf_max;
    short _Fract sf_test2 = sf_min;
    
    /* 2. Arithmetic operations */
    _Accum a_sum = a_max + a_mid;
    _Accum a_diff = a_min - a_mid;
    
    /* 3. Multiplication (can overflow) */
    short _Accum sa_prod = sa_max * 1.5rk;
    
    /* 4. Division */
    long _Accum la_div = la_max / 2.0rk;
    
    /* 5. Comparisons with boundaries */
    int cmp1 = (sf_max > 0.9r);
    int cmp2 = (a_min < -32767.0rk);
    int cmp3 = (la_max == la_max);
    
    /* 6. Casting between types */
    _Fract f_from_sf = (_Fract)sf_max;
    short _Accum sa_from_a = (short _Accum)a_mid;
    
    /* 7. Compound assignment */
    _Accum a_compound = 100.0rk;
    a_compound += 32700.0rk;
    
    /* 8. Function arguments (force constant folding) */
    volatile _Accum v1 = 32767.9999999999999964rk;  /* Max */
    volatile _Accum v2 = -32768.0rk;                /* Min */
    volatile _Accum v3 = 0.0rk;                     /* Zero */
    
    /* 9. Array initialization with fixed-point constants */
    _Fract farray[4] = {
        0.9999999999999964r,    /* Max */
        -1.0r,                  /* Min */
        0.0r,                   /* Zero */
        0.5r                    /* Mid */
    };
    
    /* 10. Struct with fixed-point members */
    struct {
        short _Fract sf;
        _Accum a;
        unsigned _Fract uf;
    } fixed_struct = {
        0.99999999976716936r,
        32767.9999999999999964rk,
        0.9999999999999964ur
    };
    
    /* 11. Test overflow in constant expressions */
    /* These should trigger the boundary checks in the compiler */
    const short _Accum overflow_test1 = 128.0rk;        /* Should saturate/wrap */
    const short _Accum overflow_test2 = -129.0rk;       /* Should saturate/wrap */
    const unsigned short _Accum uoverflow_test = 256.0urk; /* Should saturate/wrap */
    
    /* 12. Test exact boundary values */
    const _Fract exact_max = 0.999969482421875r;        /* Exact max for _Fract? */
    const _Fract exact_min = -1.0r;
    
    /* 13. Mixed precision operations */
    long _Accum mixed = la_mid + (_Accum)sa_max;
    
    /* 14. Test with different optimization levels by using volatile */
    volatile short _Fract vsf = 0.99999999976716936r;
    volatile _Accum va = 32767.9999999999999964rk;
    
    /* Force evaluation by using the values */
    printf("Values (some may be truncated/saturated):\n");
    printf("sf_max: %f\n", (double)sf_test1);
    printf("a_min: %f\n", (double)a_min);
    printf("la_max: %f\n", (double)la_max);
    printf("usf_max: %f\n", (double)usf_max);
    
    return 0;
}
