/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

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
    
    /* Values just beyond limits (by 1 LSB) */
    const signed _Accum just_above_max = (signed _Accum)1.0000001r;
    const signed _Accum just_below_min = (signed _Accum)-1.0000001r;
    
    /* Complex constant expressions that approach limits */
    constexpr signed _Accum c1 = (signed _Accum)0.999999r * 1.000001r;
    constexpr signed _Accum c2 = (signed _Accum)(-0.999999r) * 1.000001r;
    
    /* These conversions should trigger range checks */
    volatile signed short _Fract v1 = (signed short _Fract)c1;
    volatile signed short _Fract v2 = (signed short _Fract)c2;
    
    /* Test 2: Mixed precision with arithmetic overflow */
    signed long _Accum la1 = 5000.0k;
    signed long _Accum la2 = 5000.0k;
    
    /* Multiplication that overflows when converted to narrower type */
    signed _Accum product = (signed _Accum)la1 * (signed _Accum)la2;
    volatile signed short _Fract narrow1 = (signed short _Fract)product;
    
    /* Test 3: Boundary testing with function calls to prevent optimization */
    signed short _Fract sf1 = get_sfract_max();
    /* Add a small value to cause overflow */
    signed _Accum sf1_plus = (signed _Accum)sf1 + (signed _Accum)0.0001r;
    volatile signed short _Fract narrow2 = (signed short _Fract)sf1_plus;
    
    /* Test 4: Unsigned fixed-point with overflow/underflow */
    unsigned short _Fract uf_max = get_ufract_max();
    unsigned _Accum ua1 = (unsigned _Accum)uf_max * (unsigned _Accum)1.1r;
    
    /* This conversion should trigger unsigned range check (a_low.ugt(max_s)) */
    volatile unsigned short _Fract narrow3 = (unsigned short _Fract)ua1;
    
    /* Test negative to unsigned conversion (should trigger min check) */
    signed _Accum neg_val = (signed _Accum)-0.5r;
    volatile unsigned short _Fract narrow4 = (unsigned short _Fract)neg_val;
    
    /* Test 5: Saturation qualifier tests */
    _Sat signed short _Fract sat_sf1 = 0.999969482421875r;
    _Sat signed _Accum sa1 = (signed _Accum)1.5r;
    
    /* Saturated operations followed by conversion */
    _Sat signed short _Fract sat_result = sat_sf1 + (_Sat signed short _Fract)0.1r;
    volatile signed short _Fract narrow5 = (signed short _Fract)sa1;
    
    /* Test 6: Loop-based computations with mostly constant values */
    signed _Accum accum = 0.0k;
    for (int i = 0; i < 3; i++) {
        accum = accum + (signed _Accum)0.33333333333333333333r;
    }
    /* accum should be ~1.0, convert to narrower type */
    volatile signed short _Fract narrow6 = (signed short _Fract)accum;
    
    /* Test 7: Minimum value testing */
    signed long _Accum sla_min = get_saccum_min();
    /* Multiply to make it even smaller (more negative) */
    signed long _Accum sla_min2 = sla_min * (signed long _Accum)1.1k;
    volatile signed _Accum narrow7 = (signed _Accum)sla_min2;
    
    /* Test 8: Mixed signed/unsigned with different precisions */
    unsigned long _Accum ula = (unsigned long _Accum)18446744073709551.615uk;
    signed short _Fract sf_conv = (signed short _Fract)ula;
    volatile signed short _Fract narrow8 = sf_conv;
    
    /* Test 9: Direct maximum value conversion */
    const signed _Accum max_saccum = 9223372036854775.807k;  /* Approx max */
    volatile signed short _Fract narrow9 = (signed short _Fract)max_saccum;
    
    /* Test 10: Values exactly at conversion boundaries */
    /* This should test a_high == max_r && a_low.ugt(max_s) path */
    const unsigned _Accum ua_boundary = (unsigned _Accum)1.0r - (unsigned _Accum)0.0000000001r;
    volatile unsigned short _Fract narrow10 = (unsigned short _Fract)ua_boundary;
    
    /* Create observable output to prevent dead code elimination */
    signed short _Fract checksum = narrow1 + narrow2 + narrow3 + narrow4 
                                 + narrow5 + narrow6 + narrow7 + narrow8 
                                 + narrow9 + narrow10;
    
    /* Use the results */
    consume_accum((signed _Accum)checksum);
    
    printf("Fixed-point range checking test completed.\n");
    printf("Checksum (as float): %f\n", (double)(signed _Accum)checksum);
    
    return 0;
}
