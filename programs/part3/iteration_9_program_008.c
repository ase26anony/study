/* fixed-point-test.c
 * Test program for GCC fixed-point range checking logic
 * Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test
 */

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
    volatile signed _Accum v = x;
    return v + 0.0k;
}

/* Main test function */
int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed fixed-point conversions with boundary values */
    {
        /* Get maximum signed short _Fract at runtime to prevent compile-time elimination */
        volatile signed short _Fract sf_max = get_sfract_max();
        
        /* Convert to different precisions - should trigger range checks */
        signed _Fract f1 = sf_max;                     /* Same precision */
        signed _Accum a1 = sf_max;                     /* Wider precision */
        signed short _Fract sf1 = (signed short _Fract)a1; /* Narrower precision */
        
        /* Operations near boundaries */
        signed _Accum a2 = a1 * 2.0k;                  /* Should overflow for narrower types */
        signed short _Fract sf2 = (signed short _Fract)a2; /* Triggers range check */
        
        /* Use volatile to force materialization */
        volatile signed _Fract vf1 = f1;
        volatile signed short _Fract vsf2 = sf2;
        
        checksum += (int)(vf1 * 1000r);
        checksum += (int)(vsf2 * 1000r);
    }
    
    /* Test 2: Unsigned fixed-point with overflow scenarios */
    {
        unsigned short _Fract uf_max = get_ufract_max();
        
        /* Create value just beyond maximum */
        unsigned _Accum ua1 = (unsigned _Accum)uf_max + 0.0001uk;
        unsigned short _Fract uf1 = (unsigned short _Fract)ua1; /* Triggers max check */
        
        /* Test with compile-time constants that should be folded */
        const unsigned long _Fract ulf_max = 0.99999999976716935634613037109375ulr;
        const unsigned _Fract uf_const = (unsigned _Fract)ulf_max; /* Range check */
        
        volatile unsigned short _Fract vuf1 = uf1;
        volatile unsigned _Fract vuf_const = uf_const;
        
        checksum += (int)(vuf1 * 1000ur);
        checksum += (int)(vuf_const * 1000ur);
    }
    
    /* Test 3: Mixed signed/unsigned conversions */
    {
        signed _Accum sa_min = get_saccum_min();
        
        /* Negative to unsigned conversion - should trigger range checks */
        unsigned _Fract uf2 = (unsigned _Fract)sa_min; /* Negative to unsigned */
        
        /* Signed with positive overflow */
        signed _Accum sa3 = 9223372036854775.807k; /* Near max */
        signed _Accum sa4 = get_saccum_val(sa3) * 1.5k; /* Overflow */
        signed short _Fract sf3 = (signed short _Fract)sa4; /* Triggers a_high.sgt(max_r) */
        
        volatile unsigned _Fract vuf2 = uf2;
        volatile signed short _Fract vsf3 = sf3;
        
        checksum += (int)(vuf2 * 1000ur);
        checksum += (int)(vsf3 * 1000r);
    }
    
    /* Test 4: Saturation qualifier tests */
    {
        /* Saturated types have different overflow behavior */
        _Sat signed short _Fract ssf1 = 0.999969482421875sr;
        _Sat signed _Accum sa5 = 9223372036854775.807sk;
        
        /* Operations that would overflow without saturation */
        _Sat signed short _Fract ssf2 = ssf1 + 0.1sr;  /* Saturates to max */
        _Sat signed _Accum sa6 = sa5 * 2.0sk;          /* Saturates */
        
        /* Convert between saturated and non-saturated */
        signed short _Fract sf4 = ssf2;                /* Range check */
        _Sat signed short _Fract ssf3 = (_Sat signed short _Fract)sa6;
        
        volatile _Sat signed short _Fract vssf2 = ssf2;
        volatile signed short _Fract vsf4 = sf4;
        
        checksum += (int)(vssf2 * 1000sr);
        checksum += (int)(vsf4 * 1000r);
    }
    
    /* Test 5: Complex constant expressions for constant folding */
    {
        /* These should be evaluated at compile-time, triggering range checks */
        constexpr signed long _Accum sla1 = 4611686018427387.903lk; /* 0.5 * max */
        constexpr signed long _Accum sla2 = sla1 * 3.0lk;           /* 1.5 * max */
        constexpr signed _Fract cf1 = (signed _Fract)sla2;          /* Range check */
        
        /* More complex expression */
        constexpr unsigned short _Fract cuf1 = 0.5ur;
        constexpr unsigned short _Fract cuf2 = 0.75ur;
        constexpr unsigned _Accum cua1 = (unsigned _Accum)cuf1 * (unsigned _Accum)cuf2;
        constexpr unsigned short _Fract cuf3 = (unsigned short _Fract)cua1 * 4.0ur;
        
        volatile signed _Fract vcf1 = cf1;
        volatile unsigned short _Fract vcuf3 = cuf3;
        
        checksum += (int)(vcf1 * 1000r);
        checksum += (int)(vcuf3 * 1000ur);
    }
    
    /* Test 6: Loop with fixed-point accumulation */
    {
        signed short _Fract sf_acc = 0.0r;
        unsigned _Accum ua_acc = 0.0uk;
        
        /* Small loop to create non-trivial control flow */
        for (int i = 0; i < 4; i++) {
            sf_acc += 0.25r;
            ua_acc += 0.33333333333333333333uk;
            
            /* Periodic conversions that may trigger range checks */
            if (i == 2) {
                signed _Accum sa_temp = (signed _Accum)sf_acc * 3.0k;
                sf_acc = (signed short _Fract)sa_temp; /* Possible overflow */
            }
        }
        
        /* Final conversion that might overflow */
        unsigned short _Fract uf_final = (unsigned short _Fract)ua_acc;
        
        volatile signed short _Fract vsf_acc = sf_acc;
        volatile unsigned short _Fract vuf_final = uf_final;
        
        checksum += (int)(vsf_acc * 1000r);
        checksum += (int)(vuf_final * 1000ur);
    }
    
    /* Output checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
