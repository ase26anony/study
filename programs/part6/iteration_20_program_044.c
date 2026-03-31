/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume_result(const void *ptr, int size) {
    asm volatile("" : : "r"(ptr), "r"(size) : "memory");
}

/* Helper to create complex expressions */
__attribute__((noinline, noipa))
long _Accum compute_product(long _Accum a, long _Accum b) {
    /* This multiplication may overflow the fixed-point range */
    return a * b;
}

__attribute__((noinline, noipa))
_Accum shift_and_multiply(_Accum a, _Accum b, int shift) {
    /* Complex expression that may trigger range analysis */
    _Accum temp = a * b;
    /* Simulate left shift via multiplication by power of two */
    for (int i = 0; i < shift; i++) {
        temp = temp + temp;  /* temp * 2 */
    }
    return temp;
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[8] = {0.0k};
    long _Accum long_results[4] = {0.0lk};
    
    /* Initialize with volatile to prevent constant propagation */
    int seed1 = vi;
    unsigned int seed2 = vu;
    
    /* Loop with varying values to force dynamic range analysis */
    for (int iter = 0; iter < 100; iter++) {
        /* Use seeds to create varying but bounded values */
        int base = (seed1 + iter) % 256;
        unsigned int ubase = (seed2 + iter) % 256;
        
        /* Test 1: Signed accumulative types near maximum range */
        {
            /* Values very close to maximum representable */
            long _Accum la = (long _Accum)(0.999999999lk);
            long _Accum lb = (long _Accum)(0.999999999lk);
            
            /* This product should mathematically overflow the range */
            long _Accum lc = compute_product(la, lb);
            
            /* Additional operation that might trigger overflow check */
            long _Accum ld = lc + lc;  /* Could overflow */
            
            long_results[iter % 4] = ld;
        }
        
        /* Test 2: Unsigned fractional types near 1.0 */
        {
            unsigned _Fract uf1 = (unsigned _Fract)(0.9999999ur);
            unsigned _Fract uf2 = (unsigned _Fract)(0.0000001ur);
            
            /* Addition that could wrap around */
            unsigned _Fract uf3 = uf1 + uf2;
            
            /* Convert to signed for storage */
            results[0] += (_Accum)uf3;
        }
        
        /* Test 3: Signed fractional types at boundaries */
        {
            _Fract f1 = (_Fract)(0.999999k);  /* Near +1.0 */
            _Fract f2 = (_Fract)(-0.999999k); /* Near -1.0 */
            
            /* Multiplication near the limits */
            _Fract f3 = f1 * f2;  /* Should be near -1.0 */
            
            /* Complex expression with conditional */
            _Fract f4 = (base > 128) ? f1 : f2;
            _Fract f5 = f3 * f4;
            
            results[1] += (_Accum)f5;
        }
        
        /* Test 4: Mixed integer and fixed-point with casts */
        {
            int i = base - 128;  /* Range: -128 to 127 */
            _Accum a = (_Accum)i * 0.5k;
            
            /* Left shift simulation that could overflow */
            _Accum b = shift_and_multiply(a, 2.0k, 2);
            
            results[2] += b;
        }
        
        /* Test 5: Very large shift operations */
        {
            short _Fract sf = (short _Fract)(0.5r);
            /* Create value that when shifted might overflow */
            _Accum shifted = (_Accum)sf;
            
            /* Multiple shifts - compiler must analyze range */
            for (int s = 0; s < 4; s++) {
                shifted = shifted + shifted;  /* shift left by 1 */
            }
            
            results[3] += shifted;
        }
        
        /* Test 6: Accumulating near overflow */
        {
            _Accum acc = 0.0k;
            for (int i = 0; i < 10; i++) {
                acc += (_Accum)(0.999999k / 10.0k);
            }
            /* acc should be very close to 1.0 */
            results[4] = acc;
        }
        
        /* Test 7: Check overflow via comparison (may trigger sgt/ugt checks) */
        {
            long _Accum max_val = (long _Accum)(0.999999999lk);
            long _Accum test_val = long_results[iter % 4];
            
            /* This comparison may trigger the exact uncovered code */
            if (test_val > max_val) {
                /* Force saturation */
                long_results[iter % 4] = max_val;
            }
        }
        
        /* Test 8: Unsigned saturation check */
        {
            unsigned _Accum ua = (unsigned _Accum)(0.99999999uk);
            unsigned _Accum ub = (unsigned _Accum)(0.00000001uk);
            unsigned _Accum uc = ua + ub;
            
            /* Convert for storage */
            results[5] += (_Accum)uc;
        }
    }
    
    /* Final hash computation to use all results */
    int hash = 0;
    for (int i = 0; i < 6; i++) {
        /* Create integer representation for hashing */
        int val = (int)(results[i] * 1000000);
        hash ^= val ^ (val >> 16);
    }
    for (int i = 0; i < 4; i++) {
        long long val = (long long)(long_results[i] * 1000000000LL);
        hash ^= (int)val ^ (int)(val >> 32);
    }
    
    /* Prevent dead code elimination */
    consume_result(results, sizeof(results));
    consume_result(long_results, sizeof(long_results));
    
    return hash & 0xFF;
}
