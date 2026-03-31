/* fixed-point-test.c */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

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
    /* Force materialization */
    (void)ptr;
}

int main(void) {
    volatile int checksum = 0;
    
    /* Test 1: Signed conversions with boundary values */
    {
        /* Maximum signed short _Fract */
        const signed short _Fract max_sfract = 0.999969482421875r;
        
        /* Convert to narrower type - should trigger max_r/max_s checks */
        signed char _Fract narrow1 = max_sfract;  /* Q0.7 */
        
        /* One LSB beyond max */
        const signed short _Fract beyond_max = (signed short _Fract)1.0r;
        signed char _Fract narrow2 = beyond_max;  /* Should trigger overflow check */
        
        volatile signed char _Fract v1 = narrow1;
        volatile signed char _Fract v2 = narrow2;
        checksum += (int)(v1 * 1000r);
        checksum += (int)(v2 * 1000r);
    }
    
    /* Test 2: Unsigned conversions with overflow */
    {
        /* Maximum unsigned _Accum */
        const unsigned _Accum max_uaccum = 4294967.295uk;  /* U16.16 max */
        
        /* Convert to unsigned _Fract - triggers a_high.sgt(max_r) check */
        unsigned _Fract uf1 = max_uaccum;  /* U0.16 */
        
        /* Beyond max by arithmetic */
        unsigned _Accum beyond = max_uaccum + (unsigned _Accum)0.001uk;
        unsigned _Fract uf2 = beyond;
        
        consume(&uf1);
        consume(&uf2);
        checksum += (int)(uf1 * 1000ur);
        checksum += (int)(uf2 * 1000ur);
    }
    
    /* Test 3: Complex constant expressions forcing range checks */
    {
        /* These should be evaluated at compile-time, invoking the target logic */
        constexpr signed _Accum a = (signed _Accum)0.5k * 3.14159265k;
        constexpr signed short _Fract b = a;  /* Conversion with range check */
        
        constexpr unsigned long _Accum c = (unsigned long _Accum)1000000.0ulk;
        constexpr unsigned _Fract d = c;  /* Large value conversion */
        
        volatile signed short _Fract vb = b;
        volatile unsigned _Fract vd = d;
        checksum += (int)(vb * 1000r);
        checksum += (int)(vd * 1000ur);
    }
    
    /* Test 4: Mixed signed/unsigned with saturation */
    {
        /* Using _Sat types */
        signed _Sat _Accum sat1 = (signed _Sat _Accum)5000.0k;
        signed _Sat short _Fract sat2 = sat1;  /* Narrowing with saturation */
        
        unsigned _Sat _Accum usat1 = (unsigned _Sat _Accum)(-1.0k);
        unsigned _Sat _Fract usat2 = usat1;  /* Underflow to 0 with saturation */
        
        /* Force evaluation */
        volatile signed _Sat short _Fract vsat2 = sat2;
        volatile unsigned _Sat _Fract vusat2 = usat2;
        checksum += (int)(vsat2 * 1000r);
        checksum += (int)(vusat2 * 1000ur);
    }
    
    /* Test 5: Boundary value testing with function calls */
    {
        /* These prevent complete compile-time evaluation */
        signed short _Fract sf_max = get_sfract_max();
        unsigned short _Fract uf_max = get_ufract_max();
        signed long _Accum sl_min = get_saccum_min();
        
        /* Conversions that should trigger range checks */
        signed char _Fract from_sf = sf_max;      /* Q0.15 -> Q0.7 */
        unsigned char _Fract from_uf = uf_max;    /* U0.16 -> U0.8 */
        signed _Accum from_sl = sl_min;           /* Q31.32 -> Q15.16 */
        
        /* Near boundary operations */
        signed short _Fract near_max = sf_max - (signed short _Fract)0.0001r;
        signed char _Fract near_conv = near_max;
        
        consume(&from_sf);
        consume(&from_uf);
        consume(&from_sl);
        consume(&near_conv);
        
        checksum += (int)(from_sf * 1000r);
        checksum += (int)(from_uf * 1000ur);
        checksum += (int)(from_sl * 0.001k);
        checksum += (int)(near_conv * 1000r);
    }
    
    /* Test 6: Arithmetic overflow followed by conversion */
    {
        /* Multiplication that overflows _Accum range */
        const signed _Accum a1 = (signed _Accum)30000.0k;
        const signed _Accum a2 = (signed _Accum)30000.0k;
        signed _Accum product = a1 * a2;  /* May overflow Q15.16 */
        
        /* Convert overflowed result - triggers range checking */
        signed _Fract f1 = product;
        
        /* Addition at boundary */
        unsigned _Accum ua1 = (unsigned _Accum)4294966.0uk;  /* Near max */
        unsigned _Accum ua2 = (unsigned _Accum)2.0uk;
        unsigned _Accum sum = ua1 + ua2;  /* Exceeds U16.16 max */
        unsigned _Fract f2 = sum;
        
        volatile signed _Fract vf1 = f1;
        volatile unsigned _Fract vf2 = f2;
        checksum += (int)(vf1 * 1000r);
        checksum += (int)(vf2 * 1000ur);
    }
    
    /* Test 7: Loop with fixed-point accumulation */
    {
        /* Small loop to create semi-constant values */
        signed _Accum acc = (signed _Accum)0.0k;
        for (int i = 0; i < 3; i++) {
            acc += (signed _Accum)10000.0k;
        }
        
        /* Convert accumulated value */
        signed short _Fract converted = acc;
        
        /* Negative accumulation */
        signed _Accum neg_acc = (signed _Accum)0.0k;
        for (int i = 0; i < 5; i++) {
            neg_acc -= (signed _Accum)5000.0k;
        }
        signed short _Fract neg_converted = neg_acc;
        
        consume(&converted);
        consume(&neg_converted);
        checksum += (int)(converted * 1000r);
        checksum += (int)(neg_converted * 1000r);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
