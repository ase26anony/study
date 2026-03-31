/* Test program to cover fixed-value.cc boundary calculations */
/* Compile with: gcc -std=c99 -ffixed-point -O2 test_fixed.c */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with different integer/fractional bit combinations */
    
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
    short _Accum sa_max = 127.99999999999999r;   /* Near max for short _Accum */
    short _Accum sa_min = -128.0r;               /* Min for short _Accum */
    short _Accum sa_mid = 64.5r;
    
    /* Accum types (15 integer bits, 16 fractional bits) */
    _Accum a_max = 32767.9999999999999999r;      /* Near max for _Accum */
    _Accum a_min = -32768.0r;
    _Accum a_mid = 16384.25r;
    
    /* Long accum types (31 integer bits, 32 fractional bits) */
    long _Accum la_max = 2147483647.9999999999999999999999999999999999999r;
    long _Accum la_min = -2147483648.0r;
    long _Accum la_mid = 1073741824.125r;
    
    /* Unsigned variants */
    unsigned short _Fract usf_max = 0.99999999976716936ur;
    unsigned short _Fract usf_min = 0.0ur;
    
    unsigned _Fract uf_max = 0.9999999999999999ur;
    unsigned _Fract uf_min = 0.0ur;
    
    unsigned long _Fract ulf_max = 0.9999999999999999999999999999999ur;
    unsigned long _Fract ulf_min = 0.0ur;
    
    unsigned short _Accum usa_max = 255.99999999999999ur;
    unsigned short _Accum usa_min = 0.0ur;
    
    unsigned _Accum ua_max = 65535.9999999999999999ur;
    unsigned _Accum ua_min = 0.0ur;
    
    unsigned long _Accum ula_max = 4294967295.9999999999999999999999999999999999999ur;
    unsigned long _Accum ula_min = 0.0ur;
    
    /* Saturated variants */
    _Sat short _Fract ssf = 0.75r;
    _Sat _Fract sf = -0.25r;
    _Sat long _Fract slf = 0.9999999999999999999999999999999r;
    
    _Sat short _Accum ssa = 100.5r;
    _Sat _Accum sa = -20000.75r;
    _Sat long _Accum sla = 1000000000.5r;
    
    /* Test boundary assignments that should trigger the uncovered comparison logic */
    /* These assignments with values at or beyond representable limits should 
       cause the compiler to evaluate the boundary checks in fixed-value.cc */
    
    /* Force evaluation of max boundary check */
    short _Fract sf_test1 = 1.0r;  /* This should be clamped to max */
    short _Fract sf_test2 = -1.0000001r;  /* This should be clamped to min */
    
    _Accum a_test1 = 32768.0r;     /* Beyond max, should trigger boundary check */
    _Accum a_test2 = -32768.0000000000000001r; /* Beyond min */
    
    long _Accum la_test1 = 2147483648.0r;  /* Beyond max for long _Accum */
    long _Accum la_test2 = -2147483648.0000000000000000000000000000000000001r;
    
    /* Test unsigned boundaries */
    unsigned short _Fract usf_test = 1.0ur;  /* Beyond max for unsigned */
    unsigned _Accum ua_test = 65536.0ur;     /* Beyond max */
    
    /* Complex expressions that should trigger boundary calculations */
    /* These force the compiler to compute intermediate values and check bounds */
    _Accum a_expr1 = (_Accum)32767.0r + (_Accum)1.0r;  /* Should be at or near max */
    _Accum a_expr2 = (_Accum)-32768.0r - (_Accum)0.0000000000000001r;
    
    long _Accum la_expr1 = (long _Accum)2147483647.0r + (long _Accum)0.9999999999999999999999999999999999999r;
    long _Accum la_expr2 = (long _Accum)-2147483648.0r - (long _Accum)0.0000000000000000000000000000000000001r;
    
    /* Mixed-type expressions */
    short _Accum mixed1 = (short _Accum)((short _Fract)0.9r * 128.0r);
    _Accum mixed2 = (_Accum)((_Fract)0.9999r * 32768.0r);
    
    /* Test with conditional expressions that depend on boundary checks */
    int check1 = (sf_test1 > 0.99999999976716935r) ? 1 : 0;
    int check2 = (a_test1 < 32767.9999999999999998r) ? 1 : 0;
    int check3 = (la_test2 > -2147483648.0r) ? 1 : 0;
    
    /* Array initializations with boundary values */
    short _Fract sf_array[3] = {0.99999999976716936r, -1.0r, 0.0r};
    _Accum a_array[3] = {32767.9999999999999999r, -32768.0r, 0.0r};
    long _Accum la_array[3] = {
        2147483647.9999999999999999999999999999999999999r,
        -2147483648.0r,
        0.0r
    };
    
    /* Struct with fixed-point members */
    struct {
        short _Fract sf;
        _Accum a;
        long _Accum la;
        unsigned short _Fract usf;
    } fixed_struct = {
        0.99999999976716936r,
        32767.9999999999999999r,
        2147483647.9999999999999999999999999999999999999r,
        0.99999999976716936ur
    };
    
    /* Use volatile to prevent optimization from removing computations */
    volatile short _Fract vsf = 0.99999999976716936r;
    volatile _Accum va = 32767.9999999999999999r;
    volatile long _Accum vla = 2147483647.9999999999999999999999999999999999999r;
    
    /* Perform operations that might overflow/underflow */
    vsf = vsf * vsf;
    va = va + (_Accum)0.0000000000000001r;
    vla = vla - (long _Accum)0.0000000000000000000000000000000000001r;
    
    /* Return something to use the variables (prevents dead code elimination) */
    return (int)(sf_max + f_max + lf_max + sa_max + a_max + la_max +
                 usf_max + uf_max + ulf_max + usa_max + ua_max + ula_max +
                 ssf + sf + slf + ssa + sa + sla +
                 sf_test1 + sf_test2 + a_test1 + a_test2 + la_test1 + la_test2 +
                 usf_test + ua_test + a_expr1 + a_expr2 + la_expr1 + la_expr2 +
                 mixed1 + mixed2 + sf_array[0] + a_array[0] + la_array[0] +
                 fixed_struct.sf + fixed_struct.a + fixed_struct.la + fixed_struct.usf +
                 vsf + va + vla + check1 + check2 + check3);
}
