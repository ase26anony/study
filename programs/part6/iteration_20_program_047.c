/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi1 = 1, vi2 = -1, vi3 = 0;
volatile unsigned int vu1 = 1, vu2 = 0x7FFFFFFF;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[32];
    int result_idx = 0;
    
    /* Initialize with volatile seeds to prevent constant propagation */
    volatile _Accum seed_acc = 0.5k;
    volatile long _Accum seed_lacc = -0.999999999k;
    volatile _Fract seed_fract = 0.9999999r;
    volatile unsigned _Fract seed_ufract = 0.9999999ur;
    volatile short _Accum seed_sacc = 0.9hk;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 16; i++) {
        /* Use volatile seeds in computations */
        _Accum a1 = seed_acc * i;
        long _Accum a2 = seed_lacc * i;
        _Fract f1 = seed_fract;
        unsigned _Fract uf1 = seed_ufract;
        
        /* Test 1: Signed accumulative types near maximum */
        /* These multiplications should approach or exceed max range */
        long _Accum la = 0.999999999k;  /* Very close to max */
        long _Accum lb = 0.999999999k;
        long _Accum lc = la * lb;  /* Product may overflow */
        
        /* Force range analysis with left shift */
        _Accum shifted = a1;
        for (int s = 0; s < 4; s++) {
            shifted = shifted * 2.0k;  /* Equivalent to left shift */
        }
        
        /* Test 2: Complex expression with intermediate overflow check */
        /* The compiler may need to compute range for (a1 * a1) */
        _Accum temp = a1 * a1;
        _Accum temp2 = temp * 2.0k;
        
        /* Store intermediate results */
        if (result_idx < 32) results[result_idx++] = temp2;
        
        /* Test 3: Unsigned fractional types near 1.0 */
        unsigned _Fract uf = 0.9999999ur;
        /* Operations that could wrap to 0 */
        unsigned _Fract uf_add = uf + 0.0000001ur;
        unsigned _Fract uf_mul = uf * 1.0000001ur;
        
        /* Test 4: Signed fractional types near boundaries */
        _Fract sf_pos = 0.9999999r;
        _Fract sf_neg = -0.9999999r;
        _Fract sf_prod = sf_pos * sf_neg;  /* Should be near -1.0 */
        
        /* Test 5: Mix with integer promotions and casts */
        int int_val = vi1 + i;
        _Accum from_int = (_Accum)int_val * 0.5k;
        long _Accum from_int_large = (_Accum)vu1 * 0.25k;
        
        /* Test 6: Conditional expressions with fixed-point */
        _Accum cond_result = (i & 1) ? sf_pos : sf_neg;
        cond_result = cond_result * 0.5k;
        
        /* Test 7: Explicit overflow checking pattern */
        /* Simulate overflow check by comparing against limits */
        _Accum check_val = a1 * 1.5k;
        /* This comparison may trigger the range analysis */
        if (check_val > 0.999999k || check_val < -0.999999k) {
            check_val = (check_val > 0) ? 0.999999k : -0.999999k;
        }
        
        /* Test 8: Left shift simulation with multiplication */
        short _Accum sa = seed_sacc;
        /* Simulate left shift by multiplying by powers of 2 */
        short _Accum sa_shifted = sa;
        for (int shift = 0; shift < 3; shift++) {
            sa_shifted = sa_shifted * 2.0hk;
        }
        
        /* Test 9: Nested operations that could overflow */
        long _Accum nested = la;
        for (int j = 0; j < 3; j++) {
            nested = nested * lb;
            if (result_idx < 32) results[result_idx++] = nested;
        }
        
        /* Test 10: Boundary case with explicit max/min values */
        /* Use values that should trigger max_r/min_r comparisons */
        _Accum boundary_test = (i == 0) ? 0.999999k : -0.999999k;
        boundary_test = boundary_test * boundary_test;
        
        /* Store more results */
        if (result_idx < 32) results[result_idx++] = from_int;
        if (result_idx < 32) results[result_idx++] = cond_result;
        if (result_idx < 32) results[result_idx++] = check_val;
        if (result_idx < 32) results[result_idx++] = boundary_test;
        
        /* Modify seeds slightly each iteration */
        seed_acc = seed_acc * 0.9k;
        seed_lacc = seed_lacc * 0.95k;
    }
    
    /* Additional edge case tests outside loop */
    
    /* Test A: Direct maximum value operations */
    long _Accum max_val = 0.999999999k;
    long _Accum max_squared = max_val * max_val;
    
    /* Test B: Minimum value operations */
    long _Accum min_val = -0.999999999k;
    long _Accum min_squared = min_val * min_val;
    
    /* Test C: Near-overflow multiplication */
    _Accum near_max1 = 0.9k;
    _Accum near_max2 = 1.1k;  /* > 1.0, will overflow when multiplied */
    _Accum overflow_test = near_max1 * near_max2;
    
    /* Test D: Integer to fixed-point with large values */
    int large_int = 1000000;
    _Accum from_large_int = (_Accum)large_int * 0.001k;
    
    /* Store final results */
    if (result_idx < 32) results[result_idx++] = max_squared;
    if (result_idx < 32) results[result_idx++] = min_squared;
    if (result_idx < 32) results[result_idx++] = overflow_test;
    if (result_idx < 32) results[result_idx++] = from_large_int;
    
    /* Test E: Complex expression tree */
    _Accum x = 0.75k;
    _Accum y = 0.8k;
    _Accum z = 0.9k;
    /* This creates a complex expression that might overflow */
    _Accum complex_expr = (x * y) * (z * 1.2k) * (x + y) * (z - 0.1k);
    if (result_idx < 32) results[result_idx++] = complex_expr;
    
    /* Test F: Shift-like behavior with saturation */
    _Accum shift_sim = 0.5k;
    for (int shift = 0; shift < 5; shift++) {
        shift_sim = shift_sim * 2.0k;  /* Each multiply by 2 = left shift by 1 */
        if (result_idx < 32) results[result_idx++] = shift_sim;
    }
    
    /* Prevent optimization of results */
    consume(results, sizeof(results));
    
    /* Create a simple hash of results to return */
    int hash = 0;
    for (int i = 0; i < result_idx && i < 32; i++) {
        /* Access fixed-point as integer for hashing */
        union {
            _Accum f;
            int i;
        } u;
        u.f = results[i];
        hash ^= u.i ^ (i * 0x5A5A5A5A);
    }
    
    return hash & 0xFF;
}
