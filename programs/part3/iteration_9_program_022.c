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
    return -9223372036854775.807k; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) volatile signed _Fract consume_sfract(signed _Fract x) {
    volatile signed _Fract dummy = x;
    (void)dummy;
    return x;
}

__attribute__((noinline)) volatile unsigned _Fract consume_ufract(unsigned _Fract x) {
    volatile unsigned _Fract dummy = x;
    (void)dummy;
    return x;
}

int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed fixed-point conversions with boundary values */
    {
        /* Initialize with maximum representable values */
        const signed short _Fract sf_max = 0.999969482421875r;  /* Q0.15: 32767/32768 */
        const signed short _Accum sa_max = 32767.999969482421875k; /* Q15.16 */
        
        /* Operations that should trigger range checks */
        volatile signed _Fract v1;
        
        /* This multiplication produces a value that needs range checking
           when converted to narrower type */
        v1 = sf_max * 1.5r;  /* Should overflow Q0.15 range */
        checksum += (int)(v1 * 1000r);
        
        /* Convert from _Accum to _Fract - requires range checking */
        const signed _Fract f_from_acc = (signed _Fract)sa_max;
        checksum += (int)(f_from_acc * 1000r);
        
        /* Near-boundary arithmetic */
        const signed _Fract f1 = 0.9r;
        const signed _Fract f2 = 0.8r;
        signed short _Fract narrow_result;
        
        /* Sum > 1.0, needs checking when assigned to short _Fract */
        narrow_result = (signed short _Fract)(f1 + f2);
        checksum += (int)(narrow_result * 1000r);
    }
    
    /* Test 2: Unsigned fixed-point with overflow beyond maximum */
    {
        const unsigned short _Fract uf_max = 0.999969482421875ur; /* U0.16: 65535/65536 */
        const unsigned _Accum ua_large = 2.0uk;  /* > 1.0 */
        
        volatile unsigned _Fract v2;
        
        /* This will overflow when converted to _Fract */
        v2 = (unsigned _Fract)ua_large;
        checksum += (int)(v2 * 1000ur);
        
        /* Multiplication that exceeds range */
        unsigned _Fract uf1 = 0.9ur;
        unsigned _Fract uf2 = 1.1ur;  /* Actually clamped to 1.0ur */
        unsigned short _Fract u_narrow;
        
        u_narrow = (unsigned short _Fract)(uf1 * uf2);
        checksum += (int)(u_narrow * 1000ur);
    }
    
    /* Test 3: Mixed signed/unsigned conversions */
    {
        signed _Accum sa_negative = -0.5k;
        unsigned _Fract uf_from_signed;
        
        /* Negative to unsigned conversion - triggers range checking */
        uf_from_signed = (unsigned _Fract)sa_negative;
        checksum += (int)(uf_from_signed * 1000ur);
        
        /* Large positive signed to unsigned */
        signed _Accum sa_large = 1.5k;
        uf_from_signed = (unsigned _Fract)sa_large;
        checksum += (int)(uf_from_signed * 1000ur);
    }
    
    /* Test 4: Saturation qualifier tests */
    {
        _Sat signed short _Fract sat_sf;
        _Sat unsigned short _Fract sat_uf;
        
        /* These should saturate rather than overflow */
        sat_sf = get_sfract_max() + 0.1r;  /* Should saturate to MAX */
        checksum += (int)(sat_sf * 1000r);
        
        sat_uf = get_ufract_max() + 0.1ur; /* Should saturate to MAX */
        checksum += (int)(sat_uf * 1000ur);
        
        /* Negative saturation */
        _Sat signed _Fract sat_neg = -1.5r;  /* Should saturate to -1.0 */
        checksum += (int)(sat_neg * 1000r);
    }
    
    /* Test 5: Complex constant expressions forcing compile-time evaluation */
    {
        /* These should be evaluated at compile-time, triggering the range checking */
        constexpr signed _Accum ca1 = (signed _Accum)0.75r * 2.0r;  /* 1.5 */
        constexpr signed short _Fract cf1 = (signed short _Fract)ca1;  /* Needs range check */
        
        constexpr unsigned _Accum ca2 = (unsigned _Accum)0.8ur * 1.5ur;
        constexpr unsigned short _Fract cf2 = (unsigned short _Fract)ca2;
        
        /* Use volatile to force materialization */
        volatile signed short _Fract v_cf1 = cf1;
        volatile unsigned short _Fract v_cf2 = cf2;
        
        checksum += (int)(v_cf1 * 1000r);
        checksum += (int)(v_cf2 * 1000ur);
    }
    
    /* Test 6: Loop with boundary accumulation */
    {
        signed _Fract accum = 0.0r;
        const signed _Fract increment = 0.3r;
        
        /* Small loop to create near-boundary values */
        for (int i = 0; i < 4; i++) {
            accum += increment;
        }
        
        /* Convert accumulated value (1.2) to short _Fract (max 0.999...) */
        signed short _Fract narrowed = (signed short _Fract)accum;
        checksum += (int)(narrowed * 1000r);
    }
    
    /* Test 7: Exact boundary value testing */
    {
        /* Values at exact boundaries */
        const signed _Fract exact_max = 0.99999999976716935634613037109375r;
        const signed _Fract just_above_max = 1.00000000023283064365386962890625r;
        const signed _Fract just_below_max = 0.9999999995343387126922607421875r;
        
        volatile signed short _Fract v3, v4, v5;
        
        v3 = (signed short _Fract)exact_max;
        v4 = (signed short _Fract)just_above_max;  /* Should trigger overflow check */
        v5 = (signed short _Fract)just_below_max;
        
        checksum += (int)(v3 * 1000r);
        checksum += (int)(v4 * 1000r);
        checksum += (int)(v5 * 1000r);
    }
    
    /* Test 8: Bit-exact boundary for i_f_bits precision */
    {
        /* Create values that exercise the specific bit patterns in the uncovered code */
        /* max_s = max_s.zext(i_f_bits) and min_s = min_s.alshift(i_f_bits, ...) */
        
        /* For 16 fractional bits (short _Fract), create values around 2^16 boundaries */
        signed long _Accum large_val = 65536.0k;  /* 2^16 exactly */
        signed short _Fract from_large = (signed short _Fract)large_val;  /* Should overflow */
        
        checksum += (int)(from_large * 1000r);
        
        /* Negative boundary */
        signed long _Accum neg_large = -65537.0k;
        signed short _Fract from_neg = (signed short _Fract)neg_large;  /* Should underflow */
        
        checksum += (int)(from_neg * 1000r);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
