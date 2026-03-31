/* fixed-point-test.c */
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

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed _Accum */
}

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    /* volatile to force materialization */
    volatile signed _Accum temp = val;
    return temp;
}

int main(void) {
    /* Test 1: Signed fixed-point conversions with boundary values */
    printf("Test 1: Signed boundary conversions\n");
    
    /* Maximum representable signed short _Fract */
    const signed short _Fract max_sfract = 0.999969482421875r;
    
    /* Try to convert to narrower type - should trigger range check */
    volatile signed char _Fract target1;
    target1 = max_sfract;  /* This fits */
    
    /* Create value just beyond max by adding smallest increment */
    const signed short _Fract beyond_max = max_sfract + (signed short _Fract)0.000030517578125r;
    
    /* This conversion should trigger a_high.sgt(max_r) check */
    volatile signed char _Fract target2;
    target2 = beyond_max;  /* May saturate or overflow */
    
    /* Test 2: Complex constant expressions forcing range checking */
    printf("Test 2: Complex constant expressions\n");
    
    const signed _Accum a1 = (signed _Accum)0.5k;
    const signed _Accum a2 = (signed _Accum)3.0k;
    const signed _Accum product = a1 * a2;  /* 1.5k */
    
    /* Convert _Accum to _Fract - different scaling factors */
    const signed short _Fract f1 = (signed short _Fract)product;
    
    /* Create value that overflows when converted */
    const signed _Accum large_accum = (signed _Accum)1000.0k;
    volatile signed short _Fract f2;
    f2 = (signed short _Fract)large_accum;  /* Should trigger range check */
    
    /* Test 3: Mixed signed/unsigned conversions */
    printf("Test 3: Signed/unsigned mixed conversions\n");
    
    const unsigned short _Fract max_ufract = 0.999969482421875ur;
    const signed short _Fract negative_sfract = -0.5r;
    
    /* Convert signed negative to unsigned - should trigger min check */
    volatile unsigned short _Fract u1;
    u1 = (unsigned short _Fract)negative_sfract;
    
    /* Test 4: Saturation qualifier testing */
    printf("Test 4: Saturation qualifier tests\n");
    
    /* Regular (non-saturating) types */
    signed _Accum regular_accum = (signed _Accum)5000.0k;
    signed short _Fract regular_fract;
    regular_fract = (signed short _Fract)regular_accum;  /* Overflow */
    
    /* Saturating types */
    signed _Sat _Accum sat_accum = (signed _Sat _Accum)5000.0k;
    signed _Sat short _Fract sat_fract;
    sat_fract = (signed _Sat short _Fract)sat_accum;  /* Should saturate */
    
    /* Test 5: Values requiring a_low.ugt(max_s) check */
    printf("Test 5: Testing a_low.ugt(max_s) path\n");
    
    /* For i_f_bits = 16, max_s = 0xFFFF after zext */
    /* We need a_high == max_r (0) and a_low > max_s */
    const unsigned long _Fract ulf_max = 0.99999999976716935634613037109375ulr;
    
    /* Convert to narrower unsigned type */
    volatile unsigned short _Fract usf;
    usf = (unsigned short _Fract)ulf_max;  /* Should trigger a_low.ugt(max_s) */
    
    /* Test 6: Minimum value checks (negative overflow) */
    printf("Test 6: Minimum value checks\n");
    
    const signed _Accum min_accum = (signed _Accum)-1000.0k;
    volatile signed short _Fract min_fract;
    min_fract = (signed short _Fract)min_accum;  /* Underflow */
    
    /* Test 7: Loop with constant iteration to force range checking */
    printf("Test 7: Loop-based range checking\n");
    
    signed _Accum accum_sum = (signed _Accum)0.0k;
    for (int i = 0; i < 3; i++) {
        accum_sum = accum_sum + (signed _Accum)0.33333333333333333333k;
    }
    /* accum_sum should be ~1.0k */
    
    volatile signed short _Fract loop_result;
    loop_result = (signed short _Fract)accum_sum;  /* Conversion with range check */
    
    /* Test 8: Using function calls to break constant propagation */
    printf("Test 8: Function-based boundary testing\n");
    
    signed short _Fract dynamic_max = get_sfract_max();
    /* Add epsilon to push beyond max */
    signed short _Fract beyond_dynamic = dynamic_max + (signed short _Fract)0.000030517578125r;
    
    volatile signed char _Fract narrow_result;
    narrow_result = (signed char _Fract)beyond_dynamic;
    
    /* Test 9: Unsigned maximum boundary */
    unsigned short _Fract uf_max = get_ufract_max();
    /* Try to exceed maximum */
    unsigned short _Fract uf_beyond = uf_max + (unsigned short _Fract)0.000030517578125ur;
    
    volatile unsigned char _Fract uf_narrow;
    uf_narrow = (unsigned char _Fract)uf_beyond;
    
    /* Test 10: Minimum signed value */
    signed _Accum saccum_min = get_saccum_min();
    volatile signed short _Fract sf_min;
    sf_min = (signed short _Fract)saccum_min;  /* Should trigger min range check */
    
    /* Force all values to be used to prevent optimization */
    consume_accum(product);
    consume_accum(large_accum);
    consume_accum(regular_accum);
    consume_accum(sat_accum);
    consume_accum(accum_sum);
    consume_accum(saccum_min);
    
    /* Create a simple checksum from all results */
    int checksum = 0;
    checksum += (int)(target1 * 1000r);
    checksum += (int)(target2 * 1000r);
    checksum += (int)(f1 * 1000r);
    checksum += (int)(f2 * 1000r);
    checksum += (int)(u1 * 1000ur);
    checksum += (int)(regular_fract * 1000r);
    checksum += (int)(sat_fract * 1000r);
    checksum += (int)(usf * 1000ur);
    checksum += (int)(min_fract * 1000r);
    checksum += (int)(loop_result * 1000r);
    checksum += (int)(narrow_result * 1000r);
    checksum += (int)(uf_narrow * 1000ur);
    checksum += (int)(sf_min * 1000r);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed (some may produce expected overflows)\n");
    
    return 0;
}
