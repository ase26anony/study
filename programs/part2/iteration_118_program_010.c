/* Test program to exercise fixed-point range calculation logic in GCC */
#include <stdio.h>

/* Use ISO C23 fixed-point types with GCC extensions */
#define FORCE_EVAL(x) do { volatile __typeof__(x) _x = (x); (void)_x; } while(0)

/* Struct with mixed fixed-point types */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    signed _Sat _Accum sata;
    long _Accum la;
    unsigned long _Accum ula;
};

/* Function to force constant folding with fixed-point */
static const signed _Fract const_fract = 0.999999r;
static const unsigned _Sat _Fract const_sat_fract = 0.999999ur;

/* Array initialized with fixed-point constants */
static const signed _Fract fract_array[] = {
    0.0r, 0.5r, -0.5r, 0.999999r, -0.999999r,
    0.0000001r, -0.0000001r
};

/* Compile-time conditional using fixed-point constants */
#if defined(__GNUC__) && (__GNUC__ >= 7)
#define USE_FIXED_POINT 1
#else
#define USE_FIXED_POINT 0
#endif

int main(void) {
    /* 1. Declare fixed-point variables at representable limits */
    unsigned short _Fract usf_max = 0.999999hr;
    signed _Fract sf_max = 0.999999r;
    signed _Fract sf_min = -1.0r;
    unsigned _Sat _Fract usf_sat = 0.999999ur;
    signed _Sat _Accum sa_sat = 0.999999999999999999r;  /* Near max */
    long _Accum la_max = 0.999999999999999999lr;
    long _Accum la_min = -1.0lr;
    unsigned long _Accum ula_max = 0.999999999999999999ulr;
    
    /* 2. Force constant folding with ternary operator */
    const signed _Fract folded = (const_fract > 0.5r) ? 
        (const_fract * const_fract) : (const_fract / 2.0r);
    FORCE_EVAL(folded);
    
    /* 3. Saturation arithmetic that should overflow/underflow */
    usf_sat = usf_sat + 0.5ur;  /* Should saturate to max */
    sa_sat = sa_sat * 2.0r;     /* Should saturate */
    
    /* 4. Mixed-type conversions and arithmetic */
    int int_from_fract = (int)(sf_max * 1000);
    float float_from_accum = (float)la_max;
    signed _Fract fract_from_int = (signed _Fract)int_from_fract;
    signed _Accum accum_from_float = (signed _Accum)float_from_accum;
    
    /* 5. Loop with fixed-point operations (small count for unrolling) */
    signed _Fract loop_accum = 0.0r;
    for (int i = 0; i < 4; i++) {
        /* Conditional based on fixed-point comparison */
        if (loop_accum > 0.5r) {
            loop_accum = loop_accum * 0.5r;
        } else {
            loop_accum = loop_accum + 0.25r;
        }
        
        /* Mix with array access */
        loop_accum = loop_accum + fract_array[i % 7];
    }
    FORCE_EVAL(loop_accum);
    
    /* 6. Struct initialization with mixed fixed-point types */
    struct FixedPointData data = {
        .usf = 0.999999hr,
        .sf = -0.999999r,
        .usatf = 0.999999ur,
        .sata = 0.999999999999999999r,
        .la = -0.999999999999999999lr,
        .ula = 0.999999999999999999ulr
    };
    
    /* 7. Complex expression with multiple conversions */
    long long result = (long long)(
        (data.sf * 1000.0r) + 
        (signed _Accum)(data.usf * 2.0hr) +
        (float)data.sata / 2.0f
    );
    
    /* 8. Use __builtin_constant_p to create constant-only paths */
    if (__builtin_constant_p(const_sat_fract)) {
        /* This should trigger constant evaluation */
        const unsigned _Sat _Fract test_sat = const_sat_fract + const_sat_fract;
        FORCE_EVAL(test_sat);
    }
    
    /* 9. Edge case: operations at exact boundaries */
    signed _Fract boundary_test = 0.999999r * 0.999999r;  /* Near 1.0 */
    signed _Fract underflow_test = -0.999999r * 0.999999r; /* Near -1.0 */
    
    /* 10. Shift-like behavior using multiplication by powers of 2 */
    signed _Accum shift_test = (signed _Accum)0.5r;
    for (int i = 0; i < 3; i++) {
        shift_test = shift_test * 2.0r;  /* Simulate left shift */
    }
    
    /* Prevent dead code elimination */
    printf("Results: %d %f %lld\n", 
           int_from_fract, 
           float_from_accum,
           result);
    
    /* Use volatile to force evaluation */
    volatile signed _Fract vol_fract = boundary_test + underflow_test;
    (void)vol_fract;
    
    return 0;
}
