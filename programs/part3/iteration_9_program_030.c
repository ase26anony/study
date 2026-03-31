/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed_test.c -o fixed_test */

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

__attribute__((noinline)) volatile void consume(volatile void *ptr) {
    /* Force materialization */
    (void)ptr;
}

int main(void) {
    volatile int checksum = 0;
    
    /* Test 1: Signed conversions with boundary values */
    {
        /* These should trigger max_r/max_s comparisons */
        const signed short _Fract sf_max = 0.999969482421875r;
        const signed short _Fract sf_min = -1.0r;
        
        /* Just beyond MAX - will overflow when converting */
        const signed _Accum sa_near_max = (signed _Accum)0.999999999767r * 1.0000001r;
        
        /* Convert to narrower type - should trigger range check */
        signed short _Fract sf1 = (signed short _Fract)sa_near_max;
        checksum += (int)(sf1 * 1000r);
        
        /* Use volatile to force computation */
        volatile signed _Accum vsa = sa_near_max;
        signed short _Fract sf2 = (signed short _Fract)vsa;
        checksum += (int)(sf2 * 1000r);
    }
    
    /* Test 2: Unsigned conversions with overflow */
    {
        /* Max unsigned short _Fract */
        unsigned short _Fract uf_max = get_ufract_max();
        
        /* Create value just beyond max by adding epsilon */
        unsigned _Accum ua_overflow = (unsigned _Accum)uf_max + 
                                     (unsigned _Accum)0.0000001r;
        
        /* This conversion should trigger a_high.sgt(max_r) or a_low.ugt(max_s) */
        unsigned short _Fract uf1 = (unsigned short _Fract)ua_overflow;
        checksum += (int)(uf1 * 1000ur);
        
        /* Test with compile-time constant expression */
        const unsigned _Accum ua_const = (unsigned _Accum)1.0000001r * 
                                        (unsigned _Accum)0.9999999r;
        unsigned short _Fract uf2 = (unsigned short _Fract)ua_const;
        checksum += (int)(uf2 * 1000ur);
    }
    
    /* Test 3: Mixed signed/unsigned with saturation */
    {
        /* Saturated types should still do range checking */
        signed short _Fract _Sat sf_sat1 = 0.999969482421875r;
        signed _Accum sa_large = (signed _Accum)1.5r;
        
        /* Conversion with potential overflow */
        signed short _Fract _Sat sf_sat2 = (signed short _Fract _Sat)sa_large;
        checksum += (int)(sf_sat2 * 1000r);
        
        /* Negative overflow for signed */
        signed _Accum sa_neg = get_saccum_min();
        signed short _Fract _Sat sf_sat3 = (signed short _Fract _Sat)sa_neg;
        checksum += (int)(sf_sat3 * 1000r);
    }
    
    /* Test 4: Complex constant expressions forcing range checks */
    {
        /* These should be evaluated at compile-time but still need range checks */
        constexpr signed _Accum ca1 = (signed _Accum)0.999999999767r;
        constexpr signed _Accum ca2 = (signed _Accum)1.000000000233r;
        constexpr signed _Accum ca_product = ca1 * ca2;
        
        /* Conversion that should trigger overflow check */
        const signed short _Fract cf_result = (signed short _Fract)ca_product;
        checksum += (int)(cf_result * 1000r);
        
        /* Another with different scaling */
        constexpr unsigned long _Accum ula = (unsigned long _Accum)1.999999999999r;
        constexpr unsigned short _Fract uf_conv = (unsigned short _Fract)ula;
        checksum += (int)(uf_conv * 1000ur);
    }
    
    /* Test 5: Loop with small iteration count to create semi-constant values */
    {
        signed _Accum sa_sum = 0.0k;
        for (int i = 0; i < 3; i++) {
            sa_sum += (signed _Accum)0.333333333333r;
        }
        /* sa_sum should be ~1.0, but compiler might not fold completely */
        
        /* Convert to narrower type - may need range check */
        signed short _Fract sf_loop = (signed short _Fract)sa_sum;
        checksum += (int)(sf_loop * 1000r);
        
        /* Similar with unsigned */
        unsigned _Accum ua_sum = 0.0uk;
        for (int i = 0; i < 4; i++) {
            ua_sum += (unsigned _Accum)0.25r;
        }
        unsigned short _Fract uf_loop = (unsigned short _Fract)ua_sum;
        checksum += (int)(uf_loop * 1000ur);
    }
    
    /* Test 6: Direct boundary value testing */
    {
        /* Values at exact boundaries */
        const signed _Accum sa_exact_max = (signed _Accum)0.999969482421875r;
        const signed _Accum sa_just_above = sa_exact_max + (signed _Accum)0.000000000001r;
        
        /* These conversions exercise the comparison logic */
        signed short _Fract sf_exact = (signed short _Fract)sa_exact_max;
        signed short _Fract sf_above = (signed short _Fract)sa_just_above;
        
        checksum += (int)(sf_exact * 1000r);
        checksum += (int)(sf_above * 1000r);
        
        /* For unsigned, test zero boundary too */
        unsigned _Accum ua_just_below_zero = (unsigned _Accum)-0.000000000001r;
        unsigned short _Fract uf_below = (unsigned short _Fract)ua_just_below_zero;
        checksum += (int)(uf_below * 1000ur);
    }
    
    /* Test 7: Multiplication that overflows range */
    {
        /* Multiply two values that produce result beyond target range */
        signed short _Fract sf_a = 0.9r;
        signed short _Fract sf_b = 1.1r;  /* Actually clamped to 0.999... */
        
        /* When converted to _Accum, multiplied, then back to _Fract */
        signed _Accum sa_prod = (signed _Accum)sf_a * (signed _Accum)sf_b;
        signed short _Fract sf_result = (signed short _Fract)sa_prod;
        checksum += (int)(sf_result * 1000r);
    }
    
    /* Prevent dead code elimination */
    consume(&checksum);
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
