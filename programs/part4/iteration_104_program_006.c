/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int flag : 1;
    sfract_t value : 15;  /* Will trigger range calculations */
    unsigned int count : 8;
    fract_t scale : 8;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf, int shift) {
    /* Operations that may trigger alshift/zext/sext logic */
    struct fixed_bitfield result = bf;
    
    /* Shift operations on bit-field members */
    result.count <<= shift;
    result.count >>= (shift / 2);
    
    /* Fixed-point arithmetic with bit-field member */
    result.value = (_Fract)((int)result.value * (1 << shift));
    result.scale = (_Fract)((int)result.scale / (1 << (shift % 4)));
    
    return result;
}

/* Function with explicit range comparisons */
static sat_accum_t check_range_bounds(accum_t value, accum_t min_val, accum_t max_val) {
    sat_accum_t result;
    
    /* Explicit comparisons that may generate sgt/ugt operations */
    if (value > max_val) {
        result = (_Sat _Accum)max_val;
    } else if (value < min_val) {
        result = (_Sat _Accum)min_val;
    } else {
        /* Ternary operator with mixed-type comparison */
        result = (_Sat _Accum)((value > (accum_t)0.5k) ? value * (accum_t)0.8k : value * (accum_t)1.2k);
    }
    
    /* Additional comparison against computed bounds */
    accum_t midpoint = (min_val + max_val) / (accum_t)2.0k;
    if (value > midpoint && value < max_val) {
        result = (_Sat _Accum)(result * (accum_t)0.9k);
    }
    
    return result;
}

int main(void) {
    /* Initialize fixed-point variables across different ranges */
    volatile sfract_t sf1 = 0.5hr;
    volatile sfract_t sf2 = -0.25hr;
    volatile fract_t f1 = 0.333r;
    volatile fract_t f2 = -0.666r;
    volatile accum_t a1 = 100.0k;
    volatile accum_t a2 = -50.0k;
    volatile ufract_t uf1 = 0.75ur;
    volatile sat_fract_t sat_f = 0.8r;
    volatile sat_accum_t sat_a = 500.0k;
    
    /* Accumulators for loop computations */
    accum_t accum_pos = 0.0k;
    accum_t accum_neg = 0.0k;
    fract_t fract_accum = 0.0r;
    
    /* Non-constant bounds to prevent optimization */
    volatile int iterations = 100;
    volatile accum_t dynamic_max = 1000.0k;
    volatile accum_t dynamic_min = -1000.0k;
    
    /* Loop-based range widening */
    for (int i = 0; i < iterations; i++) {
        /* Mixed precision operations */
        accum_pos += (accum_t)sf1 * (accum_t)i;
        accum_neg += (accum_t)sf2 * (accum_t)(i / 2);
        
        /* Multiplication that can cause overflow */
        fract_accum = fract_accum * f1 * (_Fract)(i % 10);
        
        /* Conditional operations based on range comparisons */
        if (accum_pos > dynamic_max) {
            accum_pos = dynamic_max * (accum_t)0.5k;
        } else if (accum_pos < (accum_t)0.0k) {
            accum_pos = accum_pos * (accum_t)(-1.0k);
        }
        
        /* Nested comparisons */
        if (accum_neg < dynamic_min || accum_neg > (accum_t)0.0k) {
            accum_neg = (_Accum)((int)accum_neg % 100);
        }
        
        /* Mixed integer/fixed-point context */
        int int_val = (int)accum_pos + (int)accum_neg;
        fract_accum = (_Fract)(fract_accum + (_Fract)(int_val % 100) / (_Fract)100.0r);
        
        /* Call to range checking function */
        sat_a = check_range_bounds(accum_pos, dynamic_min, dynamic_max);
        
        /* Bit-field operations every 10 iterations */
        if (i % 10 == 0) {
            struct fixed_bitfield bf = {0, 0.5hr, 1, 0.25r};
            bf = process_fixed_bitfield(bf, i % 4);
            fract_accum = (_Fract)(fract_accum + bf.value + bf.scale);
        }
        
        /* Shift operations on integer types derived from fixed-point */
        unsigned int temp = (unsigned int)(accum_pos * (accum_t)10.0k);
        temp = temp << (i % 8);
        temp = temp >> ((i % 8) / 2);
        accum_pos = accum_pos + (_Accum)temp / (_Accum)1000.0k;
    }
    
    /* Additional explicit range comparisons outside loop */
    accum_t test_vals[] = {accum_pos, accum_neg, (accum_t)fract_accum * 100.0k};
    sat_accum_t sat_results[3];
    
    for (int i = 0; i < 3; i++) {
        /* Complex comparison expressions */
        if (test_vals[i] > (dynamic_max * (accum_t)0.75k) && 
            test_vals[i] < dynamic_max) {
            sat_results[i] = (_Sat _Accum)(test_vals[i] * (accum_t)0.8k);
        } else if (test_vals[i] < (dynamic_min * (accum_t)0.75k) ||
                   test_vals[i] > (accum_t)0.0k) {
            sat_results[i] = (_Sat _Accum)(test_vals[i] / (accum_t)2.0k);
        } else {
            sat_results[i] = (_Sat _Accum)test_vals[i];
        }
    }
    
    /* Compute checksum to verify execution */
    long checksum = 0;
    checksum += (long)(accum_pos * 1000.0k);
    checksum += (long)(accum_neg * 1000.0k);
    checksum += (long)(fract_accum * 1000.0r);
    checksum += (long)(sat_a * 1000.0k);
    for (int i = 0; i < 3; i++) {
        checksum += (long)(sat_results[i] * 1000.0k);
    }
    
    printf("Checksum: %ld\n", checksum);
    printf("Results: pos=%Lf, neg=%Lf, fract=%f\n", 
           (long double)accum_pos, 
           (long double)accum_neg, 
           (double)fract_accum);
    
    return 0;
}
