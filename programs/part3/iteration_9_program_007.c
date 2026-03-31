/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) signed _Accum get_saccum_mid(void) {
    return 0.5k;
}

/* Volatile stores to force materialization */
volatile signed short _Fract volatile_sfract;
volatile unsigned short _Fract volatile_ufract;
volatile signed _Accum volatile_saccum;
volatile signed long _Accum volatile_slaccum;

int main(void) {
    /* Test 1: Boundary values for signed types */
    const signed short _Fract sf_max = 0.999969482421875r;      /* Q0.15 max */
    const signed short _Fract sf_min = -1.0r;                   /* Q0.15 min */
    const signed short _Fract sf_near_max = 0.99993896484375r;  /* One LSB less */
    
    const signed _Accum sacc_max = 9223372036854775.807k;       /* Q31.32 max */
    const signed _Accum sacc_min = -9223372036854775.808k;      /* Q31.32 min */
    
    /* Complex constant expressions that should trigger range checking */
    const signed _Accum c1 = (signed _Accum)0.999969482421875r * 1000.0k;
    const signed _Accum c2 = (signed _Accum)(-0.999969482421875r) * 1000.0k;
    
    /* These conversions should exercise a_high.sgt(max_r) logic */
    signed short _Fract t1 = (signed short _Fract)c1;  /* Large positive -> overflow */
    signed short _Fract t2 = (signed short _Fract)c2;  /* Large negative -> underflow */
    
    volatile_sfract = t1;
    volatile_sfract = t2;
    
    /* Test 2: Exercise a_low.ugt(max_s) for unsigned */
    const unsigned short _Fract uf_max = 0.999969482421875ur;
    const unsigned _Accum uacc_large = 18446744073709551.615uk; /* Large unsigned */
    
    /* This should trigger unsigned overflow check */
    unsigned short _Fract t3 = (unsigned short _Fract)uacc_large;
    volatile_ufract = t3;
    
    /* Test 3: Mixed precision with saturation */
    _Sat signed short _Fract sat_sf;
    _Sat unsigned short _Fract sat_uf;
    
    /* These should use different saturation logic paths */
    sat_sf = (signed short _Fract)(sacc_max * 2.0k);  /* Positive overflow */
    sat_uf = (unsigned short _Fract)(-1.0k);          /* Negative to unsigned */
    
    volatile_sfract = sat_sf;
    volatile_ufract = sat_uf;
    
    /* Test 4: Values just at boundaries */
    /* Create value just above max for short _Fract */
    signed _Accum just_above = (signed _Accum)sf_max;
    just_above = just_above + (signed _Accum)0.0001k;
    
    /* Create value just below min */
    signed _Accum just_below = (signed _Accum)sf_min;
    just_below = just_below - (signed _Accum)0.0001k;
    
    /* These conversions should hit boundary conditions */
    signed short _Fract t4 = (signed short _Fract)just_above;
    signed short _Fract t5 = (signed short _Fract)just_below;
    
    volatile_sfract = t4;
    volatile_sfract = t5;
    
    /* Test 5: Using opaque function returns to break constant propagation */
    signed short _Fract dynamic_sf = get_sfract_max();
    unsigned short _Fract dynamic_uf = get_ufract_max();
    signed long _Accum dynamic_slacc = get_saccum_min();
    signed _Accum dynamic_sacc = get_saccum_mid();
    
    /* Force conversions with non-constant values */
    for (int i = 0; i < 3; i++) {
        dynamic_sf = (signed short _Fract)(dynamic_sacc * 3.0k);
        dynamic_uf = (unsigned short _Fract)(dynamic_slacc + 9223372036854776.0k);
        dynamic_sacc = dynamic_sacc * 1.1k;
        
        volatile_sfract = dynamic_sf;
        volatile_ufract = dynamic_uf;
        volatile_saccum = dynamic_sacc;
        volatile_slaccum = dynamic_slaccum;
    }
    
    /* Test 6: Specific i_f_bits scenarios */
    /* Convert between different fractional bit counts */
    const signed long _Fract slf_val = 0.999999999999999999r;  /* Q0.63 */
    signed short _Fract t6 = (signed short _Fract)slf_val;     /* Q0.15 */
    
    const unsigned long _Fract ulf_val = 0.999999999999999999ur; /* U0.64 */
    unsigned short _Fract t7 = (unsigned short _Fract)ulf_val;   /* U0.16 */
    
    volatile_sfract = t6;
    volatile_ufract = t7;
    
    /* Test 7: Arithmetic that overflows intermediate precision */
    signed _Accum a1 = 5000000000.0k;  /* 5e9 */
    signed _Accum a2 = 5000000000.0k;  /* 5e9 */
    signed _Accum product = a1 * a2;    /* 2.5e19, may overflow Q31.32 */
    
    /* Convert overflowed result */
    signed short _Fract t8 = (signed short _Fract)product;
    volatile_sfract = t8;
    
    /* Compute checksum of final values for observable behavior */
    signed _Accum checksum = 0.0k;
    checksum += (signed _Accum)volatile_sfract;
    checksum += (signed _Accum)volatile_ufract;
    checksum += volatile_saccum;
    checksum += (signed _Accum)volatile_slaccum;
    
    /* Output something to prevent complete optimization */
    printf("Checksum (approx): %lld\n", (long long)(checksum * 1000.0k));
    
    return 0;
}
