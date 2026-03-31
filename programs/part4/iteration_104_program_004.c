/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef _Sat _Fract sat_fract_t;

/* Bit-field structure for shift operations */
struct bitfield_fixed {
    unsigned int mantissa : 12;
    unsigned int fraction : 4;
    fract_t fixed_val;
};

/* Function using bit-fields and shifts */
static fract_t process_bitfield(struct bitfield_fixed *bf, int shift) {
    /* Operations that may trigger alshift/zext/sext */
    unsigned int temp = bf->mantissa << shift;
    bf->fraction = temp & 0xF;
    
    /* Convert to fixed-point with potential range checks */
    fract_t result = (fract_t)temp / (fract_t)256;
    
    /* Explicit comparison that may generate sgt/ugt */
    if (result > (fract_t)0.5r) {
        result = (fract_t)0.5r;
    }
    
    return result;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables across range */
    volatile sfract_t sf1 = 0.5hr;
    volatile sfract_t sf2 = -0.25hr;
    accum_t acc1 = 0.0lk;
    accum_t acc2 = 0.0lk;
    sat_fract_t sat1 = 0.7r;
    usfract_t usf1 = 0.8ur;
    
    /* Mixed integer/fixed-point variables */
    int int_val = 100;
    unsigned long ulong_val = 200;
    
    /* Bit-field structure */
    struct bitfield_fixed bf = {
        .mantissa = 2048,
        .fraction = 8,
        .fixed_val = 0.3r
    };
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Multi-step arithmetic forcing range calculations */
        acc1 += (accum_t)sf1 * (accum_t)0.01lk;
        acc2 = acc2 * (accum_t)0.99lk - (accum_t)sf2;
        
        /* Mixed precision operations */
        fract_t temp = (fract_t)int_val / (fract_t)256;
        sf1 = sf1 + (sfract_t)temp;
        
        /* Explicit range comparisons - may trigger uncovered logic */
        if (acc1 > (accum_t)10.0lk) {
            acc1 = (accum_t)10.0lk;
        } else if (acc1 < (accum_t)-10.0lk) {
            acc1 = (accum_t)-10.0lk;
        }
        
        /* Ternary operator with comparison */
        acc2 = (acc2 > (accum_t)5.0lk) ? (accum_t)5.0lk : 
               (acc2 < (accum_t)-5.0lk) ? (accum_t)-5.0lk : acc2;
        
        /* Saturated arithmetic */
        sat1 = sat1 + (sat_fract_t)0.1r;
        
        /* Compare against volatile to prevent constant folding */
        if (sf1 > sf2) {
            sf2 = sf2 + (sfract_t)0.01hr;
        }
        
        /* Mixed integer/fixed comparison */
        if ((int)acc1 > int_val / 2) {
            int_val++;
        }
        
        /* Call bitfield function periodically */
        if (i % 10 == 0) {
            bf.fixed_val = process_bitfield(&bf, i % 4);
        }
        
        /* Shift operations on integer affecting fixed-point */
        ulong_val = (ulong_val << 1) | 1;
        usf1 = (usfract_t)(ulong_val % 256) / (usfract_t)256;
    }
    
    /* Additional complex comparisons */
    accum_t max_bound = (accum_t)15.0lk;
    accum_t min_bound = (accum_t)-15.0lk;
    
    /* Nested comparisons that may generate the exact pattern */
    if (acc1 > max_bound || 
        (acc1 == max_bound && acc2 > (accum_t)0.0lk)) {
        acc1 = max_bound;
    }
    
    if (acc2 < min_bound ||
        (acc2 == min_bound && (usfract_t)acc2 < usf1)) {
        acc2 = min_bound;
    }
    
    /* Compute checksum to verify execution */
    int checksum = 0;
    checksum += (int)(acc1 * 100);
    checksum += (int)(acc2 * 100);
    checksum += (int)(sf1 * 1000);
    checksum += (int)(sf2 * 1000);
    checksum += (int)(sat1 * 1000);
    checksum += (int)(usf1 * 1000);
    checksum += bf.mantissa;
    checksum += bf.fraction;
    
    printf("Checksum: %d\n", checksum);
    printf("Results: acc1=%.4Lk, acc2=%.4Lk, sf1=%.4hr, sf2=%.4hr\n", 
           (long accum_t)acc1, (long accum_t)acc2, (short fract_t)sf1, (short fract_t)sf2);
    
    return 0;
}
