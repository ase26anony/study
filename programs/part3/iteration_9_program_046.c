/* fixed-point-test.c - Test program for GCC fixed-point range checking */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

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
    return -9223372036854775808lk; /* MIN for signed long _Accum (Q31.32) */
}

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    volatile signed _Accum sink = val;
    return sink;
}

int main(void) {
    /* Test 1: Boundary values for signed types */
    const signed short _Fract sf_max = 0.999969482421875r;     /* Q0.15 MAX */
    const signed short _Fract sf_min = -1.0r;                  /* Q0.15 MIN */
    const signed short _Fract sf_near_max = 0.99993896484375r; /* MAX - 1 LSB */
    
    /* Test 2: Conversion with overflow - signed _Accum to signed _Fract */
    const signed _Accum sa_large = 1.9990234375k;  /* > MAX for short _Fract */
    const signed _Accum sa_small = -1.9990234375k; /* < MIN for short _Fract */
    
    /* Force range checking during conversion */
    volatile signed short _Fract conv1 = (signed short _Fract)sa_large;
    volatile signed short _Fract conv2 = (signed short _Fract)sa_small;
    
    /* Test 3: Arithmetic near boundaries */
    signed short _Fract sf1 = sf_near_max;
    sf1 = sf1 + (signed short _Fract)0.000030517578125r;  /* Add 1 LSB to near MAX */
    
    signed short _Fract sf2 = sf_min;
    sf2 = sf2 - (signed short _Fract)0.000030517578125r;  /* Subtract 1 LSB from MIN */
    
    /* Test 4: Complex constant expressions requiring range checking */
    constexpr signed _Accum ca1 = (signed _Accum)0.5r * 3.9990234375k;
    constexpr signed _Accum ca2 = (signed _Accum)(-0.75r) * 2.998046875k;
    
    /* These conversions should trigger range checks */
    volatile signed short _Fract cf1 = (signed short _Fract)ca1;
    volatile signed short _Fract cf2 = (signed short _Fract)ca2;
    
    /* Test 5: Unsigned types with overflow */
    const unsigned short _Fract uf_max = 0.999969482421875ur; /* U0.16 MAX */
    const unsigned short _Fract uf_near_max = 0.99993896484375ur;
    
    unsigned short _Fract uf1 = uf_near_max;
    uf1 = uf1 + (unsigned short _Fract)0.00006103515625ur;  /* Add 2 LSBs */
    
    /* Test 6: Mixed signed/unsigned conversions */
    signed _Accum sa_mid = 0.5k;
    volatile unsigned short _Fract uf_conv = (unsigned short _Fract)sa_mid;
    
    /* Test 7: Saturation qualifier tests */
    signed short _Fract _Sat sf_sat1 = sf_max;
    sf_sat1 = sf_sat1 + (signed short _Fract)0.000030517578125r;  /* Should saturate */
    
    unsigned short _Fract _Sat uf_sat1 = uf_max;
    uf_sat1 = uf_sat1 + (unsigned short _Fract)0.00006103515625ur; /* Should saturate */
    
    /* Test 8: Loop with fixed-point accumulation */
    signed short _Fract accum = 0.0r;
    for (int i = 0; i < 4; i++) {
        accum = accum + (signed short _Fract)0.25r;
    }
    /* accum should be 1.0, which requires range check for some conversions */
    volatile signed _Accum accum_conv = (signed _Accum)accum;
    
    /* Test 9: Multiplication overflow scenarios */
    const signed short _Fract a = 0.999969482421875r;  /* MAX */
    const signed short _Fract b = 1.0r;                /* 1.0 in Q0.15 */
    
    /* This multiplication in higher precision then conversion */
    signed _Accum mul_result = (signed _Accum)a * (signed _Accum)b;
    volatile signed short _Fract mul_conv = (signed short _Fract)mul_result;
    
    /* Test 10: Values just beyond precision boundaries */
    /* i_f_bits affects the mask, so test with values that have specific bit patterns */
    const signed long _Accum sla1 = 1.99999999976716935634613037109375lk;  /* Just under 2.0 */
    const signed long _Accum sla2 = 2.00000000023283064365386962890625lk;  /* Just over 2.0 */
    
    /* Convert to narrower type - should trigger a_high.sgt(max_r) check */
    volatile signed _Accum conv_sla1 = (signed _Accum)sla1;
    volatile signed _Accum conv_sla2 = (signed _Accum)sla2;
    
    /* Test 11: Specific bit patterns to exercise a_low.ugt(max_s) */
    /* max_s is (-1) zero-extended by i_f_bits, so we need values where 
       low bits exceed this masked value */
    const unsigned long _Accum ula = 18446744073709551615.99999999976716935634613037109375ulk;
    volatile unsigned _Accum conv_ula = (unsigned _Accum)ula;
    
    /* Test 12: Negative values with specific low bits */
    const signed long _Accum sla_neg = -0.00000000023283064365386962890625lk;
    volatile signed _Accum conv_sla_neg = (signed _Accum)sla_neg;
    
    /* Use opaque functions to get boundary values */
    volatile signed short _Fract dyn_sf_max = get_sfract_max();
    volatile unsigned short _Fract dyn_uf_max = get_ufract_max();
    volatile signed long _Accum dyn_saccum_min = get_saccum_min();
    
    /* Convert dynamic boundary values */
    volatile signed _Accum conv_dyn1 = (signed _Accum)dyn_sf_max;
    volatile signed short _Fract conv_dyn2 = (signed short _Fract)dyn_saccum_min;
    
    /* Prevent dead code elimination */
    consume_accum(conv_dyn1);
    consume_accum((signed _Accum)conv_dyn2);
    
    /* Create a simple checksum for observable behavior */
    uint32_t checksum = 0;
    checksum += *(uint16_t*)&conv1;
    checksum += *(uint16_t*)&conv2;
    checksum += *(uint16_t*)&sf1;
    checksum += *(uint16_t*)&sf2;
    checksum += *(uint16_t*)&cf1;
    checksum += *(uint16_t*)&cf2;
    checksum += *(uint16_t*)&uf1;
    checksum += *(uint16_t*)&uf_conv;
    checksum += *(uint16_t*)&sf_sat1;
    checksum += *(uint16_t*)&uf_sat1;
    checksum += *(uint16_t*)&accum;
    checksum += *(uint16_t*)&mul_conv;
    
    printf("Fixed-point test completed. Checksum: %u\n", checksum);
    return (checksum != 0) ? 0 : 1;
}
