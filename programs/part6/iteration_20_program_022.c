/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks in GCC's VRP
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Dummy function to prevent optimization */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *ptr = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = ptr[i];
    }
}

/* Fixed-point type aliases for clarity */
typedef _Fract fract_t;
typedef _Accum accum_t;
typedef long _Accum long_accum_t;
typedef unsigned _Fract ufract_t;
typedef short _Fract short_fract_t;
typedef long _Fract long_fract_t;

/* Array to store results and prevent dead code elimination */
static fract_t results[256];
static int result_idx = 0;

int main(void) {
    /* Volatile seeds to prevent constant folding */
    volatile int seed1 = 0x7FFFFFFF;
    volatile int seed2 = 0x80000000;
    volatile int seed3 = 0x3FFFFFFF;
    volatile int seed4 = 0xC0000000;
    
    /* Initialize with boundary values */
    accum_t max_accum = 0.999999999k;  /* Near max for _Accum */
    accum_t min_accum = -0.999999999k; /* Near min for _Accum */
    ufract_t max_ufract = 0.9999999ur; /* Near max for unsigned _Fract */
    fract_t max_fract = 0.9999999r;    /* Near max for signed _Fract */
    fract_t min_fract = -0.9999999r;   /* Near min for signed _Fract */
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 100; i++) {
        /* Read volatile seeds to create data-dependent values */
        int s1 = seed1;
        int s2 = seed2;
        int s3 = seed3;
        int s4 = seed4;
        
        /* Create fixed-point values from integer seeds */
        accum_t a1 = (accum_t)(s1 >> 16) * 0.0000152587890625k; /* ~1/65536 */
        accum_t a2 = (accum_t)(s2 >> 16) * 0.0000152587890625k;
        
        /* Complex expression that may overflow - targeting the uncovered comparison */
        /* This should trigger range analysis for multiplication */
        accum_t prod = a1 * a2;
        
        /* Left shift that could overflow - another path to range checking */
        accum_t shifted = prod;
        for (int j = 0; j < 3; j++) {
            shifted = shifted + shifted;  /* Effectively left shift by 1 */
        }
        
        /* Mix with boundary values */
        accum_t mixed1 = max_accum * a1;
        accum_t mixed2 = min_accum * a2;
        
        /* Operations on fractional types near boundaries */
        ufract_t u1 = max_ufract;
        ufract_t u2 = (ufract_t)(s3 & 0xFF) * 0.00392156862745098ur; /* ~1/255 */
        ufract_t u_sum = u1 + u2;  /* May overflow for unsigned */
        
        fract_t f1 = max_fract;
        fract_t f2 = min_fract;
        fract_t f_prod = f1 * f2;  /* Negative product, but range check still needed */
        
        /* Conditional expressions with fixed-point */
        fract_t cond_result = (s4 > 0) ? max_fract : min_fract;
        cond_result = cond_result * (fract_t)((s1 & 0xFF) * 0.00392156862745098r);
        
        /* Accumulate results in array */
        if (result_idx < 256) {
            /* Convert and store various results */
            results[result_idx++] = (fract_t)prod;
            results[result_idx++] = (fract_t)shifted;
            results[result_idx++] = (fract_t)mixed1;
            results[result_idx++] = (fract_t)mixed2;
            results[result_idx++] = (fract_t)u_sum;
            results[result_idx++] = f_prod;
            results[result_idx++] = cond_result;
        }
        
        /* Modify seeds for next iteration */
        seed1 = seed1 ^ (s1 << 13);
        seed2 = seed2 ^ (s2 >> 17);
        seed3 = seed3 + 1;
        seed4 = seed4 - 1;
    }
    
    /* Additional boundary case tests */
    {
        /* Test with long accum types */
        long_accum_t la1 = 0.999999999999999999k;
        long_accum_t la2 = 0.999999999999999999k;
        long_accum_t la_prod = la1 * la2;  /* Should trigger max range check */
        
        /* Test with short fract */
        short_fract_t sf1 = 0.9999hr;
        short_fract_t sf2 = 0.9999hr;
        short_fract_t sf_prod = sf1 * sf2;
        
        /* Test with long fract */
        long_fract_t lf1 = 0.999999999999999999lr;
        long_fract_t lf2 = 0.999999999999999999lr;
        long_fract_t lf_prod = lf1 * lf2;
        
        /* Store these results too */
        if (result_idx < 250) {
            results[result_idx++] = (fract_t)la_prod;
            results[result_idx++] = (fract_t)sf_prod;
            results[result_idx++] = (fract_t)lf_prod;
        }
    }
    
    /* Force multiple overflow scenarios in single expressions */
    {
        volatile int v = 0x40000000;
        int scale = v;
        
        /* This complex expression should trigger the specific comparison:
         * if (a_high.sgt(max_r) || (a_high == max_r && a_low.ugt(max_s)))
         */
        accum_t complex_expr = max_accum;
        for (int k = 0; k < 4; k++) {
            /* Chain operations that could overflow */
            complex_expr = complex_expr * (accum_t)(scale >> (k * 4)) * 0.000244140625k; /* 1/4096 */
            
            /* Alternate with additions */
            complex_expr = complex_expr + (accum_t)((scale << k) & 0x7FF) * 0.00048828125k; /* 1/2048 */
        }
        
        if (result_idx < 255) {
            results[result_idx++] = (fract_t)complex_expr;
        }
    }
    
    /* Create a hash from results to return */
    int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Access result as bytes to create hash */
        unsigned char *bytes = (unsigned char *)&results[i];
        for (size_t j = 0; j < sizeof(fract_t); j++) {
            hash = (hash * 31) + bytes[j];
        }
    }
    
    /* Consume results to prevent dead code elimination */
    consume(results, sizeof(results));
    
    return hash & 0xFF;
}
