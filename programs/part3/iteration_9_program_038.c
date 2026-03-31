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
    return -9223372036854775.807lk; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) signed _Accum get_saccum_val(signed _Accum x) {
    volatile signed _Accum v = x;
    return v + 0.0k;
}

/* Main test program */
int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed fract boundary tests */
    {
        /* At maximum boundary */
        const signed short _Fract sf_max = 0.999969482421875r;
        const signed short _Fract sf_near_max = 0.99993896484375r; /* One LSB less */
        
        /* Operations that should trigger range checks */
        volatile signed _Accum a1 = (signed _Accum)sf_max * 2.0k;  /* Overflow */
        volatile signed short _Fract r1 = (signed short _Fract)a1; /* Conversion check */
        
        /* Complex constant expression */
        constexpr signed _Accum ca = (signed _Accum)0.5r * 3.0r;
        volatile signed short _Fract r2 = (signed short _Fract)ca;
        
        checksum += (int)(r1 * 1000r);
        checksum += (int)(r2 * 1000r);
    }
    
    /* Test 2: Unsigned fract with overflow beyond maximum */
    {
        unsigned short _Fract uf1 = get_ufract_max();
        unsigned short _Fract uf2 = 0.000030517578125ur; /* One LSB */
        
        /* This sum overflows the unsigned fract range */
        volatile unsigned _Accum ua = (unsigned _Accum)uf1 + (unsigned _Accum)uf2;
        volatile unsigned short _Fract ur = (unsigned short _Fract)ua;
        
        /* Test with constant just above max */
        const unsigned short _Fract uf_max_plus = 1.000000000000000ur; /* > 1.0 */
        volatile unsigned short _Fract ur2 = uf_max_plus;
        
        checksum += (int)(ur * 1000ur);
        checksum += (int)(ur2 * 1000ur);
    }
    
    /* Test 3: Mixed signed/unsigned conversions */
    {
        signed long _Accum sla = get_saccum_min(); /* Large negative */
        volatile unsigned short _Fract usf = (unsigned short _Fract)sla; /* Range check */
        
        /* Positive overflow from signed to unsigned */
        signed _Accum sa_pos = 1.5k; /* > 1.0 */
        volatile unsigned _Fract uf_pos = (unsigned _Fract)sa_pos;
        
        checksum += (int)(usf * 1000ur);
        checksum += (int)(uf_pos * 1000ur);
    }
    
    /* Test 4: Saturation qualifier tests */
    {
        _Sat signed short _Fract ssf1 = 0.999969482421875sr; /* At max */
        _Sat signed short _Fract ssf2 = 0.000030517578125sr; /* One LSB */
        
        /* These should saturate rather than overflow */
        volatile _Sat signed short _Fract ssf_sum = ssf1 + ssf2;
        volatile _Sat signed short _Fract ssf_prod = ssf1 * 2.0sr;
        
        /* Convert saturated to non-saturated */
        volatile signed short _Fract nsf = ssf_sum;
        
        checksum += (int)(ssf_sum * 1000sr);
        checksum += (int)(ssf_prod * 1000sr);
        checksum += (int)(nsf * 1000r);
    }
    
    /* Test 5: Loop with boundary values */
    {
        signed short _Fract sf = 0.0r;
        for (int i = 0; i < 3; i++) {
            /* Build up to near maximum */
            sf += 0.333333333333333r;
            
            /* Convert to different precision */
            volatile signed _Accum sa = (signed _Accum)sf * 1.5k;
            volatile signed short _Fract sf2 = (signed short _Fract)sa;
            
            checksum += (int)(sf2 * 1000r);
        }
    }
    
    /* Test 6: Exact boundary value comparisons */
    {
        /* These should exercise a_high == max_r && a_low.ugt(max_s) */
        const signed long _Accum exact_max_fract = 
            (signed long _Accum)0.999969482421875r;
        
        /* Add a tiny amount to push over the boundary */
        volatile signed long _Accum just_over = exact_max_fract + 0.0000000001lk;
        volatile signed short _Fract converted = (signed short _Fract)just_over;
        
        /* Test minimum boundary for signed */
        const signed long _Accum exact_min_fract = 
            (signed long _Accum)-1.0r;
        volatile signed long _Accum just_under = exact_min_fract - 0.0000000001lk;
        volatile signed short _Fract converted2 = (signed short _Fract)just_under;
        
        checksum += (int)(converted * 1000r);
        checksum += (int)(converted2 * 1000r);
    }
    
    /* Test 7: Unsigned with i_f_bits affecting range */
    {
        /* Test different precisions */
        unsigned _Accum ua1 = 0.999999999999999999uk; /* Near 1.0 */
        unsigned _Accum ua2 = 0.000000000000000001uk; /* Tiny */
        
        volatile unsigned short _Fract usf1 = (unsigned short _Fract)ua1;
        volatile unsigned short _Fract usf2 = (unsigned short _Fract)(ua1 + ua2);
        
        /* This should overflow for short fract */
        volatile unsigned _Accum ua_large = 2.0uk;
        volatile unsigned short _Fract usf3 = (unsigned short _Fract)ua_large;
        
        checksum += (int)(usf1 * 1000ur);
        checksum += (int)(usf2 * 1000ur);
        checksum += (int)(usf3 * 1000ur);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
