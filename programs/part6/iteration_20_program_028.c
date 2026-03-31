/* test_fixed_range.c
 * Designed to trigger fixed-point range analysis overflow checks in GCC
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_range.c -o test_fixed_range.o
 */

#include <stdint.h>

/* Prevent dead code elimination */
static void __attribute__((noinline)) consume_result(const void *p, int size) {
    volatile char sink;
    const char *bytes = (const char *)p;
    for (int i = 0; i < size; i++) {
        sink = bytes[i];
    }
}

int main(void) {
    /* Volatile seeds to prevent constant folding */
    volatile int seed_i = 100;
    volatile unsigned int seed_u = 200;
    volatile long seed_l = -50;
    
    /* Fixed-point arrays for accumulation */
    _Accum accum_results[8];
    unsigned _Fract ufrac_results[8];
    _Fract frac_results[8];
    long _Accum long_accum_results[8];
    
    /* Initialize with boundary values */
    accum_results[0] = 0.999999999k;        /* Near max for _Accum */
    accum_results[1] = -0.999999999k;       /* Near min for _Accum */
    ufrac_results[0] = 0.9999999ur;         /* Near max for unsigned _Fract */
    frac_results[0] = 0.9999999r;           /* Near max for signed _Fract */
    frac_results[1] = -0.9999999r;          /* Near min for signed _Fract */
    long_accum_results[0] = 0.9999999999999999lk; /* Near max for long _Accum */
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 8; i++) {
        /* Use volatile seeds to create data-dependent values */
        int idx = (seed_i + i) & 7;
        unsigned int uidx = (seed_u + i) & 7;
        long lidx = (seed_l + i) & 7;
        
        /* Complex fixed-point expressions designed to trigger overflow checks */
        
        /* 1. Multiplication near boundaries - may overflow */
        _Accum a1 = accum_results[idx] * 0.9999999k;
        _Accum a2 = accum_results[(idx + 1) & 7] * 0.9999999k;
        _Accum product = a1 * a2;  /* Product of two near-max values */
        
        /* 2. Left shift simulation for fixed-point */
        /* Equivalent to multiplication by power of two */
        int shift_amt = (i % 4) + 1;
        _Accum shifted = product;
        for (int s = 0; s < shift_amt; s++) {
            shifted = shifted * 0.5k;  /* Division by 2, but compiler may analyze as shift */
        }
        
        /* 3. Mixed-type expressions with integer promotion */
        int int_val = seed_i + i;
        _Accum mixed = (_Accum)int_val * accum_results[idx];
        
        /* 4. Unsigned fractional operations near 1.0 */
        unsigned _Fract u1 = ufrac_results[uidx];
        unsigned _Fract u2 = 0.0000001ur * (i + 1);
        unsigned _Fract u_sum = u1 + u2;  /* Potential wrap-around */
        
        /* 5. Conditional expressions with fixed-point */
        _Fract f_cond = (i & 1) ? frac_results[idx] : -frac_results[(idx + 1) & 7];
        f_cond = f_cond * 0.9999999r;
        
        /* 6. Long accum operations with extreme values */
        long _Accum la = long_accum_results[lidx & 7];
        long _Accum lb = 0.9999999999999999lk;
        long _Accum long_product = la * lb;  /* Very likely to trigger overflow check */
        
        /* 7. Accumulate with saturation-like behavior */
        /* This should trigger range comparison against max/min */
        _Accum temp = accum_results[idx] + 0.0000001k;
        
        /* Check if we're at boundary (simulating overflow check) */
        if (temp > 0.9999999k) {
            temp = 0.9999999k;  /* Saturate */
        } else if (temp < -0.9999999k) {
            temp = -0.9999999k; /* Saturate */
        }
        
        /* 8. Complex expression chain */
        _Accum chain = accum_results[idx];
        chain = chain * 0.75k;
        chain = chain + 0.25k;
        chain = chain * 0.9999999k;  /* Push to boundary */
        
        /* Store results to arrays */
        accum_results[idx] = product + shifted + mixed + chain;
        ufrac_results[uidx] = u_sum;
        frac_results[idx] = f_cond;
        long_accum_results[lidx & 7] = long_product;
        
        /* Additional boundary-pushing expressions */
        
        /* 9. Short fract operations */
        short _Fract sf = 0.9999hr;
        sf = sf * 0.9999hr;
        
        /* 10. Unsigned accum */
        unsigned _Accum ua = 0.999999999uk;
        ua = ua * 0.999999999uk;
        
        /* 11. Explicit cast to/from integer with scaling */
        /* This often triggers range analysis for conversion */
        int scale_int = seed_i * (i + 1);
        _Accum from_int = (_Accum)scale_int * 0.001k;
        
        /* 12. Nested conditional with fixed-point */
        _Accum nested = (i < 4) ? 
                       ((seed_i > 0) ? accum_results[0] : accum_results[1]) :
                       ((seed_i < 0) ? -accum_results[2] : accum_results[3]);
        nested = nested * 0.9999999k;
        
        /* Store more results */
        if (i < 4) {
            accum_results[i + 4] = from_int + nested;
        }
    }
    
    /* Final complex expression designed specifically to trigger the 
     * uncovered range comparison block for signed overflow */
    {
        /* Create values that should be near the maximum representable range */
        volatile _Accum v1 = 0.999999999k;
        volatile _Accum v2 = 0.999999999k;
        
        /* This multiplication should mathematically overflow the fixed-point range
         * The compiler's range analysis should detect this */
        _Accum final_product = v1 * v2;
        
        /* Additional left-shift-like operation */
        for (int j = 0; j < 3; j++) {
            final_product = final_product * 0.9999999k;
        }
        
        /* Store in array */
        accum_results[0] = final_product;
    }
    
    /* Another attempt with long accum */
    {
        long _Accum la1 = 0.9999999999999999lk;
        long _Accum la2 = 0.9999999999999999lk;
        long _Accum la3 = la1 * la2;  /* Should trigger overflow check */
        
        /* Chain operations to create complex range */
        for (int j = 0; j < 2; j++) {
            la3 = la3 * 0.9999999999999999lk;
        }
        
        long_accum_results[0] = la3;
    }
    
    /* Consume results to prevent optimization */
    consume_result(accum_results, sizeof(accum_results));
    consume_result(ufrac_results, sizeof(ufrac_results));
    consume_result(frac_results, sizeof(frac_results));
    consume_result(long_accum_results, sizeof(long_accum_results));
    
    /* Return hash of results */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= *((int*)&accum_results[i]);
        hash ^= *((int*)&ufrac_results[i]);
        hash ^= *((int*)&frac_results[i]);
        hash ^= *((int*)&long_accum_results[i]) ^ (*((int*)&long_accum_results[i] + 1));
    }
    
    return hash & 0xFF;
}
