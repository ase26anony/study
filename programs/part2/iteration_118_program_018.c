/* Compile with: gcc -O2 -std=c23 -Wno-psabi -fdump-tree-original fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Struct with mixed fixed-point types to test aggregate initialization */
struct FixedPointData {
    unsigned short _Fract usf;
    signed _Fract sf;
    unsigned _Sat _Fract usatf;
    long _Accum la;
    unsigned long long _Sat _Accum ullsata;
};

/* Function to force constant folding with fixed-point */
static const signed _Fract get_max_fract(void) {
    /* This should trigger range calculation for _Fract */
    const signed _Fract max_fract = 0.999999r;
    const signed _Fract half = 0.5r;
    
    /* Operation that might overflow in constant folding */
    return max_fract + half;
}

int main(void) {
    volatile int result = 0; /* volatile to prevent dead code elimination */
    
    /* 1. Fixed-point types with extreme values */
    const unsigned _Fract uf_max = 0.999999r;
    const signed _Fract sf_min = -0.999999r;
    const signed _Fract sf_max = 0.999999r;
    const long _Accum la_max = 0.999999999999999999lk;
    const long _Accum la_min = -0.999999999999999999lk;
    
    /* 2. Saturation types with operations that should saturate */
    unsigned _Sat _Fract usat1 = 0.75r;
    unsigned _Sat _Fract usat2 = 0.5r;
    
    /* This addition should saturate to max */
    usat1 = usat1 + 0.5r;
    
    /* 3. Mixed precision in expressions */
    short _Fract sf1 = 0.7hr;
    _Accum a1 = 0.8k;
    
    /* 4. Array with fixed-point initialization */
    const signed _Fract fract_array[4] = {
        -0.999999r,  /* min */
        0.0r,
        0.5r,
        0.999999r    /* max */
    };
    
    /* 5. Struct with mixed fixed-point types */
    struct FixedPointData data = {
        .usf = 0.999999hr,
        .sf = -0.5r,
        .usatf = 0.999999r,
        .la = 0.999999999999999999lk,
        .ullsata = 0.999999999999999999999999999999999999llk
    };
    
    /* 6. Loop with fixed-point operations (small for unrolling) */
    for (int i = 0; i < 3; i++) {
        /* Conditional based on fixed-point comparison */
        if (sf1 > 0.5hr) {
            /* Multiplication that could overflow */
            sf1 = sf1 * 0.9hr;
        } else {
            /* Addition that could underflow */
            sf1 = sf1 + (-0.8hr);
        }
        
        /* Mix with integer */
        a1 = a1 + (_Accum)i * 0.1k;
    }
    
    /* 7. Explicit casts that should trigger range checks */
    int int_from_fract = (int)(sf_max * 100);
    float float_from_accum = (float)la_max;
    
    /* 8. Compile-time constant expression using ternary */
    const signed _Fract const_ternary = 
        (0.999999r > 0.5r) ? 0.999999r + 0.000001r : 0.0r;
    
    /* 9. Use __builtin_constant_p to create constant-only paths */
#if __GNUC__ >= 5
    if (__builtin_constant_p(0.999999r + 0.000001r)) {
        /* This expression should trigger overflow check in constant folder */
        const signed _Fract overflow_test = 0.999999r + 0.000001r;
        result = (int)(overflow_test * 1000);
    }
#endif
    
    /* 10. Left shift simulation with fixed-point */
    /* Convert to integer, shift, convert back */
    int temp_int = (int)(la_max * (1LL << 31));
    long _Accum shifted = (long _Accum)temp_int / (1LL << 31);
    
    /* 11. Operations at boundaries */
    unsigned _Sat _Fract boundary_test = 0.999999r;
    boundary_test = boundary_test + 0.000001r;  /* Should saturate */
    
    signed _Sat _Fract sboundary_test = -0.999999r;
    sboundary_test = sboundary_test - 0.000001r;  /* Should saturate at min */
    
    /* 12. Complex expression with multiple conversions */
    result = (int)((float)sf1 * 100.0f) + 
             (int)((double)a1 * 100.0) +
             int_from_fract;
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    printf("Saturated unsigned: %u\n", (unsigned)(usat1 * 1000000));
    printf("Saturated signed: %d\n", (int)(sboundary_test * 1000000));
    
    /* Use all variables to prevent elimination */
    volatile double dump = (double)uf_max + (double)sf_min + (double)la_max + 
                          (double)data.la + (double)shifted;
    (void)dump;
    
    return 0;
}

/* Additional compile-time test in global scope */
#ifdef __cplusplus
constexpr unsigned _Fract global_const_fract = 0.999999r;
#else
static const unsigned _Fract global_const_fract = 0.999999r;
#endif

/* Array indexed by fixed-point derived value */
static int fixed_array[4] = {0};
static void init_array(void) {
    /* This should trigger constant folding with range check */
    const signed _Fract idx_val = 0.999999r;
    int idx = (int)(idx_val * 3);  /* Convert to array index */
    if (idx >= 0 && idx < 4) {
        fixed_array[idx] = 1;
    }
}
