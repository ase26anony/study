/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775808.0lk; /* MIN for signed long _Accum (Q31.32) */
}

__attribute__((noinline)) signed _Accum get_saccum_half(void) {
    return 0.5k;
}

/* Function to consume values and prevent dead code elimination */
volatile signed short _Fract global_sfract;
volatile unsigned short _Fract global_ufract;
volatile signed _Accum global_saccum;
volatile signed long _Accum global_slaccum;

__attribute__((noinline)) void consume_values(signed short _Fract sf, 
                                             unsigned short _Fract uf,
                                             signed _Accum sa,
                                             signed long _Accum sla) {
    global_sfract = sf;
    global_ufract = uf;
    global_saccum = sa;
    global_slaccum = sla;
}

int main(void) {
    /* Test 1: Boundary values for signed types */
    const signed short _Fract sf_max = 0.999969482421875r;  /* Q0.15 max */
    const signed short _Fract sf_min = -1.0r;               /* Q0.15 min */
    const signed short _Fract sf_near_max = 0.99993896484375r; /* MAX - 1 LSB */
    
    /* Test 2: Complex constant expressions that approach limits */
    const signed _Accum sa1 = (signed _Accum)0.75k * 1.33333333333333333333k; /* ~1.0 */
    const signed _Accum sa2 = (signed _Accum)(-0.75k) * 1.33333333333333333333k; /* ~-1.0 */
    
    /* Test 3: Values that will overflow when converted to narrower types */
    volatile signed _Accum volatile_sa = 1.5k;  /* > sf_max when converted */
    volatile signed _Accum volatile_sa_neg = -1.1k; /* < sf_min when converted */
    
    /* Test 4: Unsigned boundary testing */
    const unsigned short _Fract uf_max = 0.999969482421875ur; /* U0.16 max */
    const unsigned short _Fract uf_near_max = 0.99993896484375ur; /* MAX - 1 LSB */
    
    /* Test 5: Saturated types */
    signed short _Fract _Sat sf_sat;
    unsigned short _Fract _Sat uf_sat;
    
    /* Test 6: Mixed precision arithmetic with potential overflow */
    
    /* Operation 1: Convert _Accum to _Fract near maximum */
    signed short _Fract conv1 = (signed short _Fract)sa1;  /* Should be near max */
    signed short _Fract conv2 = (signed short _Fract)volatile_sa; /* Should overflow */
    
    /* Operation 2: Negative overflow */
    signed short _Fract conv3 = (signed short _Fract)volatile_sa_neg; /* Should underflow */
    
    /* Operation 3: Multiplication that exceeds range */
    signed short _Fract mul_result;
    {
        /* Force computation in wider type then narrow conversion */
        signed _Accum temp = (signed _Accum)sf_max * 1.1k;
        mul_result = (signed short _Fract)temp;  /* Should trigger range check */
    }
    
    /* Operation 4: Unsigned overflow */
    unsigned short _Fract uconv1 = (unsigned short _Fract)1.2k; /* > 1.0, should overflow */
    
    /* Operation 5: Saturation tests */
    sf_sat = (signed short _Fract _Sat)volatile_sa;  /* Should saturate to MAX */
    uf_sat = (unsigned short _Fract _Sat)volatile_sa; /* Should saturate to MAX (positive) */
    
    /* Operation 6: Negative to unsigned conversion */
    unsigned short _Fract uconv_neg = (unsigned short _Fract)(-0.5k); /* Should overflow to 0 */
    
    /* Operation 7: Complex expression with multiple conversions */
    const signed _Accum complex_expr = (signed _Accum)0.999k + (signed _Accum)0.001k;
    signed short _Fract conv_complex = (signed short _Fract)complex_expr; /* Exact boundary */
    
    /* Operation 8: Loop with accumulating error */
    signed short _Fract accum = 0.0r;
    for (int i = 0; i < 5; i++) {
        accum += 0.2r;  /* 5 * 0.2 = 1.0, but with rounding */
    }
    signed short _Fract loop_result = accum;  /* Should be near 1.0 */
    
    /* Operation 9: Test minimum values for signed long _Accum */
    signed long _Accum sl_min = get_saccum_min();
    signed short _Fract conv_from_min = (signed short _Fract)sl_min; /* Large negative */
    
    /* Operation 10: Mixed signed/unsigned */
    unsigned short _Fract mixed_conv = (unsigned short _Fract)get_saccum_half();
    
    /* Operation 11: Values just beyond 1 LSB of maximum */
    signed _Accum just_above_max = (signed _Accum)sf_max + (signed _Accum)0.0001k;
    signed short _Fract conv_just_above = (signed short _Fract)just_above_max;
    
    /* Operation 12: Values just beyond 1 LSB of minimum */
    signed _Accum just_below_min = (signed _Accum)sf_min - (signed _Accum)0.0001k;
    signed short _Fract conv_just_below = (signed short _Fract)just_below_min;
    
    /* Consume all results to prevent optimization */
    consume_values(conv1, uconv1, sa1, sl_min);
    
    /* Also consume other important values through volatile stores */
    global_sfract = conv2;
    global_sfract = conv3;
    global_sfract = mul_result;
    global_ufract = uconv_neg;
    global_sfract = conv_complex;
    global_sfract = loop_result;
    global_sfract = conv_from_min;
    global_ufract = mixed_conv;
    global_sfract = conv_just_above;
    global_sfract = conv_just_below;
    global_sfract = sf_sat;
    global_ufract = uf_sat;
    
    /* Create a simple checksum for observable behavior */
    signed _Accum checksum = (signed _Accum)conv1 + (signed _Accum)conv2 + 
                            (signed _Accum)conv3 + (signed _Accum)mul_result +
                            (signed _Accum)uconv1 + (signed _Accum)uconv_neg +
                            (signed _Accum)conv_complex + (signed _Accum)loop_result +
                            (signed _Accum)conv_from_min + (signed _Accum)mixed_conv +
                            (signed _Accum)conv_just_above + (signed _Accum)conv_just_below +
                            (signed _Accum)sf_sat + (signed _Accum)uf_sat;
    
    /* Print something to ensure execution */
    printf("Checksum (as integer): %lld\n", (long long)(checksum * 1000k));
    
    return 0;
}
