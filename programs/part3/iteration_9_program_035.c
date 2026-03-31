/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract (Q0.15) */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract (U0.16) */
}

__attribute__((noinline)) signed long _Accum get_saccum_min(void) {
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) volatile void consume(volatile void *ptr) {
    /* Force materialization of value */
    (void)ptr;
}

int main(void) {
    volatile int checksum = 0;
    
    /* Test 1: Signed conversions with boundary values */
    {
        /* Maximum signed short _Fract */
        const signed short _Fract max_sf = 0.999969482421875r;
        
        /* Convert to narrower type - should trigger range check */
        signed char _Fract narrow_sf = (signed char _Fract)max_sf;
        
        /* Multiply to exceed range */
        volatile signed short _Fract v1 = max_sf;
        volatile signed short _Fract v2 = 1.0001r;  /* Slightly > 1 */
        signed short _Fract prod = v1 * v2;  /* May overflow */
        
        /* Convert overflowed product */
        signed char _Fract conv1 = (signed char _Fract)prod;
        checksum += (int)(conv1 * 1000r);
        
        consume(&conv1);
    }
    
    /* Test 2: Unsigned conversions with overflow */
    {
        /* Maximum unsigned _Fract */
        const unsigned short _Fract max_uf = 0.999969482421875ur;
        
        /* Values just beyond maximum */
        volatile unsigned short _Fract v3 = max_uf;
        volatile unsigned short _Fract v4 = 0.0001ur;  /* Small increment */
        
        /* Addition that overflows */
        unsigned short _Fract sum = v3 + v4;
        
        /* Convert to narrower unsigned type */
        unsigned char _Fract conv2 = (unsigned char _Fract)sum;
        checksum += (int)(conv2 * 1000ur);
        
        /* Test with compile-time constant expression */
        constexpr unsigned short _Fract c1 = 0.999969482421875ur;
        constexpr unsigned short _Fract c2 = 0.000030517578125ur; /* 1 LSB */
        constexpr unsigned short _Fract c3 = c1 + c2;  /* Exactly at overflow boundary */
        
        unsigned char _Fract conv3 = (unsigned char _Fract)c3;
        checksum += (int)(conv3 * 1000ur);
        
        consume(&conv2);
        consume(&conv3);
    }
    
    /* Test 3: Mixed signed/unsigned conversions */
    {
        /* Large signed _Accum value */
        volatile signed long _Accum sla = 5000.0k;
        volatile signed long _Accum slb = 5000.0k;
        
        /* Multiplication that exceeds _Fract range */
        signed long _Accum slprod = sla * slb;
        
        /* Convert to signed _Fract - should trigger range check */
        signed short _Fract conv4 = (signed short _Fract)slprod;
        checksum += (int)(conv4 * 1000r);
        
        /* Negative overflow test */
        volatile signed long _Accum slc = -5000.0k;
        signed long _Accum slprod2 = slc * slb;  /* Large negative */
        signed short _Fract conv5 = (signed short _Fract)slprod2;
        checksum += (int)(conv5 * 1000r);
        
        consume(&conv4);
        consume(&conv5);
    }
    
    /* Test 4: Saturation qualifier tests */
    {
        /* Saturated types have different overflow behavior */
        volatile signed short _Sat _Fract sat_sf1 = 0.999969482421875r;
        volatile signed short _Sat _Fract sat_sf2 = 0.0001r;
        
        /* This should saturate rather than wrap */
        signed short _Sat _Fract sat_sum = sat_sf1 + sat_sf2;
        
        /* Convert saturated result to non-saturated type */
        signed short _Fract conv6 = (signed short _Fract)sat_sum;
        checksum += (int)(conv6 * 1000r);
        
        /* Test with unsigned saturated */
        volatile unsigned short _Sat _Fract sat_uf1 = 0.999969482421875ur;
        unsigned short _Sat _Fract sat_uf2 = sat_uf1 + 0.0001ur;
        
        unsigned short _Fract conv7 = (unsigned short _Fract)sat_uf2;
        checksum += (int)(conv7 * 1000ur);
        
        consume(&conv6);
        consume(&conv7);
    }
    
    /* Test 5: Complex constant expressions */
    {
        /* Force constant folding with boundary values */
        const signed _Accum ca1 = (signed _Accum)0.5r;
        const signed _Accum ca2 = (signed _Accum)2.0r;
        const signed _Accum ca3 = (signed _Accum)0.999969482421875r;
        
        /* Expression that should be evaluated at compile time */
        const signed _Accum cexpr = ca1 * ca2 * ca3 * 1000.0k;
        
        /* Convert to different precision */
        signed short _Fract conv8 = (signed short _Fract)cexpr;
        checksum += (int)(conv8 * 1000r);
        
        /* Another complex expression */
        constexpr unsigned _Accum uc1 = 0.999969482421875ur;
        constexpr unsigned _Accum uc2 = 1.000030517578125ur;  /* 1 + 1 LSB */
        constexpr unsigned _Accum ucprod = uc1 * uc2;
        
        unsigned short _Fract conv9 = (unsigned short _Fract)ucprod;
        checksum += (int)(conv9 * 1000ur);
        
        consume(&conv8);
        consume(&conv9);
    }
    
    /* Test 6: Loop with fixed-point computations */
    {
        signed short _Fract accum = 0.0r;
        
        /* Small loop - values are mostly constant but control flow exists */
        for (int i = 0; i < 3; i++) {
            volatile signed short _Fract increment = 0.333333333333333r;
            accum += increment;
            
            /* Convert intermediate result to different type */
            signed char _Fract temp = (signed char _Fract)accum;
            checksum += (int)(temp * 1000r);
        }
        
        /* Final conversion that might trigger range check */
        signed char _Fract final_conv = (signed char _Fract)accum;
        checksum += (int)(final_conv * 1000r);
        
        consume(&final_conv);
    }
    
    /* Test 7: Exact boundary value tests */
    {
        /* Test with values at exact bit boundaries */
        
        /* For signed short _Fract (Q0.15), max is (2^15-1)/2^15 = 32767/32768 */
        const signed short _Fract exact_max = 0.999969482421875r;  /* 32767/32768 */
        
        /* One LSB less than max */
        const signed short _Fract almost_max = 0.99993896484375r;  /* 32766/32768 */
        
        /* Convert both to narrower type */
        signed char _Fract conv10 = (signed char _Fract)exact_max;
        signed char _Fract conv11 = (signed char _Fract)almost_max;
        
        checksum += (int)(conv10 * 1000r);
        checksum += (int)(conv11 * 1000r);
        
        /* Test negative boundary */
        const signed short _Fract exact_min = -1.0r;  /* -32768/32768 */
        const signed short _Fract almost_min = -0.999969482421875r;  /* -32767/32768 */
        
        signed char _Fract conv12 = (signed char _Fract)exact_min;
        signed char _Fract conv13 = (signed char _Fract)almost_min;
        
        checksum += (int)(conv12 * 1000r);
        checksum += (int)(conv13 * 1000r);
        
        consume(&conv10);
        consume(&conv11);
        consume(&conv12);
        consume(&conv13);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
