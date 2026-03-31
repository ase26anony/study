/* fixed-point-test.c */
/* Compile with: gcc -std=c99 -c fixed-point-test.c -o fixed-point-test.o */

/* Test various fixed-point types to trigger boundary value calculations */
int main(void) {
    /* Short fract types (7 fractional bits) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                  /* Min for short _Fract */
    short _Fract sf_mid = 0.5r;
    
    /* Fract types (15 fractional bits) */
    _Fract f_max = 0.9999999999999999r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;
    _Fract f_mid = 0.0r;
    
    /* Long fract types (31 fractional bits) */
    long _Fract lf_max = 0.9999999999999999999999999999999r;
    long _Fract lf_min = -1.0r;
    long _Fract lf_mid = -0.5r;
    
    /* Unsigned fract types */
    unsigned short _Fract usf_max = 0.99999999976716936ur;
    unsigned short _Fract usf_min = 0.0ur;
    unsigned _Fract uf_max = 0.9999999999999999ur;
    unsigned _Fract uf_min = 0.0ur;
    unsigned long _Fract ulf_max = 0.9999999999999999999999999999999ur;
    unsigned long _Fract ulf_min = 0.0ur;
    
    /* Accum types (varying integer and fractional bits) */
    short _Accum sa_max = 32767.99999999976716936k;  /* 15 integer, 16 fractional */
    short _Accum sa_min = -32768.0k;
    short _Accum sa_mid = 0.0k;
    
    _Accum a_max = 32767.9999999999999999k;          /* 15 integer, 16 fractional */
    _Accum a_min = -32768.0k;
    _Accum a_mid = 100.5k;
    
    long _Accum la_max = 2147483647.9999999999999999999999999999999lk;  /* 31 int, 32 frac */
    long _Accum la_min = -2147483648.0lk;
    long _Accum la_mid = -1000.25lk;
    
    /* Unsigned accum types */
    unsigned short _Accum usa_max = 65535.99999999976716936uk;  /* 16 int, 15 frac */
    unsigned short _Accum usa_min = 0.0uk;
    
    unsigned _Accum ua_max = 65535.9999999999999999uk;          /* 16 int, 15 frac */
    unsigned _Accum ua_min = 0.0uk;
    
    unsigned long _Accum ula_max = 4294967295.9999999999999999999999999999999ulk;  /* 32 int, 31 frac */
    unsigned long _Accum ula_min = 0.0ulk;
    
    /* Saturated types - these should trigger boundary checks */
    _Sat short _Fract ssf = 2.0r;  /* Should saturate to max */
    _Sat short _Fract ssf2 = -2.0r; /* Should saturate to min */
    
    _Sat short _Accum ssa = 40000.0k;  /* Exceeds range, should saturate */
    _Sat short _Accum ssa2 = -40000.0k; /* Below range, should saturate */
    
    _Sat unsigned short _Fract susf = 2.0ur;  /* Should saturate to max */
    _Sat unsigned short _Accum susa = 70000.0uk;  /* Exceeds range */
    
    /* Complex expressions that require compile-time evaluation */
    const short _Fract csf = 0.25r + 0.75r;  /* Should be 1.0, but clamped to max */
    const _Accum ca = 10000.0k * 3.5k;       /* Requires compile-time multiplication */
    const long _Accum cla = la_max / 2.0lk;  /* Division at compile time */
    
    /* Array initializations with fixed-point constants */
    short _Fract sf_array[3] = {0.0r, 0.5r, 1.0r};
    _Accum a_array[3] = {-32768.0k, 0.0k, 32767.9999999999999999k};
    
    /* Mixed operations that might trigger overflow checks */
    volatile short _Fract vsf = sf_max + 0.0000001r;  /* Potential overflow */
    volatile _Accum va = a_max * 1.0000001k;          /* Potential overflow */
    
    /* Switch with fixed-point cases (requires comparisons) */
    short _Fract switch_var = 0.75r;
    switch (switch_var > 0.5r) {
        case 1:
            vsf = 0.9r;
            break;
        default:
            vsf = 0.1r;
    }
    
    /* Return computed values to prevent dead code elimination */
    return (int)(sf_max * 1000r) + 
           (int)(f_max * 1000r) + 
           (int)(lf_max * 1000r) +
           (int)(sa_max) + 
           (int)(a_max) + 
           (int)(la_max / 1000lk) +
           (int)(ssf * 100r) + 
           (int)(ssa / 100k);
}
