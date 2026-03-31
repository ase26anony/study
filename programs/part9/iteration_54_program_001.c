/* Test program to cover fixed-value.cc boundary calculations */
/* Compile with: gcc -std=c99 -ffixed-point -O2 test_fixed.c -o test_fixed */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with boundary values */
    
    /* Short fract types */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    short _Fract sf_mid = 0.5r;
    
    /* Fract types */
    _Fract f_max = 0.99999999999999999r;         /* Near max for _Fract */
    _Fract f_min = -1.0r;
    _Fract f_zero = 0.0r;
    
    /* Long fract types */
    long _Fract lf_max = 0.99999999999999999999999999999999999999r;
    long _Fract lf_min = -1.0r;
    
    /* Unsigned fract types */
    unsigned short _Fract usf_max = 1.0ur;       /* Max for unsigned short _Fract */
    unsigned short _Fract usf_min = 0.0ur;
    unsigned _Fract uf_max = 1.0ur;
    unsigned long _Fract ulf_max = 1.0ur;
    
    /* Accum types with different integer parts */
    short _Accum sa_max = 32767.99999999976716936rk;  /* Max for short _Accum */
    short _Accum sa_min = -32768.0rk;                 /* Min for short _Accum */
    
    _Accum a_max = 32767.99999999999999999rk;         /* Max for _Accum */
    _Accum a_min = -32768.0rk;
    
    long _Accum la_max = 2147483647.99999999999999999999999999999999999999rk;
    long _Accum la_min = -2147483648.0rk;
    
    /* Unsigned accum types */
    unsigned short _Accum usa_max = 65535.99999999976716936urk;
    unsigned _Accum ua_max = 65535.99999999999999999urk;
    unsigned long _Accum ula_max = 4294967295.99999999999999999999999999999999999999urk;
    
    /* Saturated types */
    _Sat short _Fract ssf = 0.5r;
    _Sat _Fract sf_sat = 0.75r;
    _Sat short _Accum ssa = 100.5rk;
    _Sat unsigned short _Fract susf = 0.5ur;
    
    /* Test operations that might trigger boundary checks */
    short _Fract sf_sum = sf_max + sf_mid;      /* Potential overflow check */
    _Fract f_diff = f_max - f_min;              /* Range calculation */
    short _Accum sa_prod = sa_max * 0.5rk;      /* Scaling operation */
    
    /* Test assignments with constants that need boundary validation */
    volatile short _Fract test_sf = 1.5r;       /* Should trigger max check */
    volatile short _Fract test_sf2 = -1.1r;     /* Should trigger min check */
    
    /* Test with compile-time constant expressions */
    const short _Fract const_sf = 0.99999999976716936r;
    const _Accum const_a = 32767.99999999999999999rk;
    const long _Accum const_la = -2147483648.0rk;
    
    /* Test mixed operations */
    _Accum mixed = (_Accum)sf_max * a_max;
    short _Fract cast_test = (short _Fract)f_max;
    
    /* Force evaluation of boundary conditions through conditionals */
    if (sf_max > 0.9r) {
        printf("sf_max is > 0.9\n");
    }
    
    if (sa_min < -100.0rk) {
        printf("sa_min is < -100\n");
    }
    
    /* Array initializations with boundary values */
    short _Fract sf_array[3] = {sf_min, 0.0r, sf_max};
    _Accum a_array[3] = {a_min, 0.0rk, a_max};
    
    /* Test with different fractional bit configurations */
    /* These should trigger different i_f_bits calculations */
    
    /* Return statement to avoid "unused variable" warnings */
    return (int)(sf_max + f_max + lf_max + usf_max + uf_max + ulf_max +
                 sa_max + sa_min + a_max + a_min + la_max + la_min +
                 usa_max + ua_max + ula_max + ssf + sf_sat + ssa + susf +
                 sf_sum + f_diff + sa_prod + test_sf + test_sf2 +
                 const_sf + const_a + const_la + mixed + cast_test +
                 sf_array[0] + a_array[0]);
}
