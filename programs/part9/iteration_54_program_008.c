/* Test program to cover fixed-value.cc lines 264-277 */
/* Compile with: gcc -std=c99 -ffixed-point -O2 -fprofile-arcs -ftest-coverage */

#include <stdio.h>

/* Test various fixed-point types with different integer/fractional bit combinations */
void test_fract_types(void) {
    /* Short fract types - 7 fractional bits, 1 integer bit (including sign) */
    short _Fract sf_max = 0.99999999976716936r;  /* Near max for short _Fract */
    short _Fract sf_min = -1.0r;                 /* Min for short _Fract */
    unsigned short _Fract usf_max = 0.99999999976716936r; /* Near max for unsigned */
    
    /* Fract types - 15 fractional bits, 1 integer bit */
    _Fract f_max = 0.9999999999999999r;          /* Near max for _Fract */
    _Fract f_min = -1.0r;                        /* Min for _Fract */
    unsigned _Fract uf_max = 0.9999999999999999r; /* Near max for unsigned */
    
    /* Long fract types - 31 fractional bits, 1 integer bit */
    long _Fract lf_max = 0.9999999999999999999999999999999r; /* Near max */
    long _Fract lf_min = -1.0r;                   /* Min for long _Fract */
    unsigned long _Fract ulf_max = 0.9999999999999999999999999999999r;
    
    /* Accum types - have both integer and fractional parts */
    short _Accum sa_max = 32767.99999999976716936rk;  /* Max for short _Accum */
    short _Accum sa_min = -32768.0rk;                 /* Min for short _Accum */
    unsigned short _Accum usa_max = 65535.99999999976716936rk; /* Max for unsigned */
    
    _Accum a_max = 32767.9999999999999999rk;          /* Max for _Accum */
    _Accum a_min = -32768.0rk;                        /* Min for _Accum */
    unsigned _Accum ua_max = 65535.9999999999999999rk; /* Max for unsigned */
    
    long _Accum la_max = 2147483647.9999999999999999999999999999999rk; /* Max */
    long _Accum la_min = -2147483648.0rk;             /* Min for long _Accum */
    unsigned long _Accum ula_max = 4294967295.9999999999999999999999999999999rk;
    
    /* Test saturated types */
    _Sat short _Fract ssf = 0.5r;
    _Sat unsigned short _Fract susf = 0.5r;
    _Sat short _Accum ssa = 100.0rk;
    _Sat unsigned short _Accum susa = 100.0rk;
    
    /* Force boundary value computations through assignments */
    sf_max = 0.99999999976716936r;
    sf_min = -1.0r;
    usf_max = 0.99999999976716936r;
    
    f_max = 0.9999999999999999r;
    f_min = -1.0r;
    uf_max = 0.9999999999999999r;
    
    lf_max = 0.9999999999999999999999999999999r;
    lf_min = -1.0r;
    ulf_max = 0.9999999999999999999999999999999r;
    
    sa_max = 32767.99999999976716936rk;
    sa_min = -32768.0rk;
    usa_max = 65535.99999999976716936rk;
    
    a_max = 32767.9999999999999999rk;
    a_min = -32768.0rk;
    ua_max = 65535.9999999999999999rk;
    
    la_max = 2147483647.9999999999999999999999999999999rk;
    la_min = -2147483648.0rk;
    ula_max = 4294967295.9999999999999999999999999999999rk;
    
    /* Test overflow/underflow with saturated arithmetic */
    ssf = 2.0r;  /* Should saturate to max */
    susf = 2.0r; /* Should saturate to max */
    ssa = 40000.0rk; /* Should saturate to max for short _Accum */
    susa = 70000.0rk; /* Should saturate to max for unsigned short _Accum */
    
    /* Test with compile-time constant expressions that require range checking */
    const short _Fract c_sf = 0.99999999976716936r;
    const _Fract c_f = 0.9999999999999999r;
    const long _Fract c_lf = 0.9999999999999999999999999999999r;
    const short _Accum c_sa = 32767.99999999976716936rk;
    const _Accum c_a = 32767.9999999999999999rk;
    const long _Accum c_la = 2147483647.9999999999999999999999999999999rk;
    
    /* Use volatile to prevent optimization */
    volatile short _Fract v_sf = c_sf;
    volatile _Fract v_f = c_f;
    volatile long _Fract v_lf = c_lf;
    volatile short _Accum v_sa = c_sa;
    volatile _Accum v_a = c_a;
    volatile long _Accum v_la = c_la;
    
    (void)v_sf; (void)v_f; (void)v_lf;
    (void)v_sa; (void)v_a; (void)v_la;
}

/* Additional test with arithmetic operations that might trigger range checks */
void test_arithmetic(void) {
    /* Test operations that might overflow */
    short _Accum sa1 = 30000.0rk;
    short _Accum sa2 = 3000.0rk;
    short _Accum sa3 = sa1 + sa2;  /* Potential overflow for short _Accum */
    
    _Accum a1 = 30000.0rk;
    _Accum a2 = 3000.0rk;
    _Accum a3 = a1 + a2;
    
    /* Test multiplication that might exceed range */
    short _Fract sf1 = 0.9r;
    short _Fract sf2 = 0.9r;
    short _Fract sf3 = sf1 * sf2;
    
    /* Test with different scaling */
    long _Accum la1 = 1000000.0rk;
    long _Accum la2 = 1000000.0rk;
    long _Accum la3 = la1 * la2 / 1000000.0rk;
    
    /* Use volatile to prevent dead code elimination */
    volatile short _Accum v_sa3 = sa3;
    volatile _Accum v_a3 = a3;
    volatile short _Fract v_sf3 = sf3;
    volatile long _Accum v_la3 = la3;
    
    (void)v_sa3; (void)v_a3; (void)v_sf3; (void)v_la3;
}

/* Test with array initializations that require constant folding */
void test_array_init(void) {
    /* Array initialization with fixed-point constants */
    short _Fract sf_array[3] = {
        0.99999999976716936r,  /* Max */
        -1.0r,                 /* Min */
        0.5r                   /* Mid */
    };
    
    short _Accum sa_array[3] = {
        32767.99999999976716936rk,  /* Max */
        -32768.0rk,                 /* Min */
        0.0rk                       /* Zero */
    };
    
    long _Accum la_array[3] = {
        2147483647.9999999999999999999999999999999rk,  /* Max */
        -2147483648.0rk,                               /* Min */
        1000.0rk                                       /* Arbitrary */
    };
    
    /* Use volatile to prevent optimization */
    volatile short _Fract v_sf0 = sf_array[0];
    volatile short _Fract v_sf1 = sf_array[1];
    volatile short _Accum v_sa0 = sa_array[0];
    volatile short _Accum v_sa1 = sa_array[1];
    volatile long _Accum v_la0 = la_array[0];
    volatile long _Accum v_la1 = la_array[1];
    
    (void)v_sf0; (void)v_sf1; (void)v_sa0; (void)v_sa1; (void)v_la0; (void)v_la1;
}

int main(void) {
    printf("Testing fixed-point type range calculations...\n");
    
    test_fract_types();
    test_arithmetic();
    test_array_init();
    
    printf("Tests completed.\n");
    return 0;
}
