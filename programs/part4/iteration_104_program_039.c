/* Test program to exercise fixed-point range analysis and comparison logic */
#include <stdio.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;

/* Bit-field structure with fixed-point conversion */
struct bitfield_fixed {
    unsigned int mantissa : 10;
    unsigned int exponent : 5;
    unsigned int sign : 1;
};

/* Function using bit-fields and shifts with fixed-point */
static accum_t process_bitfield(struct bitfield_fixed bf) {
    /* Convert bit-field to fixed-point with shifting */
    ufract_t uf = (ufract_t)bf.mantissa / 1024.0r;
    
    /* Left shift simulation through multiplication */
    accum_t shifted = (accum_t)uf;
    for (unsigned i = 0; i < bf.exponent; i++) {
        shifted *= 2.0lk;
    }
    
    /* Apply sign */
    if (bf.sign) {
        shifted = -shifted;
    }
    
    return shifted;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables across range */
    sfract_t sf1 = 0.5hr;
    sfract_t sf2 = -0.25hr;
    accum_t acc1 = 0.0lk;
    accum_t acc2 = 1000.0lk;
    fract_t f1 = 0.1r;
    fract_t f2 = 0.9r;
    sat_fract_t sat1 = 0.5r;
    
    /* Volatile variables to prevent constant folding */
    volatile fract_t vbound = 0.75r;
    volatile int viter = 8;
    
    /* Loop with range-widening operations */
    for (int i = 0; i < viter; i++) {
        /* Mixed precision arithmetic */
        acc1 += (accum_t)sf1 * (accum_t)f1;
        acc2 -= (accum_t)sf2 * (accum_t)f2;
        
        /* Saturated arithmetic */
        sat1 += 0.3r;
        
        /* Range comparisons triggering sgt/ugt logic */
        if (acc1 > (accum_t)vbound) {
            /* Exceeds upper bound - scale down */
            acc1 *= 0.5lk;
            sf1 = -0.1hr;
        } else if (acc1 < (accum_t)(-vbound)) {
            /* Below lower bound - scale up */
            acc1 *= 1.5lk;
            sf1 = 0.1hr;
        }
        
        /* Ternary operator with mixed types */
        f1 = (acc2 > 500.0lk) ? 0.8r : 0.2r;
        
        /* Complex conditional with explicit bounds */
        accum_t temp = acc1 + acc2;
        accum_t max_bound = 2000.0lk;
        accum_t min_bound = -2000.0lk;
        
        if (temp > max_bound || temp < min_bound) {
            /* Force range clipping */
            acc2 = (temp > max_bound) ? max_bound : min_bound;
        }
        
        /* Shift-like operations through multiplication */
        if (i % 3 == 0) {
            f2 *= 2.0r;  /* Simulates left shift */
        } else if (i % 3 == 1) {
            f2 *= 0.5r;  /* Simulates right shift */
        }
    }
    
    /* Bit-field operations */
    struct bitfield_fixed bf = {512, 3, 0};  /* 0.5 * 2^3 = 4.0 */
    accum_t bf_result = process_bitfield(bf);
    
    /* Additional explicit range checks */
    accum_t computed = acc1 + acc2 + bf_result;
    
    /* Multiple comparisons in single expression */
    int range_ok = (computed > -10000.0lk) && 
                   (computed < 10000.0lk) &&
                   (computed != 0.0lk);
    
    /* Conditional based on range check */
    fract_t final_fract;
    if (range_ok) {
        final_fract = (fract_t)(computed / 100.0lk);
    } else {
        /* Use saturation */
        sat_fract_t sat_temp = (sat_fract_t)computed;
        final_fract = (fract_t)sat_temp;
    }
    
    /* Compute checksum */
    unsigned int checksum = 0;
    checksum += (unsigned int)(acc1 * 100.0lk);
    checksum += (unsigned int)(acc2 * 100.0lk);
    checksum += (unsigned int)(final_fract * 1000.0r);
    checksum += (unsigned int)(sat1 * 1000.0r);
    
    printf("Checksum: %u\n", checksum);
    printf("acc1: %ld.%06ld\n", 
           (long)(acc1), 
           (long)((acc1 - (long)acc1) * 1000000));
    printf("acc2: %ld.%06ld\n", 
           (long)(acc2), 
           (long)((acc2 - (long)acc2) * 1000000));
    
    return 0;
}
