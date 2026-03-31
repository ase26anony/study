/* Test program for GCC fixed-point arithmetic range calculations */
/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-value-test.c -o fixed-value-test */

#include <stdio.h>

/* Force compile-time evaluation with constexpr */
#define EVAL_CONST(expr) __builtin_constant_p(expr) ? (expr) : (expr)

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    long _Accum la;
    unsigned long long _Sat _Accum ullsata;
};

/* Array initialization with fixed-point constants */
static const struct FixedPointData init_data[] = {
    {0.999999hr, -0.999999r, 0.999999ur, 9223372036854775.807lk, 18446744073709551615.999999999uk},
    {0.000001hr, 0.000001r, 0.000001ur, -9223372036854775.807lk, 0.000000001uk},
    {0.5hr, -0.5r, 0.5ur, 0.0lk, 0.5uk}
};

/* Compile-time conditional using preprocessor */
#if defined(__STDC_IEC_60559_DFP__)
#define USE_EXTREME_VALUES 1
#else
#define USE_EXTREME_VALUES 0
#endif

int main(void) {
    /* Declare fixed-point variables with extreme values */
    unsigned _Sat _Fract usat_max = 0.999999999999999999ur;  /* Near max unsigned fract */
    signed _Sat _Fract sat_min = -0.999999999999999999r;     /* Near min signed fract */
    long _Accum la_extreme = 9223372036854775.807lk;         /* Near max long accum */
    unsigned long long _Accum ulla_max = 18446744073709551615.999999999uk;
    
    /* Force constant folding with ternary operator */
    static const unsigned _Fract uf_const = 
        (0.999999999999999999ur > 0.5ur) ? 0.999999999999999999ur : 0.5ur;
    
    /* Mixed-type arithmetic that may overflow */
    volatile unsigned _Sat _Fract result1 = usat_max + usat_max;  /* Should saturate */
    volatile signed _Sat _Fract result2 = sat_min - 0.5r;         /* Should saturate */
    
    /* Conversions that trigger range checks */
    int int_from_accum = (int)la_extreme;  /* Conversion with potential overflow */
    float float_from_fract = (float)uf_const;
    
    /* Complex expression with multiple conversions */
    long long ll_result = (long long)((la_extreme * 2.0lk) / 1.0lk);
    
    /* Loop with fixed iterations for unrolling */
    for (int i = 0; i < 3; i++) {
        /* Conditional based on fixed-point comparison */
        if (init_data[i].usf > 0.5hr) {
            /* Arithmetic that may overflow */
            unsigned _Sat _Fract temp = init_data[i].usatf * 2.0ur;
            (void)temp;  /* Use variable */
        }
        
        /* Mixed-type operations */
        long _Accum scaled = init_data[i].la << 1;  /* Left shift may overflow */
        
        /* Cast to integer with potential overflow */
        int as_int = (int)init_data[i].sf;
        
        /* Prevent dead code elimination */
        volatile int dummy = as_int;
        (void)dummy;
    }
    
    /* Compile-time evaluation using __builtin_constant_p */
    if (__builtin_constant_p(0.999999999999999999ur + 0.000000000000000001ur)) {
        /* This expression forces range calculation at compile time */
        const unsigned _Fract compile_time_fract = 
            EVAL_CONST(0.999999999999999999ur + 0.000000000000000001ur);
        (void)compile_time_fract;
    }
    
    /* Array indexing with fixed-point derived index */
    int idx = (int)(uf_const * 3.0ur);
    if (idx >= 0 && idx < 3) {
        volatile long _Accum selected = init_data[idx].la;
        (void)selected;
    }
    
    /* Explicit overflow attempt */
    unsigned long long _Sat _Accum overflow_test = ulla_max;
    for (int i = 0; i < 2; i++) {
        overflow_test = overflow_test + ulla_max;  /* Should saturate */
    }
    
    /* Print results to prevent optimization */
    printf("Results: %d %f %lld\n", int_from_accum, float_from_fract, ll_result);
    
    return 0;
}

/* Additional compile-time tests using preprocessor */
#if USE_EXTREME_VALUES
/* These initializers should trigger range checks */
static const signed _Fract extreme_fracts[] = {
    -0.999999999999999999r,
    0.999999999999999999r,
    -0.000000000000000001r,
    0.000000000000000001r
};

static const unsigned long long _Accum extreme_accums[] = {
    18446744073709551615.999999999uk,
    0.000000001uk,
    9223372036854775807.999999999k,
    -9223372036854775808.000000000k
};
#endif
