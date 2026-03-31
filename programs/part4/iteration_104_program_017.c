/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum laccum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;

/* Bit-field structure with fixed-point */
struct fixed_bitfield {
    unsigned int mantissa : 10;
    signed int exponent : 6;
    sfract_t frac_part;
};

/* Function using bit-fields and shifts */
static sat_fract_t process_bitfield(struct fixed_bitfield *bf, int shift) {
    volatile sfract_t v1 = 0.25r;
    volatile sfract_t v2 = -0.75r;
    
    /* Shift operations that may trigger alshift logic */
    unsigned int shifted = bf->mantissa << shift;
    
    /* Mixed integer/fixed-point operations */
    fract_t result = (fract_t)(shifted) * 0.5r + bf->frac_part;
    
    /* Explicit range comparison */
    if (result > 0.8r) {
        return 0.9r;
    } else if (result < -0.8r) {
        return -0.9r;
    }
    
    /* Ternary with range check */
    return (result > 0) ? result : -result;
}

int main(void) {
    /* Declare and initialize fixed-point variables */
    volatile sfract_t sf1 = 0.125r;
    volatile sfract_t sf2 = -0.625r;
    volatile fract_t f1 = 0.5r;
    volatile fract_t f2 = -0.25r;
    volatile laccum_t acc1 = 0.0lk;
    volatile laccum_t acc2 = 0.0lk;
    volatile ufract_t uf1 = 0.75ur;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {512, 10, 0.125r};
    
    /* Loop-based range widening */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        /* Multi-step arithmetic operations */
        acc1 = acc1 + (laccum_t)sf1 * 0.25lk;
        acc2 = acc2 - (laccum_t)sf2 * 0.125lk;
        
        /* Mixed precision operations */
        f1 = f1 * (fract_t)(i % 10) * 0.1r;
        f2 = f2 + (fract_t)(i % 5) * 0.05r;
        
        /* Explicit range comparisons triggering sgt/ugt */
        volatile fract_t bound1 = (fract_t)(i) * 0.01r;
        volatile fract_t bound2 = (fract_t)(100 - i) * 0.01r;
        
        if (f1 > bound1) {
            /* Different operations based on comparison */
            f1 = f1 * 0.9r;
        } else if (f1 < -bound1) {
            f1 = f1 * 1.1r;
        }
        
        /* Ternary operator with range check */
        f2 = (f2 > bound2) ? bound2 : ((f2 < -bound2) ? -bound2 : f2);
        
        /* Saturated arithmetic */
        sat_fract_t sat1 = (sat_fract_t)f1 + (sat_fract_t)f2;
        sat_fract_t sat2 = (sat_fract_t)f1 * (sat_fract_t)f2;
        
        /* Check for overflow conditions */
        if (sat1 == 1.0r || sat1 == -1.0r) {
            checksum += 1;
        }
        if (sat2 == 1.0r || sat2 == -1.0r) {
            checksum += 2;
        }
        
        /* Update bit-field and call processing function */
        bf.mantissa = (bf.mantissa * 3 + i) & 0x3FF;
        bf.frac_part = (sfract_t)(i % 100) * 0.01r;
        
        sat_fract_t processed = process_bitfield(&bf, i % 8);
        checksum += (int)(processed * 1000r);
    }
    
    /* Final range comparisons with volatile bounds */
    volatile fract_t final_bound = 0.5r;
    volatile fract_t neg_bound = -0.5r;
    
    if (f1 > final_bound) {
        checksum += 1000;
    } else if (f1 < neg_bound) {
        checksum += 2000;
    }
    
    /* Mixed integer/fixed-point in condition */
    int int_val = (int)(f2 * 1000r);
    unsigned int uint_val = (unsigned int)(uf1 * 1000ur);
    
    if (int_val > 500 || uint_val < 250) {
        checksum += 3000;
    }
    
    /* Cast accumulators with range checks */
    long long ll_acc1 = (long long)(acc1 * 1000lk);
    long long ll_acc2 = (long long)(acc2 * 1000lk);
    
    if (ll_acc1 > 1000000LL || ll_acc2 < -1000000LL) {
        checksum += 4000;
    }
    
    /* Print checksum to verify execution */
    printf("Checksum: %d\n", checksum);
    
    /* Additional explicit comparisons to trigger uncovered logic */
    {
        volatile laccum_t test1 = 0.999lk;
        volatile laccum_t test2 = -0.999lk;
        volatile laccum_t test3 = 0.0lk;
        
        /* Complex comparison chain */
        if (test1 > 0.9lk && test2 < -0.9lk) {
            test3 = test1 + test2;
        }
        
        /* Shift-like operations through multiplication */
        sfract_t shifted = sf1 * (sfract_t)(1 << 3) * 0.125r;
        
        /* Final overflow check */
        sat_fract_t final_sat = (sat_fract_t)test3 + (sat_fract_t)shifted;
        if (final_sat == 1.0r || final_sat == -1.0r) {
            printf("Saturation detected\n");
        }
    }
    
    return 0;
}
