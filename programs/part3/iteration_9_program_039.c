/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant folding */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -1.99999999976716935634613037109375lk; /* MIN for signed long _Accum (Q15.16) */
}

__attribute__((noinline)) signed _Accum get_saccum_half(void) {
    return 0.5k;
}

/* Force materialization of values */
volatile signed short _Fract volatile_sfract;
volatile unsigned short _Fract volatile_ufract;
volatile signed _Accum volatile_saccum;
volatile signed long _Accum volatile_slaccum;

int main(void) {
    /* Test 1: Boundary values for signed fract types */
    const signed short _Fract sf_max = 0.999969482421875r;  /* Q0.15 max */
    const signed short _Fract sf_min = -1.0r;               /* Q0.15 min */
    const signed short _Fract sf_near_max = 0.99993896484375r; /* MAX - 1 LSB */
    
    /* Test 2: Conversion from _Accum to _Fract with overflow */
    const signed _Accum sa_val1 = 1.5k;  /* Will overflow when converted to short _Fract */
    const signed _Accum sa_val2 = -1.1k; /* Will underflow when converted to short _Fract */
    
    /* Test 3: Complex constant expressions */
    constexpr signed _Accum ca1 = (signed _Accum)0.75r * 2.0k;  /* 1.5k */
    constexpr signed _Accum ca2 = (signed _Accum)(-0.875r) * 1.5k; /* -1.3125k */
    
    /* Test 4: Mixed precision with boundary testing */
    signed short _Fract sf1, sf2, sf3;
    unsigned short _Fract uf1, uf2;
    signed _Accum sa1, sa2;
    signed long _Accum sla1, sla2;
    
    /* Exercise a_high.sgt(max_r) - positive overflow case */
    /* Convert from _Accum (Q15.16) to short _Fract (Q0.15) */
    sa1 = 1.99999999976716935634613037109375k; /* Just below max for Q15.16 */
    sf1 = (signed short _Fract)sa1;  /* Should trigger range check */
    volatile_sfract = sf1;
    
    /* Exercise a_high == max_r && a_low.ugt(max_s) */
    /* Need value where high part equals max_r (0) and low part > max_s */
    sa2 = get_saccum_half() * 3.999k;  /* ~1.9995k - carefully crafted */
    sf2 = (signed short _Fract)sa2;
    volatile_sfract = sf2;
    
    /* Test with saturation qualifier */
    signed short _Fract _Sat sf_sat;
    sf_sat = (signed short _Fract _Sat)sa1;  /* Different code path */
    volatile_sfract = sf_sat;
    
    /* Test negative overflow (a_high.slt(min_r) path) */
    sla1 = get_saccum_min();  /* Minimum signed long _Accum */
    sf3 = (signed short _Fract)sla1;  /* Should trigger negative overflow check */
    volatile_sfract = sf3;
    
    /* Test unsigned overflow */
    unsigned _Accum ua1 = 1.99999999976716935634613037109375uk; /* Max unsigned _Accum */
    uf1 = (unsigned short _Fract)ua1;  /* Should overflow */
    volatile_ufract = uf1;
    
    /* Test with one LSB beyond maximum */
    uf2 = get_ufract_max();
    /* Add a small value that would exceed range if not saturated */
    uf2 = uf2 + (unsigned short _Fract)0.0001ur;
    volatile_ufract = uf2;
    
    /* Test arithmetic that overflows then conversion */
    signed _Accum sa3 = 0.9k;
    signed _Accum sa4 = 0.8k;
    signed _Accum sa_sum = sa3 + sa4;  /* 1.7k > 1.0 for _Fract */
    signed short _Fract sf_sum = (signed short _Fract)sa_sum;
    volatile_sfract = sf_sum;
    
    /* Test multiplication overflow */
    signed _Accum sa5 = 1.2k;
    signed _Accum sa6 = 1.3k;
    signed _Accum sa_prod = sa5 * sa6;  /* ~1.56k > 1.0 */
    signed short _Fract sf_prod = (signed short _Fract)sa_prod;
    volatile_sfract = sf_prod;
    
    /* Loop with constant iterations to create control flow */
    signed short _Fract sf_accum = 0.0r;
    for (int i = 0; i < 3; i++) {
        sf_accum = sf_accum + (signed short _Fract)0.333r;
    }
    /* Convert accumulated value (0.999r) to narrower type */
    signed _Fract sf_narrow = (signed _Fract)sf_accum;
    volatile_sfract = sf_narrow;
    
    /* Test mixed signed/unsigned conversion */
    signed _Accum sa_neg = -0.5k;
    unsigned short _Fract uf_from_signed = (unsigned short _Fract)sa_neg;
    volatile_ufract = uf_from_signed;
    
    /* Create a checksum to ensure all computations are used */
    unsigned int checksum = 0;
    
    /* Use all volatile variables to prevent optimization */
    checksum += *(unsigned int*)&volatile_sfract;
    checksum += *(unsigned int*)&volatile_ufract;
    
    /* Simple output to prevent complete optimization */
    printf("Checksum: %u\n", checksum % 1000);
    
    return 0;
}
