/* test_fixed_point_range.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O2 -ffixed-point -fdump-tree-vrp-details -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 10;
static volatile int vi2 = -5;
static volatile unsigned int vu1 = 100;
static volatile _Fract vf1 = 0.5r;
static volatile _Accum va1 = 0.5k;
static volatile long _Accum vla1 = 0.999999999lk;

/* Noinline function to prevent dead code elimination */
__attribute__((noinline)) 
void consume_results(_Fract *farr, _Accum *aarr, long _Accum *laarr, int n) {
    volatile int sink = 0;
    for (int i = 0; i < n; i++) {
        sink += *(int*)&farr[i];
        sink += *(long*)&aarr[i];
        sink += *(long long*)&laarr[i];
    }
    (void)sink;
}

int main(void) {
    /* Arrays to store intermediate results */
    _Fract f_results[10];
    _Accum a_results[10];
    long _Accum la_results[10];
    int result_index = 0;
    
    /* Seed values near boundaries */
    unsigned _Fract uf_max = 0.9999999ur;  /* Very close to max unsigned fract */
    signed _Fract sf_max = 0.9999999r;     /* Very close to max signed fract */
    signed _Fract sf_min = -0.9999999r;    /* Very close to min signed fract */
    _Accum a_max = 0.9999999k;             /* Close to max _Accum */
    _Accum a_min = -0.9999999k;            /* Close to min _Accum */
    long _Accum la_max = 0.999999999lk;    /* Close to max long _Accum */
    long _Accum la_min = -0.999999999lk;   /* Close to min long _Accum */
    
    /* Use volatile seeds to force range analysis */
    int seed1 = vi1;
    int seed2 = vi2;
    unsigned int useed = vu1;
    _Fract fseed = vf1;
    _Accum aseed = va1;
    long _Accum laseed = vla1;
    
    /* Loop with varying values to force range analysis across iterations */
    for (int i = 0; i < 5; i++) {
        /* 1. Operations on unsigned _Fract near overflow boundary */
        unsigned _Fract uf1 = uf_max;
        unsigned _Fract uf2 = (unsigned _Fract)(useed + i) / 256.0ur;
        unsigned _Fract uf_prod = uf1 * uf2;  /* May overflow */
        
        /* Convert to signed for storage */
        f_results[result_index++] = (_Fract)uf_prod;
        
        /* 2. Signed _Fract operations with mixed signs */
        signed _Fract sf1 = (i % 2) ? sf_max : sf_min;
        signed _Fract sf2 = fseed * i;
        signed _Fract sf_sum = sf1 + sf2;  /* May overflow in either direction */
        
        /* Left shift simulation through multiplication */
        signed _Fract sf_shifted = sf_sum * 2.0r;  /* Like left shift by 1 */
        
        f_results[result_index++] = sf_shifted;
        
        /* 3. _Accum multiplication near limits */
        _Accum a1 = a_max * (1.0k - (aseed * i / 10.0k));
        _Accum a2 = a_min * (1.0k + (aseed * i / 10.0k));
        
        /* This product should trigger max/min range checks */
        _Accum a_prod = a1 * a2;
        
        /* Additional scaling to push beyond limits */
        _Accum a_scaled = a_prod * 1.5k;
        
        a_results[i] = a_scaled;
        
        /* 4. long _Accum with explicit integer promotion */
        long _Accum la1 = la_max;
        int scale_int = seed1 + i * seed2;  /* Can be positive or negative */
        
        /* Mixed integer/fixed-point operation - triggers range analysis */
        long _Accum la_scaled = la1 * (long _Accum)scale_int;
        
        /* Additional near-overflow operation */
        long _Accum la2 = laseed * i;
        long _Accum la_prod = la_scaled * la2;
        
        la_results[i] = la_prod;
        
        /* 5. Conditional expressions with fixed-point */
        _Accum a_cond = (scale_int > 0) ? a_max : a_min;
        _Accum a_mixed = a_cond * aseed;
        
        /* Simulate left shift through multiplication by power of two */
        int shift_amount = (i % 3) + 1;
        _Accum a_shifted = a_mixed;
        for (int s = 0; s < shift_amount; s++) {
            a_shifted = a_shifted * 2.0k;  /* Each multiplication may overflow */
        }
        
        a_results[i + 5] = a_shifted;
        
        /* 6. Saturating arithmetic simulation (triggers range checks) */
        short _Fract sf_small = 0.9999hr;
        short _Fract sf_small2 = 0.9999hr;
        short _Fract sf_small_prod = sf_small * sf_small2;
        
        /* Convert through _Fract */
        f_results[result_index++] = (_Fract)sf_small_prod;
    }
    
    /* 7. Complex expression with multiple fixed-point types */
    long _Accum final_la = 0.0lk;
    for (int i = 0; i < 5; i++) {
        /* Mix different fixed-point types in computation */
        _Accum a_val = a_results[i];
        long _Accum la_val = la_results[i];
        
        /* Convert and combine - may overflow at different stages */
        long _Accum converted = (long _Accum)a_val;
        final_la = final_la + converted * la_val;
        
        /* Additional scaling near overflow point */
        if (i % 2 == 0) {
            final_la = final_la * 0.999999lk;
        } else {
            final_la = final_la * 1.000001lk;
        }
    }
    
    la_results[5] = final_la;
    
    /* 8. Edge case: minimum value operations */
    signed _Fract sf_edge = sf_min;
    for (int j = 0; j < 3; j++) {
        sf_edge = sf_edge * 0.99r;  /* Should stay within bounds */
        sf_edge = sf_edge - 0.01r;  /* May underflow */
    }
    f_results[9] = sf_edge;
    
    /* Consume results to prevent optimization */
    consume_results(f_results, a_results, la_results, 10);
    
    /* Return hash of results */
    int hash = 0;
    for (int i = 0; i < 10; i++) {
        hash ^= *(int*)&f_results[i];
        hash ^= *(int*)&a_results[i];
        if (i < 6) {
            hash ^= *(int*)&la_results[i];
            hash ^= *((int*)&la_results[i] + 1);
        }
    }
    
    return hash & 0xFF;
}
