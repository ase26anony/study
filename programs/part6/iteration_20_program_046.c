/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent optimization of inputs */
volatile int vi = 100;
volatile unsigned int vu = 200;
volatile long vl = -1000;
volatile unsigned long vul = 1000;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef unsigned short _Fract usfract_t;
typedef unsigned _Fract ufract_t;
typedef unsigned long _Fract ulfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;
typedef unsigned short _Accum usaccum_t;
typedef unsigned _Accum uaccum_t;
typedef unsigned long _Accum ulaccum_t;

int main(void) {
    /* Initialize volatile seeds */
    volatile int seed1 = vi;
    volatile unsigned int seed2 = vu;
    volatile long seed3 = vl;
    volatile unsigned long seed4 = vul;
    
    /* Arrays to store results */
    fract_t fract_results[10] = {0};
    accum_t accum_results[10] = {0};
    laccum_t laccum_results[10] = {0};
    ufract_t ufract_results[10] = {0};
    uaccum_t uaccum_results[10] = {0};
    
    int result_index = 0;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 10; i++) {
        /* Vary the seeds each iteration */
        int int_val = seed1 + i * 10;
        unsigned int uint_val = seed2 + i * 20;
        long long_val = seed3 + i * 100;
        unsigned long ulong_val = seed4 + i * 200;
        
        /* ===== SIGNED ACCUM TYPES - near maximum range ===== */
        
        /* _Accum operations that approach max range */
        accum_t a1 = (accum_t)int_val / 256;  /* Convert to _Accum */
        accum_t a2 = (accum_t)long_val / 256;
        
        /* Multiplication that could overflow */
        accum_t a_prod = a1 * a2;
        
        /* Left shift that could overflow */
        accum_t a_shifted = a1;
        for (int j = 0; j < 3; j++) {
            a_shifted = a_shifted * 2.0k;  /* Simulate left shift */
        }
        
        /* Near-maximum value for _Accum */
        accum_t a_near_max = 0.999999999999999k;  /* Very close to max */
        accum_t a_near_min = -0.999999999999999k; /* Very close to min */
        
        /* Complex expression that could overflow */
        accum_t a_complex = (a_near_max * a_near_max) * 1.5k;
        
        accum_results[result_index] = a_prod + a_shifted + a_complex;
        result_index = (result_index + 1) % 10;
        
        /* ===== LONG _ACCUM - larger range ===== */
        
        laccum_t la1 = (laccum_t)long_val / 65536;
        laccum_t la2 = (laccum_t)ulong_val / 65536;
        
        /* Operations near boundaries */
        laccum_t la_near_max = 0.999999999999999999k;  /* long _Accum max */
        laccum_t la_near_min = -0.999999999999999999k; /* long _Accum min */
        
        /* Multiplication that mathematically exceeds range */
        laccum_t la_prod = la_near_max * la_near_max;
        
        /* Left shift simulation */
        laccum_t la_shifted = la1;
        for (int j = 0; j < 5; j++) {
            la_shifted = la_shifted * 4.0k;  /* Aggressive scaling */
        }
        
        laccum_results[i] = la_prod + la_shifted;
        
        /* ===== UNSIGNED FRACT TYPES - near 1.0 ===== */
        
        ufract_t uf1 = (ufract_t)uint_val / 65535;
        ufract_t uf_near_one = 0.9999999ur;  /* Very close to 1.0 */
        
        /* Addition that could wrap */
        ufract_t uf_sum = uf_near_one + 0.0000001ur;
        
        /* Multiplication near 1.0 */
        ufract_t uf_prod = uf_near_one * uf_near_one;
        
        ufract_results[i] = uf_sum + uf_prod;
        
        /* ===== SIGNED FRACT TYPES - near -1.0 and 1.0 ===== */
        
        fract_t f_near_pos = 0.9999999r;   /* Near +1.0 */
        fract_t f_near_neg = -0.9999999r;  /* Near -1.0 */
        
        /* Complex expression mixing near-boundary values */
        fract_t f_expr = (f_near_pos * f_near_neg) + 
                        (f_near_pos - f_near_neg) * 0.5r;
        
        /* Conditional expression forcing range analysis */
        fract_t f_cond = (int_val > 0) ? f_near_pos : f_near_neg;
        f_cond = f_cond * (fract_t)((i % 3) + 1);
        
        fract_results[i] = f_expr + f_cond;
        
        /* ===== UNSIGNED ACCUM TYPES ===== */
        
        uaccum_t ua1 = (uaccum_t)ulong_val / 256;
        uaccum_t ua_near_max = 0.999999999999999uk;  /* Near max */
        
        /* Operations that could overflow */
        uaccum_t ua_prod = ua_near_max * 1.5uk;
        uaccum_t ua_shifted = ua1 * 4.0uk;  /* Simulate left shift */
        
        uaccum_results[i] = ua_prod + ua_shifted;
        
        /* ===== MIXED TYPE OPERATIONS ===== */
        
        /* Fixed-point with integer promotions */
        int int_from_fixed = (int)(a_near_max * 256);
        accum_t fixed_from_int = (accum_t)int_from_fixed / 128;
        
        /* Complex chain of operations */
        accum_t chain = fixed_from_int;
        chain = chain * a_near_min;
        chain = chain + (accum_t)f_near_pos;
        chain = chain * 2.0k;
        
        accum_results[(i + 5) % 10] += chain;
        
        /* ===== DATA-DEPENDENT OVERFLOW CHECKS ===== */
        
        /* Simulate overflow checking logic */
        accum_t test_val = a_near_max;
        for (int scale = 1; scale <= 4; scale++) {
            accum_t scaled = test_val * (accum_t)scale;
            
            /* This comparison may trigger the range analysis */
            if (scaled > 0.99k || scaled < -0.99k) {
                /* Force compiler to consider boundary cases */
                test_val = test_val * 0.5k;
            } else {
                test_val = scaled;
            }
        }
        
        fract_results[(i + 3) % 10] += (fract_t)test_val;
    }
    
    /* Additional edge case: maximum left shift */
    {
        accum_t max_accum = 0.999999999999999k;
        accum_t min_accum = -0.999999999999999k;
        
        /* These operations are likely to trigger overflow checks */
        accum_t max_squared = max_accum * max_accum;
        accum_t min_squared = min_accum * min_accum;
        accum_t max_times_min = max_accum * min_accum;
        
        /* Chain operations to create complex range */
        accum_t final_accum = max_squared;
        for (int i = 0; i < 4; i++) {
            final_accum = final_accum * 1.1k;
        }
        
        accum_results[9] = final_accum + max_times_min + min_squared;
    }
    
    /* Additional edge case: unsigned near overflow */
    {
        ufract_t uf_max = 0.9999999ur;
        ufract_t uf_small = 0.0000001ur;
        
        /* This sum would mathematically exceed 1.0 */
        for (int i = 0; i < 5; i++) {
            uf_max = uf_max + uf_small;
        }
        
        ufract_results[9] = uf_max;
    }
    
    /* Consume results to prevent optimization */
    consume(fract_results, sizeof(fract_results));
    consume(accum_results, sizeof(accum_results));
    consume(laccum_results, sizeof(laccum_results));
    consume(ufract_results, sizeof(ufract_results));
    consume(uaccum_results, sizeof(uaccum_results));
    
    /* Create a simple hash of results for return value */
    int hash = 0;
    for (int i = 0; i < 10; i++) {
        hash ^= *(int*)&fract_results[i];
        hash ^= *(int*)&accum_results[i];
        hash ^= *(int*)&ufract_results[i];
        hash ^= *(int*)&uaccum_results[i];
    }
    
    return hash & 0xFF;  /* Return non-zero to indicate execution */
}
