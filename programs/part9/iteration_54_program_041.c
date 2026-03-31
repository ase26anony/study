/* Test program to cover fixed-value.cc boundary calculations */
/* Compile with: gcc -std=c99 -ffixed-point -O2 test_fixed.c -o test_fixed */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with boundary values */
    
    /* 1. Short fract types (7 fractional bits) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    unsigned short _Fract usf_max = 1.0r;        /* Max for unsigned short _Fract */
    unsigned short _Fract usf_min = 0.0r;        /* Min for unsigned short _Fract */
    
    /* 2. Fract types (15 fractional bits) */
    _Fract f_max = 0.9999999999999999r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;                        /* Min for _Fract */
    unsigned _Fract uf_max = 1.0r;               /* Max for unsigned _Fract */
    unsigned _Fract uf_min = 0.0r;               /* Min for unsigned _Fract */
    
    /* 3. Long fract types (31 fractional bits) */
    long _Fract lf_max = 0.9999999999999999999999999999999r;  /* Near max */
    long _Fract lf_min = -1.0r;                               /* Min */
    unsigned long _Fract ulf_max = 1.0r;                      /* Max */
    unsigned long _Fract ulf_min = 0.0r;                      /* Min */
    
    /* 4. Short accum types (7 integer, 8 fractional bits) */
    short _Accum sa_max = 127.99999999976716936rk;  /* Near max for short _Accum */
    short _Accum sa_min = -128.0rk;                 /* Min for short _Accum */
    unsigned short _Accum usa_max = 255.99999999976716936rk;  /* Max for unsigned */
    unsigned short _Accum usa_min = 0.0rk;                    /* Min for unsigned */
    
    /* 5. Accum types (15 integer, 16 fractional bits) */
    _Accum a_max = 32767.9999999999999999rk;        /* Near max for _Accum */
    _Accum a_min = -32768.0rk;                      /* Min for _Accum */
    unsigned _Accum ua_max = 65535.9999999999999999rk;  /* Max for unsigned */
    unsigned _Accum ua_min = 0.0rk;                     /* Min for unsigned */
    
    /* 6. Long accum types (31 integer, 32 fractional bits) */
    long _Accum la_max = 2147483647.9999999999999999999999999999999rk;  /* Near max */
    long _Accum la_min = -2147483648.0rk;                               /* Min */
    unsigned long _Accum ula_max = 4294967295.9999999999999999999999999999999rk;  /* Max */
    unsigned long _Accum ula_min = 0.0rk;                                        /* Min */
    
    /* 7. Saturated types - should trigger boundary checks */
    _Sat short _Fract ssf = 1.5r;  /* Should saturate to max */
    _Sat short _Fract ssf2 = -1.5r; /* Should saturate to min */
    _Sat unsigned short _Fract susf = 1.5r;  /* Should saturate to max */
    _Sat unsigned short _Fract susf2 = -0.5r; /* Should saturate to min (0) */
    
    _Sat short _Accum ssa = 128.5rk;  /* Should saturate to max */
    _Sat short _Accum ssa2 = -129.0rk; /* Should saturate to min */
    _Sat unsigned short _Accum susa = 256.5rk;  /* Should saturate to max */
    _Sat unsigned short _Accum susa2 = -0.5rk;  /* Should saturate to min (0) */
    
    /* 8. Test overflow/underflow with arithmetic */
    short _Fract sf1 = 0.5r;
    short _Fract sf2 = 0.6r;
    short _Fract sf_sum = sf1 + sf2;  /* Should be > 1.0, may trigger checks */
    
    short _Accum sa1 = 100.0rk;
    short _Accum sa2 = 100.0rk;
    short _Accum sa_sum = sa1 + sa2;  /* 200 > 127.999..., may trigger checks */
    
    /* 9. Test with explicit casts that might overflow */
    short _Accum sa_from_int = (short _Accum)256;  /* Should overflow */
    short _Accum sa_from_neg = (short _Accum)-129; /* Should underflow */
    
    /* 10. Test boundary comparisons */
    int cmp1 = (sf_max > 0.9r);
    int cmp2 = (sa_min < -100.0rk);
    int cmp3 = (la_max > 2000000000.0rk);
    
    /* 11. Test with different fractional bit configurations */
    /* Force different i_f_bits values in the compiler */
    
    /* Use volatile to prevent optimization */
    volatile short _Fract vsf = sf_max;
    volatile short _Accum vsa = sa_max;
    volatile long _Accum vla = la_min;
    
    /* Print some values to prevent dead code elimination */
    printf("Short fract max: %hd\n", (short)(vsf * 256));
    printf("Short accum max: %hd\n", (short)(vsa / 256));
    printf("Long accum min: %ld\n", (long)(vla / 65536));
    
    return 0;
}
