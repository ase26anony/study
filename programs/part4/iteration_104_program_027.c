/* Fixed-point test program targeting GCC's fixed-value.cc uncovered lines */
#include <stdio.h>

/* Fixed-point types with different precisions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;
typedef unsigned short _Fract usfract_t;

/* Saturating versions */
typedef _Sat short _Fract sat_sfract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure for shift operations */
struct bitfield_fixed {
    unsigned int mantissa : 16;
    unsigned int exponent : 8;
    unsigned int sign : 1;
    sat_sfract_t fixed_val;
};

/* Function using bit-fields and shifts */
static sat_accum_t process_bitfield(struct bitfield_fixed *bf, int shift) {
    /* Operations that may trigger alshift/zext/sext */
    unsigned int shifted = bf->mantissa << shift;
    bf->exponent = shifted & 0xFF;
    
    /* Convert to fixed-point with potential range checks */
    sat_accum_t result = (sat_accum_t)bf->fixed_val * (sat_accum_t)shifted;
    
    /* Explicit comparison that may generate sgt/ugt */
    if (result > (sat_accum_t)0.5k) {
        result = result * (sat_accum_t)0.75k;
    } else if (result < (sat_accum_t)-0.5k) {
        result = result + (sat_accum_t)0.25k;
    }
    
    return result;
}

/* Main function with loop-based range widening */
int main(void) {
    /* Initialize with values spanning different ranges */
    volatile fract_t f1 = 0.25r;
    volatile fract_t f2 = -0.75r;
    volatile accum_t a1 = 100.5k;
    volatile accum_t a2 = -200.25k;
    volatile sat_accum_t sa1 = 0.0k;
    volatile sat_sfract_t sf1 = 0.5hr;
    
    /* Accumulators for loop operations */
    accum_t acc1 = 0.0k;
    accum_t acc2 = 0.0k;
    sat_accum_t sat_acc = 0.0k;
    
    /* Mixed integer/fixed-point variables */
    int int_val = 100;
    unsigned long ulong_val = 1000;
    
    /* Bit-field structure */
    struct bitfield_fixed bf = {
        .mantissa = 0xABCD,
        .exponent = 0x12,
        .sign = 0,
        .fixed_val = 0.3hr
    };
    
    /* Loop forcing range analysis */
    for (int i = 0; i < 100; i++) {
        /* Multi-step computations with potential overflow */
        acc1 = acc1 + f1 * (accum_t)i;
        acc2 = acc2 - f2 * (accum_t)(i * 2);
        
        /* Saturating accumulator with explicit bounds checking */
        sat_acc = sat_acc + (sat_accum_t)(a1 * 0.01k);
        
        /* Conditional blocks with explicit range comparisons */
        if (acc1 > (accum_t)50.0k) {
            /* Operations when above threshold */
            acc1 = acc1 * (accum_t)0.9k;
            if (acc2 < (accum_t)-100.0k) {
                acc2 = acc2 + (accum_t)10.0k;
            }
        } else if (acc1 < (accum_t)-50.0k) {
            /* Operations when below threshold */
            acc1 = acc1 / (accum_t)2.0k;
        }
        
        /* Ternary operator with mixed types */
        fract_t temp = (i % 3 == 0) ? f1 : f2;
        acc1 = acc1 + temp * (accum_t)0.1k;
        
        /* Mixed integer/fixed-point operation */
        if (i % 10 == 0) {
            accum_t mixed = (accum_t)int_val * a1;
            if (mixed > (accum_t)5000.0k) {
                int_val = int_val / 2;
            }
        }
        
        /* Update volatile bounds to prevent constant folding */
        if (i % 20 == 0) {
            f1 = f1 * 0.95r;
            a1 = a1 - 1.0k;
        }
        
        /* Call bitfield function periodically */
        if (i % 7 == 0) {
            sat_accum_t bf_result = process_bitfield(&bf, i % 5);
            sat_acc = sat_acc + bf_result;
            
            /* Check against computed bounds */
            if (sat_acc > (sat_accum_t)0.8k || sat_acc < (sat_accum_t)-0.8k) {
                sat_acc = sat_acc * (sat_accum_t)0.5k;
            }
        }
        
        /* Complex conditional with AND/OR */
        if ((acc1 > (accum_t)25.0k && acc2 < (accum_t)-25.0k) ||
            (sat_acc > (sat_accum_t)0.6k && i % 3 == 1)) {
            sf1 = sf1 * 0.8hr;
        }
    }
    
    /* Additional fixed-point operations outside loop */
    laccum_t large_acc = (laccum_t)acc1 * (laccum_t)acc2;
    
    /* Explicit overflow checking */
    sat_accum_t test_sat = (sat_accum_t)1.5k;
    for (int j = 0; j < 10; j++) {
        test_sat = test_sat * (sat_accum_t)1.1k;
        /* This should trigger saturation logic */
    }
    
    /* Convert unsigned to fixed-point with range considerations */
    usfract_t uf1 = (usfract_t)0.5r;
    for (int k = 0; k < 5; k++) {
        uf1 = uf1 * (usfract_t)0.8r;
    }
    
    /* Compute checksum */
    int checksum = 0;
    checksum += (int)(acc1 * 100.0k);
    checksum += (int)(acc2 * 100.0k);
    checksum += (int)(sat_acc * 100.0k);
    checksum += (int)(large_acc / 10.0lk);
    checksum += (int)(test_sat * 100.0k);
    checksum += (int)(uf1 * 100.0r);
    checksum += (int)(sf1 * 100.0hr);
    
    printf("Checksum: %d\n", checksum);
    
    /* Final explicit comparisons that might trigger the uncovered code */
    volatile accum_t final_test = acc1 + acc2;
    if (final_test > (accum_t)0.0k) {
        printf("Positive result: %f\n", (double)final_test);
    } else if (final_test < (accum_t)0.0k) {
        printf("Negative result: %f\n", (double)final_test);
    }
    
    return 0;
}
