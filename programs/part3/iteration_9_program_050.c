/* fixed-value-test.c - Test program for GCC fixed-point range checking logic */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-value-test.c -o fixed-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant propagation */
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
    volatile signed _Accract sink = val;
    return sink;
}

int main(void) {
    /* Test 1: Signed fixed-point conversions with boundary values */
    printf("Test 1: Signed boundary conversions\n");
    
    /* Maximum representable signed short _Fract */
    const signed short _Fract max_sfract = 0.999969482421875r;
    
    /* Values that will trigger range checks during conversion */
    const signed _Accum a1 = (signed _Accum)max_sfract;  /* Within range */
    const signed _Accum a2 = a1 * 2.0k;                  /* May overflow when converted down */
    
    /* Conversion that should trigger a_high.sgt(max_r) check */
    volatile signed short _Fract sf1 = (signed short _Fract)a2;
    
    /* Test 2: Complex constant expressions forcing range evaluation */
    printf("Test 2: Complex constant expressions\n");
    
    /* Expression that computes near MAX for target type */
    const signed _Accum c1 = (signed _Accum)0.5r * (signed _Accum)1.999r;
    const signed short _Fract cf1 = (signed short _Fract)c1;  /* Should be in range */
    
    /* Expression that exceeds range when converted */
    const signed _Accum c2 = (signed _Accum)1.5r * (signed _Accum)1.5r;
    volatile signed short _Fract cf2 = (signed short _Fract)c2;  /* Should trigger overflow check */
    
    /* Test 3: Mixed precision with unsigned types */
    printf("Test 3: Unsigned boundary tests\n");
    
    const unsigned short _Fract max_ufract = 0.999969482421875ur;
    const unsigned _Accum ua1 = (unsigned _Accum)max_ufract;
    
    /* This multiplication will overflow unsigned short _Fract range */
    const unsigned _Accum ua2 = ua1 * 2.0uk;
    volatile unsigned short _Fract uf1 = (unsigned short _Fract)ua2;
    
    /* Test 4: Negative values for signed types (testing min_r/min_s logic) */
    printf("Test 4: Negative boundary tests\n");
    
    const signed _Accum neg_acc = -1.5k;
    /* Conversion to narrower type with negative value */
    volatile signed short _Fract neg_sf = (signed short _Fract)neg_acc;
    
    /* Very negative value that might underflow when converted */
    const signed long _Accum very_neg = -10000.0lk;
    volatile signed _Accum sn1 = (signed _Accum)very_neg;
    
    /* Test 5: Saturation qualifier tests */
    printf("Test 5: Saturation tests\n");
    
    /* Saturated types should use different overflow handling */
    _Sat signed short _Fract sat_sf1 = (signed short _Fract)2.0r;  /* Should saturate to MAX */
    _Sat unsigned short _Fract sat_uf1 = (unsigned short _Fract)(-0.5ur); /* Should saturate to 0 */
    
    /* Mix saturated and non-saturated in expressions */
    _Sat signed _Accum sat_acc = (signed _Accum)1.5k;
    signed short _Fract reg_sf = (signed short _Fract)sat_acc;  /* Conversion may trigger checks */
    
    /* Test 6: Loop-based computations to prevent optimization */
    printf("Test 6: Loop-based boundary testing\n");
    
    signed _Accum loop_acc = 0.0k;
    volatile signed short _Fract loop_results[4];
    
    for (int i = 0; i < 4; i++) {
        /* Build up to overflow gradually */
        loop_acc = loop_acc + 0.8k;
        /* Conversion in loop - compiler may still constant fold but control flow exists */
        loop_results[i] = (signed short _Fract)loop_acc;
    }
    
    /* Test 7: Exact boundary value testing */
    printf("Test 7: Exact boundary value tests\n");
    
    /* Values at exact boundaries */
    const signed _Accum exact_max_saccum = 9223372036854775.807k;  /* ~MAX for signed _Accum */
    const signed _Accum just_above_max = exact_max_saccum + 0.001k;
    
    /* These conversions should trigger the specific comparisons in the uncovered code:
       a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) */
    volatile signed short _Fract b1 = (signed short _Fract)exact_max_saccum;
    volatile signed short _Fract b2 = (signed short _Fract)just_above_max;
    
    /* Test 8: Bit-pattern manipulation to create specific double_int values */
    printf("Test 8: Bit-pattern tests\n");
    
    /* Use unions to create specific bit patterns that might trigger edge cases */
    union {
        signed _Accum acc;
        unsigned long long bits;
    } u;
    
    /* Create a value where high part equals max_r but low part > max_s */
    u.bits = 0x7FFFFFFFFFFFFFFFULL;  /* Pattern that might trigger a_low.ugt(max_s) */
    volatile signed short _Fract pattern_test = (signed short _Fract)u.acc;
    
    /* Final checksum to ensure all computations are used */
    volatile int checksum = 0;
    checksum += (int)(sf1 * 1000r);
    checksum += (int)(cf1 * 1000r);
    checksum += (int)(cf2 * 1000r);
    checksum += (int)(uf1 * 1000ur);
    checksum += (int)(neg_sf * 1000r);
    checksum += (int)(sat_sf1 * 1000r);
    checksum += (int)(reg_sf * 1000r);
    
    for (int i = 0; i < 4; i++) {
        checksum += (int)(loop_results[i] * 1000r);
    }
    
    checksum += (int)(b1 * 1000r);
    checksum += (int)(b2 * 1000r);
    checksum += (int)(pattern_test * 1000r);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed (some may produce expected overflow warnings)\n");
    
    return 0;
}
