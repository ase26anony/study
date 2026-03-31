/* fixed-point-test.c
 * Designed to trigger GCC's fixed-point range analysis logic
 * Compile with: gcc -O2 -ffixed-point -fstrict-overflow -o fixed_test fixed-point-test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Bit-field structure to force shift operations */
struct bitfield_fixed {
    unsigned int mantissa : 16;
    signed int sign : 1;
    unsigned int exponent : 7;
};

/* Function using bit-fields and shifts with fixed-point */
static long _Fract process_bitfield(struct bitfield_fixed bf) {
    /* Force shift operations that may trigger range calculations */
    volatile unsigned int shift_val = bf.exponent;
    
    /* Convert to fixed-point with shifting */
    short _Fract result = 0.5r;
    
    /* Left shift simulation */
    if (shift_val > 0) {
        /* This may trigger alshift operations in range analysis */
        for (unsigned int i = 0; i < shift_val && i < 8; i++) {
            result *= 2.0r;
        }
    }
    
    return (long _Fract)result;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Declare fixed-point variables spanning different ranges */
    volatile short _Fract f1 = 0.25r;
    volatile short _Fract f2 = -0.75r;
    volatile long _Accum a1 = 100.0k;
    volatile long _Accum a2 = -200.0k;
    
    /* Saturating types to force overflow checks */
    _Sat short _Fract sf1 = 0.8r;
    _Sat short _Fract sf2 = 0.9r;
    
    /* Accumulators for loop operations */
    long _Accum acc1 = 0.0k;
    long _Accum acc2 = 0.0k;
    long _Fract frac_acc = 0.0r;
    
    /* Mixed integer/fixed-point variables */
    volatile int int_bound = 100;
    volatile unsigned long ul_bound = 1000UL;
    
    /* Loop-based range widening */
    for (int i = 0; i < 50; i++) {
        /* Force range calculations with conditional operations */
        if (i % 3 == 0) {
            /* Multiplication that may saturate */
            sf1 *= 1.1r;
            sf2 *= 0.9r;
            
            /* Explicit range comparison - may trigger sgt/ugt */
            if (sf1 > 0.5r && sf2 < 0.5r) {
                acc1 += (long _Accum)sf1 * 2.0k;
            }
        }
        
        /* Addition with potential overflow */
        acc2 += a1 + (long _Accum)(i * 0.1k);
        
        /* Mixed-type conditional with explicit bounds */
        long _Accum temp = acc1 - acc2;
        
        /* Complex comparison that may invoke range logic */
        if ((temp > -50.0k && temp < 50.0k) || 
            (temp.ugt((unsigned long _Accum)ul_bound))) {
            frac_acc += (long _Fract)(temp * 0.01r);
        }
        
        /* Force range analysis with ternary operator */
        long _Accum bound_check = (i < int_bound) ? 
                                 (long _Accum)(i * 0.5k) : 
                                 (long _Accum)(int_bound * 0.5k);
        
        /* Another comparison that may trigger uncovered logic */
        if (bound_check.ugt((unsigned long _Accum)(ul_bound / 2))) {
            a1 *= 0.95k;
        } else {
            a2 += 1.0k;
        }
        
        /* Periodic reset to create varying ranges */
        if (i % 10 == 9) {
            /* Force re-evaluation of ranges */
            volatile int reset = i;
            if (reset & 1) {
                acc1 = (long _Accum)(reset * 0.2k);
            } else {
                acc2 = (long _Accum)(reset * -0.2k);
            }
        }
    }
    
    /* Bit-field operations */
    struct bitfield_fixed bf = {12345, 0, 5};
    long _Fract bf_result = process_bitfield(bf);
    
    /* Additional fixed-point operations outside loop */
    _Sat long _Accum sat_acc = 0.0k;
    for (int j = 0; j < 20; j++) {
        /* Operations that may trigger saturation logic */
        sat_acc += (j % 2 == 0) ? 50.0k : -30.0k;
        
        /* Explicit comparison with cast */
        if ((unsigned long _Accum)sat_acc > (unsigned long _Accum)200.0k) {
            sat_acc = 200.0k;
        }
    }
    
    /* Final computations with mixed types */
    long _Fract final_frac = frac_acc + bf_result;
    long _Accum final_acc = acc1 + acc2 + a1 + a2 + sat_acc;
    
    /* Convert to integer checksum to verify execution */
    int checksum = 0;
    checksum += (int)(final_frac * 1000.0r);
    checksum += (int)(final_acc);
    checksum += (int)(sf1 * 100.0r);
    checksum += (int)(sf2 * 100.0r);
    
    /* Print results to prevent optimization */
    printf("Checksum: %d\n", checksum);
    printf("Final accum: %Lf\n", (long double)final_acc);
    printf("Final frac: %Lf\n", (long double)final_frac);
    
    return 0;
}
