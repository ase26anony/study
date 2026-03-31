/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef _Sat short _Fract sat_sfract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf: 8;
    int sbf: 8;
    sfract_t fbf: 8;
};

/* Function using bit-fields and shifts */
static accum_t bitfield_shift_ops(struct mixed_bf *bf, int shift_count) {
    volatile accum_t result = 0.0k;
    
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int temp = bf->ubf << shift_count;
    result = (accum_t)temp / 256.0k;
    
    /* Conditional based on shifted value */
    if (temp > 128U) {
        result += 0.5k;
    } else {
        result -= 0.25k;
    }
    
    /* More shift operations */
    int shifted = bf->sbf >> 1;
    result += (accum_t)shifted / 128.0k;
    
    return result;
}

/* Main function with loop-based range widening */
int main(void) {
    /* Initialize fixed-point variables across different ranges */
    volatile sfract_t sf1 = 0.5hr;
    volatile sfract_t sf2 = -0.25hr;
    volatile accum_t acc1 = 100.0k;
    volatile accum_t acc2 = -50.0k;
    volatile sat_sfract_t sat_sf = 0.75hr;
    volatile usfract_t usf = 0.8uhr;
    
    /* Bit-field structure */
    struct mixed_bf bf = {64, -32, 0.125hr};
    
    /* Accumulators for loop */
    accum_t loop_acc = 0.0k;
    sfract_t sf_acc = 0.0hr;
    
    /* Volatile bounds to prevent constant folding */
    volatile int volatile_bound = 100;
    volatile accum_t volatile_acc_bound = 50.0k;
    
    /* Loop with range-widening operations */
    for (int i = 0; i < volatile_bound; i++) {
        /* Mixed precision operations */
        accum_t temp_acc = acc1 + (accum_t)sf1 * 2.0k;
        
        /* Conditional with explicit range comparison */
        if (temp_acc > volatile_acc_bound) {
            /* This may trigger sgt/ugt comparisons in range analysis */
            acc1 = acc1 * 0.9k;
            sf_acc += 0.01hr;
        } else if (temp_acc < -volatile_acc_bound) {
            acc1 = acc1 * 1.1k;
            sf_acc -= 0.01hr;
        } else {
            /* Middle range operations */
            acc1 = temp_acc;
            sf_acc = sf_acc * 0.99hr;
        }
        
        /* Saturated arithmetic that requires overflow checks */
        sat_sf = sat_sf + 0.1hr;
        
        /* Mixed integer/fixed-point context */
        int int_val = (int)(acc2 * 10);
        if (int_val > 0 && int_val < 100) {
            acc2 = acc2 + (accum_t)int_val / 100.0k;
        }
        
        /* Accumulate with potential overflow */
        loop_acc += acc1 * 0.01k;
        
        /* Ternary operator with range comparison */
        sf2 = (sf_acc > 0.5hr) ? 0.9hr : 
              (sf_acc < -0.5hr) ? -0.9hr : sf_acc;
        
        /* Bit-field operations every 10 iterations */
        if (i % 10 == 0) {
            accum_t bf_result = bitfield_shift_ops(&bf, i % 5);
            loop_acc += bf_result;
            bf.ubf = (bf.ubf * 3) % 256;
        }
        
        /* Explicit overflow check */
        if (loop_acc > 1000.0k || loop_acc < -1000.0k) {
            loop_acc = loop_acc * 0.5k;
        }
    }
    
    /* Additional fixed-point operations outside loop */
    accum_t final_acc = acc1 + acc2 + (accum_t)sf1 + (accum_t)sf2;
    
    /* Mixed-type conditional */
    fract_t mixed_result;
    if ((final_acc > 0.0k) && (usf > 0.5uhr)) {
        mixed_result = 0.75r;
    } else if ((final_acc < 0.0k) || (sf_acc < 0.0hr)) {
        mixed_result = -0.25r;
    } else {
        mixed_result = 0.0r;
    }
    
    /* Compute checksum */
    long checksum = 0;
    checksum += (long)(sf1 * 1000);
    checksum += (long)(sf2 * 1000);
    checksum += (long)(acc1 * 100);
    checksum += (long)(acc2 * 100);
    checksum += (long)(loop_acc * 100);
    checksum += (long)(mixed_result * 1000);
    checksum += (long)(usf * 1000);
    checksum += (long)(sat_sf * 1000);
    
    printf("Checksum: %ld\n", checksum);
    
    /* Additional edge cases */
    {
        /* Test with explicit shift operations on fixed-point */
        unsigned int shift_test = 0xFF;
        for (int shift = 1; shift < 8; shift++) {
            unsigned int shifted = shift_test << shift;
            accum_t shifted_fp = (accum_t)shifted / 256.0k;
            
            /* Comparison that may trigger uncovered logic */
            if (shifted_fp > 0.5k && shifted < 0x800) {
                checksum += shift;
            }
        }
        
        /* Test negative shifts */
        int neg_shift = -4;
        int shifted_neg = 128 >> (-neg_shift);
        if ((accum_t)shifted_neg / 128.0k < 1.0k) {
            checksum += 1000;
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
