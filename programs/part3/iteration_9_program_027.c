/* fixed-point-test.c
 * Tests GCC's fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant propagation */
__attribute__((noinline)) signed short _Fract get_sfract_max(void) {
    return 0.999969482421875r;  /* MAX short _Fract */
}

__attribute__((noinline)) unsigned short _Fract get_ufract_max(void) {
    return 0.999969482421875ur; /* MAX unsigned short _Fract */
}

__attribute__((noinline)) signed _Accum get_saccum_max(void) {
    return 32767.999969482421875k; /* MAX short _Accum */
}

__attribute__((noinline)) signed _Accum get_saccum_min(void) {
    return -32768.0k; /* MIN short _Accum */
}

__attribute__((noinline)) volatile void consume(volatile void *ptr) {
    /* Force materialization */
    (void)ptr;
}

int main(void) {
    volatile int checksum = 0;
    
    /* Test 1: Conversion from _Accum to _Fract with overflow */
    {
        /* These should trigger range checks */
        const signed _Accum a1 = 1.5k;
        const signed _Accum a2 = 2.0k;
        
        /* Multiplication that overflows when converted to _Fract */
        const signed _Accum product = a1 * a2;  /* 3.0k */
        
        /* This conversion should trigger a_high.sgt(max_r) check */
        volatile signed short _Fract f1 = (signed short _Fract)product;
        checksum += (int)(f1 * 1000r);
        
        /* Test with values just at the boundary */
        const signed short _Accum boundary = 0.999969482421875k; /* MAX _Fract as _Accum */
        volatile signed short _Fract f2 = (signed short _Fract)(boundary * 1.0001k);
        checksum += (int)(f2 * 1000r);
    }
    
    /* Test 2: Boundary value testing for signed types */
    {
        /* Use opaque functions to get boundary values */
        signed short _Fract sf_max = get_sfract_max();
        signed _Accum sa_max = get_saccum_max();
        signed _Accum sa_min = get_saccum_min();
        
        /* Test conversions at exact boundaries */
        volatile signed short _Fract f3 = (signed short _Fract)sa_max;  /* Should overflow */
        volatile signed short _Fract f4 = (signed short _Fract)sa_min;  /* Should underflow */
        
        /* Test with one LSB beyond boundary */
        volatile signed short _Fract f5 = sf_max + (signed short _Fract)0.0001r;
        checksum += (int)(f3 * 1000r) + (int)(f4 * 1000r) + (int)(f5 * 1000r);
        
        /* Complex constant expression */
        constexpr signed _Accum c1 = (signed _Accum)0.5r * 3.0r;
        constexpr signed _Accum c2 = (signed _Accum)0.999969482421875r * 1.1r;
        volatile signed short _Fract f6 = (signed short _Fract)(c1 * c2);
        checksum += (int)(f6 * 1000r);
    }
    
    /* Test 3: Unsigned fixed-point with overflow */
    {
        unsigned short _Fract uf_max = get_ufract_max();
        
        /* Operations that should overflow unsigned range */
        volatile unsigned short _Fract uf1 = uf_max + (unsigned short _Fract)0.0001ur;
        volatile unsigned short _Fract uf2 = uf_max * (unsigned short _Fract)1.1ur;
        
        /* Conversion from signed negative to unsigned */
        const signed _Accum neg_val = -1.5k;
        volatile unsigned short _Fract uf3 = (unsigned short _Fract)neg_val;
        
        checksum += (int)(uf1 * 1000ur) + (int)(uf2 * 1000ur) + (int)(uf3 * 1000ur);
    }
    
    /* Test 4: Mixed precision with saturation */
    {
        /* Saturated types should invoke different overflow handling */
        _Sat signed short _Fract sf_sat1 = 0.9r;
        _Sat signed short _Fract sf_sat2 = 0.8r;
        
        /* This addition would overflow without saturation */
        _Sat signed short _Fract sf_sat3 = sf_sat1 + sf_sat2;
        
        /* Mix saturated and non-saturated in expression */
        signed short _Fract sf_reg = 0.7r;
        volatile _Sat signed short _Fract sf_sat4 = (_Sat signed short _Fract)(sf_sat3 + sf_reg);
        
        /* Convert saturated _Accum to non-saturated _Fract */
        _Sat signed _Accum sa_sat = 1000.0k;
        volatile signed short _Fract f7 = (signed short _Fract)sa_sat;
        
        checksum += (int)(sf_sat4 * 1000r) + (int)(f7 * 1000r);
    }
    
    /* Test 5: Complex nested expressions with loops */
    {
        /* Small loop to create semi-constant values */
        signed short _Fract accum = 0.0r;
        for (int i = 0; i < 3; i++) {
            accum += (signed short _Fract)0.333r;
        }
        
        /* This should be ~0.999r, close to MAX */
        volatile signed _Accum converted = (signed _Accum)accum;
        
        /* Multiply to push over the boundary */
        volatile signed _Accum overflow_test = converted * 1.5k;
        volatile signed short _Fract f8 = (signed short _Fract)overflow_test;
        
        checksum += (int)(f8 * 1000r);
    }
    
    /* Test 6: Direct boundary constant expressions */
    {
        /* These constants should directly exercise the range checking code */
        const signed long _Accum large_val = 9223372036854775.807k;  /* Near MAX */
        const signed long _Accum small_val = -9223372036854775.808k; /* Near MIN */
        
        /* Convert to much smaller types to force range checks */
        volatile signed short _Fract f9  = (signed short _Fract)large_val;
        volatile signed short _Fract f10 = (signed short _Fract)small_val;
        
        /* Test with one ULP differences */
        const signed short _Fract max_minus_ulp = 0.999938964843750r;  /* MAX - 1ULP */
        const signed short _Fract max_plus_ulp  = 1.0r;                /* MAX + 1ULP (overflow) */
        
        volatile signed _Accum a3 = (signed _Accum)max_minus_ulp;
        volatile signed _Accum a4 = (signed _Accum)max_plus_ulp;
        
        checksum += (int)(f9 * 1000r) + (int)(f10 * 1000r) + (int)(a3) + (int)(a4);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use consume to prevent optimization */
    consume(&checksum);
    
    return 0;
}
