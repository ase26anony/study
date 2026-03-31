/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

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
    sfract_t sf_val;
    usfract_t usf_val;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf, int shift) {
    /* Operations that may trigger alshift/zext/sext logic */
    bf.integer <<= shift;
    bf.fraction >>= (shift % 4);
    
    /* Fixed-point operations with bit-field context */
    sfract_t temp = bf.sf_val;
    for (int i = 0; i < shift; i++) {
        temp += (sfract_t)0.1hr;
        if (temp > (sfract_t)0.5hr) {
            temp = (sfract_t)0.0hr;
        }
    }
    bf.sf_val = temp;
    
    return bf;
}

/* Main test function */
int main(void) {
    /* Initialize fixed-point variables across range */
    sfract_t sf1 = 0.5hr;
    sfract_t sf2 = -0.25hr;
    fract_t f1 = 0.75r;
    accum_t a1 = 100.0lk;
    accum_t a2 = -50.0lk;
    sat_fract_t sat_f = 0.8r;
    sat_accum_t sat_a = 200.0lk;
    
    /* Volatile bounds to prevent constant folding */
    volatile fract_t volatile_bound = 0.6r;
    volatile accum_t volatile_accum = 150.0lk;
    
    /* Accumulators for loop operations */
    accum_t accum_min = -1000.0lk;
    accum_t accum_max = 1000.0lk;
    accum_t accumulator = 0.0lk;
    
    /* Mixed integer/fixed-point variables */
    int int_counter = 0;
    unsigned long ulong_bound = 100;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {
        .sign = 0,
        .integer = 5,
        .fraction = 128,
        .sf_val = 0.3hr,
        .usf_val = 0.7uhr
    };
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Force range calculations through mixed operations */
        accum_t temp_acc = accumulator;
        
        /* Operation that may trigger sgt/ugt comparisons */
        if (temp_acc > accum_max) {
            accum_max = temp_acc;
        }
        if (temp_acc < accum_min) {
            accum_min = temp_acc;
        }
        
        /* Complex conditional with explicit range comparisons */
        if (accumulator > (accum_t)50.0lk && 
            accumulator < (accum_t)200.0lk) {
            /* Middle range operations */
            accumulator += a1 * (accum_t)0.1lk;
            sf1 = sf1 * (sfract_t)0.9hr;
        } else if (accumulator <= (accum_t)(-100.0lk)) {
            /* Negative range operations */
            accumulator += a2;
            sf2 = sf2 - (sfract_t)0.05hr;
        } else {
            /* Default operations */
            accumulator += (accum_t)10.0lk;
        }
        
        /* Compare against volatile bounds (non-constant) */
        if (accumulator > volatile_accum) {
            accumulator -= (accum_t)5.0lk;
        }
        
        /* Mixed-type comparison */
        if ((int)accumulator > int_counter) {
            int_counter++;
        }
        
        /* Saturated arithmetic */
        sat_f = sat_f + (sat_fract_t)0.2r;
        sat_a = sat_a - (sat_accum_t)1.0lk;
        
        /* Ternary operator with range implications */
        fract_t f_conditional = (accumulator > 0) ? f1 : (fract_t)0.25r;
        f1 = f_conditional * (fract_t)0.95r;
        
        /* Periodically process bit-field */
        if (i % 7 == 0) {
            bf = process_fixed_bitfield(bf, (i % 3) + 1);
        }
        
        /* Overflow-inducing multiplication */
        if (i % 11 == 0) {
            accum_t test_overflow = accumulator * (accum_t)2.0lk;
            /* Explicit check that may trigger range logic */
            if (test_overflow > (accum_t)500.0lk || 
                test_overflow < (accum_t)(-500.0lk)) {
                accumulator = accumulator / (accum_t)2.0lk;
            }
        }
    }
    
    /* Final range comparisons */
    accum_t final_range_check;
    if (accumulator > accum_max) {
        final_range_check = accum_max;
    } else if (accumulator < accum_min) {
        final_range_check = accum_min;
    } else {
        final_range_check = accumulator;
    }
    
    /* Compute checksum */
    unsigned long checksum = 0;
    checksum += (unsigned long)(accumulator * 1000);
    checksum += (unsigned long)(sf1 * 1000);
    checksum += (unsigned long)(sf2 * 1000);
    checksum += (unsigned long)(f1 * 1000);
    checksum += (unsigned long)(sat_f * 1000);
    checksum += (unsigned long)(sat_a / 10);
    checksum += bf.integer;
    checksum += bf.fraction;
    checksum += int_counter;
    
    printf("Checksum: %lu\n", checksum);
    printf("Final accumulator: %Lf\n", (long double)accumulator);
    printf("Range: [%Lf, %Lf]\n", (long double)accum_min, (long double)accum_max);
    
    return 0;
}
