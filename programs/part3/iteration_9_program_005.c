/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (UQ0.15) */
}

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -1.99999988079071044921875k; /* Near min for signed _Accum (Q15.16) */
}

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    volatile signed _Accract dummy = val;
    (void)dummy;
    return val;
}

int main(void) {
    /* Test 1: Signed fixed-point conversions with boundary values */
    printf("Test 1: Signed fixed-point boundary conversions\n");
    
    /* Maximum values for different signed fixed-point types */
    const signed short _Fract max_sfract = 0.999969482421875r;
    const signed _Fract max_fract = 0.9999997615814208984375r;
    const signed _Accum max_accum = 1.99999988079071044921875k;
    
    /* Minimum values (negative) */
    const signed short _Fract min_sfract = -1.0r;
    const signed _Accum min_accum = -2.0k;
    
    /* Test conversions that should trigger range checks */
    volatile signed short _Fract v1;
    
    /* 1a: Convert from wider to narrower type - near max boundary */
    v1 = (signed short _Fract)max_fract;  /* Should fit but needs checking */
    
    /* 1b: Convert from _Accum to _Fract - potentially overflow */
    volatile signed _Fract v2 = (signed _Fract)max_accum;
    
    /* 1c: Arithmetic that produces values at boundaries */
    const signed _Accum a1 = max_accum / 2.0k;
    const signed _Accum a2 = max_accum / 2.0k + 0.0001k;
    volatile signed short _Fract v3 = (signed short _Fract)(a1 + a2);
    
    /* Test 2: Unsigned fixed-point with overflow checks */
    printf("Test 2: Unsigned fixed-point overflow scenarios\n");
    
    const unsigned short _Fract max_usfract = 0.999969482421875ur;
    const unsigned _Accum max_uaccum = 1.99999988079071044921875uk;
    
    /* 2a: Convert unsigned _Accum to unsigned _Fract */
    volatile unsigned short _Fract uv1 = (unsigned short _Fract)max_uaccum;
    
    /* 2b: Arithmetic that exceeds unsigned range */
    const unsigned _Accum ua1 = max_uaccum;
    const unsigned _Accum ua2 = 0.0001uk;
    volatile unsigned _Fract uv2 = (unsigned _Fract)(ua1 + ua2);
    
    /* Test 3: Mixed signed/unsigned conversions */
    printf("Test 3: Mixed signed/unsigned conversions\n");
    
    /* 3a: Positive signed to unsigned (should work) */
    volatile unsigned _Fract uv3 = (unsigned _Fract)(max_fract);
    
    /* 3b: Negative signed to unsigned (should trigger range check) */
    volatile unsigned _Fract uv4 = (unsigned _Fract)(min_sfract);
    
    /* Test 4: Saturated arithmetic types */
    printf("Test 4: Saturated fixed-point operations\n");
    
    /* 4a: Saturated addition that would overflow */
    _Sat signed short _Fract sf1 = max_sfract;
    _Sat signed short _Fract sf2 = 0.1r;
    _Sat signed short _Fract sf3 = sf1 + sf2;  /* Should saturate to max */
    
    /* 4b: Convert saturated result to non-saturated type */
    volatile signed short _Fract v4 = sf3;
    
    /* 4c: Saturated unsigned overflow */
    _Sat unsigned _Accum usa1 = max_uaccum;
    _Sat unsigned _Accum usa2 = 0.1uk;
    _Sat unsigned _Accum usa3 = usa1 + usa2;  /* Should saturate */
    volatile unsigned _Fract uv5 = (unsigned _Fract)usa3;
    
    /* Test 5: Complex constant expressions forcing range checks */
    printf("Test 5: Complex constant expressions\n");
    
    /* These should be evaluated at compile-time with range checking */
    constexpr signed _Accum ca1 = (signed _Accum)0.75k * 2.5k;  /* 1.875k */
    constexpr signed _Accum ca2 = (signed _Accum)(-0.5k) * 3.5k; /* -1.75k */
    
    /* Convert to narrower types requiring range checks */
    volatile signed short _Fract v5 = (signed short _Fract)ca1;
    volatile signed short _Fract v6 = (signed short _Fract)ca2;
    
    /* More complex expression */
    constexpr signed _Accum ca3 = (ca1 + ca2) * 1.5k;
    volatile signed _Fract v7 = (signed _Fract)ca3;
    
    /* Test 6: Loop with fixed-point accumulation */
    printf("Test 6: Loop-based accumulation\n");
    
    /* Small loop to prevent complete constant folding */
    signed _Accum accum = 0.0k;
    for (int i = 0; i < 3; i++) {
        accum += 0.7k;
    }
    /* Convert accumulated value requiring range check */
    volatile signed short _Fract v8 = (signed short _Fract)accum;
    
    /* Test 7: Values just beyond boundaries */
    printf("Test 7: Just beyond boundary values\n");
    
    /* 7a: Just above max for short _Fract */
    const signed _Fract just_above = (signed _Fract)1.0001r;
    volatile signed short _Fract v9 = (signed short _Fract)just_above;
    
    /* 7b: Just below min for short _Fract */
    const signed _Fract just_below = (signed _Fract)(-1.0001r);
    volatile signed short _Fract v10 = (signed short _Fract)just_below;
    
    /* 7c: For unsigned, just above max */
    const unsigned _Accum ujust_above = max_uaccum + 0.0000001uk;
    volatile unsigned short _Fract uv6 = (unsigned short _Fract)ujust_above;
    
    /* Test 8: Using opaque function results */
    printf("Test 8: Opaque function boundary values\n");
    
    signed short _Fract opaque_max = get_sfract_max();
    unsigned short _Fract opaque_umax = get_ufract_max();
    signed _Accum opaque_min = get_saccum_min();
    
    /* Operations with opaque values */
    volatile signed short _Fract v11 = opaque_max + 0.0001r;
    volatile unsigned short _Fract uv7 = opaque_umax * 1.1ur;
    volatile signed _Fract v12 = (signed _Fract)opaque_min;
    
    /* Consume values to prevent optimization */
    consume_accum(ca1);
    consume_accum(ca2);
    consume_accum(ca3);
    
    /* Generate a simple checksum for observable behavior */
    uint32_t checksum = 0;
    
    /* Use all volatile variables in checksum calculation */
    checksum += *(uint32_t*)&v1;
    checksum += *(uint32_t*)&v2;
    checksum += *(uint32_t*)&v3;
    checksum += *(uint32_t*)&uv1;
    checksum += *(uint32_t*)&uv2;
    checksum += *(uint32_t*)&uv3;
    checksum += *(uint32_t*)&uv4;
    checksum += *(uint32_t*)&v4;
    checksum += *(uint32_t*)&uv5;
    checksum += *(uint32_t*)&v5;
    checksum += *(uint32_t*)&v6;
    checksum += *(uint32_t*)&v7;
    checksum += *(uint32_t*)&v8;
    checksum += *(uint32_t*)&v9;
    checksum += *(uint32_t*)&v10;
    checksum += *(uint32_t*)&uv6;
    checksum += *(uint32_t*)&v11;
    checksum += *(uint32_t*)&uv7;
    checksum += *(uint32_t*)&v12;
    
    printf("Final checksum: %u\n", checksum);
    printf("All tests completed (some may produce expected overflows)\n");
    
    return 0;
}
