/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 100;
static volatile int vi4 = -100;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *ptr = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = ptr[i];
    }
}

/* Fixed-point type definitions for clarity */
typedef _Fract fract_t;
typedef _Accum accum_t;
typedef long _Accum long_accum_t;
typedef unsigned _Fract ufract_t;
typedef short _Fract short_fract_t;
typedef long _Fract long_fract_t;
typedef unsigned _Accum uaccum_t;

int main(void) {
    /* Array to accumulate results */
    accum_t results[32] = {0};
    int result_idx = 0;
    
    /* Volatile seeds to prevent compile-time evaluation */
    volatile fract_t f_seed = 0.5r;
    volatile accum_t a_seed = 0.5k;
    volatile ufract_t uf_seed = 0.5ur;
    volatile long_accum_t la_seed = 0.5lk;
    
    /* Test 1: Signed accumulative types near maximum range */
    for (int i = 0; i < 4; i++) {
        /* Values approaching max representable */
        accum_t a1 = 0.999999999k;  /* Very close to max */
        accum_t a2 = 0.999999999k;
        
        /* Multiplication that would overflow if not range-checked */
        accum_t product = a1 * a2;
        results[result_idx++] = product;
        
        /* Left shift that could overflow */
        accum_t shifted = a1;
        for (int j = 0; j < 3; j++) {
            shifted = shifted * 2.0k;  /* Simulate left shift */
        }
        results[result_idx++] = shifted;
    }
    
    /* Test 2: Unsigned fractional types near 1.0 */
    {
        ufract_t u1 = 0.9999999ur;  /* Very close to 1.0 */
        ufract_t u2 = 0.0000001ur;
        
        /* Addition that could wrap around */
        ufract_t sum = u1 + u2;
        /* Convert to signed for storage */
        results[result_idx++] = (accum_t)sum;
        
        /* Multiplication near overflow */
        ufract_t u3 = 0.9999999ur;
        ufract_t u4 = 0.9999999ur;
        ufract_t u_product = u3 * u4;
        results[result_idx++] = (accum_t)u_product;
    }
    
    /* Test 3: Signed fractional types at boundaries */
    {
        fract_t f_min = -1.0r;
        fract_t f_max = 0.9999999r;
        
        /* Complex expression mixing min and max */
        fract_t f_expr = (f_min * f_max) + (f_max * 0.5r);
        results[result_idx++] = (accum_t)f_expr;
        
        /* Conditional expression with boundary values */
        fract_t f_cond = (vi1 > 0) ? f_max : f_min;
        results[result_idx++] = (accum_t)f_cond;
    }
    
    /* Test 4: Mix with integer promotions and casts */
    {
        int int_val = vi3;
        /* Cast integer to fixed-point and multiply */
        accum_t from_int = (accum_t)int_val * 0.123456789k;
        results[result_idx++] = from_int;
        
        /* Negative integer case */
        int neg_int = vi4;
        accum_t from_neg_int = (accum_t)neg_int * 0.987654321k;
        results[result_idx++] = from_neg_int;
    }
    
    /* Test 5: Loop with varying fixed-point values */
    {
        /* Array of boundary values */
        accum_t boundaries[] = {
            0.999999999k, -0.999999999k,
            0.5k, -0.5k,
            0.000000001k, -0.000000001k
        };
        
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                /* Multiplications that might overflow */
                accum_t temp = boundaries[i] * boundaries[j];
                
                /* Additional shift-like operation */
                accum_t shifted = temp * 1.999999k;
                
                /* Store if not obviously saturated */
                if (shifted < 0.999k && shifted > -0.999k) {
                    results[result_idx++] = shifted;
                }
            }
        }
    }
    
    /* Test 6: Long accum operations */
    {
        long_accum_t la1 = 0.99999999999999999lk;
        long_accum_t la2 = 0.99999999999999999lk;
        
        /* This product is extremely close to overflow */
        long_accum_t la_product = la1 * la2;
        results[result_idx++] = (accum_t)la_product;
        
        /* Chain of multiplications */
        long_accum_t chain = la1;
        for (int i = 0; i < 4; i++) {
            chain = chain * 0.999999999lk;
        }
        results[result_idx++] = (accum_t)chain;
    }
    
    /* Test 7: Explicit overflow checking simulation */
    {
        accum_t test_val = 0.999999999k;
        accum_t increment = 0.000000001k;
        
        /* Manually check for overflow before operation */
        accum_t potential_sum = test_val + increment;
        
        /* Simulate saturation logic */
        accum_t saturated = (potential_sum > 0.999999999k) ? 
                            0.999999999k : potential_sum;
        results[result_idx++] = saturated;
        
        /* Underflow check */
        accum_t neg_val = -0.999999999k;
        accum_t neg_potential = neg_val - increment;
        accum_t neg_saturated = (neg_potential < -0.999999999k) ?
                                -0.999999999k : neg_potential;
        results[result_idx++] = neg_saturated;
    }
    
    /* Test 8: Mixed-type expressions */
    {
        short_fract_t sf = 0.9999hr;
        fract_t f = 0.9999999r;
        accum_t a = 0.999999999k;
        
        /* Mixed precision multiplication */
        accum_t mixed = (accum_t)sf * (accum_t)f * a;
        results[result_idx++] = mixed;
        
        /* Division near boundaries */
        accum_t numerator = 0.999999999k;
        accum_t denominator = 0.500000001k;
        accum_t quotient = numerator / denominator;
        results[result_idx++] = quotient;
    }
    
    /* Ensure we don't exceed array bounds */
    if (result_idx > 32) result_idx = 32;
    
    /* Consume results to prevent optimization */
    consume(results, sizeof(results[0]) * result_idx);
    
    /* Create a simple hash of results for return value */
    int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Convert fixed-point to integer for hashing */
        int val = (int)(results[i] * 1000000);
        hash ^= (val << (i % 16)) | (val >> (16 - (i % 16)));
    }
    
    return hash & 0xFF;
}
