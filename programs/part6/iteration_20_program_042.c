/* test_fixed_point_ranges.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -fdump-tree-vrp-details -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile long vl = 0;
static volatile unsigned int vu = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Fixed-point type declarations */
typedef _Fract fr_t;
typedef _Accum ac_t;
typedef long _Accum lac_t;
typedef unsigned _Fract ufr_t;
typedef unsigned _Accum uac_t;
typedef short _Fract sfr_t;
typedef long _Fract lfr_t;

int main(void) {
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = vi;
    volatile long lseed = vl;
    volatile unsigned int useed = vu;
    
    /* Arrays to store results */
    fr_t fr_results[8] = {0};
    ac_t ac_results[8] = {0};
    lac_t lac_results[8] = {0};
    ufr_t ufr_results[8] = {0};
    uac_t uac_results[8] = {0};
    
    int idx = 0;
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < 8; i++) {
        /* Use seeds to create data-dependent values */
        int int_val = seed + i * 1000;
        unsigned int uint_val = useed + i * 1000;
        long long_val = lseed + i * 10000L;
        
        /* ===== SIGNED ACCUM TYPES - near maximum range ===== */
        
        /* _Accum operations that should trigger max range checks */
        ac_t a1 = (ac_t)int_val / 256;  /* Convert to _Accum */
        ac_t a2 = 0.999999k;            /* Very close to max */
        
        /* Multiplication that could overflow */
        ac_t a_prod = a1 * a2;
        
        /* Left shift that could overflow */
        ac_t a_shifted = a_prod;
        for (int s = 0; s < 3; s++) {
            a_shifted = a_shifted * 2.0k;  /* Simulate left shift */
        }
        
        /* Check if we're at or near maximum */
        ac_t a_max_check = 0.999999999k;
        if (a_shifted > 0.9k) {
            /* Force range analysis for conditional */
            a_shifted = a_shifted * 1.1k;  /* Potential overflow */
        }
        
        ac_results[idx] = a_shifted;
        idx = (idx + 1) & 7;
        
        
        /* ===== LONG _ACCUM - testing wider ranges ===== */
        
        lac_t la1 = (lac_t)long_val / 65536;
        lac_t la2 = 0.999999999999999k;  /* Very close to long _Accum max */
        
        /* Complex expression that could overflow */
        lac_t la_temp = la1 * la2;
        lac_t la_result = la_temp;
        
        /* Multiple operations to create intermediate ranges */
        for (int j = 0; j < 2; j++) {
            la_result = la_result * 1.5k;
            la_result = la_result / 1.2k;
        }
        
        /* Explicit check against maximum (should trigger sgt/ugt comparisons) */
        lac_t la_max = 0.9999999999999999k;
        if (la_result > la_max * 0.99k) {
            /* This branch should trigger max range comparison */
            la_result = la_max;
        }
        
        lac_results[i] = la_result;
        
        
        /* ===== UNSIGNED _FRACT - near 1.0 ===== */
        
        ufr_t uf1 = (ufr_t)uint_val / 65536;
        ufr_t uf2 = 0.9999999ur;  /* Very close to 1.0 */
        
        /* Operations that could wrap around */
        ufr_t uf_sum = uf1 + uf2;
        if (uf_sum > 0.999ur) {
            uf_sum = uf_sum * 1.1ur;  /* Potential overflow */
        }
        
        ufr_results[i] = uf_sum;
        
        
        /* ===== SIGNED _FRACT - near -1.0 and 1.0 ===== */
        
        fr_t f1 = (fr_t)(int_val % 200 - 100) / 100.0r;  /* Range -1.0 to 1.0 */
        fr_t f2 = 0.999999r;  /* Very close to 1.0 */
        fr_t f3 = -0.999999r; /* Very close to -1.0 */
        
        /* Mix positive and negative near limits */
        fr_t f_prod = f1 * f2;
        fr_t f_prod2 = f1 * f3;
        
        /* Choose based on sign */
        fr_t f_result = (f1 > 0.0r) ? f_prod : f_prod2;
        
        /* Additional scaling that could overflow */
        for (int s = 0; s < 2; s++) {
            f_result = f_result * 1.5r;
        }
        
        fr_results[i] = f_result;
        
        
        /* ===== UNSIGNED _ACCUM - large values ===== */
        
        uac_t ua1 = (uac_t)uint_val / 256;
        uac_t ua2 = 999999.999uk;  /* Large value */
        
        /* Multiplication that could exceed range */
        uac_t ua_prod = ua1 * ua2;
        
        /* Simulate left shift through multiplication */
        uac_t ua_shifted = ua_prod;
        for (int s = 0; s < 4; s++) {
            ua_shifted = ua_shifted * 2.0uk;
        }
        
        uac_results[i] = ua_shifted;
        
        
        /* ===== MIXED TYPE OPERATIONS ===== */
        
        /* Promote fixed-point to integer and back */
        int temp_int = (int)(a_prod * 256);
        ac_t back_to_fixed = (ac_t)temp_int / 256;
        
        /* Use in conditional expression */
        ac_t mixed_result = (temp_int > 1000) ? 
                           (back_to_fixed * 1.5k) : 
                           (back_to_fixed * 0.5k);
        
        /* Additional overflow potential */
        if (mixed_result > 0.8k) {
            mixed_result = mixed_result * mixed_result;
        }
        
        ac_results[(i + 1) & 7] = mixed_result;
        
        
        /* ===== SHORT _FRACT edge cases ===== */
        
        sfr_t sf1 = 0.999r;  /* Maximum for short _Fract */
        sfr_t sf2 = (sfr_t)(i % 100) / 100.0r;
        
        /* Operation that could overflow short _Fract */
        sfr_t sf_result = sf1 * sf2;
        
        /* Force multiple operations */
        for (int j = 0; j < 3; j++) {
            sf_result = sf_result * 1.2r;
            sf_result = sf_result / 1.1r;
        }
        
        fr_results[(i + 2) & 7] = (fr_t)sf_result;
    }
    
    /* ===== FINAL OVERFLOW-PRONE EXPRESSIONS ===== */
    
    /* Create expressions that definitely need range checking */
    lac_t final_lac = 0.0k;
    for (int i = 0; i < 4; i++) {
        final_lac = final_lac + lac_results[i] * lac_results[7 - i];
    }
    
    /* This multiplication of near-maximum values should trigger
     * the specific sgt/ugt comparisons in fixed-value.cc */
    ac_t final_ac = ac_results[0];
    for (int i = 1; i < 8; i++) {
        final_ac = final_ac * ac_results[i];
        if (final_ac > 0.99k) {
            /* Force compiler to consider the overflow case */
            final_ac = final_ac * 0.999999k;
        }
    }
    
    /* Mix all results to create complex range */
    uac_t final_uac = (uac_t)0;
    for (int i = 0; i < 8; i++) {
        final_uac = final_uac + uac_results[i];
        if (final_uac > 1000000.0uk) {
            final_uac = final_uac / 2.0uk;
        }
    }
    
    /* Store final values to prevent elimination */
    fr_t final_results[4];
    final_results[0] = (fr_t)final_lac;
    final_results[1] = (fr_t)final_ac;
    final_results[2] = (fr_t)final_uac;
    final_results[3] = fr_results[0] * fr_results[7];
    
    /* Consume results to prevent dead code elimination */
    consume(fr_results, sizeof(fr_results));
    consume(ac_results, sizeof(ac_results));
    consume(lac_results, sizeof(lac_results));
    consume(ufr_results, sizeof(ufr_results));
    consume(uac_results, sizeof(uac_results));
    consume(final_results, sizeof(final_results));
    
    /* Return a hash of results */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= *(int*)(&fr_results[i]);
        hash ^= *(int*)(&ac_results[i]);
        hash ^= *(int*)(&ufr_results[i]);
    }
    
    return hash & 0xFF;
}
