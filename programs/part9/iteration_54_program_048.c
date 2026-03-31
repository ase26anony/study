/* fixed-point-test.c */
/* Compile with: gcc -std=c99 -ffixed-point -O2 -c fixed-point-test.c */

/* Test various fixed-point types to trigger boundary value computations */
void test_fixed_point(void) {
    /* Short fract types (7 fractional bits) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    
    /* Fract types (15 fractional bits) */
    _Fract f_max = 0.9999999999999999r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;                        /* Min for _Fract */
    
    /* Long fract types (31 fractional bits) */
    long _Fract lf_max = 0.9999999999999999999999999999999r;  /* Near max */
    long _Fract lf_min = -1.0r;                               /* Min */
    
    /* Short accum types (7 integer bits, 8 fractional bits) */
    short _Accum sa_max = 127.99999999976716936rk;  /* Near max */
    short _Accum sa_min = -128.0rk;                 /* Min */
    
    /* Accum types (15 integer bits, 16 fractional bits) */
    _Accum a_max = 32767.9999999999999999rk;        /* Near max */
    _Accum a_min = -32768.0rk;                      /* Min */
    
    /* Long accum types (31 integer bits, 32 fractional bits) */
    long _Accum la_max = 2147483647.9999999999999999999999999999999rk;  /* Near max */
    long _Accum la_min = -2147483648.0rk;                               /* Min */
    
    /* Unsigned variants */
    unsigned short _Fract usf_max = 0.99999999976716936ur;  /* Max for unsigned short _Fract */
    unsigned _Fract uf_max = 0.9999999999999999ur;          /* Max for unsigned _Fract */
    unsigned long _Fract ulf_max = 0.9999999999999999999999999999999ur;  /* Max */
    
    unsigned short _Accum usa_max = 255.99999999976716936urk;  /* Max for unsigned short _Accum */
    unsigned _Accum ua_max = 65535.9999999999999999urk;        /* Max for unsigned _Accum */
    unsigned long _Accum ula_max = 4294967295.9999999999999999999999999999999urk;  /* Max */
    
    /* Saturated variants */
    _Sat short _Fract ssf = 0.5r;
    _Sat short _Accum ssa = 100.5rk;
    _Sat unsigned short _Fract susf = 0.5ur;
    _Sat unsigned short _Accum susa = 200.5urk;
    
    /* Test boundary assignments that should trigger range checks */
    /* These assignments may be optimized away but will force the compiler */
    /* to compute boundary values during constant folding */
    
    /* Force computations with different i_f_bits combinations */
    volatile short _Fract vsf = sf_max;
    volatile _Fract vf = f_max;
    volatile long _Fract vlf = lf_max;
    
    volatile short _Accum vsa = sa_max;
    volatile _Accum va = a_max;
    volatile long _Accum vla = la_max;
    
    volatile unsigned short _Fract vusf = usf_max;
    volatile unsigned _Fract vuf = uf_max;
    volatile unsigned long _Fract vulf = ulf_max;
    
    volatile unsigned short _Accum vusa = usa_max;
    volatile unsigned _Accum vua = ua_max;
    volatile unsigned long _Accum vula = ula_max;
    
    /* Test minimum values */
    volatile short _Fract vsf_min = sf_min;
    volatile _Fract vf_min = f_min;
    volatile long _Fract vlf_min = lf_min;
    
    volatile short _Accum vsa_min = sa_min;
    volatile _Accum va_min = a_min;
    volatile long _Accum vla_min = la_min;
    
    /* Test overflow/underflow cases with explicit casts */
    /* These should trigger the comparison logic in the uncovered lines */
    int i;
    
    /* Force evaluation of boundary comparisons */
    i = (int)(sf_max * 1000);
    i = (int)(f_max * 1000);
    i = (int)(lf_max * 1000);
    
    i = (int)(sa_max * 1000);
    i = (int)(a_max * 1000);
    i = (int)(la_max * 1000);
    
    /* Test with negative values */
    i = (int)(sf_min * 1000);
    i = (int)(f_min * 1000);
    i = (int)(lf_min * 1000);
    
    i = (int)(sa_min * 1000);
    i = (int)(a_min * 1000);
    i = (int)(la_min * 1000);
    
    /* Prevent dead code elimination */
    (void)vsf; (void)vf; (void)vlf;
    (void)vsa; (void)va; (void)vla;
    (void)vusf; (void)vuf; (void)vulf;
    (void)vusa; (void)vua; (void)vula;
    (void)vsf_min; (void)vf_min; (void)vlf_min;
    (void)vsa_min; (void)va_min; (void)vla_min;
    (void)i;
}

/* Additional test with complex expressions */
void test_complex_expressions(void) {
    /* Mixed-type expressions that require range checking */
    short _Fract sf1 = 0.5r;
    short _Fract sf2 = 0.3r;
    short _Fract sf3 = sf1 + sf2;  /* Should be in range */
    
    short _Accum sa1 = 100.5rk;
    short _Accum sa2 = 27.3rk;
    short _Accum sa3 = sa1 + sa2;  /* Should be in range */
    
    /* These might trigger boundary checks during constant folding */
    const short _Fract csf_max = 0.99999999976716936r;
    const short _Fract csf_min = -1.0r;
    
    const short _Accum csa_max = 127.99999999976716936rk;
    const short _Accum csa_min = -128.0rk;
    
    /* Use in expressions */
    volatile short _Fract vsf = csf_max * 0.5r;
    volatile short _Accum vsa = csa_max * 0.5rk;
    
    (void)sf3; (void)sa3; (void)vsf; (void)vsa;
}

/* Main function to call tests */
int main(void) {
    test_fixed_point();
    test_complex_expressions();
    return 0;
}
