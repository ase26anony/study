/* Test fixed-point boundary value calculations to cover lines 264-277 in fixed-value.cc */
/* Compile with: gcc -std=c99 -ffixed-point -O0 -ftest-coverage -fprofile-arcs test_fixed.c */

#include <stdio.h>

int main(void) {
    /* Test various fixed-point types with boundary values */
    
    /* 1. Short fract (7 fractional bits) - test max boundary */
    short _Fract sf_max = 0.99999999976716936r;  /* Very close to max */
    short _Fract sf_min = -1.0r;                 /* Minimum value */
    
    /* 2. Fract (15 fractional bits) */
    _Fract f_max = 0.9999999999999999r;          /* Close to max */
    _Fract f_min = -1.0r;
    
    /* 3. Long fract (31 fractional bits) */
    long _Fract lf_max = 0.9999999999999999999999999999999r;
    long _Fract lf_min = -1.0r;
    
    /* 4. Short accum (7 fractional bits, 8 integer bits) */
    short _Accum sa_max = 255.99999999976716936rk;
    short _Accum sa_min = -256.0rk;
    
    /* 5. Accum (15 fractional bits, 16 integer bits) */
    _Accum a_max = 65535.9999999999999999rk;
    _Accum a_min = -65536.0rk;
    
    /* 6. Long accum (31 fractional bits, 32 integer bits) */
    long _Accum la_max = 4294967295.9999999999999999999999999999999rk;
    long _Accum la_min = -4294967296.0rk;
    
    /* 7. Unsigned variants */
    unsigned short _Fract usf_max = 0.99999999976716936ur;
    unsigned short _Fract usf_min = 0.0ur;
    
    unsigned _Fract uf_max = 0.9999999999999999ur;
    unsigned _Fract uf_min = 0.0ur;
    
    unsigned short _Accum usa_max = 255.99999999976716936urk;
    unsigned short _Accum usa_min = 0.0urk;
    
    /* 8. Saturated variants */
    _Sat short _Fract ssf_max = 0.99999999976716936r;
    _Sat short _Fract ssf_min = -1.0r;
    
    _Sat short _Accum ssa_max = 255.99999999976716936rk;
    _Sat short _Accum ssa_min = -256.0rk;
    
    /* 9. Test overflow cases with explicit casts */
    /* These should trigger boundary checks */
    short _Fract sf_overflow = (short _Fract)1.5r;      /* Should saturate or wrap */
    short _Fract sf_underflow = (short _Fract)-1.1r;    /* Should saturate or wrap */
    
    short _Accum sa_overflow = (short _Accum)300.0rk;   /* Exceeds max of 255.999... */
    short _Accum sa_underflow = (short _Accum)-300.0rk; /* Below min of -256.0 */
    
    /* 10. Test with arithmetic that might overflow */
    short _Fract sf1 = 0.5r;
    short _Fract sf2 = 0.6r;
    short _Fract sf_sum = sf1 + sf2;  /* 1.1r -> should trigger boundary check */
    
    short _Accum sa1 = 200.0rk;
    short _Accum sa2 = 100.0rk;
    short _Accum sa_sum = sa1 + sa2;  /* 300.0rk -> should trigger boundary check */
    
    /* 11. Test boundary with multiplication */
    short _Fract sf_mul = 0.9r * 0.9r;  /* 0.81r - within range */
    short _Accum sa_mul = 200.0rk * 1.5rk;  /* 300.0rk - should trigger boundary */
    
    /* 12. Test exact boundary values */
    /* These constants should be exactly representable */
    short _Fract sf_exact_max = 0.9921875r;  /* 1 - 2^-7 */
    short _Fract sf_exact_min = -1.0r;
    
    short _Accum sa_exact_max = 255.9921875rk;  /* 256 - 2^-7 */
    short _Accum sa_exact_min = -256.0rk;
    
    /* 13. Test with different fractional bit counts through typedefs */
    typedef short _Fract sf7_t;      /* 7 fractional bits */
    typedef _Fract f15_t;            /* 15 fractional bits */
    typedef long _Fract lf31_t;      /* 31 fractional bits */
    
    sf7_t sf7_max = 0.99999999976716936r;
    f15_t f15_max = 0.9999999999999999r;
    lf31_t lf31_max = 0.9999999999999999999999999999999r;
    
    /* 14. Force compile-time evaluation with const */
    const short _Fract const_sf_max = 0.99999999976716936r;
    const short _Accum const_sa_min = -256.0rk;
    
    /* 15. Test array initialization with boundary values */
    short _Fract sf_array[4] = {
        0.0r,
        0.99999999976716936r,
        -1.0r,
        0.5r
    };
    
    short _Accum sa_array[4] = {
        0.0rk,
        255.99999999976716936rk,
        -256.0rk,
        128.5rk
    };
    
    /* 16. Test struct with fixed-point members */
    struct FixedPointStruct {
        short _Fract sf;
        short _Accum sa;
        unsigned _Fract uf;
    };
    
    struct FixedPointStruct fps = {
        .sf = 0.99999999976716936r,
        .sa = 255.99999999976716936rk,
        .uf = 0.9999999999999999ur
    };
    
    /* 17. Test with conditional expressions */
    short _Fract sf_cond = (1 > 0) ? 0.99999999976716936r : 0.0r;
    short _Accum sa_cond = (1 > 0) ? 255.99999999976716936rk : 0.0rk;
    
    /* 18. Test with function arguments */
    /* (function defined below) */
    test_fixed_arg(0.99999999976716936r, 255.99999999976716936rk);
    
    /* 19. Test with global variables */
    /* (defined below) */
    global_sf = 0.99999999976716936r;
    global_sa = 255.99999999976716936rk;
    
    /* 20. Test with volatile to force certain optimizations */
    volatile short _Fract vol_sf = 0.99999999976716936r;
    volatile short _Accum vol_sa = 255.99999999976716936rk;
    
    /* Print values to prevent optimization */
    printf("sf_max = %f\n", (double)sf_max);
    printf("sa_min = %f\n", (double)sa_min);
    printf("sf_sum = %f\n", (double)sf_sum);
    printf("sa_sum = %f\n", (double)sa_sum);
    
    return 0;
}

/* Global fixed-point variables */
short _Fract global_sf;
short _Accum global_sa;

/* Function taking fixed-point arguments */
void test_fixed_arg(short _Fract sf, short _Accum sa) {
    short _Fract local_sf = sf;
    short _Accum local_sa = sa;
    
    /* Force computation */
    local_sf = local_sf + 0.0r;
    local_sa = local_sa + 0.0rk;
    
    printf("arg_sf = %f, arg_sa = %f\n", (double)local_sf, (double)local_sa);
}

/* Additional test with switch cases */
void test_fixed_switch(short _Fract sf) {
    switch ((int)(sf * 100)) {
        case 0: printf("zero\n"); break;
        case 99: printf("max\n"); break;
        case -100: printf("min\n"); break;
        default: printf("other\n"); break;
    }
}
