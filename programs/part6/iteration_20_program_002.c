/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 0;
static volatile unsigned int vu1 = 0xFFFFFFFF;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, used))
static void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Complex fixed-point computations designed to trigger range analysis */
int main(void) {
    /* Array to accumulate results */
    _Accum results[16];
    int result_idx = 0;
    
    /* Seed values near boundaries */
    volatile _Accum a_near_max = 0.999999999k;      /* Very close to max */
    volatile _Accum a_near_min = -0.999999999k;     /* Very close to min */
    volatile unsigned _Fract uf_near_one = 0.9999999ur;
    volatile _Fract sf_near_one = 0.9999999r;
    volatile _Fract sf_near_minus_one = -0.9999999r;
    volatile long _Accum la_near_max = 0.99999999999999999lk;
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < 8; i++) {
        /* Use volatile to prevent constant propagation */
        volatile int shift = vi1 + i;
        volatile int scale = vi2 + i * 2;
        
        /* 1. Signed _Accum multiplication near overflow boundary */
        /* This should trigger max range check for signed accum */
        _Accum a1 = a_near_max;
        _Accum a2 = (_Accum)scale * 0.5k + 0.999k;
        _Accum product = a1 * a2;  /* May overflow */
        results[result_idx++] = product;
        
        /* 2. Left shift of fixed-point (converted to integer shift) */
        /* This often triggers range analysis for shifted values */
        _Accum shifted = a_near_max;
        /* Simulate left shift via multiplication by power of two */
        for (int s = 0; s < shift % 4; s++) {
            shifted = shifted * 2.0k;  /* Each multiply by 2 is like left shift */
        }
        results[result_idx++] = shifted;
        
        /* 3. Unsigned _Fract addition near 1.0 */
        /* Should trigger unsigned max range check */
        unsigned _Fract uf1 = uf_near_one;
        unsigned _Fract uf2 = 0.0000001ur * (i + 1);
        unsigned _Fract uf_sum = uf1 + uf2;  /* May saturate/wrap */
        /* Convert to signed for storage */
        results[result_idx++] = (_Accum)uf_sum;
        
        /* 4. Complex expression with mixed types and conditional */
        /* Forces range analysis across multiple operations */
        _Fract sf1 = (i & 1) ? sf_near_one : sf_near_minus_one;
        _Fract sf2 = (_Fract)vi3 * 0.001r + 0.5r;
        _Accum mixed = (_Accum)sf1 * (_Accum)sf2 * a_near_max;
        
        /* Conditional that depends on overflow-like check */
        if (mixed > 0.9k || mixed < -0.9k) {
            /* Scale down if near boundaries */
            mixed = mixed * 0.5k;
        }
        results[result_idx++] = mixed;
        
        /* 5. long _Accum with very precise near-max value */
        /* Tests wider fixed-point types */
        long _Accum la1 = la_near_max;
        long _Accum la2 = 1.00000000000000001lk;  /* Slightly > 1 */
        long _Accum la_product = la1 * la2;  /* Should trigger overflow check */
        results[result_idx++] = (_Accum)la_product;  /* Narrow for storage */
        
        /* 6. Explicit cast from integer with large value */
        /* Forces range analysis for conversion */
        int large_int = vu1 >> (i * 2);  /* Varying large int */
        _Accum from_int = (_Accum)large_int * 0.0001k;
        results[result_idx++] = from_int;
        
        /* 7. Chain of operations that could overflow at intermediate step */
        _Accum chain = 0.5k;
        for (int j = 0; j <= i; j++) {
            chain = chain * 1.8k;  /* Growth factor > 1 */
        }
        results[result_idx++] = chain;
        
        /* 8. Expression designed to hit the specific sgt/ugt comparison */
        /* Multiply by values that approach the reciprocal of the scaling factor */
        _Accum x = 0.9999999k;
        _Accum y = x / 0.5000001k;  /* Nearly 2.0, causes value near 2.0*x */
        _Accum z = x * y;  /* Should be very close to 2.0*x*x */
        results[result_idx++] = z;
    }
    
    /* Additional edge case: minimum value operations */
    _Accum min_test = a_near_min * a_near_min;  /* Square of near-min */
    results[result_idx++] = min_test;
    
    /* Test with explicit overflow check pattern */
    volatile _Accum v1 = 0.999999999k;
    volatile _Accum v2 = 0.999999999k;
    _Accum test_prod = v1 * v2;
    
    /* Manual overflow check similar to what compiler might generate */
    int overflow = 0;
    if (test_prod >= 0.999999999k || test_prod <= -0.999999999k) {
        overflow = 1;
    }
    results[result_idx++] = test_prod * (overflow ? 0.5k : 1.0k);
    
    /* Ensure all results are used */
    consume(results, sizeof(results));
    
    /* Return hash of results to prevent optimization */
    int hash = 0;
    for (int i = 0; i < 16 && i < result_idx; i++) {
        hash ^= *((int*)&results[i]);
    }
    return hash & 0xFF;
}
