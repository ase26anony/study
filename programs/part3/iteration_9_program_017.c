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
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) signed _Accum get_saccum_half(void) {
    return 0.5k;
}

/* Volatile stores to force materialization */
volatile signed short _Fract volatile_sfract;
volatile unsigned short _Fract volatile_ufract;
volatile signed _Accum volatile_saccum;

int main(void) {
    /* Test 1: Boundary values for signed fract types */
    const signed short _Fract sfract_max = 0.999969482421875r;  /* Q0.15 max */
    const signed short _Fract sfract_min = -1.0r;               /* Q0.15 min */
    const signed short _Fract sfract_near_max = 0.99993896484375r; /* MAX - 1LSB */
    
    /* Test 2: Complex constant expressions that require range checking */
    const signed _Accum saccum1 = (signed _Accum)0.75k * 2.0k;  /* 1.5k */
    const signed short _Fract sfract_from_accum = (signed short _Fract)saccum1;
    
    /* Test 3: Values that should trigger overflow in conversion */
    const signed _Accum saccum_overflow = (signed _Accum)1.5k;
    volatile_saccum = saccum_overflow;
    signed short _Fract sfract_overflow_test = (signed short _Fract)volatile_saccum;
    
    /* Test 4: Unsigned fract boundary testing */
    const unsigned short _Fract ufract_max = 0.999969482421875ur;
    const unsigned short _Fract ufract_near_max = 0.99993896484375ur;
    
    /* Test 5: Mixed signed/unsigned conversions */
    signed _Accum negative_accum = -0.5k;
    unsigned short _Fract ufract_from_signed = (unsigned short _Fract)negative_accum;
    
    /* Test 6: Saturation qualifier testing */
    _Sat signed short _Fract sat_sfract1 = (signed short _Fract)2.0k;  /* Should saturate */
    _Sat unsigned short _Fract sat_ufract1 = (unsigned short _Fract)(-0.5k);
    
    /* Test 7: Arithmetic that produces values at boundaries */
    signed short _Fract sfract_sum = sfract_near_max + (signed short _Fract)0.000030517578125r; /* MAX */
    signed short _Fract sfract_overflow = sfract_max + (signed short _Fract)0.000030517578125r; /* Overflow */
    
    /* Test 8: Loop with fixed iteration to create semi-constant values */
    signed _Accum accum_sum = 0.0k;
    for (int i = 0; i < 3; i++) {
        accum_sum += (signed _Accum)0.333333333333333333k;
    }
    signed short _Fract sfract_from_loop = (signed short _Fract)accum_sum; /* ~1.0k */
    
    /* Test 9: Multiplication that overflows fract range */
    const signed _Accum saccum_large = (signed _Accum)0.9k;
    const signed _Accum saccum_large2 = (signed _Accum)1.2k;
    signed _Accum product = saccum_large * saccum_large2; /* 1.08k > 1.0 */
    signed short _Fract sfract_product = (signed short _Fract)product;
    
    /* Test 10: Using opaque function results */
    signed short _Fract sfract_opaque = get_sfract_max();
    sfract_opaque = sfract_opaque + (signed short _Fract)0.000030517578125r;
    
    /* Test 11: Minimum value testing for signed types */
    signed short _Fract sfract_min_test = sfract_min - (signed short _Fract)0.000030517578125r;
    
    /* Test 12: Unsigned overflow beyond maximum */
    unsigned short _Fract ufract_overflow = ufract_max + (unsigned short _Fract)0.000030517578125ur;
    
    /* Test 13: Convert from wider to narrower with different fractional bits */
    /* signed long _Accum has more fractional bits than signed _Accum */
    signed long _Accum slaccum = (signed long _Accum)0.999999999999999k;
    signed _Accum saccum_from_long = (signed _Accum)slaccum;
    signed short _Fract sfract_from_long = (signed short _Fract)slaccum;
    
    /* Test 14: Mixed saturation and non-saturation */
    _Sat signed short _Fract sat_result = (signed short _Fract)((signed _Accum)1.5k + (signed _Accum)0.5k);
    signed short _Fract nonsat_result = (signed short _Fract)((signed _Accum)1.5k + (signed _Accum)0.5k);
    
    /* Force all results to be used to prevent optimization */
    volatile_sfract = sfract_from_accum;
    volatile_sfract = sfract_overflow_test;
    volatile_ufract = ufract_from_signed;
    volatile_sfract = sfract_sum;
    volatile_sfract = sfract_overflow;
    volatile_sfract = sfract_from_loop;
    volatile_sfract = sfract_product;
    volatile_sfract = sfract_opaque;
    volatile_sfract = sfract_min_test;
    volatile_ufract = ufract_overflow;
    volatile_saccum = saccum_from_long;
    volatile_sfract = sfract_from_long;
    volatile_sfract = sat_result;
    volatile_sfract = nonsat_result;
    
    /* Create a simple checksum to ensure all computations are used */
    signed _Accum checksum = (signed _Accum)sfract_from_accum +
                            (signed _Accum)sfract_overflow_test +
                            (signed _Accum)ufract_from_signed +
                            (signed _Accum)sfract_sum +
                            (signed _Accum)sfract_overflow +
                            (signed _Accum)sfract_from_loop +
                            (signed _Accum)sfract_product +
                            (signed _Accum)sfract_opaque +
                            (signed _Accum)sfract_min_test +
                            (signed _Accum)ufract_overflow +
                            saccum_from_long +
                            (signed _Accum)sfract_from_long +
                            (signed _Accum)sat_result +
                            (signed _Accum)nonsat_result;
    
    /* Output checksum to prevent dead code elimination */
    printf("Checksum: %lld\n", (long long)(checksum * 1000000k));
    
    return 0;
}
