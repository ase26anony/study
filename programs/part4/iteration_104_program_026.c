/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef _Sat short _Fract sat_sfract_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat long _Accum sat_laccum_t;

/* Bit-field structure with fixed-point */
struct fixed_bitfield {
    unsigned int mantissa : 10;
    signed int exponent : 6;
    sat_sfract_t frac_part;
    unsigned int overflow_flag : 1;
};

/* Function using bit-fields and shifts */
static sat_fract_t process_with_bitfields(struct fixed_bitfield *bf, int shift) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int temp = bf->mantissa;
    
    /* Shift operations requiring range analysis */
    temp = temp << shift;
    temp = temp >> (shift / 2);
    
    /* Convert to fixed-point with potential overflow */
    sat_fract_t result = (sat_fract_t)temp / (sat_fract_t)512;
    
    /* Mix with existing fractional part */
    result = result + bf->frac_part;
    
    /* Check for overflow condition */
    if (result > (sat_fract_t)0.9r) {
        bf->overflow_flag = 1;
        return (sat_fract_t)1.0r;
    }
    
    return result;
}

/* Main function with complex fixed-point operations */
int main(void) {
    volatile fract_t v_bound = 0.5r;  /* Volatile to prevent constant folding */
    volatile sat_laccum_t volatile_acc = 0.0lk;
    
    /* Initialize fixed-point variables */
    sfract_t f1 = 0.25hr;
    sfract_t f2 = -0.75hr;
    accum_t a1 = 100.0k;
    accum_t a2 = -50.0k;
    sat_fract_t sat1 = 0.8r;
    sat_fract_t sat2 = 0.3r;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {
        .mantissa = 512,
        .exponent = 4,
        .frac_part = 0.5hr,
        .overflow_flag = 0
    };
    
    /* Loop-based range widening */
    accum_t accumulator = 0.0k;
    sat_fract_t sat_accumulator = 0.0r;
    
    printf("Starting fixed-point range test...\n");
    
    /* Complex loop with multiple operations */
    for (int i = 0; i < 100; i++) {
        /* Mixed integer/fixed-point operations */
        int int_val = i * 2;
        fract_t scaled = (fract_t)int_val / (fract_t)256;
        
        /* Arithmetic that may overflow */
        accumulator = accumulator + a1 * scaled;
        
        /* Saturating arithmetic */
        sat_accumulator = sat_accumulator + sat1;
        
        /* Conditional with explicit range comparisons */
        if (accumulator > (accum_t)1000.0k) {
            /* This comparison may trigger sgt/ugt logic */
            accumulator = accumulator - (accum_t)500.0k;
        }
        
        /* Ternary operator with range check */
        sat_accumulator = (sat_accumulator > (sat_fract_t)0.9r) ? 
                         (sat_fract_t)0.9r : sat_accumulator;
        
        /* Compare against volatile bound */
        if (accumulator > v_bound * (accum_t)2000.0k) {
            accumulator = (accum_t)0.0k;
        }
        
        /* Multi-step computation */
        fract_t temp = f1 * f2;
        temp = temp + scaled;
        
        /* Bit-field operations every 10 iterations */
        if (i % 10 == 0) {
            sat_fract_t bf_result = process_with_bitfields(&bf, i % 8 + 1);
            sat_accumulator = sat_accumulator + bf_result;
        }
        
        /* Complex conditional with multiple comparisons */
        if ((accumulator > (accum_t)500.0k && accumulator < (accum_t)1500.0k) ||
            (sat_accumulator < (sat_fract_t)0.1r)) {
            /* Alternate computation path */
            f1 = f1 * (sfract_t)0.9hr;
            f2 = f2 * (sfract_t)0.9hr;
        }
        
        /* Update volatile accumulator */
        volatile_acc = volatile_acc + (sat_laccum_t)sat_accumulator;
    }
    
    /* Additional range boundary tests */
    accum_t test_vals[] = {
        (accum_t)0.0k,
        (accum_t)0.5k,
        (accum_t)1.0k,
        (accum_t)-1.0k,
        (accum_t)100.0k,
        (accum_t)-100.0k
    };
    
    for (int i = 0; i < 6; i++) {
        /* Explicit comparisons that may trigger the uncovered logic */
        int is_positive = test_vals[i] > (accum_t)0.0k;
        int is_large = test_vals[i] > (accum_t)50.0k;
        int is_very_large = test_vals[i] > (accum_t)1000.0k;
        
        /* Use results to prevent dead code elimination */
        if (is_positive && !is_very_large) {
            accumulator = accumulator + test_vals[i];
        }
    }
    
    /* Final checksum computation */
    int checksum = 0;
    checksum += (int)(accumulator * (accum_t)100.0k);
    checksum += (int)(sat_accumulator * (fract_t)1000.0r);
    checksum += (int)(volatile_acc * (sat_laccum_t)100.0lk);
    checksum += bf.overflow_flag * 1000;
    
    printf("Checksum: %d\n", checksum);
    printf("Final accumulator: %ld\n", (long)(accumulator * (accum_t)1000.0k));
    printf("Final sat_accumulator: %ld\n", (long)(sat_accumulator * (fract_t)1000.0r));
    
    return checksum != 0 ? 0 : 1;
}
