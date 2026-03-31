/* fixed-point-coverage.c
 * Designed to trigger GCC's fixed-point range analysis logic
 * Compile with: gcc -O2 -ffixed-point -fstrict-overflow -o fixed_test fixed-point-coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef _Sat short _Fract sat_sfract_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    usfract_t fraction;  /* unsigned fixed-point */
    volatile fract_t volatile_fract; /* volatile to prevent constant folding */
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf, int shift) {
    /* Operations that may trigger alshift/zext/sext logic */
    struct fixed_bitfield result = bf;
    
    /* Shift operations on bit-field components */
    result.integer <<= shift;
    
    /* Convert fixed-point to integer for shifting */
    unsigned int temp = (unsigned int)(bf.fraction * 256.0r);
    temp >>= shift;
    result.fraction = (usfract_t)(temp / 256.0r);
    
    /* Range comparison that may trigger sgt/ugt */
    if (result.volatile_fract > 0.5r) {
        result.sign = 1;
    } else if (result.volatile_fract < -0.5r) {
        result.sign = 0;
    }
    
    return result;
}

/* Main function with loop-based range widening */
int main(void) {
    /* Initialize fixed-point variables across different ranges */
    sfract_t sf1 = 0.5hr;
    sfract_t sf2 = -0.25hr;
    accum_t acc1 = 1000.0lk;
    accum_t acc2 = -500.0lk;
    sat_sfract_t sat_f = 0.75hr;
    
    /* Volatile variables to prevent constant folding */
    volatile fract_t v_bound1 = 0.8r;
    volatile fract_t v_bound2 = -0.3r;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {
        .sign = 0,
        .integer = 64,
        .fraction = 0.125ur,
        .volatile_fract = 0.6r
    };
    
    /* Loop accumulator for range widening */
    accum_t loop_accum = 0.0lk;
    fract_t fract_accum = 0.0r;
    
    int checksum = 0;
    
    /* Loop that forces range analysis across iterations */
    for (int i = 0; i < 100; i++) {
        /* Mixed precision operations */
        fract_t temp = (fract_t)sf1 * (fract_t)sf2;
        
        /* Range comparisons that may trigger the uncovered logic */
        if (temp > v_bound1) {
            /* Upper bound check */
            sf1 = sf1 * 0.9hr;
        } else if (temp < v_bound2) {
            /* Lower bound check */
            sf1 = sf1 * 1.1hr;
        }
        
        /* Accumulator with saturation */
        sat_f = sat_f + 0.1hr;
        
        /* Mixed integer/fixed-point context */
        loop_accum = loop_accum + (accum_t)(i * 10) / 1000.0lk;
        
        /* Complex conditional with multiple comparisons */
        fract_accum = fract_accum + temp;
        if ((fract_accum > 0.7r && fract_accum < 0.9r) ||
            (fract_accum < -0.7r && fract_accum > -0.9r)) {
            /* Middle range operations */
            acc1 = acc1 * 0.99lk;
            acc2 = acc2 * 1.01lk;
        }
        
        /* Ternary operator with range-dependent result */
        fract_t range_test = (fract_accum > 0.5r) ? 
                             (fract_accum * 0.8r) : 
                             (fract_accum * 1.2r);
        
        /* Explicit overflow check */
        if (loop_accum > 10000.0lk || loop_accum < -10000.0lk) {
            loop_accum = loop_accum / 2.0lk;
        }
        
        /* Process bit-field every 10 iterations */
        if (i % 10 == 0) {
            bf = process_fixed_bitfield(bf, (i % 4) + 1);
        }
        
        /* Update checksum with integer conversions */
        checksum += (int)(sf1 * 1000.0hr);
        checksum += (int)(fract_accum * 1000.0r);
    }
    
    /* Final range comparisons */
    accum_t max_bound = 5000.0lk;
    accum_t min_bound = -5000.0lk;
    
    if (loop_accum > max_bound) {
        printf("Exceeded max bound: %ld\n", (long)(loop_accum * 1000.0lk));
    } else if (loop_accum < min_bound) {
        printf("Below min bound: %ld\n", (long)(loop_accum * 1000.0lk));
    }
    
    /* Mixed-type final computation */
    int final_result = (int)(loop_accum * 100.0lk) + 
                      (int)(fract_accum * 1000.0r) + 
                      (int)(sf1 * 1000.0hr) +
                      checksum;
    
    printf("Final checksum: %d\n", final_result);
    printf("Loop accum: %ld\n", (long)(loop_accum * 1000.0lk));
    printf("Fract accum: %d\n", (int)(fract_accum * 1000.0r));
    
    return 0;
}
