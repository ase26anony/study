/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi1 = 1;
volatile int vi2 = -1;
volatile int vi3 = 100;
volatile int vi4 = -100;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, used))
static void consume(void *p, int size) {
    volatile char *cp = (volatile char *)p;
    for (int i = 0; i < size; i++) {
        cp[i];
    }
}

int main(void) {
    /* Array to accumulate results */
    long _Accum results[16];
    int result_idx = 0;
    
    /* Seed values that push against boundaries */
    volatile _Accum a_seed = 0.999999k;      /* Near max */
    volatile _Accum b_seed = -0.999999k;     /* Near min */
    volatile unsigned _Fract uf_seed = 0.9999999ur;
    volatile signed _Fract sf_seed = -0.9999999r;
    volatile long _Accum la_seed = 0.999999999lk;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 8; i++) {
        /* Vary the seeds based on volatile counter */
        _Accum a = a_seed * (_Accum)(vi1 + i) / 1000k;
        _Accum b = b_seed * (_Accum)(vi2 - i) / 1000k;
        
        /* 1. Multiplication near overflow boundaries */
        /* This should trigger max range checks for signed accum */
        long _Accum la1 = (long _Accum)a * (long _Accum)la_seed;
        long _Accum la2 = (long _Accum)b * (long _Accum)la_seed;
        
        /* Complex expression with intermediate overflow potential */
        long _Accum temp = la1 * la2 / 1000lk;
        
        /* Left shift simulation through multiplication */
        /* Shifting fixed-point left requires range checking */
        long _Accum shifted = temp * 4lk;  /* Equivalent to << 2 */
        
        results[result_idx++] = shifted;
        
        /* 2. Unsigned fract near overflow */
        unsigned _Fract uf = uf_seed;
        /* Operations that could wrap past 1.0 */
        for (int j = 0; j < 3; j++) {
            uf = uf + (unsigned _Fract)(0.0000001ur * (j + 1));
        }
        /* Convert to accum for storage */
        results[result_idx++] = (_Accum)uf;
        
        /* 3. Signed fract with negative boundary */
        signed _Fract sf = sf_seed;
        /* Push toward -1.0 boundary */
        sf = sf * (signed _Fract)(0.999r * (i + 1));
        results[result_idx++] = (_Accum)sf;
        
        /* 4. Integer to fixed-point conversions with range checks */
        int int_val = vi3 + i * vi4;
        /* Conversion that may overflow fixed-point range */
        _Accum from_int = (_Accum)int_val * 0.5k;
        
        /* Conditional expression forcing range analysis */
        _Accum cond_result = (int_val > 0) ? from_int : -from_int;
        results[result_idx++] = cond_result;
        
        /* 5. Nested operations that create complex ranges */
        /* a * b * c where each is near boundary */
        _Accum c = (_Accum)(vi1 + i) / 1000k;
        _Accum nested = a * b * c * 1000k;
        
        /* Simulate left shift overflow check */
        _Accum shifted2 = nested * 8k;  /* Potential overflow */
        results[result_idx++] = shifted2;
        
        /* 6. Explicit overflow checking pattern */
        /* GCC might generate range checks for this pattern */
        _Accum test_val = a * 2k;
        /* Check if we hit saturation */
        if (test_val > 0.99k || test_val < -0.99k) {
            results[result_idx++] = 0.5k;
        } else {
            results[result_idx++] = test_val;
        }
    }
    
    /* 7. Edge case: maximum representable values */
    /* These should directly trigger the max_r/max_s comparison */
    long _Accum max_accum = 0.999999999999999lk;
    long _Accum min_accum = -0.999999999999999lk;
    
    /* Operations that mathematically exceed the range */
    long _Accum overflow_test1 = max_accum * max_accum;
    long _Accum overflow_test2 = min_accum * max_accum;
    
    results[result_idx++] = overflow_test1;
    results[result_idx++] = overflow_test2;
    
    /* 8. Bit-exact boundary values */
    /* Using integer representation to create boundary values */
    typedef _Accum boundary_test_t;
    volatile int max_int = 0x7FFFFFFF;  /* Near max for _Accum */
    volatile int min_int = 0x80000000;  /* Near min for _Accum */
    
    /* Type-punning to create boundary values */
    union {
        int i;
        _Accum f;
    } u_max, u_min;
    
    u_max.i = max_int / (1 << 8);  /* Adjust for fixed-point format */
    u_min.i = min_int / (1 << 8);
    
    /* Operations on boundary values */
    _Accum boundary_op = u_max.f * 0.999k;
    results[result_idx++] = boundary_op;
    
    boundary_op = u_min.f * 0.999k;
    results[result_idx++] = boundary_op;
    
    /* Ensure we use all results */
    consume(results, sizeof(results));
    
    /* Return hash of results to prevent optimization */
    int hash = 0;
    for (int i = 0; i < 16; i++) {
        hash ^= ((int*)results)[i];
    }
    return hash & 0xFF;
}
