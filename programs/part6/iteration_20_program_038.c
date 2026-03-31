/* test_fixed_point_ranges.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 0;
static volatile unsigned int vu1 = 0x7FFFFFFF;
static volatile unsigned int vu2 = 0x80000000;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, used))
static void consume_result(const void *ptr, int size) {
    volatile char sink;
    const char *p = (const char *)ptr;
    for (int i = 0; i < size; i++) {
        sink = p[i];
    }
}

int main(void) {
    /* Array to accumulate results */
    long _Accum results[16];
    int result_idx = 0;
    
    /* Seed values that will force range analysis */
    volatile _Accum a_seed = 0.0k;
    volatile _Accum b_seed = 0.0k;
    volatile long _Accum la_seed = 0.0lk;
    volatile long _Accum lb_seed = 0.0lk;
    volatile unsigned _Fract uf_seed = 0.0ur;
    volatile _Fract f_seed = 0.0r;
    
    /* Initialize seeds with values near boundaries */
    a_seed = 0.999999k;          /* Near max for _Accum */
    b_seed = -0.999999k;         /* Near min for _Accum */
    la_seed = 0.999999999lk;     /* Near max for long _Accum */
    lb_seed = -0.999999999lk;    /* Near min for long _Accum */
    uf_seed = 0.9999999ur;       /* Near max for unsigned _Fract */
    f_seed = 0.9999999r;         /* Near max for signed _Fract */
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < 100; i++) {
        /* Vary the seeds based on volatile integers */
        _Accum a = a_seed * (_Accum)(vi1 + i % 3) / 3.0k;
        _Accum b = b_seed * (_Accum)(vi2 + i % 5) / 5.0k;
        
        /* 1. Multiplication near overflow boundaries for _Accum */
        /* This should trigger max_r/max_s comparison */
        _Accum prod1 = a * 0.999999k;
        _Accum prod2 = b * (-0.999999k);
        
        /* Complex expression with intermediate overflow check */
        _Accum temp = prod1 + prod2;
        _Accum scaled = temp * (_Accum)((i % 7) - 3) / 3.0k;
        
        results[result_idx++ % 16] = (long _Accum)scaled;
        
        /* 2. Long _Accum operations with left-shift simulation */
        /* This should trigger the min_s calculation with alshift */
        long _Accum la = la_seed * (long _Accum)(vu1 % (i + 1) + 1) / 1000000.0lk;
        long _Accum lb = lb_seed * (long _Accum)(vu2 % (i + 1) + 1) / 1000000.0lk;
        
        /* Multiplication that could overflow */
        long _Accum lprod = la * lb;
        
        /* Simulate left shift by multiplying by power of 2 */
        /* This creates values that need range checking against i_f_bits */
        int shift = (i % 4) + 1;
        long _Accum shifted = lprod;
        for (int s = 0; s < shift; s++) {
            shifted = shifted * 2.0lk;  /* Equivalent to << 1 */
        }
        
        results[result_idx++ % 16] = shifted;
        
        /* 3. Unsigned _Fract operations near 1.0 */
        /* Should trigger unsigned greater-than comparison (ugt) */
        unsigned _Fract uf = uf_seed;
        for (int j = 0; j < (i % 3); j++) {
            uf = uf * 0.9999999ur;
        }
        
        /* Addition that could wrap around */
        unsigned _Fract uf_add = uf + (unsigned _Fract)(i % 100) / 1000.0ur;
        
        /* Convert to long _Accum for storage */
        results[result_idx++ % 16] = (long _Accum)uf_add;
        
        /* 4. Signed _Fract with values near -1.0 and 1.0 */
        _Fract f = f_seed * (_Fract)((i % 2 == 0) ? 1 : -1);
        _Fract f_scaled = f * (_Fract)(i % 10) / 10.0r;
        
        /* Complex conditional expression forcing range analysis */
        _Fract f_result = (f_scaled > 0.5r) ? 
                          (f_scaled * 0.9999999r) : 
                          (f_scaled * (-0.9999999r));
        
        results[result_idx++ % 16] = (long _Accum)f_result;
        
        /* 5. Mixed-type expressions with integer promotions */
        int int_val = vi3 + i;
        _Accum mixed = (_Accum)int_val * 0.5k;
        
        /* Conditional based on overflow check simulation */
        /* This mimics the sgt/ugt comparison logic */
        _Accum test_val = mixed * 0.999999k;
        _Accum saturated;
        
        /* Manually check for overflow - compiler may analyze this */
        if (test_val > 0.999998k) {
            saturated = 0.999999k;
        } else if (test_val < -0.999998k) {
            saturated = -0.999999k;
        } else {
            saturated = test_val;
        }
        
        results[result_idx++ % 16] = (long _Accum)saturated;
        
        /* 6. Nested operations that create complex ranges */
        long _Accum nested = (long _Accum)a * (long _Accum)b;
        nested = nested * (long _Accum)f_result;
        nested = nested + (long _Accum)uf_add;
        
        /* Scale by variable amount - creates unknown range */
        int scale = (i % 8) + 1;
        long _Accum final_nested = nested;
        for (int k = 0; k < scale; k++) {
            final_nested = final_nested * 1.5lk;
        }
        
        results[result_idx++ % 16] = final_nested;
    }
    
    /* Force compiler to analyze all paths by using results */
    volatile int hash = 0;
    for (int i = 0; i < 16; i++) {
        /* Create data-dependent hash to prevent optimization */
        hash ^= ((int*)&results[i])[0];
        hash ^= ((int*)&results[i])[1];
    }
    
    /* Consume results to prevent dead code elimination */
    consume_result(results, sizeof(results));
    
    return hash != 0;
}
