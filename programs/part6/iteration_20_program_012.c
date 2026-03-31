/* Test program to trigger fixed-point range analysis overflow checks */
/* Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test.c -o test.o */

#include <stdint.h>

/* Prevent constant folding */
volatile int vi = 10;
volatile unsigned int vu = 5;
volatile long vl = -100;
volatile double vd = 0.5;

/* Dummy function to prevent optimization */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char *cp = (volatile char *)p;
    for (int i = 0; i < size; i++) {
        cp[i];
    }
}

/* Fixed-point type definitions for clarity */
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
    volatile double seed4 = vd;
    
    /* Arrays to store results */
    fract_t f_results[10] = {0};
    accum_t a_results[10] = {0};
    laccum_t la_results[10] = {0};
    ufract_t uf_results[10] = {0};
    uaccum_t ua_results[10] = {0};
    
    int result_hash = 0;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 10; i++) {
        /* Vary the seeds each iteration */
        int idx = (seed1 + i) % 10;
        unsigned int uidx = (seed2 + i) % 10;
        
        /* ===== SIGNED ACCUM TYPES - Near maximum range ===== */
        /* These should trigger max_r/max_s comparisons */
        
        /* _Accum operations near max */
        accum_t a1 = 0.999999999999999999k;  /* Very close to max */
        accum_t a2 = 0.999999999999999999k;
        
        /* Multiplication that could overflow */
        accum_t a_prod = a1 * a2;
        a_results[idx] = a_prod;
        
        /* Left shift that could overflow */
        accum_t a_shifted = a1;
        for (int j = 0; j < 3; j++) {
            a_shifted = a_shifted * 2.0k;  /* Simulate left shift */
        }
        a_results[(idx + 1) % 10] = a_shifted;
        
        /* ===== LONG _ACCUM - Larger range boundary checks ===== */
        laccum_t la_max = 0.999999999999999999999999999999999k;
        laccum_t la_min = -0.999999999999999999999999999999999k;
        
        /* Operations that approach boundaries */
        laccum_t la1 = la_max * 0.999999999999999999999999999999k;
        laccum_t la2 = la_min * 0.999999999999999999999999999999k;
        
        /* Complex expression that might overflow */
        laccum_t la_result = (la1 * la2) + (la_max * 0.5k);
        la_results[idx] = la_result;
        
        /* ===== UNSIGNED FRACT TYPES - Near 1.0 ===== */
        ufract_t uf1 = 0.9999999ur;
        ufract_t uf2 = 0.0000001ur;
        
        /* Addition that could wrap */
        ufract_t uf_sum = uf1 + uf2;
        uf_results[uidx] = uf_sum;
        
        /* Multiplication near upper bound */
        ufract_t uf_prod = uf1 * uf1;
        uf_results[(uidx + 1) % 10] = uf_prod;
        
        /* ===== UNSIGNED ACCUM TYPES ===== */
        uaccum_t ua1 = 0.999999999999999999uk;
        uaccum_t ua2 = 0.999999999999999999uk;
        
        /* Product that could exceed max */
        uaccum_t ua_prod = ua1 * ua2;
        ua_results[uidx] = ua_prod;
        
        /* ===== SIGNED FRACT TYPES - Near -1.0 and 1.0 ===== */
        fract_t f_pos = 0.9999999r;
        fract_t f_neg = -0.9999999r;
        
        /* Complex expression mixing positive and negative */
        fract_t f_expr = (f_pos * f_neg) + (f_pos * 0.5r) - (f_neg * 0.25r);
        f_results[idx] = f_expr;
        
        /* ===== MIXED TYPE OPERATIONS WITH INTEGER PROMOTIONS ===== */
        /* These trigger range calculations for conversions */
        
        /* Integer to fixed-point with scaling */
        int int_val = seed1 + i * 1000;
        accum_t scaled = (accum_t)int_val * 0.001k;
        a_results[(idx + 2) % 10] = scaled;
        
        /* Fixed-point to integer conversion in expression */
        accum_t acc_val = 0.5k + (accum_t)i * 0.1k;
        int int_result = (int)(acc_val * 1000.0k);
        fract_t f_from_int = (fract_t)int_result * 0.001r;
        f_results[(idx + 3) % 10] = f_from_int;
        
        /* ===== CONDITIONAL EXPRESSIONS WITH FIXED-POINT ===== */
        /* Force VRP to analyze both branches */
        accum_t cond_result = (i % 2) ? a1 : (a2 * 0.5k);
        a_results[(idx + 4) % 10] = cond_result;
        
        /* Ternary with boundary values */
        ufract_t uf_cond = (seed2 > 5) ? uf1 : uf2;
        uf_results[(uidx + 2) % 10] = uf_cond;
        
        /* ===== LEFT SHIFT SIMULATION VIA MULTIPLICATION ===== */
        /* This directly relates to the uncovered code which handles shifts */
        accum_t shift_base = 0.75k;
        int shift_amount = (i % 4) + 1;
        accum_t shifted = shift_base;
        
        /* Simulate left shift by repeated multiplication */
        for (int s = 0; s < shift_amount; s++) {
            shifted = shifted * 2.0k;
        }
        
        /* Check if we exceeded maximum (saturation check) */
        accum_t max_accum = 0.999999999999999999k;
        if (shifted > max_accum) {
            shifted = max_accum;
        }
        a_results[(idx + 5) % 10] = shifted;
        
        /* ===== NEGATIVE VALUE SHIFT (could trigger min_r/min_s) ===== */
        accum_t neg_shift_base = -0.75k;
        accum_t neg_shifted = neg_shift_base;
        for (int s = 0; s < shift_amount; s++) {
            neg_shifted = neg_shifted * 2.0k;
        }
        
        /* Check minimum boundary */
        accum_t min_accum = -1.0k;
        if (neg_shifted < min_accum) {
            neg_shifted = min_accum;
        }
        a_results[(idx + 6) % 10] = neg_shifted;
        
        /* Update hash with results */
        result_hash ^= *(int*)&a_results[idx];
        result_hash ^= *(int*)&f_results[idx];
        result_hash ^= *(int*)&uf_results[uidx];
    }
    
    /* Additional boundary case: direct maximum value operations */
    {
        /* These should directly trigger the max_r/max_s comparison */
        accum_t absolute_max = 0.999999999999999999k;
        accum_t almost_max = 0.999999999999999998k;
        
        /* Operation that mathematically exceeds 1.0 */
        accum_t overflow_test = absolute_max * 1.000000000000000001k;
        a_results[0] = overflow_test;
        
        /* Another near-overflow */
        accum_t near_overflow = almost_max + almost_max;
        a_results[1] = near_overflow;
        
        /* Simulate left shift of maximum value */
        accum_t max_shifted = absolute_max * 2.0k;  /* Definitely overflows */
        a_results[2] = max_shifted;
    }
    
    /* Prevent dead code elimination */
    consume(f_results, sizeof(f_results));
    consume(a_results, sizeof(a_results));
    consume(la_results, sizeof(la_results));
    consume(uf_results, sizeof(uf_results));
    consume(ua_results, sizeof(ua_results));
    
    return result_hash & 0xFF;
}
