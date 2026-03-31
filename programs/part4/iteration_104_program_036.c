/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure with fixed-point conversion */
struct bitfield_fixed {
    unsigned int mantissa : 10;
    unsigned int exponent : 5;
    unsigned int sign : 1;
};

/* Function using bit-fields and shifts with fixed-point */
static accum_t process_bitfield(struct bitfield_fixed bf) {
    /* Convert bit-field to fixed-point with shifting */
    ufract_t uf_val = (ufract_t)bf.mantissa / 1024.0r;
    
    /* Left shift simulation through multiplication */
    accum_t shifted = (accum_t)uf_val;
    for (int i = 0; i < bf.exponent; i++) {
        shifted *= 2.0lk;
    }
    
    /* Apply sign */
    if (bf.sign) {
        shifted = -shifted;
    }
    
    return shifted;
}

/* Main function with range analysis triggers */
int main(void) {
    /* Initialize fixed-point variables across range */
    sfract_t sf1 = 0.5hr;
    sfract_t sf2 = -0.25hr;
    accum_t acc1 = 0.0lk;
    accum_t acc2 = 1000.0lk;
    sat_fract_t sat_f = 0.7r;
    sat_accum_t sat_acc = 500.0lk;
    
    /* Volatile bounds to prevent constant folding */
    volatile accum_t volatile_bound = 800.0lk;
    volatile fract_t volatile_frac = 0.3r;
    
    /* Mixed integer/fixed-point variables */
    int int_counter = 0;
    unsigned long ulong_mask = 0xFFFF;
    
    /* Loop with range widening */
    for (int i = 0; i < 100; i++) {
        /* Multi-step arithmetic forcing range calculations */
        acc1 += (accum_t)sf1 * 1.5lk;
        acc2 -= (accum_t)sf2 * 2.0lk;
        
        /* Saturated operations */
        sat_f += 0.1r;
        sat_acc *= 1.01lk;
        
        /* Explicit range comparisons (triggers sgt/ugt) */
        if (acc1 > (accum_t)volatile_bound) {
            /* This should trigger max/min range calculations */
            acc1 = volatile_bound;
        }
        
        if (acc2 < -volatile_bound) {
            acc2 = -volatile_bound;
        }
        
        /* Ternary operator with mixed types */
        fract_t temp = (int_counter & 1) ? 
                      (fract_t)(0.5r * volatile_frac) : 
                      (fract_t)(0.25r / volatile_frac);
        
        /* Compare against computed bounds */
        accum_t computed_bound = (accum_t)temp * 1000.0lk;
        if (acc1 > computed_bound || acc2 < -computed_bound) {
            /* Adjust values based on range comparison */
            acc1 = computed_bound * 0.9lk;
            acc2 = -computed_bound * 0.9lk;
        }
        
        /* Bit-field operations every 10 iterations */
        if (i % 10 == 0) {
            struct bitfield_fixed bf = {
                .mantissa = (i * 31) & 0x3FF,
                .exponent = (i / 10) & 0x1F,
                .sign = i & 1
            };
            accum_t bf_result = process_bitfield(bf);
            
            /* Range check on bit-field result */
            if (bf_result > 500.0lk || bf_result < -500.0lk) {
                bf_result = (bf_result > 0) ? 500.0lk : -500.0lk;
            }
            
            acc1 += bf_result * 0.1lk;
        }
        
        int_counter++;
    }
    
    /* Additional overflow-triggering operations */
    accum_t large_val = 1000000.0lk;
    for (int i = 0; i < 50; i++) {
        large_val *= 1.1lk;
        
        /* This multiplication may overflow fixed-point range */
        sat_accum_t test_sat = sat_acc * (sat_accum_t)large_val;
        
        /* Conditional based on overflow check */
        if (test_sat > 10000.0lk || test_sat < -10000.0lk) {
            sat_acc = test_sat * 0.5lk;
        }
    }
    
    /* Final range comparisons with type conversions */
    unsigned long ul_result = (unsigned long)(acc1 * 100.0lk);
    long sl_result = (long)(acc2 * 100.0lk);
    
    /* More comparisons that might trigger the uncovered logic */
    if ((accum_t)ul_result > acc1 || (accum_t)sl_result < acc2) {
        /* Adjust if conversion changed ordering */
        ul_result = (unsigned long)acc1;
        sl_result = (long)acc2;
    }
    
    /* Compute checksum */
    uint64_t checksum = 0;
    checksum += (uint64_t)(acc1 * 1000.0lk);
    checksum += (uint64_t)(acc2 * 1000.0lk);
    checksum += (uint64_t)(sat_f * 1000.0r);
    checksum += (uint64_t)(sat_acc * 1000.0lk);
    checksum += ul_result;
    checksum += (uint64_t)sl_result;
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    printf("Final values: acc1=%.4Lk, acc2=%.4Lk\n", 
           (long double)acc1, (long double)acc2);
    
    return 0;
}
