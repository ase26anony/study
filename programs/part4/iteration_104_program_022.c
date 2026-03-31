/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    unsigned int fraction : 8;
    sat_fract_t sat_val;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(usfract_t input) {
    struct fixed_bitfield result;
    volatile usfract_t temp = input;
    
    /* Shift operations that may trigger range calculations */
    result.integer = (unsigned)(temp >> 8) & 0x7F;
    result.fraction = (unsigned)(temp << 4) >> 8;
    result.sign = (temp > 0x7FFF) ? 1 : 0;
    
    /* Fixed-point operation with potential overflow */
    result.sat_val = (_Sat _Fract)(temp * 0.5r);
    
    return result;
}

/* Main function with loops and range comparisons */
int main(void) {
    /* Initialize fixed-point variables */
    sfract_t f1 = 0.5hr;
    fract_t f2 = -0.25r;
    accum_t a1 = 1000.0lk;
    accum_t a2 = -500.0lk;
    sat_accum_t sat_acc = 0.0lk;
    volatile fract_t volatile_bound = 0.75r;
    
    /* Accumulators for range testing */
    accum_t range_accum = 0.0lk;
    fract_t fract_accum = 0.0r;
    
    /* Loop-based range widening */
    for (int i = 0; i < 100; i++) {
        /* Mixed precision operations */
        accum_t temp = a1 + (accum_t)f1 * i;
        
        /* Explicit range comparisons - may trigger sgt/ugt operations */
        if (temp > 2000.0lk) {
            /* Upper bound check */
            a1 = a1 * 0.9lk;
        } else if (temp < -1000.0lk) {
            /* Lower bound check */
            a1 = a1 * 1.1lk;
        }
        
        /* Ternary operator with fixed-point comparison */
        fract_accum = (f2 > volatile_bound) ? 
                     fract_accum * 0.8r : 
                     fract_accum + 0.1r;
        
        /* Saturated accumulator with overflow potential */
        sat_acc += (_Sat _Accum)(f1 * 0.01r);
        
        /* Range comparison against computed bounds */
        accum_t max_bound = a2 * 2.0lk;
        accum_t min_bound = a2 * 0.5lk;
        
        /* This comparison structure may trigger the uncovered logic */
        if (temp > max_bound || (temp == max_bound && (uint64_t)temp > 0)) {
            range_accum = range_accum - 50.0lk;
        }
        
        /* Update variables for next iteration */
        f1 = f1 * 0.99hr;
        f2 = f2 + 0.01r;
        a2 = a2 + 10.0lk;
    }
    
    /* Bit-field and shift operations */
    usfract_t usf = 0x5555r;
    struct fixed_bitfield bf = process_fixed_bitfield(usf);
    
    /* Additional range comparisons with type conversions */
    int int_check = 0;
    for (unsigned long j = 0; j < 50; j++) {
        /* Mixed integer/fixed-point context */
        accum_t mixed_val = a1 + (accum_t)j;
        
        /* Explicit cast and comparison */
        if ((long)mixed_val > 1000L || (unsigned long)mixed_val < 100UL) {
            int_check++;
        }
        
        /* Shift operation on fixed-point converted to integer */
        unsigned shifted = (unsigned)(usf << 2) >> 4;
        if (shifted > 0x0FFF) {
            usf = usf >> 1;
        }
    }
    
    /* Compute checksum */
    long checksum = (long)(f1 * 1000hr) + 
                   (long)(f2 * 1000r) + 
                   (long)(a1 / 10lk) + 
                   (long)(a2 / 10lk) + 
                   (long)range_accum + 
                   (long)fract_accum * 100 +
                   bf.integer + bf.fraction +
                   int_check;
    
    printf("Checksum: %ld\n", checksum);
    
    /* Final range comparison that might trigger the exact uncovered code */
    {
        accum_t final_test = a1 * a2;
        volatile accum_t volatile_max = 50000.0lk;
        volatile accum_t volatile_min = -50000.0lk;
        
        /* Multi-part comparison similar to uncovered lines */
        if (final_test > volatile_max || 
            (final_test == volatile_max && final_test > 0.0lk)) {
            printf("Above maximum range\n");
        }
        
        if (final_test < volatile_min || 
            (final_test == volatile_min && final_test < 0.0lk)) {
            printf("Below minimum range\n");
        }
    }
    
    return 0;
}
