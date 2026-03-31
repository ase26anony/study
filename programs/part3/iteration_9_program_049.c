/* fixed-point-test.c
 * Test program for GCC fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
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

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    volatile signed _Accum sink = val;
    return sink;
}

int main(void) {
    /* Test 1: Signed fract boundary tests */
    printf("Test 1: Signed _Fract boundary tests\n");
    
    const signed short _Fract sf_max = 0.999969482421875r;  /* Q0.15 max */
    const signed short _Fract sf_min = -1.0r;               /* Q0.15 min */
    const signed short _Fract sf_near_max = 0.99993896484375r; /* MAX - 1LSB */
    
    /* These conversions should trigger range checks */
    volatile signed char _Fract cf1 = sf_max;      /* Narrowing conversion */
    volatile signed char _Fract cf2 = sf_min;      /* Negative value */
    
    /* Test 2: Unsigned fract overflow tests */
    printf("Test 2: Unsigned _Fract overflow tests\n");
    
    const unsigned short _Fract uf_max = 0.999969482421875ur; /* U0.16 max */
    const unsigned short _Fract uf_near_max = 0.99993896484375ur; /* MAX - 1LSB */
    
    /* Operations that might overflow */
    unsigned short _Fract uf1 = uf_max;
    unsigned short _Fract uf2 = uf_near_max;
    
    /* This addition might overflow when converted */
    volatile unsigned char _Fract ucf1 = uf1 + 0.0001ur;
    volatile unsigned char _Fract ucf2 = uf2 + 0.0001ur;
    
    /* Test 3: _Accum to _Fract conversions with boundary values */
    printf("Test 3: _Accum to _Fract conversions\n");
    
    /* Values designed to exercise a_high.sgt(max_r) and a_low.ugt(max_s) */
    const signed _Accum sa_max = 32767.999969482421875k;   /* Max for short _Fract in _Accum */
    const signed _Accum sa_overflow = 32800.0k;           /* Just over short _Fract max */
    const signed _Accum sa_underflow = -32800.0k;         /* Just under short _Fract min */
    
    /* These conversions should trigger the uncovered range checking */
    volatile signed short _Fract ssf1 = sa_max;           /* At boundary */
    volatile signed short _Fract ssf2 = sa_overflow;      /* Should overflow */
    volatile signed short _Fract ssf3 = sa_underflow;     /* Should underflow */
    
    /* Test 4: Complex constant expressions */
    printf("Test 4: Complex constant expressions\n");
    
    /* Multi-step computation that might overflow during constant folding */
    constexpr signed _Accum ca1 = (signed _Accum)0.5k * 65535.0k;      /* 32767.5 */
    constexpr signed _Accum ca2 = (signed _Accum)0.999969482421875k;   /* ~1.0 */
    constexpr signed _Accum ca3 = ca1 * ca2;                          /* ~32767.0 */
    
    /* Convert to narrower type - should trigger range check */
    volatile signed short _Fract csf1 = ca3;
    volatile signed short _Fract csf2 = ca3 + (signed _Accum)0.0001k; /* Just over */
    
    /* Test 5: Mixed signed/unsigned conversions */
    printf("Test 5: Mixed signed/unsigned conversions\n");
    
    signed _Accum mixed_sa = 10000.5k;
    unsigned _Accum mixed_ua = 20000.5uk;
    
    /* Cross-type conversions that need range checking */
    volatile unsigned short _Fract usf1 = mixed_sa;       /* Signed to unsigned */
    volatile signed short _Fract ssf4 = mixed_ua;         /* Unsigned to signed */
    
    /* Test 6: Saturation qualifier tests */
    printf("Test 6: Saturation qualifier tests\n");
    
    /* _Sat types have different overflow behavior */
    signed short _Fract _Sat sat_sf1 = sf_max;
    signed short _Fract _Sat sat_sf2 = sat_sf1 + 0.1r;    /* Should saturate */
    
    unsigned short _Fract _Sat sat_uf1 = uf_max;
    unsigned short _Fract _Sat sat_uf2 = sat_uf1 + 0.1ur; /* Should saturate */
    
    /* Convert saturated values to non-saturated types */
    volatile signed short _Fract ns_sf1 = sat_sf2;
    volatile unsigned short _Fract ns_uf1 = sat_uf2;
    
    /* Test 7: Loop-based computations to prevent optimization */
    printf("Test 7: Loop-based boundary testing\n");
    
    signed _Accum accum = 0.0k;
    volatile signed short _Fract loop_result;
    
    /* Small loop that produces values near boundaries */
    for (int i = 0; i < 10; i++) {
        accum = accum + 3276.8k;  /* Increment that will eventually overflow */
        loop_result = accum;      /* Conversion each iteration */
    }
    
    /* Test 8: Bit-exact boundary values */
    printf("Test 8: Bit-exact boundary values\n");
    
    /* Construct values that exactly match the bit patterns being compared */
    /* These should exercise a_high == max_r && a_low.ugt(max_s) path */
    
    /* For signed short _Fract (i_f_bits = 15):
     * max_r = 0 (high and low)
     * max_s = (1 << 15) - 1 = 32767
     * We need a_high == 0 && a_low > 32767
     */
    
    /* Create an _Accum with high=0, low=32768 (32768 > 32767) */
    /* 32768 in 16.16 format is 32768 << 16 = 2147483648 */
    const signed _Accum exact_boundary = (signed _Accum)32768.0k / 65536.0k; /* Actually 0.5 */
    /* Need to adjust - let's use a different approach */
    
    /* Direct value that should trigger the comparison */
    const signed _Accum trigger_value = (signed _Accum)32768.0k; /* 32768 in 16.16 format */
    volatile signed short _Fract test_boundary = trigger_value;
    
    /* Test 9: Function calls to get boundary values */
    printf("Test 9: Function call boundary tests\n");
    
    signed short _Fract dynamic_max = get_sfract_max();
    unsigned short _Fract dynamic_umax = get_ufract_max();
    signed long _Accum dynamic_min = get_saccum_min();
    
    /* Conversions after function calls */
    volatile signed char _Fract dcf1 = dynamic_max;
    volatile unsigned char _Fract ducf1 = dynamic_umax;
    volatile signed short _Fract dsf1 = dynamic_min;  /* Large negative */
    
    /* Test 10: Multiplication overflow */
    printf("Test 10: Multiplication overflow tests\n");
    
    signed _Accum mult_a = 200.0k;
    signed _Accum mult_b = 200.0k;
    signed _Accum mult_result = mult_a * mult_b;  /* 40000 > 32767 */
    
    volatile signed short _Fract mult_conv = mult_result;  /* Should overflow */
    
    /* Consume all results to prevent optimization */
    consume_accum(mult_result);
    
    /* Create a simple checksum to ensure computations happen */
    uint32_t checksum = 0;
    
    /* Use all variables in checksum calculation */
    checksum += *(uint32_t*)&cf1;
    checksum += *(uint32_t*)&cf2;
    checksum += *(uint32_t*)&ucf1;
    checksum += *(uint32_t*)&ucf2;
    checksum += *(uint32_t*)&ssf1;
    checksum += *(uint32_t*)&ssf2;
    checksum += *(uint32_t*)&ssf3;
    checksum += *(uint32_t*)&csf1;
    checksum += *(uint32_t*)&csf2;
    checksum += *(uint32_t*)&usf1;
    checksum += *(uint32_t*)&ssf4;
    checksum += *(uint32_t*)&ns_sf1;
    checksum += *(uint32_t*)&ns_uf1;
    checksum += *(uint32_t*)&loop_result;
    checksum += *(uint32_t*)&test_boundary;
    checksum += *(uint32_t*)&dcf1;
    checksum += *(uint32_t*)&ducf1;
    checksum += *(uint32_t*)&dsf1;
    checksum += *(uint32_t*)&mult_conv;
    
    printf("Final checksum: %u\n", checksum);
    printf("All tests completed (some may overflow as expected)\n");
    
    return 0;
}
