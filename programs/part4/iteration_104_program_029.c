/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    unsigned int fraction : 8;
    sat_fract_t sat_val;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bits(sfract_t input) {
    struct fixed_bitfield result;
    volatile sfract_t volatile_input = input;
    
    /* Force range analysis with shifts */
    unsigned int shifted = ((unsigned int)volatile_input) << 4;
    result.sign = (shifted >> 15) & 1;
    result.integer = (shifted >> 8) & 0x7F;
    result.fraction = shifted & 0xFF;
    
    /* Fixed-point operation that may saturate */
    result.sat_val = volatile_input * volatile_input;
    
    return result;
}

/* Main function with loops and range comparisons */
int main(void) {
    /* Initialize fixed-point variables across range */
    sfract_t sf1 = 0.25r;
    sfract_t sf2 = -0.5r;
    accum_t acc1 = 1000.0k;
    accum_t acc2 = -500.0k;
    ufract_t uf1 = 0.75ur;
    volatile fract_t vbound = 0.8r;
    
    /* Loop accumulator */
    accum_t loop_accum = 0.0k;
    sat_fract_t sat_accum = 0.0r;
    
    /* Mixed integer/fixed-point bounds */
    int int_bound = 100;
    unsigned long ul_bound = 1000UL;
    
    printf("Starting fixed-point range test...\n");
    
    /* Loop forcing range widening */
    for (int i = 0; i < 100; i++) {
        volatile int volatile_i = i;
        
        /* Multi-step computation with overflow potential */
        acc1 = acc1 + (accum_t)(sf1 * 2.0r) * (accum_t)volatile_i;
        acc2 = acc2 - (accum_t)(sf2 * 1.5r) * (accum_t)(volatile_i / 2);
        
        /* Range comparison triggering sgt/ugt logic */
        if (acc1 > (accum_t)int_bound) {
            /* Force range calculation with explicit comparison */
            accum_t temp = acc1 * 0.9k;
            if (temp > acc2 && temp < (accum_t)ul_bound) {
                loop_accum += temp;
            }
        } else if (acc2 < -(accum_t)int_bound) {
            loop_accum -= (accum_t)(-acc2 * 0.8k);
        }
        
        /* Saturated fixed-point operations */
        sat_fract_t sat_temp = (sat_fract_t)(uf1 * (fract_t)volatile_i / 50.0r);
        sat_accum += sat_temp;
        
        /* Ternary operator with mixed types */
        fract_t f_cond = (volatile_i % 3 == 0) ? 
                         (fract_t)acc1 / 1000.0r : 
                         (fract_t)acc2 / 1000.0r;
        
        /* Compare against volatile bound */
        if (f_cond > vbound || f_cond < -vbound) {
            sf1 = sf1 * 0.9r;
            sf2 = sf2 * 0.9r;
        }
        
        /* Bit-field and shift operations */
        if (volatile_i % 10 == 0) {
            struct fixed_bitfield bf = process_fixed_bits(sf1);
            /* Use bit-field values in fixed-point context */
            uf1 = (ufract_t)bf.fraction / 256.0ur;
        }
    }
    
    /* Additional explicit range checks */
    accum_t final_check = loop_accum + (accum_t)sat_accum;
    
    /* Complex conditional with multiple comparisons */
    if ((final_check > 10000.0k && final_check < 20000.0k) ||
        (final_check < -5000.0k && final_check > -10000.0k)) {
        printf("Range check passed: %ld\n", (long)(final_check / 1k));
    } else {
        /* Force alternative path with different operations */
        accum_t adjusted = final_check * 0.5k;
        if (adjusted > (accum_t)0 && adjusted < (accum_t)ul_bound) {
            printf("Adjusted range: %ld\n", (long)(adjusted / 1k));
        }
    }
    
    /* Compute checksum */
    long checksum = (long)(loop_accum / 1k) + 
                    (long)(sat_accum * 1000r) + 
                    (long)(acc1 / 1k) + 
                    (long)(acc2 / 1k);
    
    printf("Checksum: %ld\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
