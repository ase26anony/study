/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r; /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed long _Accum (Q31.32) */
}

__attribute__((noinline)) signed long _Accum get_saccum_max(void) {
    return 9223372036854775.807k; /* Approx max for signed long _Accum (Q31.32) */
}

/* Function to consume values and prevent dead code elimination */
volatile int sink;
__attribute__((noinline)) void use_value(int val) {
    sink = val;
}

int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed fixed-point conversions with boundary values */
    {
        /* Constants at boundaries */
        const signed short _Fract sf_max = 0.999969482421875r;    /* Q0.15 max */
        const signed short _Fract sf_min = -1.0r;                 /* Q0.15 min */
        const signed short _Fract sf_near_max = 0.99993896484375r; /* One LSB less */
        
        /* Operations that should trigger range checks */
        volatile signed short _Fract v1 = sf_max;
        volatile signed short _Fract v2 = sf_near_max;
        
        /* Try to overflow in multiplication */
        signed short _Fract prod = v1 * v2;  /* ~0.9999 * ~0.9999 = ~0.9998 */
        
        /* Convert to narrower type - should trigger range check */
        signed _Fract f_result = (signed _Fract)prod;
        checksum += (int)(f_result * 1000r);
        
        /* Test with compile-time constant expression */
        const signed _Accum a1 = (signed _Accum)0.999999999999999k;
        const signed _Fract f1 = (signed _Fract)a1;  /* Conversion with range check */
        checksum += (int)(f1 * 1000r);
    }
    
    /* Test 2: Unsigned fixed-point with overflow beyond max */
    {
        unsigned short _Fract uf_max = get_ufract_max();
        unsigned short _Fract uf_small = 0.000030517578125ur; /* 1 LSB */
        
        /* Create value that would overflow if scaled differently */
        unsigned short _Fract sum = uf_max + uf_small;  /* Should saturate or wrap */
        
        /* Convert to different unsigned type - triggers range check */
        unsigned _Fract uf_conv = (unsigned _Fract)sum;
        checksum += (int)(uf_conv * 1000ur);
        
        /* Complex constant expression */
        constexpr unsigned long _Accum ula = 18446744073709551.615uk; /* Large value */
        const unsigned _Fract uf_from_ula = (unsigned _Fract)ula;  /* Range check */
        checksum += (int)(uf_from_ula * 1000ur);
    }
    
    /* Test 3: Mixed signed/unsigned conversions */
    {
        signed long _Accum sla = get_saccum_max();
        
        /* Convert signed large accum to unsigned fract - should check range */
        unsigned short _Fract usf = (unsigned short _Fract)sla;
        checksum += (int)(usf * 1000ur);
        
        /* Negative to unsigned conversion */
        signed long _Accum sla_neg = get_saccum_min();
        unsigned short _Fract usf_neg = (unsigned short _Fract)sla_neg;  /* Range check */
        checksum += (int)(usf_neg * 1000ur);
    }
    
    /* Test 4: Saturation qualifier tests */
    {
        _Sat signed short _Fract ssf_sat = 0.999969482421875r;
        _Sat signed short _Fract ssf_sat2 = 0.5r;
        
        /* Saturated addition that would overflow */
        _Sat signed short _Fract sat_sum = ssf_sat + ssf_sat2;  /* Should saturate to max */
        
        /* Convert saturated to non-saturated - still needs range check */
        signed short _Fract reg_from_sat = (signed short _Fract)sat_sum;
        checksum += (int)(reg_from_sat * 1000r);
        
        /* Mix saturated and non-saturated in expression */
        signed short _Fract mixed = ssf_sat * 1.5r;  /* Should overflow */
        signed _Fract mixed_conv = (signed _Fract)mixed;  /* Range check */
        checksum += (int)(mixed_conv * 1000r);
    }
    
    /* Test 5: Loop with fixed-point computations */
    {
        signed short _Fract accum = 0.0r;
        const signed short _Fract increment = 0.000030517578125r; /* 1 LSB */
        
        /* Small loop to build up to max value */
        for (int i = 0; i < 32767; i++) {
            accum += increment;
        }
        
        /* One more addition to potentially exceed max */
        accum += increment;
        
        /* Convert to different type - triggers range check */
        signed _Fract loop_result = (signed _Fract)accum;
        checksum += (int)(loop_result * 1000r);
    }
    
    /* Test 6: Complex constant folding expressions */
    {
        /* These should be evaluated at compile-time with range checks */
        const signed _Accum ca1 = (signed _Accum)0.999999999999999k;
        const signed _Accum ca2 = (signed _Accum)0.999999999999999k;
        const signed _Accum ca_prod = ca1 * ca2;  /* ~0.999999999999998 */
        
        /* Multiple conversions with different precisions */
        const signed long _Fract slf = (signed long _Fract)ca_prod;
        const signed _Fract sf = (signed _Fract)slf;
        const signed short _Fract ssf = (signed short _Fract)sf;
        
        checksum += (int)(ssf * 1000r);
        
        /* Expression just beyond the maximum representable value */
        const signed short _Fract max_fract = 0.999969482421875r;
        const signed short _Fract one_lsb = 0.000030517578125r;
        const signed short _Fract beyond_max = max_fract + one_lsb;  /* Would overflow */
        
        /* Convert overflowed value */
        signed _Fract conv_beyond = (signed _Fract)beyond_max;
        checksum += (int)(conv_beyond * 1000r);
    }
    
    /* Test 7: Bit-exact boundary testing */
    {
        /* Values designed to test the specific comparisons in the uncovered code:
         * a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s))
         */
        
        /* Create a value where high part equals max_r but low part exceeds max_s */
        volatile signed long _Accum boundary_val = 0.999999999999999k;
        
        /* Repeated operations to potentially create the exact boundary condition */
        for (int i = 0; i < 10; i++) {
            boundary_val = boundary_val * boundary_val;
        }
        
        /* Convert to much narrower type - should trigger all range comparisons */
        signed short _Fract narrow = (signed short _Fract)boundary_val;
        checksum += (int)(narrow * 1000r);
        
        /* Similar test for unsigned */
        volatile unsigned long _Accum u_boundary_val = 0.999999999999999uk;
        for (int i = 0; i < 5; i++) {
            u_boundary_val = u_boundary_val * 1.5uk;
        }
        
        unsigned short _Fract u_narrow = (unsigned short _Fract)u_boundary_val;
        checksum += (int)(u_narrow * 1000ur);
    }
    
    printf("Checksum: %d\n", checksum);
    use_value(checksum);  /* Prevent optimization */
    
    return 0;
}
