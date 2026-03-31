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
    return -1.999999999999999999939kr; /* Near MIN for signed long _Accum */
}

__attribute__((noinline)) signed _Accum get_saccum_val(signed _Accum x) {
    volatile signed _Accum v = x;
    return v + 0.0k;
}

/* Main test program */
int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed conversions with boundary values */
    {
        /* Get maximum signed short _Fract at runtime to prevent compile-time elimination */
        volatile signed short _Fract max_sf = get_sfract_max();
        
        /* These will trigger range checks during conversion to narrower types */
        const signed _Accum a1 = (signed _Accum)max_sf * 2.0k;  /* Just above max for short fract */
        const signed _Accum a2 = (signed _Accum)max_sf * 1.5k;  /* Within range */
        const signed _Accum a3 = (signed _Accum)max_sf * -2.0k; /* Below min */
        
        /* Conversions that should trigger the uncovered range checking logic */
        signed short _Fract f1 = (signed short _Fract)a1;  /* Overflow positive */
        signed short _Fract f2 = (signed short _Fract)a2;  /* Within range */
        signed short _Fract f3 = (signed short _Fract)a3;  /* Overflow negative */
        
        /* Use volatile to force materialization */
        volatile signed short _Fract vf1 = f1;
        volatile signed short _Fract vf2 = f2;
        volatile signed short _Fract vf3 = f3;
        
        checksum += (int)(vf1 * 1000r);
        checksum += (int)(vf2 * 1000r);
        checksum += (int)(vf3 * 1000r);
    }
    
    /* Test 2: Unsigned conversions with boundary values */
    {
        /* Maximum unsigned short _Fract */
        volatile unsigned short _Fract max_uf = get_ufract_max();
        
        /* Create values that test unsigned overflow */
        const unsigned _Accum ua1 = (unsigned _Accum)max_uf * 2.0uk;  /* Above max */
        const unsigned _Accum ua2 = (unsigned _Accum)max_uf * 1.0uk;  /* At max */
        const unsigned _Accum ua3 = (unsigned _Accum)max_uf * 0.5uk;  /* Below max */
        
        /* These conversions should trigger unsigned range checks */
        unsigned short _Fract uf1 = (unsigned short _Fract)ua1;  /* Overflow */
        unsigned short _Fract uf2 = (unsigned short _Fract)ua2;  /* At boundary */
        unsigned short _Fract uf3 = (unsigned short _Fract)ua3;  /* Within range */
        
        volatile unsigned short _Fract vuf1 = uf1;
        volatile unsigned short _Fract vuf2 = uf2;
        volatile unsigned short _Fract vuf3 = uf3;
        
        checksum += (int)(vuf1 * 1000ur);
        checksum += (int)(vuf2 * 1000ur);
        checksum += (int)(vuf3 * 1000ur);
    }
    
    /* Test 3: Mixed signed/unsigned with saturation */
    {
        /* Test saturation behavior with boundary values */
        _Sat signed short _Fract sf_sat1 = 1.5r;  /* Will saturate to MAX */
        _Sat signed short _Fract sf_sat2 = -1.5r; /* Will saturate to MIN */
        
        /* Convert between saturated and non-saturated types */
        signed short _Fract sf1 = sf_sat1;
        signed short _Fract sf2 = sf_sat2;
        
        /* Arithmetic with saturated types */
        _Sat signed _Accum sa1 = 0.5k;
        _Sat signed _Accum sa2 = 0.6k;
        _Sat signed _Accum sa_sum = sa1 + sa2;
        
        /* Conversion that may trigger range checking */
        _Sat signed short _Fract sf3 = (_Sat signed short _Fract)sa_sum;
        
        volatile _Sat signed short _Fract vsf1 = sf_sat1;
        volatile _Sat signed short _Fract vsf2 = sf_sat2;
        volatile signed short _Fract vsf3 = sf3;
        
        checksum += (int)(vsf1 * 1000r);
        checksum += (int)(vsf2 * 1000r);
        checksum += (int)(vsf3 * 1000r);
    }
    
    /* Test 4: Complex constant expressions that force compile-time evaluation */
    {
        /* These should be evaluated at compile-time, triggering the constant folding logic */
        const signed _Accum ca1 = (signed _Accum)0.999969482421875r * 1.0001k;
        const signed _Accum ca2 = (signed _Accum)-0.999969482421875r * 1.0001k;
        
        /* Convert to narrower type - should trigger range checks */
        const signed short _Fract cf1 = (signed short _Fract)ca1;
        const signed short _Fract cf2 = (signed short _Fract)ca2;
        
        /* Use in array initializer to force compile-time evaluation */
        static const signed short _Fract arr[2] = {cf1, cf2};
        
        volatile signed short _Fract vcf1 = arr[0];
        volatile signed short _Fract vcf2 = arr[1];
        
        checksum += (int)(vcf1 * 1000r);
        checksum += (int)(vcf2 * 1000r);
    }
    
    /* Test 5: Loop with fixed-point accumulation */
    {
        /* Small loop to create semi-constant values */
        signed _Accum acc = 0.0k;
        for (int i = 0; i < 3; i++) {
            acc += 0.4k;
        }
        
        /* Convert accumulated value - may trigger range check */
        signed short _Fract f_acc = (signed short _Fract)acc;
        
        /* Test with minimum values */
        signed long _Accum min_val = get_saccum_min();
        signed short _Fract f_min = (signed short _Fract)min_val;
        
        volatile signed short _Fract vf_acc = f_acc;
        volatile signed short _Fract vf_min = f_min;
        
        checksum += (int)(vf_acc * 1000r);
        checksum += (int)(vf_min * 1000r);
    }
    
    /* Test 6: Direct boundary value tests */
    {
        /* Values designed to exercise the specific comparisons in the uncovered code:
           a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)) */
        
        /* For signed short _Fract (Q0.15), i_f_bits = 15
           max_r = 0, max_s = (1 << 15) - 1 = 32767
           We need values where high part > 0 OR (high == 0 && low > 32767) */
        
        /* Case 1: high > 0 */
        signed _Accum b1 = 2.0k;  /* High part will be > 0 when converted */
        
        /* Case 2: high == 0 && low > max_s */
        /* Create a value with high=0, low=32768 (max_s + 1) */
        signed _Accum b2 = (signed _Accum)32768.0 / 32768.0k;  /* 1.0 */
        b2 = b2 * 1.0001k;  /* Slightly > 1.0 to get low > 32767 */
        
        /* Convert to trigger checks */
        signed short _Fract fb1 = (signed short _Fract)b1;
        signed short _Fract fb2 = (signed short _Fract)b2;
        
        volatile signed short _Fract vfb1 = fb1;
        volatile signed short _Fract vfb2 = fb2;
        
        checksum += (int)(vfb1 * 1000r);
        checksum += (int)(vfb2 * 1000r);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
