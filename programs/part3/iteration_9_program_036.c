/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) signed _Accum get_saccum_half(void) {
    return 0.5k;
}

/* Volatile stores to force materialization */
volatile signed short _Fract volatile_sfract;
volatile unsigned short _Fract volatile_ufract;
volatile signed _Accum volatile_saccum;
volatile signed long _Accum volatile_slaccum;

int main(void) {
    /* Test 1: Boundary values for signed types */
    const signed short _Fract sf_max = 0.999969482421875r;  /* Q0.15 max */
    const signed short _Fract sf_min = -1.0r;               /* Q0.15 min */
    const signed short _Fract sf_near_max = 0.99993896484375r; /* MAX - 1 LSB */
    
    /* Test 2: Complex constant expressions that approach limits */
    const signed _Accum sa1 = (signed _Accum)0.9999999995343387k; /* Near max */
    const signed _Accum sa2 = (signed _Accum)0.0000000004656613k; /* Very small */
    
    /* These conversions should trigger range checks */
    signed short _Fract sf1 = (signed short _Fract)sa1;  /* May overflow */
    signed short _Fract sf2 = (signed short _Fract)(sa1 + sa2); /* Definite overflow */
    
    volatile_sfract = sf1;
    volatile_sfract = sf2;
    
    /* Test 3: Operations that produce values just beyond max */
    signed _Accum sa3 = get_saccum_half();
    signed _Accum sa4 = sa3 * (signed _Accum)2.0000001k; /* Slightly > 1.0 */
    
    /* This conversion should trigger a_high.sgt(max_r) check */
    signed short _Fract sf3 = (signed short _Fract)sa4;
    volatile_sfract = sf3;
    
    /* Test 4: Negative overflow for signed types */
    signed long _Accum sla1 = get_saccum_min();
    signed long _Accum sla2 = (signed long _Accum)-0.0000000001k;
    
    /* Conversion from long _Accum to shorter type with negative overflow */
    signed _Accum sa5 = (signed _Accum)(sla1 + sla2);
    volatile_saccum = sa5;
    
    /* Test 5: Unsigned type boundary testing */
    const unsigned short _Fract uf_max = 0.999969482421875ur; /* U0.16 max */
    const unsigned short _Fract uf_near_max = 0.99993896484375ur; /* MAX - 1 LSB */
    
    /* Operations that might overflow unsigned max */
    unsigned short _Fract uf1 = uf_max;
    unsigned short _Fract uf2 = (unsigned short _Fract)(uf_near_max + uf_near_max);
    
    volatile_ufract = uf1;
    volatile_ufract = uf2;
    
    /* Test 6: Mixed signed/unsigned conversions */
    signed _Accum sa6 = (signed _Accum)0.75k;
    unsigned short _Fract uf3 = (unsigned short _Fract)sa6; /* Should work */
    unsigned short _Fract uf4 = (unsigned short _Fract)(-sa6); /* Should underflow to 0 */
    
    volatile_ufract = uf3;
    volatile_ufract = uf4;
    
    /* Test 7: Saturation qualifier testing */
    _Sat signed short _Fract ssf1 = (signed short _Fract)1.5k; /* Should saturate to max */
    _Sat unsigned short _Fract suf1 = (unsigned short _Fract)(-0.5k); /* Should saturate to 0 */
    
    volatile_sfract = ssf1;
    volatile_ufract = suf1;
    
    /* Test 8: Multi-step computation with intermediate overflow */
    signed _Accum sa7 = (signed _Accum)0.9k;
    signed _Accum sa8 = (signed _Accum)0.9k;
    signed _Accum sa9 = sa7 * sa8; /* 0.81k - safe */
    signed _Accum sa10 = sa9 * (signed _Accum)1.3k; /* ~1.053k - may overflow when converted */
    
    signed short _Fract sf4 = (signed short _Fract)sa10;
    volatile_sfract = sf4;
    
    /* Test 9: Loop with constant iteration to create predictable but non-trivial values */
    signed _Accum sa_accum = (signed _Accum)0.0k;
    for (int i = 0; i < 5; i++) {
        sa_accum += (signed _Accum)0.25k;
    }
    /* sa_accum = 1.25k, overflow when converted to short _Fract */
    signed short _Fract sf5 = (signed short _Fract)sa_accum;
    volatile_sfract = sf5;
    
    /* Test 10: Exact boundary case - value equals max representable */
    /* Construct a value that should exactly equal max_s in the uncovered code */
    signed long _Accum sla_boundary = (signed long _Accum)0.999969482421875k;
    signed short _Fract sf6 = (signed short _Fract)sla_boundary;
    volatile_sfract = sf6;
    
    /* Test 11: Value just above max (by 1 LSB) */
    /* This should trigger a_low.ugt(max_s) when max_r == a_high */
    signed long _Accum sla_overflow = (signed long _Accum)0.9999694824218751k;
    signed short _Fract sf7 = (signed short _Fract)sla_overflow;
    volatile_sfract = sf7;
    
    /* Create observable output to prevent complete optimization */
    printf("Results: %d %d %d %d\n", 
           (int)(volatile_sfract * 1000),
           (int)(volatile_ufract * 1000),
           (int)(volatile_saccum * 1000),
           (int)(volatile_slaccum * 1000));
    
    return 0;
}
