/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent dead code elimination */
static void __attribute__((noinline)) consume_result(const void *p, int size) {
    volatile char sink;
    const char *bytes = (const char *)p;
    for (int i = 0; i < size; i++) {
        sink = bytes[i];
    }
}

/* Mix of fixed-point types to exercise different modes */
typedef _Fract fract_t;
typedef _Accum accum_t;
typedef long _Accum long_accum_t;
typedef unsigned _Fract ufract_t;
typedef short _Fract short_fract_t;
typedef long _Fract long_fract_t;

/* Global volatile seeds to prevent constant folding */
volatile int seed1 = 0x7FFFFFFF;
volatile int seed2 = 0x80000000;
volatile short seed3 = 32767;
volatile short seed4 = -32768;
volatile fract_t f_seed = 0.999999r;
volatile accum_t a_seed = 0.999999999k;

int main(void) {
    /* Arrays to store intermediate results */
    accum_t accum_results[8] = {0};
    fract_t fract_results[8] = {0};
    ufract_t ufract_results[8] = {0};
    
    /* Volatile counters to force range analysis */
    volatile int i, j;
    
    /* Hash accumulator */
    unsigned int hash = 0;
    
    /* Loop with varying values to force dynamic range analysis */
    for (i = 0; i < 8; i++) {
        /* Read volatile seeds to prevent optimization */
        int s1 = seed1;
        int s2 = seed2;
        short s3 = seed3;
        short s4 = seed4;
        fract_t fs = f_seed;
        accum_t as = a_seed;
        
        /* 1. Operations on _Accum near maximum range */
        /* These should trigger max_r/max_s initialization and comparison */
        accum_t a1 = (_Accum)s1 / 32768.0k;  /* Convert to _Accum range */
        accum_t a2 = (_Accum)s2 / 32768.0k;
        
        /* Multiplication that could overflow */
        accum_t a_prod = a1 * a2;
        
        /* Left shift that could overflow */
        accum_t a_shifted = a_prod;
        for (j = 0; j < 3; j++) {
            a_shifted = a_shifted * 2.0k;  /* Simulate left shift */
        }
        
        /* 2. Operations on _Fract near ±1.0 */
        fract_t f1 = (fract_t)s3 / 32767.0r;
        fract_t f2 = (fract_t)s4 / 32767.0r;
        
        /* Complex expression that could overflow */
        fract_t f_expr = (f1 * f2) * fs;
        f_expr = f_expr + (f_expr * 0.5r);
        
        /* 3. Operations on unsigned _Fract near 1.0 */
        ufract_t uf1 = (ufract_t)(s3 & 0x7FFF) / 32767.0ur;
        ufract_t uf2 = 0.9999999ur;
        
        /* Addition that could wrap around */
        ufract_t uf_sum = uf1 + uf2;
        if (uf_sum < uf1) {  /* Check for wrap-around */
            uf_sum = 0.9999999ur;
        }
        
        /* 4. Mixed-type operations with integer promotions */
        long_accum_t la1 = (long_accum_t)s1 / 2147483648.0lk;
        long_accum_t la2 = (long_accum_t)(s1 >> 1) / 2147483648.0lk;
        
        /* Large multiplication - likely to trigger overflow check */
        long_accum_t la_prod = la1 * la2;
        
        /* 5. Conditional expressions with fixed-point */
        accum_t a_cond = (s1 > 0) ? a1 : a2;
        a_cond = a_cond * ((s2 < 0) ? 0.999999999k : 0.5k);
        
        /* 6. Explicit overflow checking pattern */
        /* This mimics the kind of check that would use the uncovered code */
        accum_t test_val = as * 1.5k;
        accum_t max_val = 0.999999999k;
        accum_t min_val = -1.0k;
        
        /* Manual saturation - should trigger range comparison */
        if (test_val > max_val) {
            test_val = max_val;
        } else if (test_val < min_val) {
            test_val = min_val;
        }
        
        /* 7. Shift operations that require range checking */
        short_fract_t sf1 = 0.9999hr;
        for (j = 0; j < 4; j++) {
            sf1 = sf1 * 0.5hr;  /* Right shift simulation */
        }
        
        /* Store results */
        accum_results[i] = a_shifted + test_val;
        fract_results[i] = f_expr;
        ufract_results[i] = uf_sum;
        
        /* Update hash to use results */
        hash ^= *(unsigned int*)&accum_results[i];
        hash ^= *(unsigned int*)&fract_results[i];
        hash ^= *(unsigned int*)&ufract_results[i];
        
        /* Modify seeds for next iteration */
        seed1 ^= 0x12345678;
        seed2 += 0x11111111;
        seed3 ^= 0x5555;
        seed4 += 0x3333;
    }
    
    /* 8. Final complex expression that combines everything */
    accum_t final_accum = 0.0k;
    for (i = 0; i < 8; i++) {
        final_accum = final_accum + accum_results[i];
        if (final_accum > 0.999999999k) {
            final_accum = 0.999999999k;
        } else if (final_accum < -1.0k) {
            final_accum = -1.0k;
        }
    }
    
    /* 9. Array operations with fixed-point */
    fract_t fract_array[4][4];
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            fract_array[i][j] = (fract_t)((i * j) % 100) / 100.0r;
            if (i > 0 && j > 0) {
                fract_array[i][j] = fract_array[i-1][j-1] * fract_array[i][j];
            }
        }
    }
    
    /* Prevent optimization */
    consume_result(accum_results, sizeof(accum_results));
    consume_result(fract_results, sizeof(fract_results));
    consume_result(ufract_results, sizeof(ufract_results));
    consume_result(fract_array, sizeof(fract_array));
    consume_result(&final_accum, sizeof(final_accum));
    
    return (int)(hash % 256);
}
