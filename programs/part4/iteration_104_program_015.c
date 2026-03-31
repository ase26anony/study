/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point types with various precisions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef _Sat short _Fract sat_sfract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure to trigger shift operations */
struct bitfield_fixed {
    unsigned int mantissa : 16;
    unsigned int exponent : 8;
    unsigned int sign : 1;
    sat_sfract_t fixed_val;
};

/* Function using bit-fields and shifts */
static sat_accum_t process_bitfield(struct bitfield_fixed *bf, int shift) {
    /* Operations that may trigger alshift/zext/sext */
    unsigned int temp = bf->mantissa;
    
    /* Shift operations requiring range analysis */
    temp = temp << shift;
    temp = temp >> (shift / 2);
    
    /* Convert to fixed-point with potential overflow */
    sat_accum_t result = (sat_accum_t)temp / 256.0hk;
    
    /* Mix with existing fixed-point value */
    result = result + bf->fixed_val * 2.0hr;
    
    return result;
}

/* Main function with loops and range comparisons */
int main(void) {
    /* Initialize fixed-point variables across range */
    volatile fract_t f1 = 0.5r;
    volatile fract_t f2 = -0.25r;
    volatile accum_t a1 = 100.0k;
    volatile accum_t a2 = -50.0k;
    sat_accum_t sat_acc = 0.0hk;
    laccum_t large_acc = 0.0lk;
    
    /* Variables for bounds comparison */
    volatile int bound1 = 100;
    volatile int bound2 = -100;
    
    /* Bit-field structure */
    struct bitfield_fixed bf = {
        .mantissa = 32768,
        .exponent = 4,
        .sign = 0,
        .fixed_val = 0.75hr
    };
    
    int checksum = 0;
    
    /* Loop 1: Range widening with accumulation */
    for (int i = 0; i < 100; i++) {
        /* Mixed precision operations */
        accum_t temp = a1 * (accum_t)f1 + a2 * (accum_t)f2;
        
        /* Conditional with explicit range comparison */
        if (temp > (accum_t)bound1) {
            /* May trigger sgt/ugt comparisons */
            a1 = a1 * 0.9k;
            sat_acc = sat_acc + 0.1hk;
        } else if (temp < (accum_t)bound2) {
            a2 = a2 * 0.9k;
            sat_acc = sat_acc - 0.1hk;
        } else {
            /* Middle range operations */
            f1 = f1 * 0.99r;
            f2 = f2 * 1.01r;
        }
        
        /* Saturating accumulator with overflow potential */
        sat_acc = sat_acc + temp * 0.01hk;
        
        /* Large accumulator with different scaling */
        large_acc = large_acc + temp * 0.001lk;
        
        /* Update bounds based on iteration */
        if (i % 10 == 0) {
            bound1 = bound1 - 1;
            bound2 = bound2 + 1;
        }
    }
    
    /* Loop 2: More complex range calculations */
    for (int i = 0; i < 50; i++) {
        /* Use bit-field function */
        sat_accum_t bf_result = process_bitfield(&bf, i % 8 + 1);
        
        /* Ternary operator with range comparison */
        accum_t threshold = (i > 25) ? 50.0k : -50.0k;
        
        /* Comparison that may invoke the uncovered logic */
        int use_result = (bf_result > threshold) ? 1 : 0;
        
        if (use_result) {
            sat_acc = sat_acc + bf_result;
            /* Shift bit-field mantissa */
            bf.mantissa = bf.mantissa << 1;
        } else {
            sat_acc = sat_acc - bf_result * 0.5hk;
            bf.mantissa = bf.mantissa >> 1;
        }
        
        /* Update fixed_val with potential overflow */
        bf.fixed_val = bf.fixed_val * 1.1hr;
    }
    
    /* Loop 3: Integer/fixed-point mixed comparisons */
    for (unsigned int j = 0; j < 1000; j++) {
        /* Convert integer to fixed-point */
        fract_t int_as_fixed = (fract_t)j / 1000.0r;
        
        /* Complex comparison expression */
        if ((j > 500 && int_as_fixed > 0.3r) || 
            (j <= 500 && int_as_fixed < -0.3r)) {
            a1 = a1 + int_as_fixed * 10.0k;
        }
        
        /* Modulo operation with fixed-point result */
        if (j % 100 == 0) {
            accum_t mod_result = (accum_t)(j % 150) / 10.0k;
            a2 = a2 - mod_result;
        }
    }
    
    /* Final range-checking conditional */
    accum_t final_check = a1 + a2 * 0.5k;
    
    /* Multi-part comparison similar to uncovered code pattern */
    if (final_check > 1000.0k) {
        sat_acc = 1.0hk;  /* Saturate to max */
    } else if (final_check < -1000.0k) {
        sat_acc = -1.0hk; /* Saturate to min */
    } else {
        /* Linear mapping */
        sat_acc = final_check / 1000.0hk;
    }
    
    /* Compute checksum from all fixed-point variables */
    checksum += (int)(f1 * 1000.0r);
    checksum += (int)(f2 * 1000.0r);
    checksum += (int)(a1 * 10.0k);
    checksum += (int)(a2 * 10.0k);
    checksum += (int)(sat_acc * 1000.0hk);
    checksum += (int)(large_acc * 100.0lk);
    checksum += bf.mantissa;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional explicit comparisons to trigger edge cases */
    {
        /* Explicit comparisons with constants near bounds */
        _Bool test1 = (sat_acc > 0.999hk);
        _Bool test2 = (sat_acc < -0.999hk);
        _Bool test3 = (large_acc > 500.0lk);
        _Bool test4 = (large_acc < -500.0lk);
        
        /* Use results to prevent dead code elimination */
        if (test1 || test2 || test3 || test4) {
            printf("Boundary conditions triggered\n");
        }
    }
    
    return 0;
}
