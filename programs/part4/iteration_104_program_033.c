/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef _Sat short _Fract sat_sfract_t;

/* Bit-field structure with fixed-point */
struct bitfield_fixed {
    unsigned int flag : 1;
    unsigned int value : 15;  /* Will be used with fixed-point conversions */
    sfract_t frac_part;
};

/* Function using bit-fields and shifts */
static void process_bitfield(struct bitfield_fixed *bf, int iterations) {
    volatile sfract_t volatile_bound = 0.5r;
    
    for (int i = 0; i < iterations; i++) {
        /* Shift operations that may trigger alshift logic */
        unsigned int shifted = bf->value << (i % 8);
        
        /* Convert to fixed-point with potential range issues */
        sfract_t converted = (sfract_t)shifted / 32768.0r;
        
        /* Conditional with explicit range comparison */
        if (converted > volatile_bound) {
            bf->frac_part = converted * 0.75r;
        } else if (converted < -volatile_bound) {
            bf->frac_part = converted * 1.25r;
        }
        
        /* Mixed integer/fixed-point operation */
        bf->value = (unsigned int)(converted * 32768.0r) & 0x7FFF;
    }
}

/* Main function with loop-based range widening */
int main(void) {
    /* Initialize fixed-point variables across range */
    volatile fract_t v1 = 0.25r;
    volatile fract_t v2 = -0.75r;
    accum_t acc1 = 0.0lk;
    accum_t acc2 = 0.0lk;
    sat_sfract_t sat_acc = 0.0hr;
    
    /* Volatile bounds to prevent constant folding */
    volatile accum_t volatile_max = 100.0lk;
    volatile accum_t volatile_min = -100.0lk;
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 1000; i++) {
        /* Multi-step computation forcing range analysis */
        fract_t temp = v1 * v2;
        
        /* Conditional with explicit comparisons */
        if (temp > 0.0r) {
            acc1 += (accum_t)temp * 0.5lk;
        } else {
            acc2 += (accum_t)temp * 2.0lk;
        }
        
        /* Check against volatile bounds */
        if (acc1 > volatile_max || acc2 < volatile_min) {
            /* Reset accumulators if out of bounds */
            acc1 = acc1 * 0.5lk;
            acc2 = acc2 * 0.5lk;
        }
        
        /* Saturated arithmetic */
        sat_acc += (sat_sfract_t)(temp * 0.25r);
        
        /* Update volatile variables */
        v1 = v1 * 0.999r;
        v2 = v2 * 0.998r;
    }
    
    /* Bit-field operations */
    struct bitfield_fixed bf = {0};
    bf.value = 0x4000;  /* Mid-range value */
    bf.frac_part = 0.5r;
    
    process_bitfield(&bf, 50);
    
    /* Mixed integer/fixed-point context */
    unsigned long int_sum = 0;
    int_sum += (unsigned long)(acc1 * 1000.0lk);
    int_sum += (unsigned long)(acc2 * 1000.0lk);
    int_sum += (unsigned long)(sat_acc * 1000.0hr);
    int_sum += bf.value;
    
    /* Print checksum to verify execution */
    printf("Checksum: %lu\n", int_sum);
    
    /* Additional explicit range comparisons */
    accum_t test_vals[] = {acc1, acc2, (accum_t)bf.frac_part};
    for (int i = 0; i < 3; i++) {
        /* Ternary operator with range check */
        accum_t result = (test_vals[i] > 50.0lk) ? test_vals[i] * 0.9lk :
                         (test_vals[i] < -50.0lk) ? test_vals[i] * 1.1lk :
                         test_vals[i];
        
        /* Complex conditional triggering sgt/ugt */
        if (result > 75.0lk && result < 150.0lk) {
            printf("Value %d in range (75, 150)\n", i);
        } else if (result < -75.0lk || result > 200.0lk) {
            printf("Value %d out of expected range\n", i);
        }
    }
    
    return 0;
}
