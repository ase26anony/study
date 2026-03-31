/* Test program to exercise fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point types with different precisions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef short _Sat _Fract ssfract_t;
typedef _Sat _Fract sfract_sat_t;
typedef _Sat _Accum saccum_sat_t;

/* Bit-field structure to force range calculations */
struct bitfield_fixed {
    unsigned int mantissa : 16;
    unsigned int exponent : 8;
    sfract_t frac_part;
    saccum_t accum_part;
};

/* Function using bit-fields and shifts (Requirement 3) */
static struct bitfield_fixed process_bitfield(struct bitfield_fixed bf) {
    /* Shift operations that may trigger alshift logic */
    unsigned int shifted = bf.mantissa << bf.exponent;
    
    /* Convert to fixed-point with potential range issues */
    sfract_t f = (sfract_t)(shifted % 256) / 256.0hr;
    
    /* Explicit range comparison (Requirement 2) */
    if (f > 0.5hr) {
        bf.frac_part = f * 0.75hr;
    } else if (f < -0.5hr) {
        bf.frac_part = f * 1.25hr;
    } else {
        bf.frac_part = f;
    }
    
    /* Mixed integer/fixed-point context (Requirement 5) */
    bf.accum_part = (saccum_t)bf.mantissa + (saccum_t)bf.exponent / 256.0hk;
    
    return bf;
}

/* Main function with loop-based range widening (Requirement 4) */
int main(void) {
    /* Declare and initialize fixed-point variables (Requirement 1) */
    volatile sfract_t f1 = 0.25hr;      /* volatile prevents constant folding */
    volatile sfract_t f2 = -0.75hr;
    volatile accum_t a1 = 100.0k;
    volatile accum_t a2 = -50.0k;
    saccum_sat_t sat_acc = 0.0hk;       /* Saturating accumulator */
    
    /* Saturating fixed-point variables */
    ssfract_t sat_f1 = 0.8hr;
    ssfract_t sat_f2 = 0.3hr;
    
    /* Bit-field structure */
    struct bitfield_fixed bf = {256, 2, 0.5hr, 10.0hk};
    
    /* Loop-based range widening (Requirement 4) */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        /* Fixed-point arithmetic with potential overflow (Requirement 1) */
        f1 = f1 + 0.01hr;
        f2 = f2 - 0.005hr;
        
        /* Multiplication that can saturate */
        sat_f1 = sat_f1 * sat_f2;
        
        /* Accumulator with mixed operations */
        a1 = a1 * 1.01k;
        a2 = a2 + 0.5k;
        
        /* Saturating accumulator with explicit bounds check */
        sat_acc = sat_acc + 0.1hk;
        
        /* Explicit range comparisons (Requirement 2) */
        if (a1 > 1000.0k) {
            a1 = a1 * 0.9k;
        }
        
        if (a2 < -100.0k) {
            a2 = a2 * 0.8k;
        }
        
        /* Ternary operator with range comparison */
        f1 = (f1 > 0.9hr) ? f1 * 0.5hr : f1 * 1.1hr;
        f2 = (f2 < -0.9hr) ? f2 * 0.5hr : f2 * 1.1hr;
        
        /* Complex conditional with AND/OR */
        if ((f1 > 0.5hr && f2 < -0.5hr) || (a1 > 500.0k && a2 < -25.0k)) {
            /* Force range calculation with mixed types */
            accum_t temp = (accum_t)f1 * 100.0k + a2;
            if (temp > 0.0k) {
                a1 = a1 - temp * 0.01k;
            }
        }
        
        /* Periodically process bitfield */
        if (i % 10 == 0) {
            bf.mantissa = (bf.mantissa * 3) % 65535;
            bf.exponent = (bf.exponent + 1) % 8;
            bf = process_bitfield(bf);
            
            /* Compare with computed bounds */
            if (bf.accum_part > 20.0hk || bf.frac_part < -0.25hr) {
                bf.accum_part = bf.accum_part * 0.5hk;
                bf.frac_part = bf.frac_part * 0.5hr;
            }
        }
        
        /* Build checksum from all values */
        checksum += (int)(f1 * 1000) + (int)(f2 * 1000) + 
                   (int)(a1 / 10) + (int)(a2 / 10) +
                   (int)(sat_acc * 100) + (int)(bf.frac_part * 1000);
    }
    
    /* Additional mixed integer/fixed-point conversions (Requirement 5) */
    unsigned long ul = 1000000UL;
    laccum_t large_acc = (laccum_t)ul / 1000.0lk;
    
    /* More explicit comparisons with different precisions */
    if (large_acc > 500.0lk) {
        large_acc = large_acc - 200.0lk;
    }
    
    /* Final checksum computation */
    checksum += (int)(large_acc / 10);
    
    /* Print result to verify execution */
    printf("Checksum: %d\n", checksum);
    printf("Final values: f1=%.4hr, f2=%.4hr, a1=%.4k, a2=%.4k\n", 
           f1, f2, a1, a2);
    
    return 0;
}
