/* Test program to exercise GCC's fixed-point range analysis logic */
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
typedef _Sat _Accum saccum_sat_t;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf1 : 5;
    signed int sbf1 : 7;
    unsigned int ubf2 : 10;
};

/* Function using bit-fields and shifts with fixed-point */
static accum_t process_with_bitfields(struct mixed_bf *bf, accum_t base) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int shift_val = bf->ubf1;
    signed int signed_shift = bf->sbf1;
    
    /* Convert bit-field to fixed-point with shifting */
    accum_t result = base;
    
    /* Left shift operation that may trigger range calculations */
    if (shift_val > 0) {
        /* This may invoke alshift logic in range analysis */
        result = result + (accum_t)(bf->ubf2 << shift_val) / 256;
    }
    
    /* Right shift with sign extension consideration */
    if (signed_shift < 0) {
        signed_shift = -signed_shift;
        /* This may trigger sext logic */
        result = result / (1 << (signed_shift & 0x7));
    }
    
    return result;
}

int main(void) {
    /* Initialize with values spanning different ranges */
    volatile sfract_t sf1 = 0.25hr;
    volatile sfract_t sf2 = -0.75hr;
    volatile accum_t acc1 = 100.0k;
    volatile accum_t acc2 = -50.0k;
    volatile laccum_t lacc1 = 1000.0lk;
    
    /* Saturating types for overflow scenarios */
    saccum_sat_t sacc_sat = 0.5k;
    ssfract_t ssf_sat = 0.125hr;
    
    /* Bit-field structure */
    struct mixed_bf bf = {3, -2, 512};
    
    /* Accumulators for loop operations */
    accum_t accum_main = 0.0k;
    laccum_t accum_wide = 0.0lk;
    
    /* Non-constant bounds to prevent folding */
    volatile int bound1 = 100;
    volatile unsigned int bound2 = 200;
    
    printf("Starting fixed-point range test...\n");
    
    /* Loop that forces range widening analysis */
    for (int i = 0; i < 50; i++) {
        /* Mixed precision operations */
        accum_t temp = acc1 + (accum_t)sf1 * 2.0k;
        
        /* Conditional with explicit range comparison */
        /* This may trigger sgt/ugt comparisons in range logic */
        if (temp > (accum_t)bound1 / 2.0k) {
            /* Operation that could overflow */
            acc1 = acc1 - 15.0k;
            sf1 = sf1 * 0.9hr;
        } else if (temp < (accum_t)(-bound1) / 3.0k) {
            /* Alternative path */
            acc1 = acc1 + 10.0k;
            sf1 = sf1 * 1.1hr;
        }
        
        /* Multi-step computation with intermediate range checks */
        accum_t step1 = acc2 * 0.8k;
        accum_t step2 = step1 + (accum_t)sf2 * 4.0k;
        
        /* Ternary operator that may invoke comparison logic */
        accum_t step3 = (step2 > 0.0k) ? step2 * 0.5k : step2 * 1.5k;
        
        /* Accumulate with potential overflow */
        accum_main = accum_main + step3;
        
        /* Explicit cast to integer for mixed context */
        int int_val = (int)(accum_main * 0.01k);
        
        /* Comparison in mixed integer/fixed-point context */
        if (int_val > bound2 / 3) {
            /* Shift-like operation through multiplication */
            lacc1 = lacc1 * 2.0lk;
        } else if (int_val < (int)(-bound2 / 4)) {
            lacc1 = lacc1 * 0.5lk;
        }
        
        /* Saturating arithmetic test */
        sacc_sat = sacc_sat + 0.3k;
        ssf_sat = ssf_sat * 0.95hr;
        
        /* Update accum_wide with potential large range */
        accum_wide = accum_wide + (laccum_t)accum_main / 10.0lk;
        
        /* Call function with bit-fields periodically */
        if (i % 7 == 0) {
            accum_t bf_result = process_with_bitfields(&bf, accum_main);
            accum_main = accum_main + bf_result * 0.1k;
        }
        
        /* Modulo operation to create cyclic pattern */
        sf2 = (sfract_t)((i % 10) - 5) / 10.0hr;
    }
    
    /* Final computations with explicit comparisons */
    accum_t final_check = accum_main;
    
    /* Multiple comparison chain that may trigger the uncovered logic */
    if (final_check > 500.0k || 
        (final_check > 250.0k && final_check < 300.0k) ||
        final_check < -400.0k) {
        lacc1 = lacc1 * 1.1lk;
    }
    
    /* Compute checksum to verify execution */
    long checksum = 0;
    checksum += (long)(accum_main * 100.0k);
    checksum += (long)(lacc1 * 10.0lk);
    checksum += (long)(sacc_sat * 1000.0k);
    checksum += (int)(sf1 * 1000);
    checksum += (int)(sf2 * 1000);
    
    printf("Checksum: %ld\n", checksum);
    printf("Final values: accum_main=%Lfk, lacc1=%Lfk\n", 
           (long double)accum_main, (long double)lacc1);
    
    return 0;
}
