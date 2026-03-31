/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks in GCC
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent dead code elimination */
static void __attribute__((noinline,noipa)) 
consume_result(const void *p, int size) {
    volatile char sink;
    const char *cp = (const char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Mix of fixed-point types near their boundaries */
int main(void) {
    /* Volatile seeds to prevent constant folding */
    volatile int seed1 = 0x7FFFFFFF;
    volatile int seed2 = 0x80000000;
    volatile unsigned int useed = 0xFFFFFFFF;
    
    /* Array to accumulate results */
    long _Accum results[8] = {0};
    int result_idx = 0;
    
    /* Force range analysis with varying values */
    for (int iter = 0; iter < 100; iter++) {
        /* Read volatile seeds to create data-dependent values */
        int s1 = seed1 - iter;
        int s2 = seed2 + iter;
        unsigned int us = useed - iter * 1000;
        
        /* 1. Signed accumulative types near max/min boundaries */
        _Accum a1 = (_Accum)s1 / 0x800000;  /* Near max _Accum */
        _Accum a2 = (_Accum)s2 / 0x800000;  /* Near min _Accum */
        
        /* Multiplication that could overflow range */
        _Accum prod = a1 * a2;
        
        /* Left shift that could overflow */
        _Accum shifted = prod << 1;
        
        /* Store intermediate result */
        if (result_idx < 8) {
            results[result_idx++] = shifted;
        }
        
        /* 2. Long accum with values near 1.0 */
        long _Accum la1 = 0.999999999lk;
        long _Accum la2 = 0.999999999lk;
        
        /* This product mathematically exceeds 1.0, testing overflow */
        long _Accum lprod = la1 * la2;
        
        /* Additional shift to potentially trigger range check */
        long _Accum lshifted = lprod << 1;
        
        if (result_idx < 8) {
            results[result_idx++] = lshifted;
        }
        
        /* 3. Unsigned fractional types near 1.0 */
        unsigned _Fract uf1 = (unsigned _Fract)us / 0x10000;
        unsigned _Fract uf2 = 0.9999999ur;
        
        /* Addition that could wrap */
        unsigned _Fract uf_sum = uf1 + uf2;
        
        /* Convert to accum for storage */
        if (result_idx < 8) {
            results[result_idx++] = (_Accum)uf_sum;
        }
        
        /* 4. Signed fractional types near boundaries */
        _Fract sf1 = (_Fract)s1 / 0x800000;
        _Fract sf2 = -0.9999999r;
        
        /* Complex expression with multiple steps */
        _Fract temp = sf1 * sf2;
        _Fract shifted_fract = temp << 1;
        
        if (result_idx < 8) {
            results[result_idx++] = (_Accum)shifted_fract;
        }
        
        /* 5. Short fixed-point types */
        short _Fract sf_short = 0.9999shr;
        short _Accum sa_short = 0.99999hk;
        
        /* Mixed-type operations */
        short _Accum mixed = sf_short * sa_short;
        short _Accum shifted_short = mixed << 2;
        
        if (result_idx < 8) {
            results[result_idx++] = (_Accum)shifted_short;
        }
        
        /* 6. Integer promotions with fixed-point */
        int i = s1 >> 8;
        _Accum from_int = (_Accum)i * 0.5k;
        _Accum shifted_from_int = from_int << 3;
        
        if (result_idx < 8) {
            results[result_idx++] = shifted_from_int;
        }
        
        /* 7. Conditional expressions with fixed-point */
        _Accum cond_result = (iter & 1) ? a1 : a2;
        _Accum cond_scaled = cond_result * 1.5k;
        
        if (result_idx < 8) {
            results[result_idx++] = cond_scaled;
        }
        
        /* 8. Saturation-like check (emulates overflow detection) */
        long _Accum test_val = lprod;
        /* This comparison may trigger the exact range check in fixed-value.cc */
        if (test_val > 0.999999999lk) {
            test_val = 0.999999999lk;
        } else if (test_val < -0.999999999lk) {
            test_val = -0.999999999lk;
        }
        
        if (result_idx < 8) {
            results[result_idx++] = test_val;
        }
    }
    
    /* 9. Additional edge cases in separate scope */
    {
        /* Force computation with maximum representable values */
        volatile _Accum max_accum = 0.9999999k;
        volatile _Accum min_accum = -0.9999999k;
        
        /* Operations that should trigger range limit checks */
        _Accum max_squared = max_accum * max_accum;
        _Accum min_squared = min_accum * min_accum;
        _Accum max_shifted = max_accum << 2;
        _Accum min_shifted = min_accum << 2;
        
        /* Store in results array if space */
        if (result_idx < 4) {
            results[result_idx++] = max_squared;
            results[result_idx++] = min_squared;
            results[result_idx++] = max_shifted;
            results[result_idx++] = min_shifted;
        }
    }
    
    /* Create a hash from results to prevent optimization */
    long _Accum hash = 0;
    for (int i = 0; i < 8; i++) {
        hash = hash * 0.12345k + results[i];
    }
    
    /* Consume results to prevent dead code elimination */
    consume_result(results, sizeof(results));
    consume_result(&hash, sizeof(hash));
    
    /* Return non-deterministic result based on hash */
    return (hash > 0) ? 0 : 1;
}
