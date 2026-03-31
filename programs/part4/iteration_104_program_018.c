/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Accum laccum;
typedef unsigned short _Fract usfract;
typedef _Sat short _Fract sat_sfract;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int mantissa : 10;
    signed int exponent : 6;
    sfract frac_part;
    usfract ufrac_part;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bits(sfract f, usfract uf, int shift) {
    struct fixed_bitfield result;
    volatile sfract vf = f;  /* Prevent constant folding */
    
    /* Operations that may trigger range calculations */
    result.mantissa = (unsigned int)(f * 1023.0hr);
    result.exponent = shift;
    
    /* Shift operations requiring range analysis */
    result.frac_part = f;
    for (int i = 0; i < shift; i++) {
        /* This may trigger alshift operations in range analysis */
        result.frac_part = result.frac_part * 0.5hr;
    }
    
    result.ufrac_part = uf;
    return result;
}

/* Main function with fixed-point arithmetic and range comparisons */
int main(void) {
    /* Initialize fixed-point variables across different ranges */
    sfract f1 = 0.5hr;
    sfract f2 = -0.25hr;
    usfract uf1 = 0.75uhr;
    laccum acc1 = 0.0lk;
    laccum acc2 = 0.0lk;
    sat_sfract sat_f = 0.9shr;
    
    volatile int bound = 100;  /* Non-constant bound */
    int checksum = 0;
    
    /* Loop with range-widening operations */
    for (int i = 0; i < bound; i++) {
        /* Mixed precision operations */
        acc1 = acc1 + (laccum)f1;
        acc2 = acc2 * 0.99lk + (laccum)f2;
        
        /* Fixed-point multiplication with potential overflow */
        f1 = f1 * 0.999hr;
        f2 = f2 * 1.01hr;  /* May go toward -1.0 */
        
        /* Explicit range comparisons - may trigger sgt/ugt operations */
        if (acc1 > 50.0lk) {
            /* This comparison likely uses the uncovered logic */
            acc1 = 25.0lk;
        }
        
        if ((uf1 > 0.5uhr) && (i % 3 == 0)) {
            uf1 = uf1 * 0.8uhr;
        }
        
        /* Ternary operator with mixed types */
        sat_f = (i % 5 == 0) ? sat_f + 0.1shr : sat_f - 0.05shr;
        
        /* Compare against computed bounds */
        laccum max_bound = (laccum)i * 0.1lk;
        if (acc2 > max_bound) {
            acc2 = max_bound * 0.5lk;
        }
        
        /* Another comparison that may use the uncovered block */
        if (f1 < -0.8hr || f1 > 0.8hr) {
            f1 = 0.0hr;
        }
    }
    
    /* Process with bit-field operations */
    struct fixed_bitfield bf = process_fixed_bits(f1, uf1, 3);
    
    /* Compute checksum */
    checksum += (int)(acc1 * 100.0lk);
    checksum += (int)(acc2 * 100.0lk);
    checksum += (int)(f1 * 1000.0hr);
    checksum += (int)(f2 * 1000.0hr);
    checksum += (int)(uf1 * 1000.0uhr);
    checksum += (int)(sat_f * 1000.0shr);
    checksum += bf.mantissa;
    checksum += bf.exponent;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional explicit comparisons in different contexts */
    {
        /* Create scenarios for zext/sext operations */
        unsigned short us = (unsigned short)(uf1 * 65535.0uhr);
        short s = (short)(f1 * 32767.0hr);
        
        /* Comparisons that may trigger the uncovered logic */
        if ((us > 32768u) ^ (s < 0)) {
            checksum += 1000;
        }
        
        /* Mixed integer/fixed-point comparison */
        int int_val = (int)(acc1);
        if (int_val > 100 || int_val < -100) {
            checksum += 2000;
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
