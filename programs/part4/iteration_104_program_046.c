/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Accum laccum;
typedef unsigned _Fract ufract;
typedef _Sat _Fract sat_fract;
typedef _Sat _Accum sat_accum;

/* Bit-field structure with fixed-point */
struct bitfield_fixed {
    unsigned int flag : 1;
    unsigned int value : 7;
    sfract sf_val;
    fract f_val;
};

/* Function using bit-fields and shifts */
static void process_bitfield(struct bitfield_fixed *bf, int iterations) {
    volatile ufract uv = 0.5r;
    volatile sfract sv = -0.25r;
    
    for (int i = 0; i < iterations; i++) {
        /* Shift operations that may trigger alshift logic */
        unsigned int shifted = bf->value << (i % 4);
        
        /* Fixed-point operations with shifted values */
        bf->sf_val = bf->sf_val + (sfract)(shifted * 0.01r);
        bf->f_val = bf->f_val * (fract)(0.9r + (i % 2) * 0.1r);
        
        /* Explicit range comparisons */
        if (bf->sf_val > (sfract)0.8r) {
            bf->sf_val = (sfract)0.8r;
        } else if (bf->sf_val < (sfract)(-0.8r)) {
            bf->sf_val = (sfract)(-0.8r);
        }
        
        /* Mixed integer/fixed-point comparison */
        if ((int)(bf->f_val * 100) > 50) {
            bf->flag = 1;
        } else {
            bf->flag = 0;
        }
        
        /* Update volatile bounds */
        uv = uv * 0.95r;
        sv = sv + (sfract)0.05r;
    }
}

int main(void) {
    /* Initialize fixed-point variables across different ranges */
    volatile fract f1 = 0.25r;
    volatile fract f2 = -0.75r;
    volatile laccum acc1 = 100.0k;
    volatile laccum acc2 = -50.0k;
    sat_fract sf1 = 0.5r;
    sat_accum sa1 = 0.0k;
    
    struct bitfield_fixed bf = {0, 32, 0.0r, 0.5r};
    
    int checksum = 0;
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Multi-step arithmetic that forces range calculations */
        f1 = f1 * 1.1r;
        f2 = f2 - 0.05r;
        
        /* Accumulator operations */
        acc1 = acc1 + (laccum)(i * 0.1k);
        acc2 = acc2 * 0.99k;
        
        /* Saturated arithmetic */
        sf1 = sf1 + (sat_fract)0.15r;
        sa1 = sa1 + (sat_accum)10.0k;
        
        /* Explicit comparisons triggering sgt/ugt logic */
        if (f1 > f2) {
            /* Range-based conditional */
            fract temp = f1 - f2;
            if (temp > 0.5r) {
                f1 = f1 * 0.9r;
            }
        } else {
            if (f2 < -0.8r) {
                f2 = -0.8r;
            }
        }
        
        /* Complex conditional with mixed types */
        if ((acc1 > 150.0k) || (acc1 == 150.0k && (unsigned long)acc2 > 100)) {
            acc1 = acc1 * 0.95k;
        }
        
        /* Ternary operator with fixed-point */
        laccum bound = (i % 3 == 0) ? 200.0k : 100.0k;
        if (acc1 > bound) {
            acc1 = bound;
        }
        
        /* Call bitfield processing every 10 iterations */
        if (i % 10 == 0) {
            process_bitfield(&bf, 5);
        }
        
        /* Update checksum with integer conversions */
        checksum += (int)(f1 * 1000);
        checksum += (int)(f2 * 1000);
        checksum += (int)(acc1 / 10);
        checksum += (int)(acc2 / 10);
    }
    
    /* Final range checks */
    volatile fract max_fract = 0.9r;
    volatile fract min_fract = -0.9r;
    
    if (f1 > max_fract || (f1 == max_fract && f2 > 0.0r)) {
        f1 = max_fract;
    }
    
    if (f2 < min_fract || (f2 == min_fract && f1 < 0.0r)) {
        f2 = min_fract;
    }
    
    /* Final checksum calculation */
    checksum += (int)(sf1 * 1000);
    checksum += (int)(sa1 / 100);
    checksum += bf.value * 100;
    checksum += bf.flag * 1000;
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: f1=%.3r, f2=%.3r, acc1=%.3k, acc2=%.3k\n", 
           f1, f2, acc1, acc2);
    
    return 0;
}
