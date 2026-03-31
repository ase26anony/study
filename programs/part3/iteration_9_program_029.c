/* Test program for fixed-point range checking logic */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-test.c -o fixed-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -32768.999969482421875lk; /* MIN for signed long _Accum */
}

__attribute__((noinline)) signed _Accum get_saccum_half(void) {
    return 0.5k;
}

__attribute__((noinline)) volatile void consume(volatile void *ptr) {
    /* Force materialization */
    (void)ptr;
}

int main(void) {
    volatile int checksum = 0;
    
    /* ====== Test 1: Signed fract boundary testing ====== */
    {
        /* Maximum representable signed short _Fract */
        const signed short _Fract max_sfract = 0.999969482421875r;
        const signed short _Fract min_sfract = -1.0r;
        
        /* Values near boundaries */
        const signed short _Fract just_below_max = 0.99993896484375r;  /* MAX - 1LSB */
        const signed short _Fract just_above_max = 1.0r;  /* Overflow */
        
        /* Operations that should trigger range checks */
        volatile signed _Fract v1 = max_sfract;
        volatile signed _Fract v2 = just_below_max;
        
        /* This addition might overflow depending on precision */
        volatile signed _Fract sum = v1 + v2;
        checksum += (int)(sum * 1000);
        
        /* Conversion to narrower type */
        volatile signed short _Fract narrow = (signed short _Fract)sum;
        checksum += (int)(narrow * 1000);
    }
    
    /* ====== Test 2: Unsigned fract overflow ====== */
    {
        const unsigned short _Fract max_ufract = 0.999969482421875ur;
        const unsigned short _Fract quarter = 0.25ur;
        
        /* Multiplication that exceeds range */
        volatile unsigned _Fract product = max_ufract * max_ufract;
        
        /* Conversion to narrower unsigned type */
        volatile unsigned short _Fract narrow_unsigned = (unsigned short _Fract)product;
        checksum += (int)(narrow_unsigned * 1000);
        
        /* Test with volatile to force computation */
        volatile unsigned _Fract v = max_ufract;
        for (int i = 0; i < 3; i++) {
            v = v + quarter;
        }
        volatile unsigned short _Fract converted = (unsigned short _Fract)v;
        checksum += (int)(converted * 1000);
    }
    
    /* ====== Test 3: Accum to Fract conversion with range checking ====== */
    {
        /* signed long _Accum has more range than signed short _Fract */
        const signed long _Accum large_accum = 2.0lk;  /* > 1.0, will overflow when converting to fract */
        const signed long _Accum small_accum = 0.5lk;
        
        /* These conversions should trigger range checks */
        volatile signed short _Fract f1 = (signed short _Fract)large_accum;
        volatile signed short _Fract f2 = (signed short _Fract)small_accum;
        
        checksum += (int)(f1 * 1000);
        checksum += (int)(f2 * 1000);
        
        /* Mixed precision arithmetic */
        volatile signed _Accum a1 = 1.5k;
        volatile signed _Accum a2 = 0.75k;
        volatile signed _Accum a3 = a1 * a2;  /* 1.125 */
        
        /* Convert to narrower fract - should fit */
        volatile signed short _Fract f3 = (signed short _Fract)a3;
        checksum += (int)(f3 * 1000);
    }
    
    /* ====== Test 4: Boundary value testing with opaque functions ====== */
    {
        /* Use opaque functions to get boundary values */
        volatile signed short _Fract sf_max = get_sfract_max();
        volatile unsigned short _Fract uf_max = get_ufract_max();
        volatile signed long _Accum sl_min = get_saccum_min();
        
        /* Operations near boundaries */
        volatile signed short _Fract sf_near_max = sf_max + (signed short _Fract)0.0001r;
        volatile unsigned short _Fract uf_near_max = uf_max + (unsigned short _Fract)0.0001ur;
        
        /* These should trigger overflow checks */
        volatile signed _Fract conv1 = (signed _Fract)sf_near_max;
        volatile unsigned _Fract conv2 = (unsigned _Fract)uf_near_max;
        
        checksum += (int)(conv1 * 1000);
        checksum += (int)(conv2 * 1000);
        
        /* Negative overflow test */
        volatile signed short _Fract from_negative = (signed short _Fract)sl_min;
        checksum += (int)(from_negative * 1000);
    }
    
    /* ====== Test 5: Saturation qualifier testing ====== */
    {
        /* Saturated types have different overflow behavior */
        _Sat signed short _Fract sat_max = 0.999969482421875r;
        _Sat unsigned short _Fract sat_umax = 0.999969482421875ur;
        
        /* These should saturate rather than overflow */
        volatile _Sat signed short _Fract sat_sum = sat_max + sat_max;
        volatile _Sat unsigned short _Fract sat_usum = sat_umax + sat_umax;
        
        /* Convert between saturated and non-saturated */
        volatile signed short _Fract from_sat = sat_sum;
        volatile unsigned short _Fract from_usat = sat_usum;
        
        checksum += (int)(from_sat * 1000);
        checksum += (int)(from_usat * 1000);
        
        /* Mixed saturation */
        volatile _Sat signed _Accum sat_accum = 10.0k;
        volatile signed short _Fract from_sat_accum = (signed short _Fract)sat_accum;
        checksum += (int)(from_sat_accum * 1000);
    }
    
    /* ====== Test 6: Complex constant expressions ====== */
    {
        /* Complex compile-time expressions */
        constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;  /* 1.5 */
        constexpr signed _Accum c2 = (signed _Accum)0.75r * 2.0r; /* 1.5 */
        constexpr signed _Accum c3 = c1 + c2;  /* 3.0 */
        
        /* These conversions should trigger range checks */
        volatile signed short _Fract f1 = (signed short _Fract)c3;  /* 3.0 > 1.0, overflow */
        volatile signed short _Fract f2 = (signed short _Fract)(c1 * get_saccum_half()); /* 0.75, should fit */
        
        checksum += (int)(f1 * 1000);
        checksum += (int)(f2 * 1000);
        
        /* More complex expression */
        constexpr unsigned _Accum uc1 = (unsigned _Accum)0.8ur * 1.2ur; /* 0.96 */
        constexpr unsigned _Accum uc2 = (unsigned _Accum)0.9ur * 1.1ur; /* 0.99 */
        constexpr unsigned _Accum uc3 = uc1 + uc2;  /* 1.95 > 1.0 */
        
        volatile unsigned short _Fract uf = (unsigned short _Fract)uc3;
        checksum += (int)(uf * 1000);
    }
    
    /* ====== Test 7: Loop-based computations ====== */
    {
        /* Use loops to create values that are mostly constant but have control flow */
        volatile signed _Accum acc = 0.0k;
        for (int i = 0; i < 4; i++) {
            acc = acc + 0.3k;
        }
        /* acc = 1.2, which exceeds fract range */
        volatile signed short _Fract loop_result = (signed short _Fract)acc;
        checksum += (int)(loop_result * 1000);
        
        /* Another loop with unsigned */
        volatile unsigned _Accum uacc = 0.0uk;
        for (int i = 0; i < 5; i++) {
            uacc = uacc + 0.25uk;
        }
        /* uacc = 1.25 > 1.0 */
        volatile unsigned short _Fract uloop_result = (unsigned short _Fract)uacc;
        checksum += (int)(uloop_result * 1000);
    }
    
    /* ====== Test 8: Extreme boundary cases ====== */
    {
        /* Test with values at exact boundaries */
        const signed long _Accum exact_max_fract = 1.0lk;  /* Exactly the maximum for fract */
        const signed long _Accum just_over = 1.0000001lk;  /* Just over */
        const signed long _Accum just_under = 0.9999999lk; /* Just under */
        
        volatile signed short _Fract e1 = (signed short _Fract)exact_max_fract;
        volatile signed short _Fract e2 = (signed short _Fract)just_over;
        volatile signed short _Fract e3 = (signed short _Fract)just_under;
        
        checksum += (int)(e1 * 1000);
        checksum += (int)(e2 * 1000);
        checksum += (int)(e3 * 1000);
        
        /* For unsigned, test zero boundary */
        const unsigned long _Accum zero = 0.0ulk;
        const unsigned long _Accum tiny = 0.0000001ulk;
        const signed long _Accum negative = -0.5lk;  /* Will underflow when converting to unsigned */
        
        volatile unsigned short _Fract z1 = (unsigned short _Fract)zero;
        volatile unsigned short _Fract z2 = (unsigned short _Fract)tiny;
        volatile unsigned short _Fract z3 = (unsigned short _Fract)negative;  /* Should underflow */
        
        checksum += (int)(z1 * 1000);
        checksum += (int)(z2 * 1000);
        checksum += (int)(z3 * 1000);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
