/* Test program to exercise GCC's fixed-point range analysis logic */
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
    unsigned int sign : 1;
    unsigned int integer : 7;
    ufract_t fraction;  /* unsigned fixed-point */
    fract_t signed_frac; /* signed fixed-point */
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf, int shift) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int temp = (bf.integer << shift) | (bf.integer >> (8 - shift));
    
    /* Convert to fixed-point with potential range checks */
    ufract_t uf = (ufract_t)temp / 256.0r;
    fract_t sf = (fract_t)((int)temp - 128) / 128.0r;
    
    /* Update structure */
    bf.fraction = uf;
    bf.signed_frac = sf;
    
    return bf;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables spanning different ranges */
    volatile fract_t f1 = 0.5r;      /* Middle of range */
    volatile fract_t f2 = -0.75r;    /* Negative value */
    volatile accum_t a1 = 100.0k;    /* Large accum */
    volatile accum_t a2 = -50.0k;    /* Negative accum */
    volatile ufract_t uf1 = 0.25ur;  /* Unsigned */
    volatile sat_fract_t sf1 = 0.8r; /* Saturated */
    
    /* Accumulators for loop computations */
    accum_t acc1 = 0.0k;
    accum_t acc2 = 0.0k;
    fract_t frac_acc = 0.0r;
    
    /* Volatile bounds to prevent constant folding */
    volatile int bound1 = 100;
    volatile int bound2 = -100;
    
    /* Mixed integer/fixed-point computations */
    unsigned long int_mask = 0xFF;
    int shift_count = 3;
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Multi-step computation forcing range analysis */
        acc1 += a1 * (accum_t)i / 1000.0k;
        acc2 += a2 * (accum_t)(i % 10) / 50.0k;
        
        /* Fixed-point multiplication with potential overflow */
        frac_acc = frac_acc * f1 + f2;
        
        /* Explicit range comparisons - may trigger sgt/ugt logic */
        if (acc1 > (accum_t)bound1 * 0.8k) {
            /* Operation when above threshold */
            acc1 = acc1 * 0.9k;
            bound1 = bound1 + 1;  /* Change bound dynamically */
        }
        
        if (acc2 < (accum_t)bound2 * 1.2k) {
            /* Operation when below threshold */
            acc2 = acc2 * 1.1k;
            bound2 = bound2 - 1;
        }
        
        /* Ternary operator with mixed types */
        fract_t temp_frac = (i % 3 == 0) ? 
            (fract_t)((int)i * 0.01r) : 
            (fract_t)(-(int)i * 0.005r);
        
        /* Comparison against computed value */
        accum_t threshold = (accum_t)(i * 2) / 10.0k;
        if (acc1 > threshold && acc2 < -threshold) {
            /* Cross-comparison triggering range logic */
            frac_acc = frac_acc * 0.5r;
        }
        
        /* Bit manipulation with fixed-point */
        unsigned int bits = (unsigned int)i & int_mask;
        ufract_t uf_temp = (ufract_t)bits / 255.0ur;
        
        /* Shift operations that may invoke alshift logic */
        bits = bits << shift_count;
        bits = bits >> (shift_count - 1);
        
        /* Mixed precision conditional */
        if ((uf_temp > 0.5ur) && (frac_acc < 0.0r)) {
            acc1 = acc1 - (accum_t)uf_temp * 0.1k;
        }
        
        /* Saturated arithmetic */
        sat_fract_t sat_temp = sf1 + (sat_fract_t)0.3r;
        sf1 = sat_temp * (sat_fract_t)0.9r;
        
        /* Prevent excessive growth */
        if (acc1 > 1000.0k) acc1 = 1000.0k;
        if (acc2 < -1000.0k) acc2 = -1000.0k;
        if (frac_acc > 0.9r) frac_acc = 0.9r;
        if (frac_acc < -0.9r) frac_acc = -0.9r;
    }
    
    /* Bit-field structure operations */
    struct fixed_bitfield bf = {0, 64, 0.5ur, -0.25r};
    for (int i = 0; i < 10; i++) {
        bf = process_fixed_bitfield(bf, i % 4 + 1);
        
        /* Comparison with bit-field derived values */
        accum_t bf_value = (accum_t)bf.integer + (accum_t)bf.signed_frac;
        if (bf_value > acc1 || bf_value < acc2) {
            /* Update accumulators based on bit-field */
            acc1 = (acc1 + bf_value) * 0.5k;
            acc2 = (acc2 - bf_value) * 0.5k;
        }
    }
    
    /* Final checksum computation */
    int checksum = 0;
    checksum += (int)(acc1 * 100.0k);      /* Convert to integer */
    checksum += (int)(acc2 * 100.0k);
    checksum += (int)(frac_acc * 1000.0r);
    checksum += (int)(sf1 * 1000.0r);
    checksum += bf.integer;
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: acc1=%.3Lfk, acc2=%.3Lfk, frac_acc=%.3r\n", 
           (long double)acc1, (long double)acc2, (double)frac_acc);
    
    return 0;
}
