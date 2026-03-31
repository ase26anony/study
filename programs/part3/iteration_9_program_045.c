/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    volatile signed _Accum sink = val;
    return sink;
}

int main(void) {
    /* Test 1: Signed fixed-point conversions with boundary values */
    
    /* Initialize with maximum representable values */
    const signed short _Fract max_sfract = 0.999969482421875r;  /* Q0.15 max */
    const signed short _Fract min_sfract = -1.0r;               /* Q0.15 min */
    
    /* Use volatile to force materialization */
    volatile signed _Accum v1 = (signed _Accum)max_sfract;
    volatile signed _Accum v2 = (signed _Accum)min_sfract;
    
    /* Create values just beyond limits */
    const signed _Accum just_above_max = (signed _Accum)1.0001r;  /* > 1.0 */
    const signed _Accum just_below_min = (signed _Accum)-1.0001r; /* < -1.0 */
    
    /* Conversion that should trigger range checking */
    signed short _Fract t1 = (signed short _Fract)just_above_max;
    signed short _Fract t2 = (signed short _Fract)just_below_min;
    
    /* Test 2: Complex constant expressions forcing range checks */
    constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;      /* 1.5 */
    constexpr signed _Accum c2 = (signed _Accum)-0.75r * 2.0r;    /* -1.5 */
    
    /* These conversions require range checking */
    const signed short _Fract from_c1 = (signed short _Fract)c1;  /* 1.5 -> saturate? */
    const signed short _Fract from_c2 = (signed short _Fract)c2;  /* -1.5 -> saturate? */
    
    /* Test 3: Mixed precision with arithmetic overflow */
    signed long _Accum la1 = 5000.0lk;
    signed long _Accum la2 = 5000.0lk;
    signed long _Accum la_product = la1 * la2;  /* Large value */
    
    /* Conversion to narrower type triggers range check */
    signed _Accum narrowed = (signed _Accum)la_product;
    
    /* Test 4: Unsigned fixed-point with overflow */
    unsigned short _Fract uf1 = 0.999969482421875ur;  /* MAX */
    unsigned short _Fract uf2 = 0.0001ur;             /* Small increment */
    
    /* Addition that overflows */
    unsigned _Accum ua_sum = (unsigned _Accum)uf1 + (unsigned _Accum)uf2;
    
    /* Conversion back triggers unsigned range check */
    unsigned short _Fract uf_result = (unsigned short _Fract)ua_sum;
    
    /* Test 5: Saturation qualifier tests */
    _Sat signed short _Fract sat1 = (signed short _Fract)1.5r;    /* Should saturate to MAX */
    _Sat signed short _Fract sat2 = (signed short _Fract)-1.5r;   /* Should saturate to MIN */
    
    /* Mix saturated and non-saturated */
    signed _Accum nsat = 2.0k;
    _Sat signed short _Fract sat3 = (signed short _Fract)nsat;    /* Conversion with saturation */
    
    /* Test 6: Loop with small iteration count for partial constant folding */
    signed _Accum accumulator = 0.0k;
    for (int i = 0; i < 3; i++) {
        accumulator += (signed _Accum)0.7r;
    }
    /* Convert accumulated value (2.1) to narrower type */
    signed short _Fract loop_result = (signed short _Fract)accumulator;
    
    /* Test 7: Boundary case - exactly at maximum */
    signed _Accum exact_max = (signed _Accum)1.0r;
    signed short _Fract at_max = (signed short _Fract)exact_max;
    
    /* Test 8: One LSB beyond maximum */
    /* For Q0.15, 1 LSB = 1/32768 ≈ 0.000030517578125 */
    signed _Accum beyond_max = (signed _Accum)1.000030517578125r;  /* MAX + 1LSB */
    signed short _Fract beyond_result = (signed short _Fract)beyond_max;
    
    /* Test 9: Use opaque function results */
    signed short _Fract opaque_max = get_sfract_max();
    signed _Accum enlarged = (signed _Accum)opaque_max * (signed _Accum)1.1r;
    signed short _Fract back_conv = (signed short _Fract)enlarged;
    
    /* Test 10: Negative to unsigned conversion */
    signed _Accum negative = -0.5k;
    unsigned short _Fract to_unsigned = (unsigned short _Fract)negative;
    
    /* Prevent dead code elimination */
    consume_accum(v1);
    consume_accum(v2);
    consume_accum((signed _Accum)t1);
    consume_accum((signed _Accum)t2);
    consume_accum(c1);
    consume_accum(c2);
    consume_accum((signed _Accum)from_c1);
    consume_accum((signed _Accum)from_c2);
    consume_accum(la_product);
    consume_accum(narrowed);
    consume_accum((signed _Accum)ua_sum);
    consume_accum((signed _Accum)uf_result);
    consume_accum((signed _Accum)sat1);
    consume_accum((signed _Accum)sat2);
    consume_accum(nsat);
    consume_accum((signed _Accum)sat3);
    consume_accum(accumulator);
    consume_accum((signed _Accum)loop_result);
    consume_accum(exact_max);
    consume_accum((signed _Accum)at_max);
    consume_accum(beyond_max);
    consume_accum((signed _Accum)beyond_result);
    consume_accum(enlarged);
    consume_accum((signed _Accum)back_conv);
    consume_accum(negative);
    consume_accum((signed _Accum)to_unsigned);
    
    /* Create observable output to prevent complete optimization */
    signed _Accum checksum = v1 + v2 + (signed _Accum)t1 + (signed _Accum)t2 +
                           c1 + c2 + (signed _Accum)from_c1 + (signed _Accum)from_c2 +
                           la_product + narrowed + (signed _Accum)ua_sum +
                           (signed _Accum)uf_result + (signed _Accum)sat1 +
                           (signed _Accum)sat2 + nsat + (signed _Accum)sat3 +
                           accumulator + (signed _Accum)loop_result + exact_max +
                           (signed _Accum)at_max + beyond_max +
                           (signed _Accum)beyond_result + enlarged +
                           (signed _Accum)back_conv + negative +
                           (signed _Accum)to_unsigned;
    
    /* Print something to ensure execution */
    printf("Checksum (as long long): %lld\n", (long long)checksum);
    
    return 0;
}
