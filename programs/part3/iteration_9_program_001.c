/* fixed-point-test.c
 * Tests GCC's fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) 
signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract */
}

__attribute__((noinline))
unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract */
}

__attribute__((noinline))
signed _Accum get_saccum_max(void) {
    return 32767.999969482421875k; /* MAX for signed _Accum (16.15 format) */
}

__attribute__((noinline))
signed _Accum get_saccum_min(void) {
    return -32768.0k; /* MIN for signed _Accum */
}

__attribute__((noinline))
void consume_fract(signed short _Fract f) {
    volatile signed short _Fract sink = f;
    (void)sink;
}

__attribute__((noinline))
void consume_accum(signed _Accum a) {
    volatile signed _Accum sink = a;
    (void)sink;
}

int main(void) {
    /* Test 1: Boundary values for signed types */
    printf("Test 1: Signed boundary conversions\n");
    
    /* At maximum boundary */
    const signed short _Fract sf_max = 0.999969482421875r;
    const signed _Accum sa_max = 32767.999969482421875k;
    
    /* Just beyond maximum (by adding smallest representable value) */
    const signed short _Fract sf_overflow = 0.999969482421875r + 0.000030517578125r;
    
    /* At minimum boundary */
    const signed short _Fract sf_min = -1.0r;
    const signed _Accum sa_min = -32768.0k;
    
    /* Just beyond minimum */
    const signed short _Fract sf_underflow = -1.0r - 0.000030517578125r;
    
    /* Test conversions that should trigger range checks */
    volatile signed short _Fract test1 = (signed short _Fract)sa_max;  /* Should fit */
    volatile signed short _Fract test2 = (signed short _Fract)(sa_max * 2.0k); /* Overflow */
    volatile signed short _Fract test3 = (signed short _Fract)sa_min;  /* Should fit */
    volatile signed short _Fract test4 = (signed short _Fract)(sa_min * 2.0k); /* Underflow */
    
    /* Test 2: Complex constant expressions */
    printf("Test 2: Complex constant expressions\n");
    
    /* These will be evaluated at compile-time, triggering range checking */
    constexpr signed _Accum ca1 = (signed _Accum)0.5r * 3.0r;
    constexpr signed _Accum ca2 = (signed _Accum)0.999969482421875r * 32768.0k;
    constexpr signed _Accum ca3 = (signed _Accum)(-0.5r) * 65536.0k;
    
    /* Convert to narrower types, forcing range checks */
    const signed short _Fract cf1 = (signed short _Fract)ca1;
    const signed short _Fract cf2 = (signed short _Fract)ca2;  /* Likely overflow */
    const signed short _Fract cf3 = (signed short _Fract)ca3;  /* Likely underflow */
    
    /* Test 3: Unsigned types with boundary values */
    printf("Test 3: Unsigned boundary conversions\n");
    
    const unsigned short _Fract uf_max = 0.999969482421875ur;
    const unsigned _Accum ua_max = 65535.999969482421875uk;
    
    /* Test unsigned overflow */
    volatile unsigned short _Fract test5 = (unsigned short _Fract)ua_max;  /* Should fit */
    volatile unsigned short _Fract test6 = (unsigned short _Fract)(ua_max + 1.0uk); /* Overflow */
    
    /* Test unsigned underflow (conversion from signed negative) */
    const signed _Accum sa_neg = -1.0k;
    volatile unsigned short _Fract test7 = (unsigned short _Fract)sa_neg;  /* Underflow */
    
    /* Test 4: Mixed precision arithmetic with saturation */
    printf("Test 4: Saturation tests\n");
    
    /* Saturated types handle overflow differently */
    signed short _Fract _Sat sf_sat;
    signed _Accum _Sat sa_sat;
    
    /* These should saturate rather than wrap */
    sf_sat = (signed short _Fract _Sat)(sa_max * 2.0k);  /* Should saturate to MAX */
    sa_sat = (signed _Accum _Sat)(sa_min * 2.0k);        /* Should saturate to MIN */
    
    /* Test 5: Prevent optimization with opaque functions */
    printf("Test 5: Opaque function tests\n");
    
    /* Get values from opaque functions to prevent compile-time elimination */
    signed short _Fract opaque_sf = get_sfract_max();
    unsigned short _Fract opaque_uf = get_ufract_max();
    signed _Accum opaque_sa_max = get_saccum_max();
    signed _Accum opaque_sa_min = get_saccum_min();
    
    /* Perform operations that need range checking */
    volatile signed short _Fract test8 = (signed short _Fract)(opaque_sa_max + 0.5k);
    volatile signed short _Fract test9 = (signed short _Fract)(opaque_sa_min - 0.5k);
    
    /* Test 6: Loop with small iteration count */
    printf("Test 6: Loop-based tests\n");
    
    signed _Accum accumulator = 0.0k;
    for (int i = 0; i < 4; i++) {
        accumulator += 16384.0k;  /* Add value that will overflow when converted */
        volatile signed short _Fract loop_test = (signed short _Fract)accumulator;
        consume_fract(loop_test);
    }
    
    /* Test 7: Multiplication near boundaries */
    printf("Test 7: Multiplication boundary tests\n");
    
    /* Multiply values that produce results near type boundaries */
    const signed _Accum near_max = 32767.0k;
    const signed _Accum small_val = 1.000030517578125k;  /* Slightly > 1 */
    
    volatile signed short _Fract mult_test1 = (signed short _Fract)(near_max * small_val);
    volatile signed short _Fract mult_test2 = (signed short _Fract)(-32767.0k * small_val);
    
    /* Consume all results to prevent dead code elimination */
    consume_fract(test1);
    consume_fract(test2);
    consume_fract(test3);
    consume_fract(test4);
    consume_fract(test5);
    consume_fract(test6);
    consume_fract(test7);
    consume_accum(sa_sat);
    consume_fract(test8);
    consume_fract(test9);
    consume_fract(mult_test1);
    consume_fract(mult_test2);
    consume_fract(sf_sat);
    consume_fract(cf1);
    consume_fract(cf2);
    consume_fract(cf3);
    
    /* Create a simple checksum for observable behavior */
    uint32_t checksum = 0;
    checksum += *(uint16_t*)&test1;
    checksum += *(uint16_t*)&test2;
    checksum += *(uint16_t*)&test3;
    checksum += *(uint16_t*)&test4;
    
    printf("Final checksum: %u\n", checksum);
    printf("All tests completed (some may produce expected overflow warnings)\n");
    
    return 0;
}
