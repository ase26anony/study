/* test_fixed_point_ranges.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_ranges.c -o test.o
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 100;
static volatile int vi2 = -100;
static volatile unsigned int vu1 = 200;
static volatile _Fract vf = 0.5r;
static volatile _Accum va = 0.5k;
static volatile unsigned _Fract vuf = 0.7ur;

/* Noinline function to prevent dead code elimination */
__attribute__((noinline)) 
void consume_results(const _Accum *arr, int n) {
    volatile _Accum sink = 0.0k;
    for (int i = 0; i < n; i++) {
        sink += arr[i];
    }
    (void)sink;
}

int main(void) {
    /* Array to store results and prevent optimization */
    _Accum results[32] = {0.0k};
    int result_idx = 0;
    
    /* Seed values that will force range calculations */
    int seed1 = vi1;
    int seed2 = vi2;
    unsigned int useed = vu1;
    
    /* Loop with varying values to force dynamic range analysis */
    for (int iteration = 0; iteration < 16; iteration++) {
        /* 1. Operations with signed accumulative types near maximum range */
        _Accum a1 = (_Accum)seed1 * 0.01k;  /* Scale to appropriate range */
        _Accum a2 = (_Accum)seed2 * 0.01k;
        
        /* Near-maximum values for _Accum */
        _Accum near_max = 0.999999999k;  /* Very close to max */
        _Accum near_min = -0.999999999k; /* Very close to min */
        
        /* Multiplication that could overflow */
        _Accum product1 = a1 * near_max;
        _Accum product2 = near_max * near_max;  /* This should approach 1.0 */
        
        /* Left shift simulation with multiplication by power of 2 */
        int shift = iteration % 4;
        _Accum shifted = product1 * (1 << shift);  /* Simulates left shift */
        
        results[result_idx++] = product1;
        results[result_idx++] = product2;
        results[result_idx++] = shifted;
        
        /* 2. Operations with unsigned fractional types */
        unsigned _Fract uf1 = (unsigned _Fract)useed / 256.0ur;
        unsigned _Fract uf2 = 0.9999999ur;  /* Very close to 1.0 */
        
        /* Addition that could wrap */
        unsigned _Fract u_sum = uf1 + uf2;
        /* Multiplication near upper bound */
        unsigned _Fract u_prod = uf1 * uf2;
        
        /* Store via conversion to _Accum */
        results[result_idx++] = (_Accum)u_sum;
        results[result_idx++] = (_Accum)u_prod;
        
        /* 3. Operations with signed fractional types */
        _Fract sf1 = (_Fract)seed1 / 128.0r;
        _Fract sf2 = vf;
        
        /* Values near boundaries */
        _Fract near_pos_one = 0.999999r;
        _Fract near_neg_one = -0.999999r;
        
        /* Complex expression that could overflow */
        _Fract complex_expr = (sf1 * near_pos_one) + (sf2 * near_neg_one);
        _Fract scaled = complex_expr * 1.5r;  /* Could exceed range */
        
        results[result_idx++] = (_Accum)complex_expr;
        results[result_idx++] = (_Accum)scaled;
        
        /* 4. Mixed integer and fixed-point operations */
        int int_val = seed1 + iteration;
        _Accum mixed1 = (_Accum)int_val * 0.125k;
        _Accum mixed2 = mixed1 * (_Accum)(1 << (iteration % 3));
        
        /* Conditional expression forcing range analysis */
        _Accum cond_result = (int_val > 0) ? mixed1 : mixed2;
        
        results[result_idx++] = mixed1;
        results[result_idx++] = mixed2;
        results[result_idx++] = cond_result;
        
        /* 5. Explicit overflow checking pattern */
        /* This mimics the kind of check that would trigger the uncovered code */
        _Accum test_val = 0.0k;
        for (int j = 0; j < 4; j++) {
            test_val += near_max * 0.25k;
            /* The compiler may need to check if test_val exceeds max */
        }
        results[result_idx++] = test_val;
        
        /* 6. Left shift operations that directly trigger range checks */
        /* Convert to same type representation used in fixed-value.cc */
        long _Accum la1 = (long _Accum)a1;
        long _Accum la2 = (long _Accum)a2;
        
        /* Simulate left shift through multiplication */
        int shift_amount = 1 + (iteration % 2);
        long _Accum shifted_accum = la1 * (1 << shift_amount);
        
        /* This multiplication should trigger max range comparison */
        long _Accum large_product = la1 * la2 * (1 << shift_amount);
        
        results[result_idx++] = (_Accum)shifted_accum;
        results[result_idx++] = (_Accum)large_product;
        
        /* Update seeds for next iteration */
        seed1 += vi1;
        seed2 += vi2;
        useed += vu1;
        
        /* Ensure we don't overflow results array */
        if (result_idx >= 28) break;
    }
    
    /* Consume results to prevent optimization */
    consume_results(results, result_idx);
    
    /* Return a hash of the results */
    int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Create a simple hash */
        hash ^= *((int*)&results[i]);
    }
    
    return hash & 0xFF;
}
