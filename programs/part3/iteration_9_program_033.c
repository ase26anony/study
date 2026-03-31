/* fixed-point-test.c
 * Test program to exercise GCC's fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807S / 2;  /* Near min for signed long _Accum */
}

__attribute__((noinline)) volatile signed _Fract consume_fract(signed _Fract x) {
    volatile signed _Fract dummy = x;
    (void)dummy;
    return x;
}

int main(void) {
    /* Test 1: Signed fixed-point conversions with boundary values */
    printf("Test 1: Signed boundary conversions\n");
    
    /* Maximum values for different signed types */
    const signed short _Fract max_sfract = 0.999969482421875r;      /* Q0.15: 32767/32768 */
    const signed _Fract max_fract = 0.99999988079071044921875r;     /* Q0.31 */
    const signed short _Accum max_saccum = 32767.999969482421875k;  /* Q15.16 */
    
    /* Create values at, above, and below boundaries */
    volatile signed short _Fract v1 = max_sfract;
    volatile signed _Fract v2 = max_fract;
    
    /* Conversion that should trigger range check: Q15.16 -> Q0.15 */
    /* This multiplies by 2^15, potentially overflowing */
    signed short _Fract conv1 = (signed short _Fract)max_saccum;
    consume_fract(conv1);
    
    /* Test 2: Complex constant expressions forcing range evaluation */
    printf("Test 2: Complex constant expressions\n");
    
    constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;  /* 1.5 */
    constexpr signed _Accum c2 = (signed _Accum)0.99999988079071044921875r * 2.0r; /* ~2.0 */
    
    /* These conversions require range checking */
    const signed short _Fract from_c1 = (signed short _Fract)c1;
    const signed short _Fract from_c2 = (signed short _Fract)c2;  /* Should overflow */
    
    volatile signed short _Fract v3 = from_c1;
    volatile signed short _Fract v4 = from_c2;
    
    /* Test 3: Mixed precision arithmetic with overflow */
    printf("Test 3: Mixed precision arithmetic\n");
    
    /* Start with value near maximum */
    signed short _Fract f1 = get_sfract_max();
    
    /* Add small value that should overflow Q0.15 */
    signed short _Fract f2 = f1 + (signed short _Fract)0.0001r;
    
    /* Multiply to create overflow */
    signed short _Fract f3 = f1 * (signed short _Fract)1.1r;
    
    /* Convert through intermediate type */
    signed _Accum a1 = (signed _Accum)f1 * (signed _Accum)1.5r;
    signed short _Fract f4 = (signed short _Fract)a1;  /* Should overflow */
    
    consume_fract(f2);
    consume_fract(f3);
    consume_fract(f4);
    
    /* Test 4: Unsigned fixed-point with overflow/underflow */
    printf("Test 4: Unsigned boundary tests\n");
    
    unsigned short _Fract uf_max = get_ufract_max();
    unsigned short _Fract uf1 = uf_max;
    
    /* Operations that should overflow unsigned Q0.16 */
    unsigned short _Fract uf2 = uf1 + (unsigned short _Fract)0.0001ur;
    unsigned short _Fract uf3 = uf1 * (unsigned short _Fract)1.1ur;
    
    /* Convert from signed negative to unsigned (underflow) */
    signed short _Fract sf_neg = -0.5r;
    unsigned short _Fract uf4 = (unsigned short _Fract)sf_neg;  /* Should underflow */
    
    volatile unsigned short _Fract vu1 = uf2;
    volatile unsigned short _Fract vu2 = uf3;
    volatile unsigned short _Fract vu3 = uf4;
    
    /* Test 5: Saturation qualifier testing */
    printf("Test 5: Saturation tests\n");
    
    /* Regular (non-saturating) overflow */
    signed short _Fract reg_overflow = max_sfract + max_sfract;
    
    /* Saturating arithmetic */
    _Sat signed short _Fract sat1 = max_sfract;
    _Sat signed short _Fract sat2 = sat1 + sat1;  /* Should saturate at max */
    
    /* Mixed sat/non-sat conversions */
    _Sat signed _Accum sat_accum = (_Sat signed _Accum)1000.0r;
    signed short _Fract from_sat = (signed short _Fract)sat_accum;  /* Should overflow */
    
    consume_fract(reg_overflow);
    consume_fract(sat2);
    consume_fract(from_sat);
    
    /* Test 6: Loop-based computations with known bounds */
    printf("Test 6: Loop-based computations\n");
    
    signed short _Fract accumulator = 0.0r;
    const signed short _Fract increment = 0.1r;
    
    for (int i = 0; i < 20; i++) {
        accumulator += increment;
        if (accumulator > max_sfract) {
            /* Force conversion when near overflow */
            signed _Fract temp = (signed _Fract)accumulator * 1.5r;
            accumulator = (signed short _Fract)temp;  /* Range check here */
        }
    }
    
    volatile signed short _Fract v_final = accumulator;
    
    /* Test 7: Exact boundary value testing */
    printf("Test 7: Exact boundary values\n");
    
    /* Values designed to exercise a_high.sgt(max_r) and a_low.ugt(max_s) */
    
    /* For Q0.15: max_r = 0, max_s = (1 << 15) - 1 = 32767 */
    /* Test a_high > max_r (0) */
    signed long _Accum large_val = 1.0L;  /* High part > 0 when scaled */
    signed short _Fract test1 = (signed short _Fract)large_val;
    
    /* Test a_high == max_r (0) but a_low > max_s (32767) */
    /* Create value where high=0, low=32768 (1 << 15) */
    signed _Accum exact_boundary = (signed _Accum)1.0r;  /* When converted to Q0.15: 1 << 15 */
    signed short _Fract test2 = (signed short _Fract)exact_boundary;
    
    /* Test negative overflow (min values) */
    signed long _Accum very_negative = get_saccum_min();
    signed short _Fract test3 = (signed short _Fract)very_negative;
    
    consume_fract(test1);
    consume_fract(test2);
    consume_fract(test3);
    
    /* Generate a checksum to ensure all computations are used */
    unsigned int checksum = 0;
    checksum += *(unsigned int*)&v_final;
    checksum += *(unsigned int*)&vu1;
    checksum += *(unsigned int*)&conv1;
    
    printf("Final checksum: %u\n", checksum);
    printf("All tests completed (some may produce expected overflows)\n");
    
    return 0;
}
