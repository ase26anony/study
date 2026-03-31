/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Accum laccum;
typedef unsigned short _Fract usfract;
typedef _Sat short _Fract sat_sfract;

/* Bit-field structure for shift operations */
struct bitfield_fixed {
    unsigned int mantissa : 16;
    unsigned int exponent : 8;
    unsigned int sign : 1;
};

/* Function using bit-fields and shifts */
static void process_bitfield_fixed(struct bitfield_fixed *bf, volatile sfract *result) {
    /* Operations that may trigger alshift/zext/sext */
    unsigned int shifted = bf->mantissa << bf->exponent;
    
    /* Convert to fixed-point with potential range checks */
    *result = (_Fract)(shifted & 0xFFFF) / 32768.0r;
    
    /* Conditional with explicit comparison */
    if (bf->exponent > 4) {
        *result = *result * 0.5r;
    }
}

int main(void) {
    /* Initialize fixed-point variables across range */
    volatile sfract f1 = 0.5r;
    volatile sfract f2 = -0.25r;
    volatile laccum a1 = 100.0k;
    volatile laccum a2 = -50.0k;
    volatile usfract uf1 = 0.75ur;
    sat_sfract sat1 = 0.9r;
    
    /* Accumulators for loop operations */
    _Accum acc1 = 0.0k;
    _Accum acc2 = 0.0k;
    _Fract fract_acc = 0.0r;
    
    /* Non-constant bounds to prevent folding */
    volatile int bound1 = 10;
    volatile int bound2 = 20;
    
    /* Bit-field structure */
    struct bitfield_fixed bf = {32767, 3, 0};
    
    /* Loop 1: Range widening through repeated operations */
    for (int i = 0; i < bound1; i++) {
        /* Mixed precision operations */
        acc1 = acc1 + (_Accum)f1 * 0.1k;
        acc2 = acc2 - (_Accum)f2 * 0.05k;
        
        /* Conditional with explicit range comparison */
        if (acc1 > (_Accum)(i * 0.5k)) {
            fract_acc = fract_acc + 0.01r;
        } else {
            fract_acc = fract_acc - 0.005r;
        }
        
        /* Ternary operator with fixed-point comparison */
        sat1 = (acc2 > 0.0k) ? 0.8r : 0.2r;
        
        /* Force range analysis with shifting bounds */
        if (i > bound2 / 3) {
            a1 = a1 * 1.1k;
        }
    }
    
    /* Loop 2: More complex range calculations */
    for (volatile int j = 0; j < 15; j++) {
        /* Operations that may trigger overflow checks */
        _Accum temp = a1 + a2;
        
        /* Explicit comparison against computed bounds */
        if (temp > (_Accum)(j * 10.0k)) {
            a1 = a1 * 0.95k;
        } else if (temp < (_Accum)(-j * 5.0k)) {
            a2 = a2 * 1.05k;
        }
        
        /* Mixed integer/fixed-point context */
        fract_acc = fract_acc * (_Fract)(j % 8) / 8.0r;
        
        /* Call bitfield function periodically */
        if (j % 4 == 0) {
            sfract bf_result;
            process_bitfield_fixed(&bf, &bf_result);
            fract_acc = fract_acc + bf_result;
            
            /* Update bitfield for next iteration */
            bf.mantissa = (bf.mantissa * 3) & 0xFFFF;
            bf.exponent = (bf.exponent + 1) & 0x7;
        }
    }
    
    /* Final computations with saturation */
    sat_sfract final_sat = sat1 * 2.0r;  /* May saturate */
    
    /* Additional range comparisons */
    _Accum final_acc = acc1 + acc2;
    if (final_acc > 100.0k || final_acc < -100.0k) {
        final_acc = (final_acc > 0.0k) ? 99.9k : -99.9k;
    }
    
    /* Compute checksum */
    unsigned long checksum = 0;
    checksum += (unsigned long)(f1 * 1000);
    checksum += (unsigned long)(f2 * 1000);
    checksum += (unsigned long)(a1 / 10);
    checksum += (unsigned long)(a2 / 10);
    checksum += (unsigned long)(uf1 * 1000);
    checksum += (unsigned long)(sat1 * 1000);
    checksum += (unsigned long)(fract_acc * 1000);
    checksum += (unsigned long)(final_acc / 10);
    checksum += (unsigned long)(final_sat * 1000);
    
    printf("Checksum: %lu\n", checksum);
    
    /* Additional conditional to exercise comparison logic */
    if ((_Accum)checksum > a1 && (_Accum)checksum < a2) {
        printf("Unexpected range\n");
    }
    
    return 0;
}
