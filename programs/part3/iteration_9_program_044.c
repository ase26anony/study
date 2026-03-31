/* fixed-point-test.c - Test program for GCC fixed-point range checking */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

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
    /* volatile to force materialization */
    volatile signed _Accum dummy = val;
    return dummy;
}

int main(void) {
    /* Test 1: Signed fixed-point conversions with boundary values */
    printf("Test 1: Signed boundary conversions\n");
    
    /* Maximum values for different signed types */
    const signed short _Fract max_sfract = 0.999969482421875r;      /* Q0.15 max */
    const signed _Fract max_fract = 0.99999999976716935634613037109375r; /* Q0.31 max */
    const signed short _Accum max_saccum = 32767.999969482421875k;  /* Q15.16 max */
    
    /* Try to convert larger types to smaller types - should trigger range checks */
    volatile signed short _Fract test1;
    
    /* Case 1: Value at exact maximum */
    test1 = max_sfract;  /* Should pass range check */
    
    /* Case 2: Value just beyond maximum (by adding smallest increment) */
    /* For Q0.15, smallest increment is 2^-15 ≈ 0.000030517578125 */
    const signed _Fract just_above_max = max_fract + 0.00000000023283064365386962890625r;
    test1 = (signed short _Fract)just_above_max;  /* Should trigger overflow check */
    
    /* Case 3: Negative value conversion */
    const signed _Accum neg_accum = -1.5k;
    test1 = (signed short _Fract)neg_accum;  /* Should pass (within range) */
    
    /* Case 4: Very negative value */
    const signed long _Accum very_neg = -9223372036854775.0lk;
    volatile signed _Fract test2 = (signed _Fract)very_neg;  /* Should trigger min range check */
    
    /* Test 2: Unsigned fixed-point conversions */
    printf("Test 2: Unsigned boundary conversions\n");
    
    const unsigned short _Fract max_ufract = 0.999969482421875ur;   /* U0.16 max */
    const unsigned _Accum max_uaccum = 65535.9999847412109375uk;    /* U16.16 max */
    
    volatile unsigned short _Fract test3;
    
    /* Case 1: At maximum */
    test3 = max_ufract;
    
    /* Case 2: Beyond maximum (try to convert larger unsigned accum) */
    test3 = (unsigned short _Fract)max_uaccum;  /* Should trigger a_high.sgt(max_r) check */
    
    /* Case 3: From signed to unsigned (negative value) */
    const signed _Accum neg_val = -0.5k;
    test3 = (unsigned short _Fract)neg_val;  /* Should trigger underflow check */
    
    /* Test 3: Mixed precision arithmetic with overflow */
    printf("Test 3: Mixed precision arithmetic\n");
    
    /* Multiply two values that overflow when converted to smaller type */
    const signed _Accum a1 = 100.0k;
    const signed _Accum b1 = 100.0k;
    const signed _Accum product1 = a1 * b1;  /* 10000 > max for _Fract */
    
    volatile signed short _Fract test4 = (signed short _Fract)product1;  /* Overflow */
    
    /* Addition near boundary */
    const signed short _Fract near_max = 0.999r;  /* Close to max */
    const signed short _Fract small = 0.001r;
    volatile signed _Fract test5 = near_max + small;  /* May overflow in some conversions */
    
    /* Test 4: Saturation qualifier tests */
    printf("Test 4: Saturation tests\n");
    
    /* Regular (non-sat) to sat conversion */
    const signed _Accum large_val = 500.0k;
    volatile signed _Sat short _Fract test6 = (signed _Sat short _Fract)large_val;  /* Should saturate */
    
    /* Sat to non-sat conversion */
    signed _Sat _Accum sat_val = 1000.0k;
    volatile signed short _Accum test7 = (signed short _Accum)sat_val;  /* Range check needed */
    
    /* Test 5: Complex constant expressions */
    printf("Test 5: Complex constant expressions\n");
    
    /* These should be evaluated at compile-time, triggering range checks */
    constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;
    constexpr signed short _Fract c2 = (signed short _Fract)c1;  /* Conversion check */
    
    constexpr unsigned _Accum c3 = (unsigned _Accum)0.8ur * 2.0ur;
    constexpr unsigned short _Fract c4 = (unsigned short _Fract)c3;
    
    /* Use volatile to force materialization */
    volatile signed short _Fract v1 = c2;
    volatile unsigned short _Fract v2 = c4;
    
    /* Test 6: Loop with small iteration count */
    printf("Test 6: Loop-based tests\n");
    
    signed _Accum accumulator = 0.0k;
    for (int i = 0; i < 5; i++) {
        accumulator += 0.2k;
        /* Convert to smaller type inside loop */
        volatile signed short _Fract loop_test = (signed short _Fract)accumulator;
    }
    
    /* Test 7: Extreme boundary cases */
    printf("Test 7: Extreme boundaries\n");
    
    /* Values designed to exercise a_low.ugt(max_s) comparison */
    /* We need values where high part equals max_r but low part exceeds max_s */
    
    /* For unsigned: max_r.high = 0, max_r.low = 0 */
    /* max_s is all 1's in i_f_bits */
    /* We need a_high == 0 && a_low > max_s */
    
    /* Create a value with high=0, low=2^(i_f_bits) [one beyond max] */
    /* For unsigned short _Fract: i_f_bits = 16, max_s = 0xFFFF */
    /* So we need a_low = 0x10000 */
    
    const unsigned _Accum extreme_unsigned = (unsigned _Accum)1.0000152587890625uk; /* Slightly > 1.0 */
    volatile unsigned short _Fract test8 = (unsigned short _Fract)extreme_unsigned;
    
    /* For signed: similar logic but with sign extension */
    /* min_s has high=0, low=1<<i_f_bits, then sign extended */
    const signed _Accum extreme_signed = (signed _Accum)(-1.000030517578125k); /* Slightly < -1.0 */
    volatile signed short _Fract test9 = (signed short _Fract)extreme_signed;
    
    /* Test 8: Function calls to break constant propagation */
    printf("Test 8: Function-based tests\n");
    
    signed short _Fract f1 = get_sfract_max();
    unsigned short _Fract f2 = get_ufract_max();
    signed long _Accum f3 = get_saccum_min();
    
    /* Convert function results to other types */
    volatile signed _Accum test10 = (signed _Accum)f1;
    volatile unsigned _Fract test11 = (unsigned _Fract)f2;
    volatile signed _Fract test12 = (signed _Fract)f3;
    
    /* Force all values to be used */
    consume_accum(test10);
    
    /* Create a simple checksum to ensure all computations are used */
    uint32_t checksum = 0;
    
    /* Use all volatile test variables in checksum */
    checksum += *(uint32_t*)&test1;
    checksum += *(uint32_t*)&test2;
    checksum += *(uint32_t*)&test3;
    checksum += *(uint32_t*)&test4;
    checksum += *(uint32_t*)&test5;
    checksum += *(uint32_t*)&test6;
    checksum += *(uint32_t*)&test7;
    checksum += *(uint32_t*)&v1;
    checksum += *(uint32_t*)&v2;
    checksum += *(uint32_t*)&test8;
    checksum += *(uint32_t*)&test9;
    checksum += *(uint32_t*)&test10;
    checksum += *(uint32_t*)&test11;
    checksum += *(uint32_t*)&test12;
    
    printf("Final checksum: %u\n", checksum);
    printf("All tests completed (some may produce expected overflow warnings)\n");
    
    return 0;
}
