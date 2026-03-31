/* Test program to exercise fixed-point range calculation logic in GCC */
/* Compile with: gcc -O2 -std=c23 -fdump-tree-original -fdump-tree-optimized test_fixed.c */

#include <stdio.h>

/* Force compile-time evaluation of fixed-point expressions */
#define EVAL_CONST(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Accum ua;
    signed long _Accum sla;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum ssata;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.999999hr, -0.999999r, 255.999999uk, -32767.999999lk},
    {0.5hr, -0.5r, 127.5uk, -16383.5lk},
    {0.0hr, 0.0r, 0.0uk, 0.0lk}
};

int main(void) {
    /* 1. Fixed-point types at representable limits */
    const unsigned short _Fract max_usf = 0.999999hr;  /* Max unsigned short fract */
    const signed _Fract min_sf = -1.0r;                /* Min signed fract */
    const unsigned _Accum max_ua = 255.999999uk;       /* Max unsigned 8.8 accum */
    const signed long _Accum min_sla = -32768.0lk;     /* Min signed 16.16 accum */
    
    /* 2. Saturation types with overflow/underflow operations */
    unsigned _Sat _Fract usat1 = 0.8r;
    unsigned _Sat _Fract usat2 = 0.9r;
    signed _Sat _Accum ssat1 = 127.5k;
    signed _Sat _Accum ssat2 = 128.5k;
    
    /* 3. Force constant folding with ternary operator */
    static const signed _Fract const_fold_test = 
        EVAL_CONST((max_usf > 0.5hr) ? 0.75r : -0.75r);
    
    /* 4. Mixed-type conversions */
    volatile int int_from_fract = (int)(max_ua * 2.0);  /* Will overflow */
    volatile float float_from_accum = (float)min_sla;
    
    /* 5. Arithmetic that may trigger range checks */
    usat1 = usat1 + usat2;  /* Should saturate to 1.0r */
    ssat1 = ssat1 + ssat2;  /* Should saturate to max */
    
    /* 6. Loop with fixed-point operations */
    signed _Accum accumulator = 0.0k;
    for (int i = 0; i < 4; i++) {
        /* Conditional based on fixed-point comparison */
        if (accumulator > 64.0k) {
            accumulator = accumulator * 0.5k;  /* May trigger range calc */
        } else {
            accumulator = accumulator + 32.0k; /* May overflow */
        }
        
        /* Mix with array access */
        accumulator = accumulator + (signed _Accum)init_data[i % 3].sf;
    }
    
    /* 7. More boundary tests */
    const signed _Fract boundary_test1 = -0.999999r;
    const signed _Fract boundary_test2 = 0.999999r;
    
    /* These comparisons should trigger range calculations */
    volatile int cmp1 = (boundary_test1 == min_sf);
    volatile int cmp2 = (boundary_test2 > 0.5r);
    
    /* 8. Shift operations (through multiplication by powers of 2) */
    unsigned _Accum shifted = max_ua * 4.0;  /* Will overflow */
    signed long _Accum shifted_signed = min_sla * 0.25;  /* Underflow check */
    
    /* 9. Complex expression with multiple conversions */
    volatile long complex_expr = 
        (long)((signed long _Accum)((float)max_ua * 1000.0f) / 2.0lk);
    
    /* 10. Preprocessor conditional with fixed-point constant */
#if 1  /* Always true, but forces evaluation */
    const signed _Fract preproc_test = 
        (max_usf > 0.5hr) ? 0.999999r : -0.999999r;
#endif
    
    /* Prevent dead code elimination */
    volatile unsigned short _Fract v1 = max_usf;
    volatile signed _Fract v2 = min_sf;
    volatile unsigned _Accum v3 = max_ua;
    volatile signed long _Accum v4 = min_sla;
    volatile unsigned _Sat _Fract v5 = usat1;
    volatile signed _Sat _Accum v6 = ssat1;
    volatile signed _Accum v7 = accumulator;
    volatile signed _Fract v8 = const_fold_test;
    volatile signed _Fract v9 = preproc_test;
    
    /* Print to prevent optimization */
    printf("Results: %d %d %d %ld\n", 
           int_from_fract, cmp1, cmp2, complex_expr);
    
    return 0;
}

/* Additional compile-time tests */
#ifdef __CHECKER__
/* These force constant evaluation during compilation */
static const unsigned _Fract compile_time_array[] = {
    0.0r, 0.25r, 0.5r, 0.75r, 0.999999r
};

static const struct FixedPointData ct_data = {
    .usf = 0.999999hr,
    .sf = -0.999999r,
    .ua = 255.999999uk,
    .sla = -32767.999999lk,
    .usatf = 0.999999r,
    .ssata = 127.999999k
};
#endif
