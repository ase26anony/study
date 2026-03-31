/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point types with different precisions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef _Sat short _Fract ssfract_t;
typedef _Sat _Accum saccum_t;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf1 : 5;
    signed int sbf1 : 7;
    unsigned int ubf2 : 10;
};

/* Function using bit-fields and shifts with fixed-point */
void bitfield_fixed_ops(struct mixed_bf *bf, sfract_t *result) {
    volatile sfract_t v1 = 0.5hr;
    volatile sfract_t v2 = -0.25hr;
    
    /* Shift operations that may trigger range calculations */
    unsigned int shift1 = bf->ubf1;
    unsigned int shift2 = bf->ubf2 >> 2;
    
    /* Mixed operations with shifts */
    sfract_t temp = v1;
    for (int i = 0; i < (shift1 & 0x3); i++) {
        /* Operations that force range analysis */
        temp = temp * 0.75hr;
        if (temp > 0.8hr) {
            temp = 0.8hr;
        }
    }
    
    /* Explicit range comparison */
    if (bf->sbf1 > 0) {
        *result = temp + v2;
    } else {
        *result = temp - v2;
    }
    
    /* Ternary operator with range check */
    *result = (shift2 > 5) ? (*result * 0.9hr) : (*result * 1.1hr);
}

int main() {
    /* Initialize fixed-point variables across range */
    volatile sfract_t sf1 = 0.75hr;
    volatile sfract_t sf2 = -0.5hr;
    volatile accum_t acc1 = 100.0k;
    volatile accum_t acc2 = -50.0k;
    volatile laccum_t lacc = 1000.0lk;
    
    /* Saturating fixed-point */
    ssfract_t ssf1 = 0.8hr;
    ssfract_t ssf2 = -0.7hr;
    
    /* Results accumulators */
    accum_t sum_acc = 0.0k;
    sfract_t sum_frac = 0.0hr;
    laccum_t prod_lacc = 1.0lk;
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Mixed precision operations */
        accum_t temp_acc = acc1 + (accum_t)sf1 * 2.0k;
        
        /* Explicit range comparisons - may trigger sgt/ugt */
        if (temp_acc > 150.0k) {
            temp_acc = temp_acc * 0.9k;
        } else if (temp_acc < -100.0k) {
            temp_acc = temp_acc * 1.1k;
        }
        
        /* Ternary with comparison */
        sfract_t temp_frac = (i % 3 == 0) ? 
            (sf1 * 0.95hr) : 
            (sf2 * 1.05hr);
        
        /* Check against computed bounds */
        accum_t upper_bound = acc2 * 2.0k;
        accum_t lower_bound = acc2 * 0.5k;
        
        if (temp_acc > upper_bound) {
            temp_acc = upper_bound;
        } else if (temp_acc < lower_bound) {
            temp_acc = lower_bound;
        }
        
        /* Accumulate results */
        sum_acc += temp_acc;
        sum_frac += temp_frac;
        
        /* Multiplication that can overflow */
        prod_lacc *= (laccum_t)((i % 10) + 1) * 0.1lk;
        
        /* Update variables for next iteration */
        sf1 = sf1 * 0.99hr;
        sf2 = sf2 * 1.01hr;
        acc1 = acc1 - 0.5k;
        acc2 = acc2 + 0.25k;
    }
    
    /* Bit-field operations */
    struct mixed_bf bf = {3, -10, 25};
    sfract_t bf_result;
    bitfield_fixed_ops(&bf, &bf_result);
    
    /* Mixed integer/fixed-point context */
    unsigned long ul_val = 1000;
    fract_t mixed_result = (fract_t)ul_val * 0.001r;
    
    /* Final range check with explicit comparisons */
    accum_t final_check = sum_acc;
    if (final_check > 5000.0k || final_check < -5000.0k) {
        final_check = 0.0k;
    }
    
    /* Compute checksum */
    int checksum = 0;
    checksum += (int)(sum_acc * 0.01k);      /* Scale down for integer conversion */
    checksum += (int)(sum_frac * 100.0hr);   /* Scale up for integer conversion */
    checksum += (int)(prod_lacc * 0.001lk);
    checksum += (int)(bf_result * 100.0hr);
    checksum += (int)(mixed_result * 1000.0r);
    checksum += (int)(final_check * 0.01k);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional explicit comparisons to trigger uncovered logic */
    volatile lfract_t lf1 = 0.999999lr;
    volatile lfract_t lf2 = -0.999999lr;
    
    for (int j = 0; j < 10; j++) {
        /* Complex conditional with multiple comparisons */
        if ((lf1 > 0.9lr && lf2 < -0.8lr) || 
            (lf1 < 0.1lr && lf2 > -0.2lr)) {
            lf1 = lf1 * 0.5lr;
            lf2 = lf2 * 0.5lr;
        }
        
        /* Shift-like behavior using multiplication */
        lf1 = lf1 * 2.0lr;
        lf2 = lf2 * 2.0lr;
        
        /* Bound checking */
        if (lf1 > 1.0lr) lf1 = 1.0lr;
        if (lf1 < -1.0lr) lf1 = -1.0lr;
        if (lf2 > 1.0lr) lf2 = 1.0lr;
        if (lf2 < -1.0lr) lf2 = -1.0lr;
    }
    
    return 0;
}
