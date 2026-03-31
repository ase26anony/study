/* fixed-point-test.c
 * Tests GCC's fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807S; /* Approx min for signed long _Accum */
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
    /* Test 1: Boundary values for signed types */
    printf("Test 1: Signed boundary conversions\n");
    
    /* Maximum signed short _Fract */
    const signed short _Fract max_sfract = 0.999969482421875r;
    const signed short _Fract min_sfract = -1.0r;
    
    /* Values just beyond boundaries */
    const signed short _Fract just_above_max = (signed short _Fract)1.0001r;
    const signed short _Fract just_below_min = (signed short _Fract)-1.0001r;
    
    /* Convert to different precision - triggers range checks */
    volatile signed char _Fract narrow1 = max_sfract;  /* Q0.7 */
    volatile signed char _Fract narrow2 = min_sfract;  /* Q0.7 */
    
    /* Test 2: Complex constant expressions */
    printf("Test 2: Complex constant expressions\n");
    
    /* These should trigger range checking during constant folding */
    const signed _Accum c1 = (signed _Accum)0.999999999999999999r * 2.0r;
    const signed _Accum c2 = (signed _Accum)-0.999999999999999999r * 2.0r;
    const signed _Accum c3 = (signed _Accum)0.5r * (signed _Accum)1.999999999999999999r;
    
    /* Convert to narrower types */
    volatile signed short _Fract f1 = c1;  /* Should overflow */
    volatile signed short _Fract f2 = c2;  /* Should underflow */
    volatile signed short _Fract f3 = c3;  /* Should be in range */
    
    /* Test 3: Mixed precision with arithmetic */
    printf("Test 3: Mixed precision arithmetic\n");
    
    signed long _Accum a1 = 0.999999999999999999S;
    signed long _Accum a2 = 0.000000000000000001S;
    
    /* Addition that overflows when converted to _Fract */
    signed long _Accum sum = a1 + a2;  /* Exactly 1.0, but _Fract max < 1.0 */
    volatile signed short _Fract sum_as_fract = sum;
    
    /* Multiplication that overflows */
    signed long _Accum prod = a1 * (signed long _Accum)1.5r;
    volatile signed short _Fract prod_as_fract = prod;
    
    /* Test 4: Unsigned types with boundary checks */
    printf("Test 4: Unsigned boundary checks\n");
    
    const unsigned short _Fract max_ufract = 0.999969482421875ur;
    const unsigned short _Fract zero = 0.0ur;
    
    /* Values that should trigger unsigned range checks */
    const unsigned short _Fract just_above_max_u = (unsigned short _Fract)1.0001ur;
    
    /* Convert to narrower unsigned type */
    volatile unsigned char _Fract narrow_u1 = max_ufract;
    volatile unsigned char _Fract narrow_u2 = zero;
    
    /* Test 5: Saturation qualifier */
    printf("Test 5: Saturation behavior\n");
    
    /* Saturated types should still trigger range checking internally */
    _Sat signed short _Fract sat_max = get_sfract_max();
    _Sat signed short _Fract sat_result;
    
    /* This addition would overflow without saturation */
    sat_result = sat_max + (signed short _Fract)0.0001r;
    
    /* Convert saturated result to non-saturated type */
    volatile signed char _Fract from_sat = sat_result;
    
    /* Test 6: Loop with constant iteration (prevents elimination) */
    printf("Test 6: Loop-based computations\n");
    
    signed long _Accum accumulator = 0.0S;
    const signed long _Accum increment = 0.333333333333333333S;
    
    for (int i = 0; i < 3; i++) {
        accumulator += increment;
    }
    
    /* accumulator should be ~1.0, which overflows when converted to _Fract */
    volatile signed short _Fract loop_result = accumulator;
    
    /* Test 7: Negative to unsigned conversion */
    printf("Test 7: Signed to unsigned conversion\n");
    
    signed short _Fract negative = -0.5r;
    /* This conversion should trigger range checking (negative to unsigned) */
    volatile unsigned short _Fract from_negative = (unsigned short _Fract)negative;
    
    /* Test 8: Exact boundary case testing */
    printf("Test 8: Exact boundary cases\n");
    
    /* These test the exact comparisons a_high == max_r && a_low.ugt(max_s) */
    const signed long _Accum exact_max_fract = (signed long _Accum)0.999969482421875r;
    const signed long _Accum just_above_exact = exact_max_fract + (signed long _Accum)0.000000000000000001S;
    
    volatile signed short _Fract exact_boundary = exact_max_fract;      /* Should pass */
    volatile signed short _Fract above_boundary = just_above_exact;    /* Should fail */
    
    /* Consume all results to prevent optimization */
    consume_fract(narrow1);
    consume_fract(narrow2);
    consume_fract(f1);
    consume_fract(f2);
    consume_fract(f3);
    consume_fract(sum_as_fract);
    consume_fract(prod_as_fract);
    consume_fract(narrow_u1);
    consume_fract(narrow_u2);
    consume_fract(from_sat);
    consume_fract(loop_result);
    consume_fract(from_negative);
    consume_fract(exact_boundary);
    consume_fract(above_boundary);
    
    consume_accum(a1);
    consume_accum(a2);
    consume_accum(sum);
    consume_accum(prod);
    consume_accum(accumulator);
    consume_accum(exact_max_fract);
    consume_accum(just_above_exact);
    
    printf("All tests completed (compiler may issue overflow warnings)\n");
    
    /* Simple checksum to ensure all computations happened */
    volatile int checksum = 0;
    checksum += (int)(narrow1 * 1000r);
    checksum += (int)(narrow2 * 1000r);
    checksum += (int)(sum_as_fract * 1000r);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
