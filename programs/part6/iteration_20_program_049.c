/* fixed-point-range-test.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -fwrapv -c fixed-point-range-test.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, used))
static void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Fixed-point type declarations */
typedef _Fract sfract_t;
typedef unsigned _Fract ufract_t;
typedef _Accum saccum_t;
typedef unsigned _Accum uaccum_t;
typedef long _Accum lsaccum_t;
typedef short _Fract ssfract_t;
typedef long _Fract lfract_t;

int main(void) {
    /* Initialize with volatile to prevent compile-time evaluation */
    int seed1 = vi;
    unsigned int seed2 = vu;
    long seed3 = vl;
    
    /* Array to accumulate results */
    saccum_t results[16];
    for (int i = 0; i < 16; i++) {
        results[i] = 0.0k;
    }
    
    /* Test various fixed-point types and operations */
    for (int iter = 0; iter < 100; iter++) {
        /* Use seeds that change per iteration (compiler doesn't know values) */
        int idx = (seed1 + iter) & 0xF;
        
        /* 1. Test signed _Accum near maximum range */
        saccum_t a_max = 0.999999999k;  /* Very close to max */
        saccum_t a_min = -0.999999999k; /* Very close to min */
        
        /* Multiplication that could overflow */
        saccum_t prod1 = a_max * a_max;  /* Should be > max representable */
        saccum_t prod2 = a_min * a_min;  /* Should be positive but near max */
        saccum_t prod3 = a_max * a_min;  /* Should be near min */
        
        /* Left shift operations (converted to multiplication) */
        saccum_t shifted1 = prod1 * 2.0k;  /* Definitely overflows */
        saccum_t shifted2 = prod2 * 1.5k;
        
        /* 2. Test unsigned _Accum */
        uaccum_t u_max = 0.999999999uk;
        uaccum_t u_near_max = 0.999999998uk;
        
        /* Operations that approach and exceed 1.0 */
        uaccum_t u_sum = u_max + u_near_max;  /* Would overflow if not saturated */
        uaccum_t u_prod = u_max * u_max;
        
        /* 3. Test _Fract types */
        sfract_t sf_max = 0.9999999r;
        sfract_t sf_min = -0.9999999r;
        ufract_t uf_max = 0.9999999ur;
        
        /* Complex expression mixing types */
        sfract_t sf_expr = sf_max * sf_max - sf_min * sf_min;
        ufract_t uf_expr = uf_max * 0.9999998ur + 0.0000001ur;
        
        /* 4. Test long _Accum with extreme values */
        lsaccum_t ls_max = 0.99999999999999999lk;
        lsaccum_t ls_min = -0.99999999999999999lk;
        
        /* Multi-step computation that creates intermediate ranges */
        lsaccum_t temp1 = ls_max * 0.5lk;
        lsaccum_t temp2 = temp1 * 2.5lk;  /* Exceeds original ls_max */
        lsaccum_t temp3 = ls_min * 0.75lk;
        lsaccum_t temp4 = temp3 * 3.0lk;  /* May underflow */
        
        /* 5. Mix with integer promotions and casts */
        int int_val = seed1 + iter;
        unsigned int uint_val = seed2 + iter;
        
        /* Casts that require range analysis */
        saccum_t from_int = (saccum_t)int_val * 0.001k;
        uaccum_t from_uint = (uaccum_t)uint_val * 0.0001uk;
        
        /* Conditional expressions with fixed-point */
        sfract_t cond_sf = (int_val > 0) ? sf_max : sf_min;
        ufract_t cond_uf = (uint_val & 1) ? uf_max : 0.5ur;
        
        /* 6. Create data-dependent overflow conditions */
        long shift_amount = (seed3 + iter) % 4;
        saccum_t shifted_var = a_max;
        
        /* Simulate left shift via multiplication by powers of 2 */
        for (int s = 0; s < shift_amount; s++) {
            shifted_var = shifted_var * 2.0k;
        }
        
        /* 7. Accumulate results in array with wrapping index */
        results[idx] = results[idx] + prod1 * 0.1k + shifted1 * 0.01k;
        idx = (idx + 1) & 0xF;
        results[idx] = results[idx] + (saccum_t)u_prod * 0.001k;
        idx = (idx + 1) & 0xF;
        results[idx] = results[idx] + (saccum_t)sf_expr;
        idx = (idx + 1) & 0xF;
        results[idx] = results[idx] + temp2 * 0.1lk;
        idx = (idx + 1) & 0xF;
        results[idx] = results[idx] + from_int;
        idx = (idx + 1) & 0xF;
        results[idx] = results[idx] + shifted_var * 0.5k;
        
        /* 8. Explicit overflow checks (simulating builtin overflow) */
        saccum_t test_val = a_max;
        saccum_t add_val = 0.000000001k;
        saccum_t sum = test_val + add_val;
        
        /* Check if we saturated (fixed-point addition saturates on overflow) */
        int overflow_possible = (test_val > 0.9k && add_val > 0.1k);
        if (overflow_possible && sum <= test_val) {
            /* This branch should be taken if saturation occurred */
            results[0] = results[0] * 0.9k;
        }
        
        /* 9. Test short and long fract types */
        ssfract_t ssf_val = 0.9999hr;
        lfract_t lf_val = 0.9999999999lr;
        
        ssfract_t ssf_prod = ssf_val * ssf_val;
        lfract_t lf_prod = lf_val * lf_val;
        
        results[5] = results[5] + (saccum_t)ssf_prod;
        results[6] = results[6] + (saccum_t)lf_prod;
        
        /* Update seeds for next iteration (compiler can't predict) */
        seed1 = seed1 + 1;
        seed2 = seed2 * 1103515245 + 12345;
        seed3 = seed3 - 3;
    }
    
    /* 10. Final complex expression combining everything */
    saccum_t final_result = 0.0k;
    for (int i = 0; i < 16; i++) {
        /* Scale each result to potentially overflow */
        saccum_t scaled = results[i] * 1.1k;
        
        /* Conditional based on range */
        if (scaled > 0.8k) {
            scaled = scaled * 0.5k;
        } else if (scaled < -0.8k) {
            scaled = scaled * 1.5k;  /* Could make it more negative */
        }
        
        final_result = final_result + scaled;
        
        /* Another potential overflow in accumulation */
        if (final_result > 0.95k) {
            final_result = final_result * 0.9k;
        } else if (final_result < -0.95k) {
            final_result = final_result * 0.9k;
        }
    }
    
    /* Store to volatile to ensure computation happens */
    results[0] = final_result;
    
    /* Prevent dead code elimination */
    consume(results, sizeof(results));
    
    /* Return hash of results */
    int hash = 0;
    for (int i = 0; i < 16; i++) {
        /* Convert to integer for hashing */
        long long bits = *(long long*)&results[i];
        hash = hash * 31 + (bits & 0xFFFFFFFF);
        hash = hash * 31 + (bits >> 32);
    }
    
    return hash & 0xFF;
}
