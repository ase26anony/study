/* Compile with: gcc -std=c99 -ffixed-point -O0 -ftest-coverage -fprofile-arcs test.c */

/* Test program to cover fixed-value.cc lines 264-277 */
/* This exercises the max/min range checking logic for fixed-point types */

int main(void) {
    /* Declare various fixed-point types with different integer/fractional bits */
    
    /* _Fract types (all fractional bits) */
    short _Fract sf;
    _Fract f;
    long _Fract lf;
    unsigned short _Fract usf;
    unsigned _Fract uf;
    unsigned long _Fract ulf;
    
    /* _Accum types (mixed integer/fractional bits) */
    short _Accum sa;
    _Accum a;
    long _Accum la;
    unsigned short _Accum usa;
    unsigned _Accum ua;
    unsigned long _Accum ula;
    
    /* Saturated versions */
    _Sat short _Fract ssf;
    _Sat _Fract sfract;
    _Sat long _Fract slf;
    _Sat short _Accum ssa;
    _Sat _Accum saccum;
    _Sat long _Accum sla;
    
    /* Initialize with boundary values to trigger range calculations */
    
    /* Near maximum values for various types */
    sf = 0.99999999976716936r;  /* Near max for short _Fract (15 fractional bits) */
    f = 0.99999999999999989r;   /* Near max for _Fract (31 fractional bits) */
    lf = 0.99999999999999999999999999999999999975r;  /* Near max for long _Fract (63 fractional bits) */
    
    /* Minimum values (negative for signed types) */
    sf = -1.0r;                 /* Minimum for short _Fract */
    f = -1.0r;                  /* Minimum for _Fract */
    lf = -1.0r;                 /* Minimum for long _Fract */
    
    /* Unsigned types at boundaries */
    usf = 0.0r;                 /* Minimum for unsigned short _Fract */
    usf = 0.99999999976716936r; /* Maximum for unsigned short _Fract */
    uf = 0.0r;                  /* Minimum for unsigned _Fract */
    uf = 0.99999999999999989r;  /* Maximum for unsigned _Fract */
    
    /* _Accum types with different integer/fractional bit combinations */
    sa = 127.99999999976716936k;    /* Near max for short _Accum (7 integer, 8 fractional) */
    a = 32767.99999999999999989k;   /* Near max for _Accum (15 integer, 16 fractional) */
    la = 9223372036854775807.99999999999999999999999999999999999975k; /* Near max for long _Accum */
    
    /* Negative boundaries for _Accum */
    sa = -128.0k;                   /* Minimum for short _Accum */
    a = -32768.0k;                  /* Minimum for _Accum */
    la = -9223372036854775808.0k;   /* Minimum for long _Accum */
    
    /* Unsigned _Accum types */
    usa = 0.0k;                     /* Minimum for unsigned short _Accum */
    usa = 255.99999999976716936k;   /* Maximum for unsigned short _Accum (8 integer, 7 fractional) */
    ua = 0.0k;                      /* Minimum for unsigned _Accum */
    ua = 65535.99999999999999989k;  /* Maximum for unsigned _Accum (16 integer, 15 fractional) */
    
    /* Saturated types - these should trigger saturation logic */
    ssf = 1.5r;                     /* Should saturate to maximum */
    ssf = -1.5r;                    /* Should saturate to minimum */
    ssa = 128.5k;                   /* Should saturate to maximum for short _Accum */
    ssa = -129.0k;                  /* Should saturate to minimum for short _Accum */
    
    /* Test overflow/underflow with arithmetic operations */
    short _Fract sf1 = 0.5r;
    short _Fract sf2 = 0.6r;
    short _Fract sf3 = sf1 + sf2;   /* Should be 1.1r but clamped to max */
    
    _Accum a1 = 20000.0k;
    _Accum a2 = 20000.0k;
    _Accum a3 = a1 + a2;            /* Should overflow for _Accum */
    
    /* Test with compile-time constant expressions */
    /* These force the compiler to evaluate at compile-time */
    const short _Fract c_sf_max = 0.99999999976716936r;
    const short _Fract c_sf_min = -1.0r;
    const _Accum c_a_max = 32767.99999999999999989k;
    const _Accum c_a_min = -32768.0k;
    
    /* Use volatile to prevent optimization */
    volatile short _Fract v_sf = c_sf_max;
    volatile _Accum v_a = c_a_min;
    
    /* Mixed precision assignments */
    _Fract f_from_sf = sf;          /* Conversion from short _Fract to _Fract */
    short _Fract sf_from_f = f;     /* Conversion from _Fract to short _Fract (may saturate) */
    
    /* Array of fixed-point values */
    short _Fract sf_array[4] = {
        -1.0r,
        -0.5r,
        0.5r,
        0.99999999976716936r
    };
    
    /* Loop to use the values */
    for (int i = 0; i < 4; i++) {
        volatile short _Fract temp = sf_array[i];
    }
    
    /* Test with different fractional bit counts through type conversions */
    /* This should trigger the i_f_bits calculations in the uncovered code */
    long _Fract lf_converted = (long _Fract)sf;  /* Conversion with different fractional bits */
    short _Fract sf_converted = (short _Fract)lf; /* This may overflow */
    
    /* Return 0 to indicate success */
    return 0;
}
