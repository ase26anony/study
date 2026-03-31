/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0x7FFFp-15; /* MAX short _Fract */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0xFFFFp-16; /* MAX unsigned short _Fract */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -0x8000000000000000p-48; /* MIN long _Accum */
}

__attribute__((noinline)) signed short _Accum get_ssaccum_value(void) {
    return 0x7FFEp-15; /* Just below MAX short _Accum */
}

/* Volatile stores to force materialization */
volatile signed short _Fract volatile_sfract;
volatile unsigned short _Fract volatile_ufract;
volatile signed _Sat short _Fract volatile_sat_sfract;

int main(void) {
    /* 1. Boundary values for different fixed-point types */
    const signed short _Fract sfract_max = 0x7FFFp-15;        /* +0.9999695 */
    const signed short _Fract sfract_min = -0x8000p-15;       /* -1.0 */
    const unsigned short _Fract ufract_max = 0xFFFFp-16;      /* +0.9999847 */
    const unsigned short _Fract ufract_min = 0x0000p-16;      /* 0.0 */
    
    const signed long _Accum saccum_max = 0x7FFFFFFFFFFFFFFFp-48;   /* +65535.999... */
    const signed long _Accum saccum_min = -0x8000000000000000p-48;  /* -65536.0 */
    const unsigned long _Accum uaccum_max = 0xFFFFFFFFFFFFFFFFp-48; /* +131071.999... */
    
    /* 2. Complex constant expressions that should trigger range checking */
    
    /* Expression that overflows short _Fract when converted */
    const signed long _Accum large_product = (signed long _Accum)0.5r * 2.5r;
    const signed short _Fract conv1 = (signed short _Fract)large_product;
    
    /* Just at the boundary - should pass range check */
    const signed short _Accum near_max = 0x7FFEp-15;  /* 0.9998779 */
    const signed short _Fract conv2 = (signed short _Fract)near_max;
    
    /* Just over the boundary - should trigger overflow check */
    const signed short _Accum over_max = 0x7FFFp-15;  /* 0.9999695, but in higher precision */
    const signed short _Fract conv3 = (signed short _Fract)over_max;
    
    /* 3. Test signed overflow scenarios */
    
    /* Multiply two values that exceed short _Fract range */
    const signed _Accum a1 = 0.9r;
    const signed _Accum a2 = 1.2r;
    const signed short _Fract prod1 = (signed short _Fract)(a1 * a2);  /* 1.08 > 0.999... */
    
    /* Add to maximum value */
    const signed short _Fract sf1 = sfract_max;
    const signed short _Fract delta = 0.0001r;
    const signed short _Fract sum1 = sf1 + delta;  /* Should overflow */
    
    /* 4. Test unsigned overflow */
    const unsigned short _Fract uf1 = ufract_max;
    const unsigned short _Fract uf2 = 0.0001r;
    const unsigned short _Fract usum = uf1 + uf2;  /* Should overflow */
    
    /* 5. Test negative overflow for signed types */
    const signed short _Fract sf2 = sfract_min;
    const signed short _Fract neg_delta = -0.0001r;
    const signed short _Fract diff1 = sf2 + neg_delta;  /* Should underflow */
    
    /* 6. Mixed signed/unsigned conversions */
    const signed short _Accum ssacc = -0.5r;
    const unsigned short _Fract uconv = (unsigned short _Fract)ssacc;  /* Negative to unsigned */
    
    /* 7. Saturation tests */
    signed _Sat short _Fract sat1 = sfract_max;
    sat1 += delta;  /* Should saturate at MAX */
    
    signed _Sat short _Accum sat2 = saccum_max;
    sat2 += (signed _Sat short _Accum)1.0r;  /* Should saturate */
    
    /* 8. Use opaque functions to get values */
    signed short _Fract opaque_max = get_sfract_max();
    unsigned short _Fract opaque_umax = get_ufract_max();
    signed long _Accum opaque_min = get_saccum_min();
    
    /* Force conversions with opaque values */
    signed short _Fract conv_opaque1 = (signed short _Fract)opaque_max;
    signed short _Fract conv_opaque2 = (signed short _Fract)(opaque_max * 1.1r);
    
    /* 9. Loop with small iteration count to create control flow */
    signed short _Fract accumulator = 0.0r;
    for (int i = 0; i < 3; i++) {
        accumulator += 0.4r;
    }
    /* Convert accumulated value to narrower type */
    signed _Fract narrow_accum = (signed _Fract)accumulator;
    
    /* 10. Test with values just beyond one LSB of boundaries */
    
    /* For signed short _Fract: MAX = 0x7FFFp-15, one more LSB = 0x8000p-15 */
    const signed short _Accum one_lsb_over = 0x8000p-15;
    const signed short _Fract conv_edge1 = (signed short _Fract)one_lsb_over;
    
    /* For unsigned: MAX = 0xFFFFp-16, one more LSB = 0x10000p-16 */
    const unsigned short _Accum one_lsb_over_unsigned = 0x10000p-16;
    const unsigned short _Fract conv_edge2 = (unsigned short _Fract)one_lsb_over_unsigned;
    
    /* 11. Store to volatile to prevent elimination */
    volatile_sfract = conv1;
    volatile_ufract = usum;
    volatile_sat_sfract = sat1;
    
    /* 12. Create a checksum from all results to ensure observable behavior */
    /* Convert fixed-point to integer for checksum */
    int checksum = 0;
    checksum += (int)(conv1 * 1000);
    checksum += (int)(conv2 * 1000);
    checksum += (int)(conv3 * 1000);
    checksum += (int)(prod1 * 1000);
    checksum += (int)(sum1 * 1000);
    checksum += (int)(usum * 1000);
    checksum += (int)(diff1 * 1000);
    checksum += (int)(uconv * 1000);
    checksum += (int)(sat1 * 1000);
    checksum += (int)(narrow_accum * 1000);
    checksum += (int)(conv_edge1 * 1000);
    checksum += (int)(conv_edge2 * 1000);
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed - check coverage for fixed-value.cc lines 264-277\n");
    
    return 0;
}
