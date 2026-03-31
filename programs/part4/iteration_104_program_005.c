/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Accum laccum;
typedef unsigned short _Fract usfract;
typedef _Sat short _Fract sat_sfract;

/* Bit-field structure with fixed-point */
struct fixed_bitfield {
    unsigned int mantissa : 10;
    unsigned int exponent : 5;
    sfract fixed_val;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf, int shift) {
    /* Shift operations that may trigger alshift logic */
    unsigned int shifted = bf.mantissa << shift;
    
    /* Convert to fixed-point with potential range checks */
    sfract result = (_Fract)shifted / 256.0r;
    
    /* Conditional with explicit comparison */
    if (result > 0.5r) {
        bf.fixed_val = result * 0.75r;
    } else {
        bf.fixed_val = result + 0.25r;
    }
    
    return bf;
}

/* Function with mixed integer/fixed-point contexts */
static laccum mixed_precision_computation(int base, fract multiplier) {
    volatile laccum accum = 0.0lk;
    volatile int bound = base * 2;
    
    /* Loop with volatile bounds to prevent constant folding */
    for (volatile int i = 0; i < bound; i++) {
        /* Mixed precision operations */
        int int_part = i * 10;
        fract frac_part = (_Fract)int_part / 1000.0r;
        
        /* Arithmetic that may overflow */
        accum += (laccum)frac_part * multiplier;
        
        /* Explicit range comparison - may trigger sgt/ugt */
        if (accum > 1.0lk) {
            accum = accum * 0.9lk;
        } else if (accum < -1.0lk) {
            accum = accum * 0.8lk + 0.1lk;
        }
        
        /* Ternary operator with fixed-point */
        accum = (i % 3 == 0) ? accum * 1.1lk : accum * 0.95lk;
    }
    
    return accum;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize fixed-point variables across ranges */
    sfract f1 = 0.25r;
    sfract f2 = -0.75r;
    usfract uf1 = 0.8ur;
    laccum acc1 = 0.0lk;
    laccum acc2 = -0.5lk;
    sat_sfract sat1 = 0.9r;
    
    /* Bit-field initialization */
    struct fixed_bitfield bf = {512, 8, 0.5r};
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Multi-step arithmetic */
        f1 = f1 * 1.01r + 0.001r;
        f2 = f2 * 0.99r - 0.002r;
        
        /* Operations that may saturate */
        sat1 = sat1 * 1.1r;
        
        /* Accumulator with potential overflow */
        acc1 += (laccum)f1 * 0.1lk;
        acc2 += (laccum)f2 * 0.05lk;
        
        /* Explicit comparisons against computed bounds */
        sfract max_bound = 0.9r - (_Fract)i / 200.0r;
        sfract min_bound = -0.8r + (_Fract)i / 300.0r;
        
        /* Conditional blocks that may trigger range logic */
        if (f1 > max_bound) {
            f1 = max_bound * 0.9r;
            checksum += 1;
        }
        
        if (f2 < min_bound) {
            f2 = min_bound * 1.1r;
            checksum += 2;
        }
        
        /* Mixed-type comparison */
        if ((int)(f1 * 1000) > i * 10) {
            uf1 = uf1 * 0.95ur;
            checksum += 3;
        }
        
        /* Call bit-field function periodically */
        if (i % 7 == 0) {
            bf = process_fixed_bitfield(bf, i % 4);
            checksum += 4;
        }
    }
    
    /* Mixed precision computation */
    laccum mixed_result = mixed_precision_computation(50, 0.75r);
    
    /* Final range checks */
    volatile laccum test_val = acc1 + acc2;
    if (test_val > 0.0lk) {
        test_val = test_val * 0.5lk;
    } else {
        test_val = test_val * 1.5lk;
    }
    
    /* Compute checksum from all results */
    checksum += (int)(f1 * 1000);
    checksum += (int)(f2 * 1000);
    checksum += (int)(uf1 * 1000);
    checksum += (int)(acc1 * 1000);
    checksum += (int)(acc2 * 1000);
    checksum += (int)(sat1 * 1000);
    checksum += (int)(mixed_result * 1000);
    checksum += (int)(test_val * 1000);
    checksum += bf.mantissa;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
