/* Test program to cover fixed-value.cc boundary calculations */
/* Compile with: gcc -std=c99 -ffixed-point -O2 test_fixed.c -o test_fixed */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with different integer/fractional bit combinations */
    
    /* 1. Short fract types (7 fractional bits) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    short _Fract sf_mid = 0.5r;
    
    /* 2. Fract types (15 fractional bits) */
    _Fract f_max = 0.9999999999999964r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;
    _Fract f_mid = 0.0r;
    
    /* 3. Long fract types (31 fractional bits) */
    long _Fract lf_max = 0.9999999999999999999999999999999r;
    long _Fract lf_min = -1.0r;
    long _Fract lf_mid = -0.5r;
    
    /* 4. Short accum types (7 integer, 8 fractional bits) */
    short _Accum sa_max = 127.99999999976716936rk;
    short _Accum sa_min = -128.0rk;
    short _Accum sa_mid = 64.5rk;
    
    /* 5. Accum types (15 integer, 16 fractional bits) */
    _Accum a_max = 32767.9999999999999964rk;
    _Accum a_min = -32768.0rk;
    _Accum a_mid = 0.0rk;
    
    /* 6. Long accum types (31 integer, 32 fractional bits) */
    long _Accum la_max = 2147483647.9999999999999999999999999999999rk;
    long _Accum la_min = -2147483648.0rk;
    long _Accum la_mid = 1073741824.5rk;
    
    /* 7. Unsigned variants */
    unsigned short _Fract usf_max = 0.99999999976716936ur;
    unsigned short _Fract usf_min = 0.0ur;
    
    unsigned _Fract uf_max = 0.9999999999999964ur;
    unsigned _Fract uf_min = 0.0ur;
    
    unsigned short _Accum usa_max = 255.99999999976716936urk;
    unsigned short _Accum usa_min = 0.0urk;
    
    /* 8. Saturated variants */
    _Sat short _Fract ssf_max = 0.99999999976716936r;
    _Sat short _Fract ssf_min = -1.0r;
    
    _Sat short _Accum ssa_max = 127.99999999976716936rk;
    _Sat short _Accum ssa_min = -128.0rk;
    
    /* 9. Test overflow/underflow with arithmetic */
    short _Fract sf_sum = sf_max + sf_mid;  /* May saturate or wrap */
    _Accum a_diff = a_max - a_mid;
    long _Accum la_prod = la_mid * 2.0rk;
    
    /* 10. Test boundary comparisons */
    int cmp1 = (sf_max == 0.99999999976716936r);
    int cmp2 = (la_min < la_mid);
    int cmp3 = (usa_max > usa_min);
    
    /* 11. Test with explicit casts that might trigger boundary checks */
    long _Accum la_from_double = (_Accum)3.14159265358979323846rk;
    short _Fract sf_from_int = (short _Fract)0.75r;
    
    /* 12. Test array initializations with boundary values */
    short _Fract sf_array[3] = {sf_min, 0.0r, sf_max};
    _Accum a_array[3] = {a_min, 0.0rk, a_max};
    
    /* 13. Test compound assignments at boundaries */
    _Accum a_compound = a_mid;
    a_compound += a_max / 2.0rk;
    a_compound -= a_min / 2.0rk;
    
    /* 14. Test with conditional expressions */
    short _Fract sf_cond = (cmp1 > 0) ? sf_max : sf_min;
    long _Accum la_cond = (la_min < 0.0rk) ? la_mid : la_max;
    
    /* 15. Test function calls with fixed-point arguments */
    /* (Using simple inline for demonstration) */
    static inline _Accum add_half(_Accum x) {
        return x + 0.5rk;
    }
    _Accum a_with_half = add_half(a_mid);
    
    /* 16. Test shift operations (for accum types) */
    /* Note: Fixed-point types don't directly support bit shifts in standard C,
       but compiler internals might use them during constant folding */
    
    /* 17. Test with different optimization levels by using volatile */
    volatile short _Fract vsf = sf_max;
    volatile _Accum va = a_min;
    
    /* 18. Test mixing types in expressions */
    _Accum mixed = a_mid + (_Accum)sf_max;
    long _Accum mixed2 = la_mid + (long _Accum)a_max;
    
    /* 19. Test compile-time constant expressions */
    const short _Fract const_sf = 0.75r;
    const _Accum const_a = 100.125rk;
    const long _Accum const_la = -1000.0625rk;
    
    /* 20. Test with struct containing fixed-point types */
    struct FixedStruct {
        short _Fract sf;
        _Accum a;
        long _Accum la;
    };
    
    struct FixedStruct fs = {
        .sf = sf_mid,
        .a = a_max,
        .la = la_min
    };
    
    /* Prevent unused variable warnings */
    (void)sf_sum;
    (void)a_diff;
    (void)la_prod;
    (void)cmp1;
    (void)cmp2;
    (void)cmp3;
    (void)la_from_double;
    (void)sf_from_int;
    (void)a_compound;
    (void)sf_cond;
    (void)la_cond;
    (void)a_with_half;
    (void)vsf;
    (void)va;
    (void)mixed;
    (void)mixed2;
    (void)const_sf;
    (void)const_a;
    (void)const_la;
    (void)fs;
    
    printf("Fixed-point test completed\n");
    
    return 0;
}
