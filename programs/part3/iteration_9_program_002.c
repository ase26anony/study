/* Test program for fixed-point range checking logic in GCC */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed_test.c -o fixed_test */

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
    return -9223372036854775.807S; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) volatile signed _Fract consume_sfract(signed _Fract x) {
    volatile signed _Fract dummy = x;
    (void)dummy;
    return x;
}

__attribute__((noinline)) volatile unsigned _Accum consume_uaccum(unsigned _Accum x) {
    volatile unsigned _Accum dummy = x;
    (void)dummy;
    return x;
}

int main(void) {
    /* Test 1: Boundary values for signed types */
    printf("Test 1: Signed boundary conversions\n");
    
    /* Constants at exact boundaries */
    const signed short _Fract sf_max = 0.999969482421875r;     /* Q0.15 max */
    const signed short _Fract sf_min = -1.0r;                  /* Q0.15 min */
    const signed short _Fract sf_near_max = 0.99993896484375r; /* One LSB less */
    
    /* These conversions should trigger range checks */
    volatile signed _Fract v1 = sf_max;           /* Exact max */
    volatile signed _Fract v2 = sf_near_max;      /* Just below max */
    
    /* Test 2: Overflow in signed _Accum to _Fract conversion */
    printf("Test 2: _Accum to _Fract overflow\n");
    
    const signed long _Accum sla_large = 100.0k;  /* Too large for _Fract */
    const signed long _Accum sla_small = 0.5k;    /* Within range */
    
    /* These will invoke range checking with i_f_bits differences */
    volatile signed _Fract f1 = (signed _Fract)sla_large;  /* Should overflow */
    volatile signed _Fract f2 = (signed _Fract)sla_small;  /* Should fit */
    
    /* Test 3: Complex constant expressions forcing range checks */
    printf("Test 3: Complex constant expressions\n");
    
    constexpr signed _Accum ca1 = (signed _Accum)0.5r * 3.0r;      /* 1.5 */
    constexpr signed _Accum ca2 = (signed _Accum)(-0.75r) * 2.0r;  /* -1.5 */
    
    /* Convert to narrower types - triggers range checking */
    const signed short _Fract cf1 = (signed short _Fract)ca1;  /* 1.5 -> clamped? */
    const signed short _Fract cf2 = (signed short _Fract)ca2;  /* -1.5 -> clamped? */
    
    volatile signed short _Fract vcf1 = cf1;
    volatile signed short _Fract vcf2 = cf2;
    
    /* Test 4: Unsigned types with boundary checks */
    printf("Test 4: Unsigned boundary checks\n");
    
    const unsigned short _Fract uf_max = 0.999969482421875ur;    /* U0.16 max */
    const unsigned short _Fract uf_near_max = 0.99993896484375ur; /* One LSB less */
    
    /* These test a_low.ugt(max_s) for unsigned case */
    volatile unsigned _Fract uf1 = uf_max;
    volatile unsigned _Fract uf2 = uf_near_max;
    
    /* Test 5: Mixed signed/unsigned conversions */
    printf("Test 5: Mixed signed/unsigned conversions\n");
    
    const signed _Accum sa_pos = 0.8k;
    const signed _Accum sa_neg = -0.8k;
    
    /* Convert signed to unsigned - tests negative value handling */
    volatile unsigned _Fract uf3 = (unsigned _Fract)sa_pos;  /* Should work */
    volatile unsigned _Fract uf4 = (unsigned _Fract)sa_neg;  /* Underflow to 0 */
    
    /* Test 6: Saturation qualifier tests */
    printf("Test 6: Saturation tests\n");
    
    /* Saturated types have different overflow behavior */
    _Sat signed short _Fract ssf1 = sf_max;
    _Sat signed short _Fract ssf2 = (signed short _Fract)(sf_max + (signed short _Fract)0.0001r);
    
    /* Convert saturated to non-saturated */
    volatile signed short _Fract vssf1 = ssf1;
    volatile signed short _Fract vssf2 = ssf2;
    
    /* Test 7: Multiplication causing overflow */
    printf("Test 7: Multiplication overflow\n");
    
    const signed _Fract m1 = 0.999r;
    const signed _Fract m2 = 0.999r;
    
    /* Multiplication in wider type, then convert down */
    const signed long _Accum prod = (signed long _Accum)m1 * (signed long _Accum)m2;
    volatile signed _Fract vprod = (signed _Fract)prod;  /* ~0.998 - should fit */
    
    /* Test 8: Loop with constant iterations */
    printf("Test 8: Loop-based computations\n");
    
    signed _Accum accumulator = 0.0k;
    for (int i = 0; i < 4; i++) {
        accumulator += 0.3k;
    }
    /* accumulator = 1.2k, converting to _Fract triggers range check */
    volatile signed _Fract vacc = (signed _Fract)accumulator;
    
    /* Test 9: Use opaque functions to break constant propagation */
    printf("Test 9: Opaque function results\n");
    
    signed short _Fract opaque_max = get_sfract_max();
    unsigned short _Fract opaque_umax = get_ufract_max();
    signed long _Accum opaque_min = get_saccum_min();
    
    /* Convert opaque results - compiler can't fully constant fold */
    volatile signed _Fract vop1 = (signed _Fract)opaque_max;
    volatile unsigned _Fract vop2 = (unsigned _Fract)opaque_umax;
    volatile signed _Fract vop3 = (signed _Fract)opaque_min;  /* Large negative */
    
    /* Test 10: Bit-exact boundary values */
    printf("Test 10: Bit-exact boundary testing\n");
    
    /* These values are crafted to exercise the specific comparisons:
       a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) */
    
    /* For conversion to signed short _Fract (Q0.15):
       max_r = 0, max_s = (1 << (i_f_bits)) - 1
       We need values where a_high > 0 OR (a_high == 0 && a_low > max_s) */
    
    /* Case 1: a_high > 0 (definitely exceeds max_r) */
    const signed long _Accum high_pos = 1.0k;  /* a_high > 0 */
    volatile signed short _Fract vhigh = (signed short _Fract)high_pos;
    
    /* Case 2: a_high == 0 but a_low > max_s */
    /* Create value where high part is 0 but low part exceeds max_s */
    /* This requires careful construction based on the scaling */
    const signed _Accum exact_max = (signed _Accum)sf_max;  /* Convert max to _Accum */
    /* Add one LSB in the _Accum representation */
    const signed _Accum just_above_max = exact_max + (signed _Accum)(0.0000001k);
    volatile signed short _Fract vabove = (signed short _Fract)just_above_max;
    
    /* Consume all results to prevent optimization */
    consume_sfract(v1);
    consume_sfract(v2);
    consume_sfract(f1);
    consume_sfract(f2);
    consume_uaccum(uf1);
    consume_uaccum(uf2);
    consume_uaccum(uf3);
    consume_uaccum(uf4);
    
    printf("All tests completed (some may produce expected overflows)\n");
    
    /* Compute a simple checksum to ensure all computations happened */
    uint32_t checksum = 0;
    checksum += *(uint32_t*)&v1;
    checksum += *(uint32_t*)&v2;
    checksum += *(uint32_t*)&f1;
    checksum += *(uint32_t*)&f2;
    
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
