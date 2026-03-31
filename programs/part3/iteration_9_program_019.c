/* fixed-point-test.c
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 * Also test with: -O2 -std=gnu11 -fno-math-errno -fno-trapping-math
 */

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

__attribute__((noinline)) volatile void use_result(volatile void *ptr) {
    /* Force compiler to materialize value */
    (void)ptr;
}

int main(void) {
    volatile int checksum = 0;
    
    /* ===== Test 1: Signed fixed-point conversions with boundary values ===== */
    
    /* Constants at boundaries */
    const signed short _Fract max_sfract = 0.999969482421875r;
    const signed short _Fract min_sfract = -1.0r;
    const signed long _Accum max_saccum = 9223372036854775.807k;
    const signed long _Accum min_saccum = -9223372036854775.807k;
    
    /* Test conversion from wider to narrower type - should trigger range checks */
    volatile signed short _Fract sf1;
    volatile signed short _Fract sf2;
    
    /* Just below max boundary */
    sf1 = (signed short _Fract)(max_saccum * 0.0000000001k);
    checksum += (int)(sf1 * 10000r);
    
    /* Just above max boundary (through addition) */
    sf2 = max_sfract;
    sf2 = sf2 + (signed short _Fract)0.0001r;  /* Overflow for Q0.15 */
    checksum += (int)(sf2 * 10000r);
    
    /* ===== Test 2: Complex constant expressions forcing range checking ===== */
    
    /* These should be evaluated at compile-time, invoking the target logic */
    const signed _Accum ca1 = (signed _Accum)0.5r * 3.0r;  /* 1.5 */
    const signed _Accum ca2 = (signed _Accum)(-0.75r) * 4.0r;  /* -3.0 */
    
    /* Convert to narrower types - triggers range checking */
    const signed short _Fract cf1 = (signed short _Fract)ca1;
    const signed short _Fract cf2 = (signed short _Fract)ca2;
    
    /* Use in more complex expressions */
    const signed _Accum ca3 = ca1 * ca2;  /* -4.5 */
    const signed short _Fract cf3 = (signed short _Fract)(ca3 * 0.1k);
    
    checksum += (int)(cf1 * 1000r);
    checksum += (int)(cf2 * 1000r);
    checksum += (int)(cf3 * 1000r);
    
    /* ===== Test 3: Unsigned fixed-point with overflow scenarios ===== */
    
    volatile unsigned short _Fract uf1, uf2;
    
    /* Start at max */
    uf1 = get_ufract_max();
    
    /* Try to overflow */
    uf1 = uf1 + (unsigned short _Fract)0.0001ur;
    checksum += (int)(uf1 * 10000ur);
    
    /* Test conversion from signed negative to unsigned (underflow) */
    signed short _Fract neg_sf = -0.5r;
    uf2 = (unsigned short _Fract)neg_sf;  /* Should saturate to 0 for non-sat */
    checksum += (int)(uf2 * 10000ur);
    
    /* ===== Test 4: Saturation qualifier tests ===== */
    
    volatile signed _Sat short _Fract sat_sf1, sat_sf2;
    volatile unsigned _Sat short _Fract sat_uf1;
    
    /* These should saturate rather than wrap */
    sat_sf1 = get_sfract_max();
    sat_sf1 = sat_sf1 + (signed _Sat short _Fract)0.1r;
    
    sat_sf2 = get_saccum_min();
    sat_sf2 = (signed _Sat short _Fract)sat_sf2;  /* Convert with saturation */
    
    sat_uf1 = get_ufract_max();
    sat_uf1 = sat_uf1 + (unsigned _Sat short _Fract)0.1ur;
    
    checksum += (int)(sat_sf1 * 10000r);
    checksum += (int)(sat_sf2 * 10000r);
    checksum += (int)(sat_uf1 * 10000ur);
    
    /* ===== Test 5: Mixed precision arithmetic ===== */
    
    /* Force range checking through mixed operations */
    volatile signed long _Accum sla1 = 5000000000.0k;  /* Large value */
    volatile signed short _Accum ssa1 = 0.5k;
    
    /* Multiplication that overflows when converted to _Fract */
    volatile signed _Accum ma1 = sla1 * ssa1;
    volatile signed short _Fract sf3 = (signed short _Fract)ma1;
    
    checksum += (int)(sf3 * 10000r);
    
    /* ===== Test 6: Loop with small iteration count ===== */
    
    volatile signed short _Fract accum = 0.0r;
    for (int i = 0; i < 3; i++) {
        /* Build up to overflow */
        accum = accum + (signed short _Fract)0.4r;
        
        /* Convert intermediate result to different type */
        volatile unsigned short _Fract temp = (unsigned short _Fract)accum;
        checksum += (int)(temp * 1000ur);
    }
    
    /* Final conversion that may overflow */
    volatile signed char _Fract scf = (signed char _Fract)accum;
    checksum += (int)(scf * 1000r);
    
    /* ===== Test 7: Direct boundary value testing ===== */
    
    /* Values designed to exercise a_high.sgt(max_r) and a_low.ugt(max_s) */
    
    /* For signed short _Fract (i_f_bits = 15), max_s should be 0x7FFF after zext */
    /* Create value where high part equals max_r (0) but low part exceeds max_s */
    
    volatile signed long _Accum boundary_test = 0.00006103515625k;  /* 1 LSB in Q0.15 */
    boundary_test = boundary_test * 32768.0k;  /* Exactly 2.0, exceeds Q0.15 range */
    
    volatile signed short _Fract bf1 = (signed short _Fract)boundary_test;
    checksum += (int)(bf1 * 10000r);
    
    /* Test negative boundary */
    boundary_test = -1.00006103515625k;  /* Just below -1.0 */
    volatile signed short _Fract bf2 = (signed short _Fract)boundary_test;
    checksum += (int)(bf2 * 10000r);
    
    /* ===== Final output ===== */
    
    printf("Checksum: %d\n", checksum);
    printf("All fixed-point tests completed.\n");
    
    /* Use volatile stores to prevent optimization */
    use_result(&sf1);
    use_result(&uf1);
    use_result(&sat_sf1);
    
    return 0;
}
