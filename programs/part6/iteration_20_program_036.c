/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume_result(const void *p, int size) {
    asm volatile("" : : "r"(p), "r"(size) : "memory");
}

/* Helper to create complex expressions */
__attribute__((noinline, noipa))
long _Accum compute_product(long _Accum a, long _Accum b, int shift) {
    /* This should trigger range analysis for multiplication */
    long _Accum temp = a * b;
    
    /* Mix with integer promotion */
    int scaled = (int)temp * shift;
    
    /* More fixed-point operations */
    _Accum mid = (_Accum)scaled * 0.5k;
    
    /* Return complex expression */
    return temp + mid;
}

int main(void) {
    /* Array to accumulate results */
    long _Accum results[8] = {0k};
    
    /* Initialize with volatile seeds to prevent constant folding */
    int seed1 = vi;
    unsigned seed2 = vu;
    long seed3 = vl;
    
    /* Test various fixed-point types near their boundaries */
    
    /* 1. Signed accumulative types - push against max/min */
    _Accum a1 = 0.999999k;  /* Very close to max _Accum */
    _Accum a2 = -0.999999k; /* Very close to min _Accum */
    
    /* 2. Long accum - even more precision */
    long _Accum la1 = 0.999999999k;
    long _Accum la2 = -0.999999999k;
    
    /* 3. Fractional types */
    _Fract f1 = 0.999999r;
    _Fract f2 = -0.999999r;
    unsigned _Fract uf1 = 0.9999999ur;
    
    /* 4. Smaller types */
    short _Accum sa1 = 0.999k;
    short _Fract sf1 = 0.99r;
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < 100; i++) {
        /* Use volatile seeds to create data-dependent values */
        int idx = (seed1 + i) & 7;
        unsigned scale = (seed2 + i * 3) & 0xF;
        long offset = seed3 + i * 1000;
        
        /* Complex expression 1: Multiplication near overflow */
        long _Accum val1 = la1;
        long _Accum val2 = la2;
        
        /* Adjust values based on loop iteration */
        if (i & 1) {
            val1 = val1 - (long _Accum)(i * 0.000000001k);
            val2 = val2 + (long _Accum)(i * 0.000000001k);
        }
        
        /* This multiplication should trigger overflow range check */
        long _Accum product = val1 * val2;
        
        /* Complex expression 2: Left shift simulation */
        /* Fixed-point left shift via multiplication by power of two */
        int shift_amt = (scale % 8) + 1;
        _Accum shifted = a1;
        for (int s = 0; s < shift_amt; s++) {
            shifted = shifted * 2.0k;  /* Simulate left shift */
        }
        
        /* Complex expression 3: Mixed-type operations */
        /* Convert between fixed-point types with different ranges */
        long _Accum from_fract = (long _Accum)f1 * (long _Accum)uf1;
        
        /* Complex expression 4: Conditional with fixed-point */
        _Accum cond_result = (product > 0k) ? 
            (_Accum)(from_fract * 0.5k) : 
            (_Accum)(-from_fract * 0.5k);
        
        /* Complex expression 5: Accumulate with saturation check */
        long _Accum accum = results[idx];
        long _Accum new_accum = accum + product * 0.1k;
        
        /* Manual overflow check - should trigger compiler's internal check */
        if (new_accum > 0.999999999k) {
            new_accum = 0.999999999k;
        } else if (new_accum < -0.999999999k) {
            new_accum = -0.999999999k;
        }
        
        results[idx] = new_accum;
        
        /* Complex expression 6: Integer to fixed-point with scaling */
        int int_val = offset + i * 100;
        _Accum scaled_fixed = (_Accum)int_val * 0.001k;
        
        /* Use in another operation */
        results[(idx + 1) & 7] += (long _Accum)scaled_fixed;
        
        /* Complex expression 7: Nested operations */
        short _Accum temp_sa = sa1 * (short _Accum)sf1;
        _Accum promoted = (_Accum)temp_sa * a1;
        results[(idx + 2) & 7] += (long _Accum)promoted;
        
        /* Complex expression 8: Very close to boundary */
        unsigned _Fract uf_temp = uf1;
        for (int j = 0; j < 3; j++) {
            uf_temp = uf_temp * 0.999999ur;
        }
        /* This addition might overflow */
        uf_temp = uf_temp + 0.0000001ur;
        
        /* Convert and store */
        results[(idx + 3) & 7] += (long _Accum)uf_temp;
    }
    
    /* Additional boundary tests in straight-line code */
    
    /* Test case A: Direct maximum value test */
    long _Accum max_test = 0.999999999k;
    long _Accum max_test2 = 0.999999999k;
    long _Accum max_product = max_test * max_test2;  /* Should overflow */
    
    /* Test case B: Minimum value test */
    long _Accum min_test = -0.999999999k;
    long _Accum min_test2 = -0.999999999k;
    long _Accum min_product = min_test * min_test2;  /* Should overflow positive */
    
    /* Test case C: Cross multiplication */
    long _Accum cross_product = max_test * min_test;  /* Large negative */
    
    /* Test case D: Shift via multiplication */
    _Accum shift_test = 0.5k;
    _Accum shifted_4 = shift_test * 16.0k;  /* 4-bit left shift */
    _Accum shifted_8 = shift_test * 256.0k; /* 8-bit left shift - may overflow */
    
    /* Test case E: Fractional type boundary */
    unsigned _Fract uf_max = 0.9999999ur;
    unsigned _Fract uf_inc = uf_max + 0.0000001ur;  /* May saturate */
    
    /* Test case F: Signed fractional boundary */
    _Fract f_max = 0.999999r;
    _Fract f_min = -0.999999r;
    _Fract f_sum = f_max + f_max;  /* May overflow */
    _Fract f_diff = f_min - f_max; /* Large negative */
    
    /* Store all test results */
    results[4] = max_product;
    results[5] = min_product;
    results[6] = cross_product;
    results[7] = (long _Accum)shifted_8 + (long _Accum)f_sum;
    
    /* Prevent dead code elimination */
    consume_result(results, sizeof(results));
    
    /* Create a hash of results to return */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        /* Mix bits from fixed-point representation */
        union {
            long _Accum f;
            long long i;
        } u;
        u.f = results[i];
        hash ^= (int)u.i ^ (int)(u.i >> 32);
    }
    
    return hash & 0xFF;
}
