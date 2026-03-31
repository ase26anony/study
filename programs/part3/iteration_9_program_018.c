/* Test program for GCC fixed-point range checking logic */
/* Compile with: gcc -O1 -std=gnu11 -Wno-psabi fixed-point-test.c -o fixed-point-test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent constant folding elimination */
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
    /* Test 1: Boundary values for signed fract */
    {
        const signed short _Fract sf_max = 0.999969482421875r;
        const signed short _Fract sf_min = -1.0r;
        const signed short _Fract sf_near_max = 0.99993896484375r; /* MAX - 1 LSB */
        
        /* These should trigger range checks during conversions */
        volatile signed char _Fract cf1 = sf_max;  /* Narrowing conversion */
        volatile signed _Fract f1 = sf_max;        /* Same width */
        
        /* Operation that might overflow */
        volatile signed short _Fract sf_result = sf_max + sf_near_max;
        consume(&sf_result);
    }
    
    /* Test 2: Unsigned fract boundary testing */
    {
        const unsigned short _Fract uf_max = 0.999969482421875ur;
        const unsigned short _Fract uf_near_max = 0.99993896484375ur;
        
        /* Force unsigned overflow check */
        volatile unsigned char _Fract ucf1 = uf_max;  /* Narrowing */
        volatile unsigned short _Fract uf_sum = uf_max + uf_near_max;
        
        /* This should trigger unsigned overflow logic */
        volatile unsigned _Fract uf_result = uf_sum;
        consume(&uf_result);
    }
    
    /* Test 3: Accum to Fract conversions with overflow */
    {
        const signed _Accum sa_large = 100.5k;
        const signed _Accum sa_huge = 500.0k;
        const signed _Accum sa_neg = -100.5k;
        
        /* These conversions require range checking */
        volatile signed short _Fract sf_from_acc = sa_large;  /* Should fit */
        volatile signed short _Fract sf_overflow = sa_huge;   /* Should overflow */
        volatile signed short _Fract sf_neg = sa_neg;         /* Negative value */
        
        /* Mixed-width arithmetic then conversion */
        volatile signed _Accum sa_mixed = sa_large * 2.0k;
        volatile signed short _Fract sf_mixed = sa_mixed;
        
        consume(&sf_from_acc);
        consume(&sf_overflow);
        consume(&sf_neg);
        consume(&sf_mixed);
    }
    
    /* Test 4: Complex constant expressions */
    {
        /* Compile-time constants that exercise range logic */
        constexpr signed _Accum ca1 = (signed _Accum)0.5r * 10.0r;
        constexpr signed _Accum ca2 = (signed _Accum)0.999969482421875r * 100.0r;
        constexpr signed short _Fract cf1 = (signed short _Fract)ca1;
        constexpr signed short _Fract cf2 = (signed short _Fract)ca2;  /* Should overflow */
        
        volatile signed short _Fract vcf1 = cf1;
        volatile signed short _Fract vcf2 = cf2;
        
        /* More complex expression */
        constexpr signed _Accum ca3 = (signed _Accum)(0.75r + 0.25r) * 200.0k;
        constexpr signed short _Fract cf3 = (signed short _Fract)ca3;
        volatile signed short _Fract vcf3 = cf3;
        
        consume(&vcf1);
        consume(&vcf2);
        consume(&vcf3);
    }
    
    /* Test 5: Saturation qualifier testing */
    {
        /* Saturated types should use different overflow handling */
        signed short _Sat _Fract ssf_max = 0.999969482421875r;
        signed short _Sat _Fract ssf_overflow;
        unsigned short _Sat _Fract usf_max = 0.999969482421875ur;
        unsigned short _Sat _Fract usf_overflow;
        
        /* Operations that would overflow non-sat types */
        ssf_overflow = ssf_max + ssf_max;  /* Should saturate to MAX */
        usf_overflow = usf_max + usf_max;  /* Should saturate to MAX */
        
        /* Conversion from saturated to non-saturated */
        volatile signed short _Fract sf_from_sat = ssf_overflow;
        volatile unsigned short _Fract uf_from_sat = usf_overflow;
        
        /* Mixed sat/non-sat operations */
        signed short _Fract sf_regular = 0.5r;
        signed short _Sat _Fract ssf_mixed = sf_regular + ssf_max;
        
        consume(&sf_from_sat);
        consume(&uf_from_sat);
        consume(&ssf_mixed);
    }
    
    /* Test 6: Loop-based boundary testing */
    {
        /* Use loops to create values near boundaries */
        signed short _Fract sf_accum = 0.0r;
        unsigned short _Fract uf_accum = 0.0ur;
        
        for (int i = 0; i < 10; i++) {
            sf_accum += 0.1r;
            uf_accum += 0.1ur;
        }
        
        /* Convert accumulated values to narrower types */
        volatile signed char _Fract scf_narrow = sf_accum;
        volatile unsigned char _Fract ucf_narrow = uf_accum;
        
        /* Create values near maximum */
        signed short _Fract sf_near_limit = 0.99993896484375r; /* MAX - 1LSB */
        for (int i = 0; i < 3; i++) {
            sf_near_limit += 0.000030517578125r; /* Add 1 LSB each iteration */
        }
        
        volatile signed short _Fract sf_final = sf_near_limit;
        
        consume(&scf_narrow);
        consume(&ucf_narrow);
        consume(&sf_final);
    }
    
    /* Test 7: Function-based boundary values */
    {
        /* Use noinline functions to get boundary values */
        signed short _Fract sf_func_max = get_sfract_max();
        unsigned short _Fract uf_func_max = get_ufract_max();
        signed _Accum sa_func_max = get_saccum_max();
        signed _Accum sa_func_min = get_saccum_min();
        
        /* Conversions that should trigger range checks */
        volatile signed char _Fract scf_from_func = sf_func_max;
        volatile unsigned char _Fract ucf_from_func = uf_func_max;
        
        /* Accum to fract conversions */
        volatile signed short _Fract sf_from_saccum = sa_func_max;
        volatile signed short _Fract sf_from_saccum_min = sa_func_min;
        
        /* Operations near boundaries */
        volatile signed short _Fract sf_plus_one = sf_func_max + 0.000030517578125r; /* +1 LSB */
        volatile signed _Accum sa_overflow = sa_func_max * 1.1k;
        volatile signed short _Fract sf_from_overflow = sa_overflow;
        
        consume(&scf_from_func);
        consume(&ucf_from_func);
        consume(&sf_from_saccum);
        consume(&sf_from_saccum_min);
        consume(&sf_plus_one);
        consume(&sf_from_overflow);
    }
    
    /* Test 8: Mixed signed/unsigned conversions */
    {
        signed short _Fract sf_pos = 0.5r;
        signed short _Fract sf_neg = -0.5r;
        
        /* Signed to unsigned conversion (should trigger range check for negative) */
        volatile unsigned short _Fract usf_from_signed = sf_pos;
        volatile unsigned short _Fract usf_from_neg = sf_neg;  /* Should overflow */
        
        /* Unsigned to signed */
        unsigned short _Fract usf_mid = 0.5ur;
        volatile signed short _Fract sf_from_unsigned = usf_mid;
        
        consume(&usf_from_signed);
        consume(&usf_from_neg);
        consume(&sf_from_unsigned);
    }
    
    /* Test 9: Extreme boundary cases */
    {
        /* Values at exact boundaries */
        const signed _Accum sa_exact_max = 32767.999969482421875k;
        const signed _Accum sa_just_over = 32768.0k;  /* Just over MAX */
        const signed _Accum sa_exact_min = -32768.0k;
        const signed _Accum sa_just_under = -32768.000030517578125k; /* Just under MIN */
        
        /* These should all trigger range checking */
        volatile signed short _Fract sf_exact_max = sa_exact_max;
        volatile signed short _Fract sf_just_over = sa_just_over;
        volatile signed short _Fract sf_exact_min = sa_exact_min;
        volatile signed short _Fract sf_just_under = sa_just_under;
        
        /* For unsigned */
        const unsigned _Accum ua_exact_max = 65535.999969482421875uk;
        const unsigned _Accum ua_just_over = 65536.0uk;
        
        volatile unsigned short _Fract uf_exact_max = ua_exact_max;
        volatile unsigned short _Fract uf_just_over = ua_just_over;
        
        consume(&sf_exact_max);
        consume(&sf_just_over);
        consume(&sf_exact_min);
        consume(&sf_just_under);
        consume(&uf_exact_max);
        consume(&uf_just_over);
    }
    
    printf("Fixed-point range checking tests completed.\n");
    return 0;
}
