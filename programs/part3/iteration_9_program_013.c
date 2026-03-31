/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    volatile signed _Accract dummy = val;
    (void)dummy;
    return val;
}

int main(void) {
    /* Test 1: Signed fixed-point boundary tests */
    printf("Test 1: Signed fixed-point conversions\n");
    
    /* Maximum signed short _Fract */
    const signed short _Fract max_sfract = 0.999969482421875r;
    volatile signed short _Fract v1 = max_sfract;
    
    /* Try to overflow by adding smallest increment */
    const signed short _Fract tiny = 0.000030517578125r; /* 1 LSB */
    signed short _Fract overflow_attempt = max_sfract + tiny;
    
    /* Convert to narrower type - should trigger range check */
    signed char _Fract narrow1 = (signed char _Fract)overflow_attempt;
    
    /* Test with _Sat qualifier */
    signed short _Fract _Sat sat1 = max_sfract;
    sat1 = sat1 + tiny;  /* Should saturate */
    
    /* Test 2: Unsigned fixed-point boundary tests */
    printf("Test 2: Unsigned fixed-point conversions\n");
    
    const unsigned short _Fract max_ufract = 0.999969482421875ur;
    volatile unsigned short _Fract v2 = max_ufract;
    
    /* Create value just above max for unsigned */
    unsigned short _Fract over_unsigned = max_ufract + (unsigned short _Fract)0.0001ur;
    
    /* Convert to signed with different scaling - triggers range check */
    signed char _Fract narrow2 = (signed char _Fract)over_unsigned;
    
    /* Test 3: _Accum to _Fract conversions with range checking */
    printf("Test 3: _Accum to _Fract conversions\n");
    
    /* Values that will overflow when converted to _Fract */
    const signed long _Accum large_accum = 100.5k;
    const signed long _Accum small_accum = -100.5k;
    
    /* These conversions should trigger the uncovered range checking code */
    signed short _Fract from_large = (signed short _Fract)large_accum;
    signed short _Fract from_small = (signed short _Fract)small_accum;
    
    /* Test 4: Complex constant expressions */
    printf("Test 4: Complex constant expressions\n");
    
    /* These should be evaluated at compile-time, triggering range checks */
    constexpr signed _Accum ca1 = (signed _Accum)0.5r * 3.0r;
    constexpr signed _Accum ca2 = (signed _Accum)0.999r / 0.5r;
    
    /* Convert to different precision - triggers range check */
    const signed short _Fract cf1 = (signed short _Fract)ca1;
    const signed short _Fract cf2 = (signed short _Fract)ca2;
    
    /* Test 5: Mixed signed/unsigned with saturation */
    printf("Test 5: Mixed signed/unsigned with saturation\n");
    
    unsigned short _Fract _Sat usat = 0.999ur;
    signed short _Fract _Sat ssat = -0.999r;
    
    /* Operations that might overflow */
    usat = usat + (unsigned short _Fract)0.1ur;
    ssat = ssat - (signed short _Fract)0.1r;
    
    /* Cross-type assignment with different ranges */
    signed char _Fract mixed1 = (signed char _Fract)usat;
    unsigned char _Fract mixed2 = (unsigned char _Fract)ssat;
    
    /* Test 6: Loop-based computations to prevent optimization */
    printf("Test 6: Loop-based computations\n");
    
    signed _Accum accum_sum = 0.0k;
    signed short _Fract fract_sum = 0.0r;
    
    /* Small loop with fixed-point arithmetic */
    for (int i = 0; i < 4; i++) {
        accum_sum = accum_sum + (signed _Accum)0.3k;
        
        /* Convert with potential overflow */
        fract_sum = fract_sum + (signed short _Fract)accum_sum;
        
        /* Force materialization */
        consume_accum(accum_sum);
    }
    
    /* Test 7: Boundary value edge cases */
    printf("Test 7: Boundary value edge cases\n");
    
    /* Values at exact boundaries */
    const signed long _Accum boundary1 = 32767.999847412109375k;  /* Near short _Fract max */
    const signed long _Accum boundary2 = -32768.0k;               /* Near short _Fract min */
    
    /* These conversions exercise a_high.sgt(max_r) and a_low.ugt(max_s) */
    signed short _Fract bconv1 = (signed short _Fract)boundary1;
    signed short _Fract bconv2 = (signed short _Fract)boundary2;
    
    /* Test 8: Using function calls to get boundary values */
    printf("Test 8: Function-based boundary tests\n");
    
    signed short _Fract fmax = get_sfract_max();
    unsigned short _Fract ufmax = get_ufract_max();
    signed long _Accum smin = get_saccum_min();
    
    /* Operations that should trigger range checks */
    signed short _Fract fmax_plus = fmax + (signed short _Fract)0.0001r;
    unsigned short _Fract ufmax_plus = ufmax + (unsigned short _Fract)0.0001ur;
    
    /* Convert between types with different ranges */
    signed char _Fract from_fmax = (signed char _Fract)fmax_plus;
    signed char _Fract from_ufmax = (signed char _Fract)ufmax_plus;
    signed short _Fract from_smin = (signed short _Fract)smin;
    
    /* Generate a checksum to prevent dead code elimination */
    volatile int checksum = 0;
    checksum += (int)(v1 * 1000r);
    checksum += (int)(v2 * 1000ur);
    checksum += (int)(narrow1 * 1000r);
    checksum += (int)(narrow2 * 1000r);
    checksum += (int)(from_large * 1000r);
    checksum += (int)(from_small * 1000r);
    checksum += (int)(fract_sum * 1000r);
    checksum += (int)(bconv1 * 1000r);
    checksum += (int)(bconv2 * 1000r);
    checksum += (int)(from_fmax * 1000r);
    checksum += (int)(from_ufmax * 1000r);
    checksum += (int)(from_smin * 1000r);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed (some may produce expected overflows)\n");
    
    return 0;
}
