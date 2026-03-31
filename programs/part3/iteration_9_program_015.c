/* fixed-point-test.c - Test program for GCC fixed-point range checking */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.9999r;  /* Close to maximum signed short _Fract */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.9999ur; /* Close to maximum unsigned short _Fract */
}

__attribute__((noinline)) signed long _Accum get_saccum_half(void) {
    return 0.5k;     /* 0.5 in signed long _Accum */
}

__attribute__((noinline)) signed _Sat long _Accum get_sat_accum(void) {
    return 0.75k;    /* Saturated accum value */
}

/* Function to mix values and break optimization */
__attribute__((noinline)) signed short _Fract mix_and_convert(signed long _Accum a, 
                                                              signed long _Accum b) {
    volatile signed long _Accum temp = a * b;
    /* Force conversion with potential overflow */
    return (signed short _Fract)temp;
}

int main(void) {
    /* 1. Boundary value declarations */
    const signed short _Fract MAX_SFRACT = 0.9999r;      /* ~1 - 1/2^15 */
    const signed short _Fract MIN_SFRACT = -1.0r;        /* Minimum signed fract */
    const unsigned short _Fract MAX_UFRACT = 0.9999ur;   /* ~1 - 1/2^16 */
    
    const signed long _Accum MAX_SACCUM = 0.9999999999k; /* ~1 - 1/2^31 */
    const signed long _Accum MIN_SACCUM = -1.0k;         /* Minimum signed accum */
    const signed long _Accum OVERFLOW_VAL = 1.5k;        /* Definitely overflows to short fract */
    
    /* 2. Complex constant expressions that should trigger range checking */
    constexpr signed long _Accum c1 = (signed long _Accum)0.75k * 0.75k;      /* 0.5625 */
    constexpr signed long _Accum c2 = (signed long _Accum)0.9k * 0.9k;        /* 0.81 */
    constexpr signed long _Accum c3 = (signed long _Accum)(-0.9k) * 0.9k;     /* -0.81 */
    
    /* 3. Test cases designed to exercise the uncovered comparison logic */
    
    /* Case A: Conversion that should be exactly at max_r, max_s boundary */
    volatile signed short _Fract f1 = (signed short _Fract)MAX_SACCUM;
    
    /* Case B: Conversion that exceeds max_r (a_high.sgt(max_r)) */
    volatile signed short _Fract f2 = (signed short _Fract)OVERFLOW_VAL;
    
    /* Case C: Conversion from unsigned with different scaling */
    volatile unsigned short _Fract uf1 = (unsigned short _Fract)(MAX_UFRACT * 2.0ur);
    
    /* Case D: Mixed signed/unsigned conversions */
    volatile signed short _Fract f3 = (signed short _Fract)((unsigned short _Fract)0.5ur * 2.0ur);
    
    /* Case E: Arithmetic followed by conversion to narrower type */
    volatile signed long _Accum accum1 = MAX_SACCUM + (signed long _Accum)0.0001k;
    volatile signed short _Fract f4 = (signed short _Fract)accum1;
    
    /* Case F: Negative overflow (should trigger min_r/min_s checking) */
    volatile signed long _Accum accum2 = MIN_SACCUM - (signed long _Accum)0.0001k;
    volatile signed short _Fract f5 = (signed short _Fract)accum2;
    
    /* 4. Use opaque functions to get values that can't be fully constant folded */
    volatile signed short _Fract f6 = (signed short _Fract)(get_saccum_half() * 2.0k);
    volatile signed short _Fract f7 = mix_and_convert(MAX_SACCUM, MAX_SACCUM);
    
    /* 5. Test with _Sat qualifier - different overflow behavior */
    volatile signed _Sat short _Fract sf1 = (signed _Sat short _Fract)OVERFLOW_VAL;
    volatile signed _Sat long _Accum sat_accum = get_sat_accum() * 2.0k;
    volatile signed _Sat short _Fract sf2 = (signed _Sat short _Fract)sat_accum;
    
    /* 6. Loop with small iteration count to create control flow but mostly constant values */
    volatile signed short _Fract loop_result = 0.0r;
    for (int i = 0; i < 3; i++) {
        signed long _Accum temp = (signed long _Accum)i * 0.5k;
        loop_result += (signed short _Fract)temp;
    }
    
    /* 7. Test unsigned boundary cases */
    volatile unsigned short _Fract uf2 = (unsigned short _Fract)(MAX_UFRACT + (unsigned short _Fract)0.0001ur);
    volatile unsigned _Sat short _Fract uf3 = (unsigned _Sat short _Fract)(MAX_UFRACT * 1.1ur);
    
    /* 8. Multi-step conversions with intermediate precision changes */
    volatile signed long _Accum step1 = c1 * c2;                     /* ~0.4556 */
    volatile signed _Accum step2 = (signed _Accum)step1;            /* Medium precision */
    volatile signed short _Fract step3 = (signed short _Fract)step2; /* Narrowest */
    
    /* 9. Create a checksum to ensure all computations are used */
    uint32_t checksum = 0;
    
    /* Access all volatile variables to ensure they're computed */
    checksum += *(uint16_t*)&f1;
    checksum += *(uint16_t*)&f2;
    checksum += *(uint16_t*)&f3;
    checksum += *(uint16_t*)&f4;
    checksum += *(uint16_t*)&f5;
    checksum += *(uint16_t*)&f6;
    checksum += *(uint16_t*)&f7;
    checksum += *(uint16_t*)&sf1;
    checksum += *(uint16_t*)&sf2;
    checksum += *(uint16_t*)&loop_result;
    checksum += *(uint16_t*)&uf1;
    checksum += *(uint16_t*)&uf2;
    checksum += *(uint16_t*)&uf3;
    checksum += *(uint16_t*)&step3;
    
    printf("Fixed-point test completed. Checksum: %u\n", checksum);
    printf("(Note: Actual values depend on fixed-point format specifics)\n");
    
    return 0;
}
