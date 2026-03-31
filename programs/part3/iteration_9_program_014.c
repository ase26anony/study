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

__attribute__((noinline)) signed _Accum get_saccum_val(signed _Accum x) {
    volatile signed _Accum dummy = x;
    return dummy;
}

int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed conversions with boundary values */
    {
        /* Maximum signed short _Fract */
        const signed short _Fract max_sfract = 0.999969482421875r;
        
        /* Try to convert to narrower type - should trigger range check */
        volatile signed char _Fract target1 = max_sfract;
        checksum += (int)(target1 * 1000r);
        
        /* Value just beyond maximum (by adding smallest increment) */
        const signed short _Fract beyond_max = max_sfract + 0.000030517578125r; /* +1 LSB */
        volatile signed char _Fract target2 = beyond_max;  /* Should trigger overflow check */
        checksum += (int)(target2 * 1000r);
    }
    
    /* Test 2: Unsigned conversions with overflow */
    {
        /* Maximum unsigned _Fract */
        const unsigned _Fract max_ufract = 0.99999999976716935634613037109375ur;
        
        /* Convert to narrower unsigned type */
        volatile unsigned short _Fract target3 = max_ufract;
        checksum += (int)(target3 * 1000ur);
        
        /* Create value that would overflow when converted */
        unsigned _Fract large_val = max_ufract;
        /* Use opaque function to prevent compile-time elimination */
        unsigned short _Fract temp = get_ufract_max();
        large_val = large_val + temp * 0.5ur;
        volatile unsigned short _Fract target4 = large_val;
        checksum += (int)(target4 * 1000ur);
    }
    
    /* Test 3: Signed _Accum to _Fract conversions */
    {
        /* Large _Accum value that will overflow _Fract range */
        const signed long _Accum large_accum = 100.0k;
        
        /* Convert to _Fract - should trigger range checking */
        volatile signed _Fract target5 = (signed _Fract)large_accum;
        checksum += (int)(target5 * 1000r);
        
        /* Negative overflow case */
        const signed long _Accum neg_large_accum = -100.0k;
        volatile signed _Fract target6 = (signed _Fract)neg_large_accum;
        checksum += (int)(target6 * 1000r);
    }
    
    /* Test 4: Complex constant expressions */
    {
        /* Create compile-time constant expression that needs range checking */
        constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;
        constexpr signed _Accum c2 = (signed _Accum)0.75r * 2.0r;
        constexpr signed _Accum c3 = c1 + c2;  /* 1.5 + 1.5 = 3.0 */
        
        /* Convert to narrower type */
        volatile signed short _Fract target7 = (signed short _Fract)c3;
        checksum += (int)(target7 * 1000r);
        
        /* More complex expression */
        constexpr signed long _Accum c4 = (signed long _Accum)0.1k * 20.0k;
        volatile signed _Fract target8 = (signed _Fract)c4;
        checksum += (int)(target8 * 1000r);
    }
    
    /* Test 5: Mixed signed/unsigned conversions */
    {
        /* Positive signed value to unsigned */
        const signed _Accum pos_signed = 0.75k;
        volatile unsigned _Fract target9 = (unsigned _Fract)pos_signed;
        checksum += (int)(target9 * 1000ur);
        
        /* Negative signed value to unsigned - should trigger special handling */
        const signed _Accum neg_signed = -0.25k;
        volatile unsigned _Fract target10 = (unsigned _Fract)neg_signed;
        checksum += (int)(target10 * 1000ur);
    }
    
    /* Test 6: Saturation qualifier tests */
    {
        /* Non-saturated operation that might overflow */
        signed _Accum ns1 = 0.9k;
        signed _Accum ns2 = 0.9k;
        signed _Accum ns_product = ns1 * ns2;  /* 0.81 - safe */
        
        /* Convert to saturated type */
        volatile signed _Sat _Fract target11 = (signed _Sat _Fract)ns_product;
        checksum += (int)(target11 * 1000r);
        
        /* Operation that would overflow without saturation */
        signed _Sat _Accum sa1 = 0.999k;
        signed _Sat _Accum sa2 = 1.5k;
        signed _Sat _Accum sa_product = sa1 * sa2;  /* Would saturate */
        
        volatile signed _Sat _Fract target12 = (signed _Sat _Fract)sa_product;
        checksum += (int)(target12 * 1000r);
    }
    
    /* Test 7: Loop-based computations to prevent optimization */
    {
        signed short _Fract accum = 0.0r;
        /* Small loop - values are mostly constant but control flow exists */
        for (int i = 0; i < 3; i++) {
            signed _Accum val = (signed _Accum)(0.3k + i * 0.1k);
            /* Use opaque function */
            val = get_saccum_val(val);
            
            /* Convert with potential range issues */
            signed short _Fract converted = (signed short _Fract)val;
            accum += converted;
        }
        checksum += (int)(accum * 1000r);
    }
    
    /* Test 8: Minimum value boundary checks */
    {
        /* Minimum signed values */
        const signed long _Accum min_saccum = get_saccum_min();
        
        /* Convert minimum to _Fract - should trigger negative range check */
        volatile signed _Fract target13 = (signed _Fract)min_saccum;
        checksum += (int)(target13 * 1000r);
        
        /* Value just below minimum for short _Fract */
        const signed short _Fract min_sfract = -1.0r;
        const signed short _Fract below_min = min_sfract - 0.000030517578125r; /* -1 - 1 LSB */
        volatile signed char _Fract target14 = below_min;
        checksum += (int)(target14 * 1000r);
    }
    
    /* Test 9: Multiplication that exceeds range */
    {
        /* Multiply two values that produce result beyond target range */
        const signed _Accum a1 = 0.9k;
        const signed _Accum a2 = 1.2k;
        const signed _Accum product = a1 * a2;  /* 1.08 */
        
        /* Convert to _Fract (max ~0.999...) */
        volatile signed _Fract target15 = (signed _Fract)product;
        checksum += (int)(target15 * 1000r);
    }
    
    /* Test 10: Unsigned overflow with addition */
    {
        unsigned _Fract u1 = get_ufract_max();
        /* Add a small value to cause overflow */
        unsigned _Fract u2 = u1 + 0.00000000023283064365386962890625ur; /* +1 LSB */
        
        /* Convert to narrower type */
        volatile unsigned short _Fract target16 = (unsigned short _Fract)u2;
        checksum += (int)(target16 * 1000ur);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
