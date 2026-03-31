/* Test program for fixed-point type boundary value calculations */
/* Compile with: gcc -std=c99 -ffixed-point -O2 test_fixed.c -o test_fixed */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with different integer/fractional bit combinations */
    
    /* 1. Short fract (7 fractional bits, 1 integer bit including sign) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    short _Fract sf_mid = 0.5r;
    
    /* 2. Fract (15 fractional bits, 1 integer bit including sign) */
    _Fract f_max = 0.9999999999999999r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;                        /* Min for _Fract */
    _Fract f_mid = 0.0r;
    
    /* 3. Long fract (31 fractional bits, 1 integer bit including sign) */
    long _Fract lf_max = 0.9999999999999999999999999999999r;  /* Near max */
    long _Fract lf_min = -1.0r;                               /* Min */
    long _Fract lf_mid = -0.5r;
    
    /* 4. Short accum (7 fractional bits, 8 integer bits including sign) */
    short _Accum sa_max = 127.99999999976716936k;  /* Near max for short _Accum */
    short _Accum sa_min = -128.0k;                 /* Min for short _Accum */
    short _Accum sa_mid = 0.0k;
    
    /* 5. Accum (15 fractional bits, 16 integer bits including sign) */
    _Accum a_max = 32767.9999999999999999k;        /* Near max for _Accum */
    _Accum a_min = -32768.0k;                      /* Min for _Accum */
    _Accum a_mid = 100.5k;
    
    /* 6. Long accum (31 fractional bits, 32 integer bits including sign) */
    long _Accum la_max = 2147483647.9999999999999999999999999999999k;  /* Near max */
    long _Accum la_min = -2147483648.0k;                               /* Min */
    long _Accum la_mid = -1000.25k;
    
    /* 7. Unsigned variants */
    unsigned short _Fract usf_max = 0.99999999976716936ur;  /* Max for unsigned short _Fract */
    unsigned short _Fract usf_min = 0.0ur;                  /* Min for unsigned short _Fract */
    
    unsigned _Fract uf_max = 0.9999999999999999ur;          /* Max for unsigned _Fract */
    unsigned _Fract uf_min = 0.0ur;                         /* Min for unsigned _Fract */
    
    unsigned short _Accum usa_max = 255.99999999976716936uk;  /* Max for unsigned short _Accum */
    unsigned short _Accum usa_min = 0.0uk;                    /* Min for unsigned short _Accum */
    
    /* 8. Saturated variants - these should trigger boundary checks */
    _Sat short _Fract ssf_max = 1.0r;      /* Should saturate at max */
    _Sat short _Fract ssf_min = -1.1r;     /* Should saturate at min */
    
    _Sat short _Accum ssa_max = 128.0k;    /* Should saturate at max */
    _Sat short _Accum ssa_min = -129.0k;   /* Should saturate at min */
    
    /* 9. Test overflow/underflow with arithmetic operations */
    short _Fract sf1 = 0.9r;
    short _Fract sf2 = 0.8r;
    short _Fract sf_sum = sf1 + sf2;       /* Should be close to max */
    
    short _Accum sa1 = 100.0k;
    short _Accum sa2 = 50.0k;
    short _Accum sa_sum = sa1 + sa2;       /* Within range */
    
    /* 10. Test with explicit casts that might trigger boundary checks */
    long _Accum la_from_double = (long _Accum)3.14159265358979323846;
    _Accum a_from_int = (_Accum)1000;
    
    /* 11. Test boundary expressions */
    /* These constant expressions should be evaluated at compile-time */
    /* and trigger the boundary checking code in fixed-value.cc */
    const short _Fract sf_boundary = 0.99999999976716936r;  /* Max representable */
    const short _Fract sf_over = 1.0r;                      /* Over max - may saturate or wrap */
    
    const long _Accum la_boundary = 2147483647.9999999999999999999999999999999k;
    const long _Accum la_under = -2147483649.0k;            /* Under min */
    
    /* 12. Test with different fractional bit counts through typedefs */
    typedef short _Fract sf_type;
    typedef _Accum a_type;
    typedef long _Accum la_type;
    
    sf_type sf_typedef = 0.75r;
    a_type a_typedef = 500.25k;
    la_type la_typedef = -50000.125k;
    
    /* 13. Array initializations with boundary values */
    short _Fract sf_array[3] = {-1.0r, 0.0r, 0.99999999976716936r};
    _Accum a_array[3] = {-32768.0k, 0.0k, 32767.9999999999999999k};
    
    /* 14. Test compound assignments that might trigger boundary checks */
    short _Accum sa_compound = 100.0k;
    sa_compound += 50.0k;
    sa_compound *= 2.0k;
    
    /* 15. Test with conditional expressions containing boundary values */
    short _Fract sf_cond = (1 > 0) ? 0.99999999976716936r : 0.0r;
    long _Accum la_cond = (1 > 0) ? -2147483648.0k : 2147483647.9999999999999999999999999999999k;
    
    /* 16. Test function arguments with boundary values */
    /* (In a real test, we'd have functions taking fixed-point params) */
    
    /* 17. Force compile-time evaluation of boundary comparisons */
    /* These should trigger the exact comparison logic in the uncovered lines */
    const int test1 = (sf_boundary == 0.99999999976716936r);
    const int test2 = (la_boundary < 2147483648.0k);
    const int test3 = (la_min == -2147483648.0k);
    
    /* Print some values to prevent optimization from removing everything */
    printf("sf_max: %f\n", (double)sf_max);
    printf("sf_min: %f\n", (double)sf_min);
    printf("sa_max: %f\n", (double)sa_max);
    printf("sa_min: %f\n", (double)sa_min);
    printf("la_max: %f\n", (double)la_max);
    printf("la_min: %f\n", (double)la_min);
    printf("usf_max: %f\n", (double)usf_max);
    printf("ssf_max: %f (saturated)\n", (double)ssf_max);
    printf("ssf_min: %f (saturated)\n", (double)ssf_min);
    
    return 0;
}
