/* Fixed-point range analysis test targeting GCC's fixed-value.cc logic */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Accum laccum;
typedef unsigned short _Fract usfract;
typedef _Sat short _Fract sat_sfract;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    usfract fractional;  /* unsigned short _Fract */
    volatile sat_sfract saturated; /* volatile saturated fixed-point */
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf, int shift) {
    /* Operations that may trigger alshift/zext/sext logic */
    struct fixed_bitfield result = bf;
    
    /* Shift operations on bit-field components */
    result.integer <<= shift;
    result.integer >>= (shift / 2);
    
    /* Fixed-point operations with shifting */
    usfract temp = result.fractional;
    for (int i = 0; i < shift; i++) {
        /* Repeated operations to force range widening */
        temp = temp * (usfract)0.5r;
    }
    result.fractional = temp;
    
    /* Conditional based on shifted values */
    if (result.integer > (1 << (shift + 1))) {
        result.saturated = (sat_sfract)0.9r;
    } else if (result.integer < (1 << (shift - 1))) {
        result.saturated = (sat_sfract)-0.9r;
    }
    
    return result;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables across different ranges */
    sfract f1 = 0.5hr;
    sfract f2 = -0.25hr;
    laccum acc1 = 0.0lk;
    laccum acc2 = 0.0lk;
    volatile fract vbound = 0.75r;  /* volatile to prevent constant folding */
    
    /* Mixed integer/fixed-point variables */
    int int_counter = 0;
    unsigned long ul_bound = 1000;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {
        .sign = 0,
        .integer = 32,
        .fractional = 0.8uhr,
        .saturated = 0.0hr
    };
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Multi-step fixed-point arithmetic */
        f1 = f1 + (sfract)0.01hr;
        f2 = f2 * (sfract)0.99hr;
        
        /* Accumulator operations forcing range analysis */
        acc1 = acc1 + (laccum)f1;
        acc2 = acc2 * (laccum)0.999lk - (laccum)f2;
        
        /* Explicit range comparisons - may trigger sgt/ugt logic */
        if (acc1 > (laccum)50.0lk) {
            /* Conditional operations based on range check */
            acc1 = acc1 * (laccum)0.5lk;
            int_counter += 1;
        }
        
        /* Comparison against volatile bound (non-constant) */
        if ((fract)f1 > vbound) {
            f1 = f1 * (sfract)0.5hr;
        }
        
        /* Ternary operator with mixed types */
        laccum temp = (int_counter > 10) ? 
                     (laccum)int_counter * 0.1lk : 
                     (laccum)int_counter * -0.1lk;
        
        /* Complex comparison chain */
        if (acc2.sgt(temp) || (acc2 == temp && f1.ugt(f2))) {
            /* This comparison structure mirrors the uncovered code pattern */
            acc2 = acc2 - temp;
        }
        
        /* Mixed integer/fixed-point operation in condition */
        if ((unsigned long)(int_counter * 100) > ul_bound) {
            ul_bound = ul_bound << 1;  /* Shift operation */
        }
        
        /* Periodically process bit-field */
        if (i % 7 == 0) {
            bf = process_fixed_bitfield(bf, (i % 5) + 1);
        }
    }
    
    /* Additional fixed-point range boundary tests */
    sat_sfract sat1 = 0.9hr;
    sat_sfract sat2 = -0.9hr;
    
    /* Operations near saturation boundaries */
    for (int i = 0; i < 20; i++) {
        sat1 = sat1 + (sat_sfract)0.1hr;  /* May saturate */
        sat2 = sat2 - (sat_sfract)0.1hr;  /* May saturate */
        
        /* Comparisons triggering range checks */
        if (sat1 > (sat_sfract)0.5hr && sat2 < (sat_sfract)-0.5hr) {
            sat1 = sat1 * (sat_sfract)0.8hr;
            sat2 = sat2 * (sat_sfract)0.8hr;
        }
    }
    
    /* Compute checksum from all results */
    unsigned long checksum = 0;
    checksum += (unsigned long)(f1 * (sfract)1000hr);
    checksum += (unsigned long)(f2 * (sfract)1000hr);
    checksum += (unsigned long)(acc1 * (laccum)0.01lk);
    checksum += (unsigned long)(acc2 * (laccum)0.01lk);
    checksum += (unsigned long)(sat1 * (sat_sfract)1000hr);
    checksum += (unsigned long)(sat2 * (sat_sfract)1000hr);
    checksum += bf.integer;
    checksum += (unsigned long)(bf.fractional * (usfract)1000uhr);
    checksum += (unsigned long)(bf.saturated * (sat_sfract)1000hr);
    checksum += int_counter;
    checksum += ul_bound;
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
