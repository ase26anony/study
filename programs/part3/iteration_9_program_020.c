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
    return -9223372036854775.807S; /* Approx min for signed long _Accum */
}

__attribute__((noinline)) signed _Accum get_saccum_mid(void) {
    return 0.5k;  /* Middle value */
}

/* Force materialization of intermediate results */
static volatile signed _Accum volatile_sink;

int main(void) {
    int checksum = 0;
    
    /* Test 1: Signed fract boundary testing */
    {
        /* Constants at boundaries */
        const signed short _Fract max_sf = 0.999969482421875r;
        const signed short _Fract min_sf = -1.0r;
        const signed short _Fract eps_sf = 0.000030517578125r; /* 1 LSB */
        
        /* Operations that should trigger range checks */
        signed _Accum a1 = (signed _Accum)max_sf * 2.0k;  /* Overflow */
        signed short _Fract b1 = (signed short _Fract)a1; /* Conversion check */
        
        /* Near overflow */
        signed _Accum a2 = (signed _Accum)max_sf + (signed _Accum)eps_sf;
        signed short _Fract b2 = (signed short _Fract)a2;
        
        /* Use volatile to force computation */
        volatile_sink = a1 + a2;
        checksum += (int)(b1 * 1000r) + (int)(b2 * 1000r);
    }
    
    /* Test 2: Unsigned fract with overflow beyond max */
    {
        unsigned short _Fract max_uf = get_ufract_max();
        unsigned _Accum large_ua = (unsigned _Accum)max_uf * 1.5uk;
        
        /* This conversion should trigger max_s comparison */
        unsigned short _Fract uf1 = (unsigned short _Fract)large_ua;
        
        /* Just at the boundary */
        unsigned _Accum boundary = (unsigned _Accum)max_uf;
        unsigned short _Fract uf2 = (unsigned short _Fract)boundary;
        
        checksum += (int)(uf1 * 1000ur) + (int)(uf2 * 1000ur);
    }
    
    /* Test 3: Complex constant expressions with mixed types */
    {
        /* Compile-time constant expressions */
        const signed _Accum c1 = (signed _Accum)0.33333333333333333333k * 3.0k;
        const signed _Accum c2 = (signed _Accum)(-0.99999999999999999999r) * 1.5k;
        
        /* Convert to narrower types - triggers range checking */
        const signed short _Fract f1 = (signed short _Fract)c1;
        const signed short _Fract f2 = (signed short _Fract)c2;
        
        /* Use in non-constant context to ensure code generation */
        signed short _Fract f3 = f1 + f2;
        checksum += (int)(f3 * 1000r);
    }
    
    /* Test 4: Saturation qualifier testing */
    {
        _Sat signed short _Fract sat_max = 0.999969482421875r;
        _Sat signed _Accum sat_acc = (signed _Accum)sat_max * 2.0k;
        
        /* Conversion between saturated types */
        _Sat signed short _Fract sat_result = (_Sat signed short _Fract)sat_acc;
        
        /* Mix saturated and non-saturated */
        signed short _Fract non_sat = (signed short _Fract)sat_acc;
        
        checksum += (int)(sat_result * 1000r) + (int)(non_sat * 1000r);
    }
    
    /* Test 5: Negative overflow for signed types */
    {
        signed long _Accum min_sla = get_saccum_min();
        signed _Accum small = get_saccum_mid();
        
        /* Create value that underflows when converted */
        signed long _Accum underflow = min_sla - small;
        signed _Accum converted = (signed _Accum)underflow;
        
        /* Convert to fract - triggers min range checking */
        signed short _Fract sf_under = (signed short _Fract)underflow;
        
        checksum += (int)(converted * 0.001k) + (int)(sf_under * 1000r);
    }
    
    /* Test 6: Loop with boundary accumulation */
    {
        signed short _Fract accum = 0.0r;
        const signed short _Fract increment = 0.1r;
        
        /* Small loop to create near-boundary values */
        for (int i = 0; i < 12; i++) {
            accum += increment;
        }
        
        /* Convert to narrower type (itself) - still triggers checks */
        signed char _Fract cf = (signed char _Fract)accum;
        
        checksum += (int)(accum * 1000r) + (int)(cf * 1000r);
    }
    
    /* Test 7: Unsigned underflow (below zero) */
    {
        unsigned _Accum ua = 0.0uk;
        signed _Accum sa = -0.5k;
        
        /* This should trigger unsigned min check (a_high == max_r comparison) */
        unsigned _Accum mixed = ua + (unsigned _Accum)sa;
        unsigned short _Fract uf = (unsigned short _Fract)mixed;
        
        checksum += (int)(uf * 1000ur);
    }
    
    /* Test 8: Exact boundary values */
    {
        /* These exact conversions should exercise equality comparisons */
        const signed _Accum exact_max_fract = (signed _Accum)0.999969482421875r;
        const signed _Accum just_above = exact_max_fract + (signed _Accum)0.0000001k;
        const signed _Accum just_below = exact_max_fract - (signed _Accum)0.0000001k;
        
        signed short _Fract f_exact = (signed short _Fract)exact_max_fract;
        signed short _Fract f_above = (signed short _Fract)just_above;
        signed short _Fract f_below = (signed short _Fract)just_below;
        
        checksum += (int)(f_exact * 1000r) + (int)(f_above * 1000r) + (int)(f_below * 1000r);
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
