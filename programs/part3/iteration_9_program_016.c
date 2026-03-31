/* fixed-point-test.c
 * Tests GCC's fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Opaque functions to prevent constant folding elimination */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX for signed short _Fract */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX for unsigned short _Fract */
}

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -1.99999237060546875k; /* Close to MIN for 16-bit fractional */
}

__attribute__((noinline)) signed _Accum get_saccum_max(void) {
    return 1.99999237060546875k;  /* Close to MAX for 16-bit fractional */
}

/* Force materialization of intermediate results */
volatile signed _Accum volatile_accum;

int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed fract boundary conversions */
    printf("Test 1: Signed fract boundaries\n");
    {
        const signed short _Fract max_sf = 0.999969482421875r;  /* MAX */
        const signed short _Fract min_sf = -1.0r;               /* MIN */
        
        /* These should trigger range checks during conversion */
        volatile signed _Accum a1 = (signed _Accum)max_sf;
        volatile signed _Accum a2 = (signed _Accum)min_sf;
        
        /* Conversion that requires range checking */
        signed char _Fract cf1 = (signed char _Fract)a1;  /* Narrowing */
        signed char _Fract cf2 = (signed char _Fract)a2;  /* Narrowing */
        
        checksum += (int)(cf1 * 1000r);
        checksum += (int)(cf2 * 1000r);
    }
    
    /* Test 2: Unsigned fract overflow scenarios */
    printf("Test 2: Unsigned fract overflow\n");
    {
        const unsigned short _Fract max_uf = 0.999969482421875ur;
        
        /* Operations near maximum */
        unsigned short _Fract uf1 = max_uf;
        unsigned short _Fract uf2 = max_uf * 0.999ur;  /* Still in range */
        
        /* This multiplication might overflow when converted */
        volatile unsigned _Accum ua1 = (unsigned _Accum)uf1 * 1.5uk;
        
        /* Convert to narrower type - triggers range check */
        unsigned char _Fract ucf = (unsigned char _Fract)ua1;
        
        checksum += (int)(ucf * 2000ur);
    }
    
    /* Test 3: Complex constant expressions */
    printf("Test 3: Complex constant expressions\n");
    {
        /* Compile-time constants that exercise range logic */
        constexpr signed _Accum ca1 = (signed _Accum)0.75r * 2.5k;
        constexpr signed _Accum ca2 = (signed _Accum)(-0.875r) * 2.25k;
        
        /* These conversions require range checking */
        const signed short _Fract sf1 = (signed short _Fract)ca1;
        const signed short _Fract sf2 = (signed short _Fract)ca2;
        
        /* Mix with volatile to force materialization */
        volatile_accum = ca1 + ca2;
        signed short _Fract sf3 = (signed short _Fract)volatile_accum;
        
        checksum += (int)(sf1 * 3000r);
        checksum += (int)(sf2 * 3000r);
        checksum += (int)(sf3 * 3000r);
    }
    
    /* Test 4: Saturation qualifier testing */
    printf("Test 4: Saturation testing\n");
    {
        _Sat signed short _Fract ssf1 = 0.999969482421875sr;  /* At max */
        _Sat signed short _Fract ssf2 = -1.0sr;               /* At min */
        
        /* These should saturate rather than overflow */
        _Sat signed short _Fract ssf3 = ssf1 + 0.0001sr;
        _Sat signed short _Fract ssf4 = ssf2 - 0.0001sr;
        
        /* Convert saturated to non-saturated */
        signed short _Fract sf4 = ssf3;
        signed short _Fract sf5 = ssf4;
        
        checksum += (int)(sf4 * 4000r);
        checksum += (int)(sf5 * 4000r);
    }
    
    /* Test 5: Loop-based boundary testing */
    printf("Test 5: Loop-based boundary testing\n");
    {
        /* Small loop to prevent complete constant folding */
        for (int i = 0; i < 3; i++) {
            signed _Accum accum = get_saccum_min() + (signed _Accum)(i * 0.5k);
            
            /* Conversion that requires range checking */
            signed short _Fract sf = (signed short _Fract)accum;
            
            /* Mix with opaque function result */
            signed short _Fract sf_max = get_sfract_max();
            signed short _Fract result = sf * sf_max;
            
            checksum += (int)(result * (1000r + i * 100r));
        }
    }
    
    /* Test 6: Extreme boundary values */
    printf("Test 6: Extreme boundary values\n");
    {
        /* Values just beyond representable range */
        const signed _Accum just_above_max = get_saccum_max() * 1.0001k;
        const signed _Accum just_below_min = get_saccum_min() * 1.0001k;
        
        /* These conversions should trigger range checks */
        volatile signed short _Fract sf6 = (signed short _Fract)just_above_max;
        volatile signed short _Fract sf7 = (signed short _Fract)just_below_min;
        
        /* Unsigned overflow case */
        unsigned _Accum ua_max = (unsigned _Accum)get_ufract_max() * 2.0uk;
        unsigned char _Fract ucf2 = (unsigned char _Fract)ua_max;
        
        checksum += (int)(sf6 * 5000r);
        checksum += (int)(sf7 * 5000r);
        checksum += (int)(ucf2 * 5000ur);
    }
    
    /* Test 7: Mixed precision arithmetic */
    printf("Test 7: Mixed precision arithmetic\n");
    {
        signed long _Accum la1 = 1.999999999999999k;  /* Near max for long accum */
        signed short _Fract sf8 = 0.999r;
        
        /* Mixed operation followed by narrowing conversion */
        signed long _Accum la2 = la1 * (signed long _Accum)sf8;
        signed short _Fract sf9 = (signed short _Fract)la2;
        
        /* Another mixed operation */
        signed _Accum accum3 = (signed _Accum)sf8 * 1.5k;
        signed char _Fract cf3 = (signed char _Fract)accum3;
        
        checksum += (int)(sf9 * 6000r);
        checksum += (int)(cf3 * 6000r);
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
