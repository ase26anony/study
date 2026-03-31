/* Test program to exercise fixed-point type boundary calculations in GCC */
/* Compile with: gcc -std=c99 -ffixed-point -O2 -c test_fixed.c */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with different integer/fractional bit combinations */
    
    /* _Fract types (all fractional bits) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    _Fract f_max = 0.99999999999999999r;         /* Near max for default _Fract */
    _Fract f_min = -1.0r;                        /* Min for default _Fract */
    long _Fract lf_max = 0.99999999999999999999999999999999999999r;  /* Near max */
    long _Fract lf_min = -1.0r;                  /* Min for long _Fract */
    
    /* _Accum types (mixed integer/fractional bits) */
    short _Accum sa_max = 32767.99999999976716936k;  /* Near max for short _Accum */
    short _Accum sa_min = -32768.0k;                 /* Min for short _Accum */
    _Accum a_max = 32767.99999999999999999k;         /* Near max for default _Accum */
    _Accum a_min = -32768.0k;                        /* Min for default _Accum */
    long _Accum la_max = 9223372036854775807.99999999999999999999999999999999999999k;  /* Near max */
    long _Accum la_min = -9223372036854775808.0k;    /* Min for long _Accum */
    
    /* Unsigned variants */
    unsigned short _Fract usf_max = 0.99999999976716936ur;  /* Max for unsigned short _Fract */
    unsigned _Fract uf_max = 0.99999999999999999ur;         /* Max for unsigned _Fract */
    unsigned long _Fract ulf_max = 0.99999999999999999999999999999999999999ur;  /* Max */
    
    unsigned short _Accum usa_max = 65535.99999999976716936uk;  /* Max for unsigned short _Accum */
    unsigned _Accum ua_max = 65535.99999999999999999uk;         /* Max for unsigned _Accum */
    unsigned long _Accum ula_max = 18446744073709551615.99999999999999999999999999999999999999uk;  /* Max */
    
    /* Saturated variants */
    _Sat short _Fract ssf = 0.5r;
    _Sat short _Accum ssa = 100.5k;
    _Sat unsigned short _Fract susf = 0.5ur;
    _Sat unsigned short _Accum susa = 100.5uk;
    
    /* Test boundary assignments that should trigger range checking */
    /* These assignments require the compiler to check if values fit in the type */
    
    /* Test with compile-time constant expressions at exact boundaries */
    short _Fract sf_exact_max = 0.99999999976716936r;
    short _Fract sf_exact_min = -1.0r;
    
    _Accum a_exact_max = 32767.99999999999999999k;
    _Accum a_exact_min = -32768.0k;
    
    /* Test overflow/underflow cases with explicit casts */
    /* These should trigger the boundary checks in the uncovered code */
    double d1 = 1.5;  /* Too large for _Fract */
    double d2 = -1.5; /* Too small for _Fract */
    double d3 = 40000.0; /* Too large for _Accum */
    double d4 = -40000.0; /* Too small for _Accum */
    
    /* Explicit casts that should trigger the boundary checking logic */
    /* The compiler needs to check if these double values fit in the fixed-point types */
    _Fract f_from_double1 = (_Fract)d1;  /* Should trigger max check */
    _Fract f_from_double2 = (_Fract)d2;  /* Should trigger min check */
    _Accum a_from_double3 = (_Accum)d3;  /* Should trigger max check */
    _Accum a_from_double4 = (_Accum)d4;  /* Should trigger min check */
    
    /* Test with integer literals that need conversion */
    int i1 = 2;
    int i2 = -2;
    _Fract f_from_int1 = (_Fract)i1;  /* Should trigger max check (2 > 1.0) */
    _Fract f_from_int2 = (_Fract)i2;  /* Should trigger min check (-2 < -1.0) */
    
    /* Complex expressions that require boundary checking */
    _Accum a_complex = (_Accum)(32767.999k + 0.001k);  /* Near boundary */
    _Fract f_complex = (_Fract)(0.999r + 0.001r);      /* Exceeds max */
    
    /* Test with different fractional bit counts through type combinations */
    /* Mixing operations between different fixed-point types */
    short _Fract sf1 = 0.5r;
    _Fract f1 = 0.7r;
    _Fract f_result = sf1 + f1;  /* Requires conversion and possible range check */
    
    /* Test assignment between different fixed-point types */
    long _Fract lf_from_sf = sf_max;  /* Should fit */
    short _Fract sf_from_lf = lf_max;  /* May overflow, needs check */
    
    /* Test with conditional expressions */
    int condition = 1;
    _Fract f_cond = condition ? 0.99999999999999999r : 0.5r;
    _Accum a_cond = condition ? 32767.99999999999999999k : 100.0k;
    
    /* Test in arithmetic operations that could overflow */
    _Accum a1 = 30000.0k;
    _Accum a2 = 3000.0k;
    _Accum a_sum = a1 + a2;  /* 33000 > 32767.999..., may overflow */
    
    /* Test with shift operations (if supported) */
    /* Fixed-point values in bitwise operations might trigger different code paths */
    
    /* Print values to prevent optimization */
    printf("sf_max = %r\n", sf_max);
    printf("sa_min = %k\n", sa_min);
    printf("usf_max = %ur\n", usf_max);
    printf("ula_max = %uk\n", ula_max);
    
    return 0;
}
