/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point types with various precisions */
typedef short _Fract sf;
typedef _Fract f;
typedef long _Fract lf;
typedef short _Accum sa;
typedef _Accum a;
typedef long _Accum la;
typedef short _Sat _Fract ssf;
typedef _Sat _Fract sfx;
typedef _Sat _Accum sa_x;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf1 : 8;
    signed int sbf1 : 8;
    unsigned int ubf2 : 12;
    signed int sbf2 : 12;
};

/* Function using bit-fields and shifts with fixed-point */
void bitfield_fixed_ops(struct mixed_bf *bf, volatile sf *result) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int temp = bf->ubf1;
    
    /* Shift operations that require range analysis */
    temp = temp << (bf->ubf2 & 0x7);  /* Variable shift amount */
    
    /* Convert to fixed-point with potential range checks */
    sf f_val = (sf)(temp * (1.0f / 256.0f));
    
    /* More shifts with fixed-point */
    int shift_amt = (int)bf->sbf1;
    if (shift_amt > 0) {
        temp = temp >> (shift_amt & 0x3);
    }
    
    /* Another fixed-point conversion */
    sf f_val2 = (sf)((temp & 0xFF) * (1.0f / 256.0f));
    
    /* Conditional based on shifted values */
    *result = (temp > 128) ? f_val : f_val2;
    
    /* Explicit comparison that might use sgt/ugt */
    if (f_val > f_val2 && (int)temp > 64) {
        *result = (sf)0.5;
    }
}

int main() {
    /* Initialize fixed-point variables across range */
    volatile sf f1 = 0.25;
    volatile sf f2 = -0.25;
    volatile a a1 = 100.0;
    volatile a a2 = -100.0;
    volatile la la1 = 500.0;
    volatile la la2 = -500.0;
    
    /* Saturated types */
    ssf sat1 = 0.75;
    sfx sat2 = -0.5;
    sa_x sat_acc = 0.0;
    
    /* Accumulators for loop */
    a acc1 = 0.0;
    a acc2 = 0.0;
    la wide_acc = 0.0;
    
    /* Mixed integer/fixed-point variables */
    int int_val = 100;
    unsigned long ul_val = 1000;
    
    /* Bit-field structure */
    struct mixed_bf bf = {128, -32, 8, -8};
    volatile sf bf_result;
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Multi-step computations forcing range evaluation */
        acc1 = acc1 + f1 * a1;
        acc2 = acc2 + f2 * a2;
        
        /* Mixed precision operations */
        wide_acc = wide_acc + (la)(acc1 * 0.1) + (la)(acc2 * 0.1);
        
        /* Saturated operations */
        sat_acc = sat_acc + sat1;
        if (sat_acc > 0.9) {
            sat_acc = sat_acc * 0.5;
        }
        
        /* Explicit range comparisons - may trigger sgt/ugt logic */
        a temp_val = acc1 + (a)(i * 0.01);
        
        /* Complex conditional with multiple comparisons */
        if (temp_val > 50.0 || (temp_val < -50.0 && acc2 > -25.0)) {
            acc1 = acc1 * 0.9;
        } else if (temp_val == 0.0 || (temp_val > 0.0 && temp_val < 10.0)) {
            acc2 = acc2 + 0.1;
        }
        
        /* Ternary operator with fixed-point comparison */
        a bound = (i % 2 == 0) ? 25.0 : 75.0;
        acc1 = (temp_val > bound) ? acc1 - 1.0 : acc1 + 0.5;
        
        /* Mixed integer/fixed-point comparison */
        if ((int)acc1 > int_val && (unsigned long)wide_acc < ul_val) {
            wide_acc = wide_acc * 1.1;
        }
        
        /* Non-constant bounds using volatile variables */
        if (acc1 > f1 * 100.0 || acc2 < f2 * 100.0) {
            /* Force range analysis with volatile operands */
            volatile a vol_bound = a1 * 0.5;
            if (acc1 > vol_bound) {
                acc1 = vol_bound;
            }
        }
        
        /* Periodically call bitfield function */
        if (i % 10 == 0) {
            bitfield_fixed_ops(&bf, &bf_result);
            acc1 = acc1 + (a)bf_result;
        }
        
        /* Shift operations in integer domain affecting fixed-point */
        int_val = int_val << 1;
        if (int_val > 10000) {
            int_val = 100;
        }
        
        /* Overflow-like check */
        if (wide_acc > 10000.0 || wide_acc < -10000.0) {
            wide_acc = wide_acc * 0.5;
        }
    }
    
    /* Compute checksum */
    long checksum = 0;
    checksum += (long)(acc1 * 1000);
    checksum += (long)(acc2 * 1000);
    checksum += (long)(wide_acc);
    checksum += (long)(sat_acc * 1000);
    checksum += (long)(bf_result * 1000);
    checksum += int_val;
    checksum += (long)ul_val;
    
    printf("Checksum: %ld\n", checksum);
    
    /* Additional explicit comparisons to trigger edge cases */
    {
        /* Create extreme values */
        la max_test = 100000.0;
        la min_test = -100000.0;
        
        /* Comparisons that might use the uncovered sgt/ugt logic */
        int cmp1 = (max_test > 50000.0) ? 1 : 0;
        int cmp2 = (min_test < -50000.0) ? 1 : 0;
        int cmp3 = (wide_acc > acc1) ? 1 : 0;
        int cmp4 = (wide_acc == max_test) ? 1 : 0;
        
        /* Use results to avoid dead code elimination */
        checksum += cmp1 + cmp2 + cmp3 + cmp4;
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
