/* Test program for fixed-point arithmetic range calculations */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -o fixed_test fixed_test.c */

#include <stdio.h>

/* Force compile-time evaluation of fixed-point expressions */
#define EVAL_CONST(expr) __builtin_constant_p(expr) ? (expr) : (expr)

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long long _Accum lla;
};

int main(void) {
    /* 1. Fixed-point types at representable limits */
    const unsigned short _Fract max_usf = 0.999999hr;  /* Near max unsigned short fract */
    const signed _Fract min_sf = -0.999999r;           /* Near min signed fract */
    const unsigned _Sat _Fract usat_max = 0.999999r;
    const signed _Sat _Accum sat_min = -32768.999999k; /* Extreme value for sat accum */
    
    /* 2. Force constant folding with ternary operators */
    static const long _Accum la1 = EVAL_CONST(0.5l * 1.999999l);
    static const long _Accum la2 = EVAL_CONST((max_usf > 0.5hr) ? 0.75l : 0.25l);
    
    /* 3. Array initialization with fixed-point constants */
    struct FixedPointData data[] = {
        { .usf = 0.0hr, .sf = -0.5r, .usatf = 0.75r, .sata = 100.0k, .lla = 500.0ll },
        { .usf = 0.999999hr, .sf = 0.999999r, .usatf = 0.999999r, .sata = 32767.999999k, .lla = 9223372036854775.807ll }
    };
    
    /* 4. Saturation arithmetic that should trigger range checks */
    unsigned _Sat _Fract sat_result = usat_max + usat_max;  /* Should saturate to 1.0r */
    signed _Sat _Accum sat_acc = sat_min - 1.0k;            /* Should saturate at min */
    
    /* 5. Mixed-type conversions */
    int int_from_fract = (int)(max_usf * 100);              /* Conversion through multiplication */
    float float_from_accum = (float)sat_min;                /* Float conversion */
    
    /* 6. Loop with fixed-point operations (small count for unrolling) */
    signed _Fract accum = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Conditional based on fixed-point comparison */
        if (accum > 0.5r) {
            accum = accum * 0.75r;  /* Multiplication near limits */
        } else {
            accum = accum + 0.25r;  /* Addition that may approach limits */
        }
        
        /* Mix with array access using fixed-point derived index */
        int idx = (int)(accum * 2.0r);
        if (idx >= 0 && idx < 2) {
            accum = accum + data[idx].sf;
        }
    }
    
    /* 7. Complex expression with multiple conversions */
    long long _Accum complex_expr = 
        (long long _Accum)((float)sat_min * 0.5f) + 
        (long long _Accum)(int_from_fract) / 100.0ll;
    
    /* 8. Preprocessor conditional with fixed-point constant */
#if __builtin_constant_p(max_usf)
    /* This path only taken if max_usf is a compile-time constant */
    const signed _Accum preproc_accum = (signed _Accum)max_usf * 1000.0k;
#else
    const signed _Accum preproc_accum = 0.0k;
#endif
    
    /* Prevent dead code elimination */
    volatile unsigned _Sat _Fract vol_sat = sat_result;
    volatile signed _Fract vol_accum = accum;
    volatile long long _Accum vol_complex = complex_expr;
    
    /* Print results (some may be optimized away, but that's OK) */
    printf("Results (may be optimized):\n");
    printf("  sat_result (hex): %x\n", *(unsigned int*)&vol_sat);
    printf("  accum (hex): %x\n", *(unsigned int*)&vol_accum);
    printf("  int_from_fract: %d\n", int_from_fract);
    printf("  float_from_accum: %f\n", float_from_accum);
    
    return 0;
}

/* Additional compile-time tests outside main */
#ifdef __CHECKER__
/* Force evaluation in other compilation contexts */
static const unsigned _Fract compile_time_array[] = {
    0.0r, 0.25r, 0.5r, 0.75r, 0.999999r,  /* Pushes against max */
    0.0r/0.5r,  /* Division that should be 0 */
    0.999999r * 0.999999r  /* Multiplication near 1.0 */
};

/* Struct with bitfield and fixed-point */
struct MixedBitfield {
    int flag : 1;
    unsigned short _Fract value : 15;  /* Not standard but tests edge cases */
};
#endif
