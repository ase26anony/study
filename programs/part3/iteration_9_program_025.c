/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
static __attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

static __attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

static __attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775808lk; /* MIN for signed long _Accum (Q31.32) */
}

static __attribute__((noinline)) signed _Accum get_saccum_half(void) {
    return 0.5k;
}

/* Function to consume values and prevent dead code elimination */
static volatile signed short _Fract volatile_sink_sf;
static volatile unsigned short _Fract volatile_sink_uf;
static volatile signed _Accum volatile_sink_sa;

static void sink_value_sf(signed short _Fract val) {
    volatile_sink_sf = val;
}

static void sink_value_uf(unsigned short _Fract val) {
    volatile_sink_uf = val;
}

static void sink_value_sa(signed _Accum val) {
    volatile_sink_sa = val;
}

int main(void) {
    /* 1. Boundary value testing for signed types */
    
    /* Maximum representable values */
    const signed short _Fract MAX_SFRACT = 0.999969482421875r;
    const signed short _Accum MAX_SACCUM = 32767.999969482421875k;
    const unsigned short _Fract MAX_UFRACT = 0.999969482421875ur;
    
    /* Minimum representable values */
    const signed short _Fract MIN_SFRACT = -1.0r;
    const signed short _Accum MIN_SACCUM = -32768.0k;
    
    /* Values just beyond boundaries (by 1 LSB) */
    const signed _Accum JUST_OVER_MAX = (signed _Accum)1.0k;
    const signed _Accum JUST_UNDER_MIN = (signed _Accum)-1.0000000004656612873077392578125k;
    
    /* 2. Complex constant expressions that trigger range checking */
    
    /* Expression that should overflow when converted to short _Fract */
    const signed _Accum overflow_expr = (signed _Accum)0.999k * 1.001k;
    
    /* Expression that should underflow */
    const signed _Accum underflow_expr = (signed _Accum)-0.999k * 1.001k;
    
    /* 3. Test cases designed to hit the uncovered comparisons */
    
    /* Case 1: a_high.sgt(max_r) - positive overflow in high bits */
    {
        /* Create a value that exceeds max_r in high bits when converted */
        signed long _Accum large_val = 65536.0lk; /* Too large for short _Accum */
        signed short _Accum target1 = (signed short _Accum)large_val;
        sink_value_sa((signed _Accum)target1);
    }
    
    /* Case 2: a_high == max_r && a_low.ugt(max_s) - high bits equal but low bits exceed */
    {
        /* This requires precise construction to have high bits equal to max_r
           but low bits greater than max_s */
        const signed _Accum max_saccum = 32767.999969482421875k;
        
        /* Try to create a value slightly above max_saccum */
        signed _Accum slightly_above = max_saccum;
        /* Add a small epsilon - this may get truncated or cause overflow */
        slightly_above += (signed _Accum)0.0000000001k;
        
        /* Convert to narrower type, triggering range check */
        signed short _Accum target2 = (signed short _Accum)slightly_above;
        sink_value_sa(target2);
    }
    
    /* Case 3: Negative overflow (should trigger min comparisons) */
    {
        signed long _Accum very_negative = -65536.0lk;
        signed short _Accum target3 = (signed short _Accum)very_negative;
        sink_value_sa((signed _Accum)target3);
    }
    
    /* 4. Mixed precision conversions with saturation */
    
    /* Non-saturated conversion that may overflow */
    signed _Accum nonsat_source = (signed _Accum)1.5k;
    signed short _Fract nonsat_target = (signed short _Fract)nonsat_source;
    sink_value_sf(nonsat_target);
    
    /* Saturated conversion - should saturate instead of overflow */
    signed _Sat _Accum sat_source = (signed _Sat _Accum)1.5k;
    signed short _Fract sat_target = (signed short _Fract)sat_source;
    sink_value_sf(sat_target);
    
    /* 5. Unsigned type boundary testing */
    
    /* Test unsigned overflow */
    unsigned short _Fract uf1 = MAX_UFRACT;
    /* Try to add a small value - may overflow depending on rounding */
    unsigned short _Fract uf2 = uf1 + 0.0001ur;
    sink_value_uf(uf2);
    
    /* Convert from signed to unsigned with negative value */
    signed _Accum negative_sa = (signed _Accum)-0.5k;
    unsigned short _Fract uf3 = (unsigned short _Fract)negative_sa;
    sink_value_uf(uf3);
    
    /* 6. Use opaque functions to get boundary values */
    
    signed short _Fract sf_max = get_sfract_max();
    unsigned short _Fract uf_max = get_ufract_max();
    signed long _Accum sl_min = get_saccum_min();
    
    /* Operations with opaque values (harder to constant fold) */
    signed short _Fract sf_test = sf_max * (signed short _Fract)0.999r;
    sink_value_sf(sf_test);
    
    /* 7. Loop with constant bounds to create control flow */
    
    signed _Accum accumulator = (signed _Accum)0.0k;
    for (int i = 0; i < 3; i++) {
        /* Build up to a value that may overflow on conversion */
        accumulator += (signed _Accum)0.4k;
        
        /* Convert in each iteration */
        signed short _Fract loop_conv = (signed short _Fract)accumulator;
        sink_value_sf(loop_conv);
    }
    
    /* 8. Complex expression that should be evaluated at compile time
          but still trigger range checking */
    
    const signed _Accum complex_expr = 
        ((signed _Accum)0.123k * (signed _Accum)8.0k) + 
        ((signed _Accum)-0.456k * (signed _Accum)2.0k);
    
    /* Convert to narrower type with different scaling */
    signed short _Fract from_complex = (signed short _Fract)complex_expr;
    sink_value_sf(from_complex);
    
    /* 9. Test with volatile intermediate to force materialization */
    
    volatile signed _Accum volatile_intermediate = (signed _Accum)0.999k;
    volatile_intermediate = volatile_intermediate * (signed _Accum)1.001k;
    signed short _Fract from_volatile = (signed short _Fract)volatile_intermediate;
    sink_value_sf(from_volatile);
    
    /* 10. Generate a checksum from all results for observable behavior */
    
    signed short _Fract checksum = nonsat_target + sat_target + from_complex + from_volatile;
    
    /* Convert checksum to integer for output */
    int result = (int)(checksum * 1000);
    
    printf("Result checksum: %d\n", result);
    printf("Volatile sink values: %d, %d, %d\n", 
           (int)(volatile_sink_sf * 1000),
           (int)(volatile_sink_uf * 1000),
           (int)(volatile_sink_sa * 1000));
    
    return 0;
}
