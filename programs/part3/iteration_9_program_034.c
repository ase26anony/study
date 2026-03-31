/* fixed-point-test.c - Test program for GCC fixed-point range checking */
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

__attribute__((noinline)) volatile signed _Accum consume_accum(signed _Accum val) {
    volatile signed _Accract dummy = val;
    (void)dummy;
    return val;
}

int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed fract boundary testing */
    {
        /* These should trigger max_r/max_s comparisons */
        const signed short _Fract sf_max = 0.999969482421875r;  /* Q0.15 max */
        const signed short _Fract sf_min = -1.0r;               /* Q0.15 min */
        
        /* Operations near boundaries */
        volatile signed short _Fract v1 = sf_max;
        volatile signed short _Fract v2 = sf_max * 0.999r;  /* Just below max */
        
        /* This addition might overflow when converted */
        signed _Accum a1 = (signed _Accum)sf_max + (signed _Accum)0.0001r;
        
        /* Convert to narrower type - should trigger range check */
        signed short _Fract r1 = (signed short _Fract)a1;
        checksum += (int)(r1 * 10000r);
        
        /* Test with compile-time constant expression */
        constexpr signed _Accum ca = (signed _Accum)0.5r * 2.5r;  /* 1.25 */
        signed short _Fract r2 = (signed short _Fract)ca;  /* In range */
        signed short _Fract r3 = (signed short _Fract)(ca * 2.0r);  /* 2.5 -> overflow */
    }
    
    /* Test 2: Unsigned fract with overflow beyond max */
    {
        unsigned short _Fract uf1 = get_ufract_max();
        unsigned short _Fract uf2 = uf1 * 1.0001ur;  /* Slightly above max */
        
        /* Convert from wider type that exceeds range */
        unsigned long _Accum ula = (unsigned long _Accum)1.5uk;
        unsigned short _Fract uf3 = (unsigned short _Fract)ula;  /* Should overflow */
        
        checksum += (int)(uf1 * 10000ur);
        checksum += (int)(uf3 * 10000ur);
    }
    
    /* Test 3: Mixed signed/unsigned conversions */
    {
        signed _Accum sa = -0.5k;
        /* Convert signed negative to unsigned - should trigger min checks */
        unsigned _Fract uf = (unsigned _Fract)sa;
        
        /* Large positive signed accum to unsigned fract */
        signed _Accum sa2 = 1.5k;
        unsigned short _Fract uf2 = (unsigned short _Fract)sa2;  /* > 1.0, overflow */
        
        checksum += (int)(uf * 10000ur);
    }
    
    /* Test 4: Saturation qualifier testing */
    {
        _Sat signed short _Fract ssf1 = 0.999969482421875sr;
        _Sat signed short _Fract ssf2 = ssf1 + 0.0001sr;  /* Should saturate */
        
        _Sat unsigned short _Fract suf1 = 0.999969482421875us;
        _Sat unsigned short _Fract suf2 = suf1 * 1.1us;  /* Should saturate at max */
        
        /* Mix saturated and non-saturated */
        signed _Accum sa = 1.2k;
        _Sat signed short _Fract ssf3 = (_Sat signed short _Fract)sa;
        
        checksum += (int)(ssf2 * 10000sr);
        checksum += (int)(suf2 * 10000us);
    }
    
    /* Test 5: Complex constant expressions forcing range checks */
    {
        /* These should be evaluated at compile-time but require range checking */
        const signed _Accum ca1 = (signed _Accum)0.75r * (signed _Accum)1.5r;  /* 1.125 */
        const signed short _Fract cf1 = (signed short _Fract)ca1;  /* In range */
        
        const signed _Accum ca2 = (signed _Accum)(-0.9r) * (signed _Accum)1.1r;  /* -0.99 */
        const signed short _Fract cf2 = (signed short _Fract)ca2;  /* In range */
        
        /* This should overflow when converted */
        const signed _Accum ca3 = (signed _Accum)0.9r * (signed _Accum)1.2r;  /* 1.08 > 1.0 */
        const signed short _Fract cf3 = (signed short _Fract)ca3;
        
        /* Use volatile to force materialization */
        volatile signed short _Fract vcf1 = cf1;
        volatile signed short _Fract vcf2 = cf2;
        volatile signed short _Fract vcf3 = cf3;
        
        checksum += (int)(vcf1 * 10000r);
        checksum += (int)(vcf2 * 10000r);
        checksum += (int)(vcf3 * 10000r);
    }
    
    /* Test 6: Loop with small iteration count */
    {
        signed short _Fract accum = 0.0r;
        for (int i = 0; i < 3; i++) {
            accum += 0.4r;  /* 0.0, 0.4, 0.8, 1.2 (overflow on last) */
        }
        /* Convert to even narrower type */
        signed char _Fract cf = (signed char _Fract)accum;
        checksum += (int)(cf * 100r);
    }
    
    /* Test 7: Exact boundary value testing */
    {
        /* Values designed to exercise a_high.sgt(max_r) and a_low.ugt(max_s) */
        
        /* For signed short _Fract (i_f_bits = 15), max is 0x7FFF/0x8000 = 0.999969... */
        /* Create value just above max by adding 1 LSB beyond representation */
        signed _Accum just_above_max = (signed _Accum)0.999969482421875r + 
                                      (signed _Accum)0.000000000000001r;
        signed short _Fract sf_overflow = (signed short _Fract)just_above_max;
        
        /* Value exactly at max */
        signed _Accum at_max = (signed _Accum)0.999969482421875r;
        signed short _Fract sf_exact = (signed short _Fract)at_max;
        
        /* Value just below max */
        signed _Accum just_below_max = (signed _Accum)0.999969482421875r - 
                                      (signed _Accum)0.000000000000001r;
        signed short _Fract sf_under = (signed short _Fract)just_below_max;
        
        checksum += (int)(sf_overflow * 10000r);
        checksum += (int)(sf_exact * 10000r);
        checksum += (int)(sf_under * 10000r);
    }
    
    /* Test 8: Negative overflow (below minimum) */
    {
        signed _Accum below_min = (signed _Accum)(-1.0r) - (signed _Accum)0.0001r;
        signed short _Fract sf_neg_overflow = (signed short _Fract)below_min;
        
        checksum += (int)(sf_neg_overflow * 10000r);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
