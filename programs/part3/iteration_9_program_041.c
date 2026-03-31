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
    return -9223372036854775.807k; /* Approx min for 64-bit accum */
}

__attribute__((noinline)) void consume_fract(signed short _Fract f) {
    volatile signed short _Fract sink = f;
    (void)sink;
}

__attribute__((noinline)) void consume_accum(signed long _Accum a) {
    volatile signed long _Accum sink = a;
    (void)sink;
}

int main(void) {
    /* Test 1: Signed fixed-point conversions with boundary values */
    printf("Test 1: Signed boundary conversions\n");
    
    /* Constants at or near maximum representable values */
    const signed short _Fract max_sfract = 0.999969482421875r;  /* Q0.15 max */
    const signed short _Fract near_max_sfract = 0.99993896484375r; /* One LSB less */
    
    /* These will trigger range checking when converting to narrower types */
    const signed long _Accum large_accum = (signed long _Accum)max_sfract;
    const signed long _Accum slightly_larger = large_accum + (signed long _Accum)0.0001k;
    
    /* Conversion that should just fit */
    signed short _Fract f1 = (signed short _Fract)large_accum;
    consume_fract(f1);
    
    /* Conversion that may overflow (depending on rounding) */
    signed short _Fract f2 = (signed short _Fract)slightly_larger;
    consume_fract(f2);
    
    /* Test 2: Complex constant expressions forcing range checks */
    printf("Test 2: Complex constant expressions\n");
    
    /* Expression that computes a value potentially beyond target range */
    constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;  /* 1.5 in Q */
    constexpr signed short _Fract c2 = (signed short _Fract)c1; /* Conversion check */
    
    /* Multi-step computation with intermediate overflow risk */
    const signed _Accum a = (signed _Accum)0.9r;
    const signed _Accum b = (signed _Accum)0.95r;
    const signed _Accum product = a * b;  /* 0.855 */
    const signed short _Fract narrow_product = (signed short _Fract)product;
    consume_fract(narrow_product);
    
    /* Test 3: Unsigned fixed-point with overflow beyond maximum */
    printf("Test 3: Unsigned overflow scenarios\n");
    
    unsigned short _Fract uf_max = 0.999969482421875ur;
    unsigned short _Fract uf_near_max = 0.99993896484375ur;
    
    /* Operations that push beyond unsigned maximum */
    volatile unsigned short _Fract uf1 = uf_max;
    volatile unsigned short _Fract uf2 = uf_near_max;
    
    /* These additions may overflow when converted */
    unsigned short _Fract uf_sum = uf1 + uf2;  /* Likely > 1.0 */
    unsigned short _Fract uf3 = (unsigned short _Fract)(uf1 * 2.0ur);
    
    /* Test 4: Mixed signed/unsigned conversions */
    printf("Test 4: Mixed signed/unsigned\n");
    
    signed short _Fract sf_neg = -0.5r;
    /* Conversion from signed negative to unsigned - should trigger range check */
    unsigned short _Fract uf_from_signed = (unsigned short _Fract)sf_neg;
    
    /* Test 5: Saturation qualifier tests */
    printf("Test 5: Saturation tests\n");
    
    /* Saturated types have different overflow behavior */
    signed short _Fract _Sat sf_sat_max = 0.999969482421875r;
    signed short _Fract _Sat sf_sat_result;
    
    /* Operation that would overflow non-saturated type */
    sf_sat_result = sf_sat_max + (signed short _Fract)0.0001r;
    
    /* Convert saturated result to non-saturated */
    signed short _Fract regular_from_sat = (signed short _Fract)sf_sat_result;
    consume_fract(regular_from_sat);
    
    /* Test 6: Loop with compile-time known iterations */
    printf("Test 6: Loop-based computations\n");
    
    signed _Accum accumulator = (signed _Accum)0.0k;
    const signed _Accum increment = (signed _Accum)0.333333333333333k;
    
    /* Small loop that builds up a value */
    for (int i = 0; i < 4; i++) {
        accumulator += increment;
    }
    
    /* Convert accumulated value to narrower type */
    signed short _Fract loop_result = (signed short _Fract)accumulator;
    consume_fract(loop_result);
    
    /* Test 7: Minimum value testing for signed types */
    printf("Test 7: Negative boundary tests\n");
    
    const signed short _Fract min_sfract = -1.0r;  /* Q0.15 min */
    const signed short _Fract near_min_sfract = -0.99993896484375r; /* One LSB above min */
    
    signed long _Accum large_neg_accum = (signed long _Accum)min_sfract;
    signed long _Accum slightly_more_neg = large_neg_accum - (signed long _Accum)0.0001k;
    
    /* These conversions test the lower bound checking */
    signed short _Fract f3 = (signed short _Fract)large_neg_accum;
    signed short _Fract f4 = (signed short _Fract)slightly_more_neg;
    consume_fract(f3);
    consume_fract(f4);
    
    /* Test 8: Multiplication overflow scenarios */
    printf("Test 8: Multiplication overflow\n");
    
    /* Multiply two values that individually fit but product may not */
    signed _Accum m1 = (signed _Accum)0.9k;
    signed _Accum m2 = (signed _Accum)1.1k;
    signed _Accum product2 = m1 * m2;  /* 0.99 */
    
    /* Convert product to narrower type - should fit */
    signed short _Fract narrow2 = (signed short _Fract)product2;
    
    /* Now try values whose product exceeds 1.0 */
    signed _Accum m3 = (signed _Accum)1.2k;
    signed _Accum m4 = (signed _Accum)1.3k;
    signed _Accum product3 = m3 * m4;  /* 1.56 > 1.0 */
    
    /* This conversion should trigger overflow detection */
    signed short _Fract narrow3 = (signed short _Fract)product3;
    consume_fract(narrow2);
    consume_fract(narrow3);
    
    /* Final checksum to ensure all values are used */
    volatile int checksum = 0;
    checksum += (int)(f1 * 1000r);
    checksum += (int)(uf_sum * 1000ur);
    checksum += (int)(loop_result * 1000r);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed (check compiler coverage output)\n");
    
    return 0;
}
