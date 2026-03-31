/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi1 = 1000;
volatile int vi2 = -1000;
volatile unsigned int vu1 = 2000;
volatile _Fract vf1 = 0.999999r;
volatile _Fract vf2 = -0.999999r;
volatile unsigned _Fract vuf1 = 0.999999ur;
volatile _Accum va1 = 0.999999999k;
volatile _Accum va2 = -0.999999999k;
volatile long _Accum vla1 = 0.99999999999999999lk;
volatile long _Accum vla2 = -0.99999999999999999lk;

/* Noinline function to prevent optimization */
__attribute__((noinline))
void consume_results(_Fract *farr, _Accum *aarr, long _Accum *laarr, int size) {
    volatile int sink = 0;
    for (int i = 0; i < size; i++) {
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
    int idx = 0;
    
    /* Use volatile seeds to prevent compile-time evaluation */
    int seed1 = vi1;
    int seed2 = vi2;
    unsigned int useed1 = vu1;
    
    /* Loop with varying fixed-point values */
    for (int i = 0; i < 5; i++) {
        /* 1. Operations on signed _Fract near boundaries */
        _Fract f1 = vf1;
        _Fract f2 = vf2;
        
        /* Multiplication that could overflow */
        _Fract f_mul = f1 * f2;  /* ~ -0.999998 */
        
        /* Left shift simulation through multiplication */
        _Fract f_shifted = f1 * 2.0r;  /* Should saturate near 1.0 */
        
        /* Complex expression with conditional */
        _Fract f_cond = (seed1 > 0) ? f1 : f2;
        f_cond = f_cond * 0.999r;
        
        f_results[idx++] = f_mul;
        f_results[idx++] = f_shifted;
        f_results[idx++] = f_cond;
        
        /* 2. Operations on unsigned _Fract */
        unsigned _Fract uf1 = vuf1;
        unsigned _Fract uf2 = 0.000001ur;
        
        /* Addition near upper bound */
        unsigned _Fract uf_sum = uf1 + uf2;  /* Should saturate to 1.0ur */
        
        /* Multiplication near 1.0 */
        unsigned _Fract uf_mul = uf1 * 0.999999ur;
        
        f_results[idx++] = (_Fract)uf_sum;
        f_results[idx++] = (_Fract)uf_mul;
        
        /* 3. Operations on _Accum types - likely to trigger the uncovered code */
        _Accum a1 = va1;
        _Accum a2 = va2;
        
        /* Multiplication of two large magnitude values */
        _Accum a_mul = a1 * a2;  /* ~ -0.999999998 */
        
        /* Shift-like operation through multiplication */
        _Accum a_scaled = a1 * 1.999999999k;  /* Near 2.0 boundary */
        
        /* Complex expression with integer promotion */
        int scale = (seed2 < 0) ? 2 : 3;
        _Accum a_int_scaled = (_Accum)scale * a1;
        
        a_results[i*2] = a_mul;
        a_results[i*2 + 1] = a_scaled;
        
        /* 4. Operations on long _Accum - more bits for range analysis */
        long _Accum la1 = vla1;
        long _Accum la2 = vla2;
        
        /* Product that mathematically exceeds range */
        long _Accum la_mul = la1 * la2;
        
        /* Left shift simulation with large multiplier */
        long _Accum la_lshift = la1 * 3.0lk;
        
        /* Conditional expression with mixed types */
        long _Accum la_cond = (useed1 > 1000) ? la1 : la2;
        la_cond = la_cond * 0.5lk;
        
        la_results[i*2] = la_mul;
        la_results[i*2 + 1] = la_lshift;
        
        /* 5. Explicit overflow checking logic */
        /* This should force range analysis to check boundaries */
        _Accum test_val = a1;
        _Accum increment = 0.000000001k;
        
        /* Loop that accumulates to boundary */
        for (int j = 0; j < 10; j++) {
            test_val = test_val + increment;
            /* Force range check by using in conditional */
            if (test_val > 0.999999999k) {
                test_val = 0.999999999k;  /* Manual saturation */
            }
        }
        a_results[8] = test_val;
        
        /* 6. Integer to fixed-point conversions with range issues */
        int large_int = seed1 * 100;
        _Accum from_int = (_Accum)large_int;  /* May overflow */
        
        unsigned int large_uint = useed1 * 100;
        unsigned _Accum from_uint = (unsigned _Accum)large_uint;
        
        a_results[9] = from_int;
        la_results[9] = (long _Accum)from_uint;
        
        /* Modify seeds to vary the computation */
        seed1 = seed1 + vi2;
        seed2 = seed2 + vi1;
        useed1 = useed1 + vu1;
    }
    
    /* 7. Additional boundary cases with explicit near-max values */
    /* These should directly exercise the range comparison logic */
    
    /* Maximum representable _Accum */
    const _Accum max_accum = 0.999999999k;
    const _Accum min_accum = -0.999999999k;
    
    /* Operations that should trigger overflow checks */
    _Accum near_max = max_accum * 0.999999999k;
    _Accum near_min = min_accum * 0.999999999k;
    
    /* Attempt to exceed maximum through addition chain */
    _Accum chain = 0.5k;
    for (int i = 0; i < 10; i++) {
        chain = chain + 0.1k;
    }
    
    /* Multiplication that mathematically exceeds 1.0 */
    _Accum a = 0.9k;
    _Accum b = 1.2k;
    _Accum overflow_test = a * b;  /* 1.08 > 1.0 */
    
    a_results[0] = near_max;
    a_results[1] = near_min;
    a_results[2] = chain;
    a_results[3] = overflow_test;
    
    /* 8. Long _Accum with more precision - more likely to hit the double-int logic */
    const long _Accum max_long_accum = 0.99999999999999999lk;
    long _Accum la_overflow = max_long_accum;
    
    /* Series of multiplications approaching and exceeding limit */
    for (int i = 0; i < 3; i++) {
        la_overflow = la_overflow * 0.99999999999999999lk;
    }
    
    /* Final multiplication that might overflow */
    la_overflow = la_overflow * 1.00000000000000001lk;
    
    la_results[0] = la_overflow;
    
    /* Prevent dead code elimination */
    consume_results(f_results, a_results, la_results, 10);
    
    /* Return hash of results */
    int hash = 0;
    for (int i = 0; i < 10; i++) {
        hash ^= *(int*)&f_results[i];
        hash ^= *(int*)&a_results[i];
        hash ^= *(int*)&la_results[i];
    }
    
    return hash & 0xFF;
}
