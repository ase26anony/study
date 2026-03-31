/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875sr; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807sk; /* Approx min for signed long _Accum (Q63.63) */
}

__attribute__((noinline)) signed _Accum get_saccum_half(void) {
    return 0.5k;
}

/* Volatile stores to force materialization */
volatile signed short _Fract volatile_sfract;
volatile unsigned short _Fract volatile_ufract;
volatile signed _Accum volatile_saccum;
volatile signed long _Accum volatile_slaccum;

int main(void) {
    /* Test 1: Boundary values for signed types */
    const signed short _Fract sf_max = 0.999969482421875sr;  /* Q0.15 max */
    const signed short _Fract sf_min = -1.0sr;               /* Q0.15 min */
    const signed short _Fract sf_near_max = 0.99993896484375sr; /* MAX - 1LSB */
    
    /* Test 2: Complex constant expressions that approach limits */
    const signed _Accum c1 = (signed _Accum)0.999999999k * 0.999999999k; /* ~0.999999998 */
    const signed _Accum c2 = (signed _Accum)(-0.999999999k) * 0.999999999k; /* ~-0.999999998 */
    
    /* Test 3: Conversions that require range checking */
    /* Convert from _Accum (Q31.31) to short _Fract (Q0.15) - likely overflow */
    signed _Accum a1 = 1.5k;  /* > 1.0, will overflow when converting to _Fract */
    signed short _Fract f1 = (signed short _Fract)a1;  /* Should trigger range check */
    volatile_sfract = f1;
    
    /* Test 4: Just at the boundary */
    signed _Accum a2 = 1.0k;  /* Exactly at boundary */
    signed short _Fract f2 = (signed short _Fract)a2;  /* Should be exactly MAX */
    volatile_sfract = f2;
    
    /* Test 5: Just below boundary */
    signed _Accum a3 = (signed _Accum)0.999999999k;  /* Slightly below 1.0 */
    signed short _Fract f3 = (signed short _Fract)a3;  /* Should convert without overflow */
    volatile_sfract = f3;
    
    /* Test 6: Negative overflow */
    signed _Accum a4 = -1.000000001k;  /* Slightly below -1.0 */
    signed short _Fract f4 = (signed short _Fract)a4;  /* Should trigger negative overflow check */
    volatile_sfract = f4;
    
    /* Test 7: Unsigned types - overflow beyond 1.0 */
    unsigned _Accum ua1 = 1.000000001uk;  /* > 1.0 */
    unsigned short _Fract uf1 = (unsigned short _Fract)ua1;  /* Should trigger unsigned overflow */
    volatile_ufract = uf1;
    
    /* Test 8: Unsigned types - underflow (negative to unsigned) */
    signed _Accum sa5 = -0.5k;
    unsigned short _Fract uf2 = (unsigned short _Fract)sa5;  /* Should trigger underflow */
    volatile_ufract = uf2;
    
    /* Test 9: Mixed precision with saturation */
    _Sat signed _Accum sat_a1 = 2.0k;  /* Will saturate to MAX */
    _Sat signed short _Fract sat_f1 = (_Sat signed short _Fract)sat_a1;  /* Additional conversion */
    volatile_sfract = sat_f1;
    
    /* Test 10: Arithmetic that overflows then conversion */
    signed _Accum a5 = 0.9k;
    signed _Accum a6 = 0.9k;
    signed _Accum prod = a5 * a6;  /* 0.81k, safe */
    signed short _Fract f5 = (signed short _Fract)(prod * 1.5k);  /* 1.215k > 1.0, overflow */
    volatile_sfract = f5;
    
    /* Test 11: Loop with small iteration count to create semi-constant values */
    signed _Accum accum = 0.0k;
    for (int i = 0; i < 3; i++) {
        accum += 0.4k;
    }
    /* accum = 1.2k after loop */
    signed short _Fract f6 = (signed short _Fract)accum;  /* 1.2k > 1.0, overflow */
    volatile_sfract = f6;
    
    /* Test 12: Use opaque function results in conversions */
    signed short _Fract f7 = (signed short _Fract)(get_saccum_half() * 3.0k);  /* 1.5k > 1.0 */
    volatile_sfract = f7;
    
    /* Test 13: Long _Accum to _Accum conversions with extreme values */
    signed long _Accum sla1 = 9223372036854775.807sk;  /* Near max */
    signed _Accum a7 = (signed _Accum)sla1;  /* Conversion with potential range check */
    volatile_saccum = a7;
    
    /* Test 14: Chain of conversions with intermediate overflow */
    signed long _Accum sla2 = -9223372036854775.807sk;  /* Near min */
    signed _Accum a8 = (signed _Accum)sla2;
    signed short _Fract f8 = (signed short _Fract)a8;  /* Multiple range checks */
    volatile_sfract = f8;
    
    /* Test 15: Values requiring the specific a_high.sgt(max_r) comparison */
    /* Create values where high word comparison matters */
    const signed long _Accum huge_val = 1000000000000.0sk;  /* Large value */
    signed _Accum a9 = (signed _Accum)huge_val;  /* Will overflow _Accum range */
    volatile_saccum = a9;
    
    /* Test 16: Values requiring a_low.ugt(max_s) comparison */
    /* Create values where high words equal but low word exceeds max_s */
    /* This is tricky but we try with values just above 1.0 in _Accum */
    signed _Accum a10 = (signed _Accum)1.0k + (signed _Accum)0.0000000001k;
    signed short _Fract f9 = (signed short _Fract)a10;  /* High word = 0, low word > max_s */
    volatile_sfract = f9;
    
    /* Compute a simple checksum to ensure all computations are used */
    signed short _Fract checksum = f1 + f2 + f3 + f4 + sat_f1 + f5 + f6 + f7 + f8 + f9;
    volatile_sfract = checksum;
    
    printf("Fixed-point range checking test completed.\n");
    printf("Checksum (as float): %f\n", (double)checksum);
    
    return 0;
}
