/* Test program to exercise fixed-point range analysis logic in GCC */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type declarations */
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
static struct fixed_bitfield process_fixed_bitfield(accum_t input) {
    struct fixed_bitfield result;
    volatile accum_t volatile_input = input;
    
    /* Force range calculations through shifts */
    unsigned int shifted = ((unsigned int)(volatile_input * 256)) >> 4;
    result.sign = (shifted >> 15) & 1;
    result.integer = (shifted >> 8) & 0x7F;
    result.fraction = shifted & 0xFF;
    
    /* Range-dependent assignment */
    result.sat_val = (volatile_input > 0.5k) ? 0.9r : 0.1r;
    
    return result;
}

/* Main function with loop-based range widening */
int main(void) {
    /* Initialize fixed-point variables across range */
    sfract_t f1 = 0.1hr;
    sfract_t f2 = -0.3hr;
    accum_t a1 = 100.0k;
    accum_t a2 = -50.0k;
    sat_accum_t sat_acc = 0.0lk;
    usfract_t uf1 = 0.5uhr;
    
    /* Volatile bounds to prevent constant folding */
    volatile accum_t volatile_bound = 75.0k;
    volatile sfract_t volatile_threshold = 0.7hr;
    
    /* Accumulators for range widening */
    accum_t accum_min = -1000.0k;
    accum_t accum_max = 1000.0k;
    fract_t fract_acc = 0.0r;
    
    int checksum = 0;
    
    /* Loop to force range analysis across iterations */
    for (int i = 0; i < 100; i++) {
        /* Mixed precision operations */
        accum_t temp = a1 + (accum_t)f1 * 2.0k;
        
        /* Range comparisons that may trigger sgt/ugt logic */
        if (temp > accum_max) {
            accum_max = temp;
            sat_acc += 0.1lk;
        } else if (temp < accum_min) {
            accum_min = temp;
            sat_acc -= 0.1lk;
        }
        
        /* Complex conditional with multiple comparisons */
        accum_t cmp_val = (i % 3 == 0) ? a2 : temp;
        if (cmp_val > volatile_bound || 
            (cmp_val == volatile_bound && (accum_t)uf1 > 0.25k)) {
            a1 = a1 * 0.9k;
        } else {
            a1 = a1 * 1.1k;
        }
        
        /* Fixed-point with saturation */
        sat_accum_t sat_temp = sat_acc + (sat_accum_t)(f2 * 10.0k);
        if (sat_temp > 0.8lk && sat_temp < 1.2lk) {
            fract_acc += 0.05r;
        }
        
        /* Shift operations that may trigger alshift logic */
        unsigned int shifted = ((unsigned int)(temp * 65536.0k)) >> (i % 8);
        if (shifted & 0x8000) {
            f1 = -f1;
        }
        
        /* Update variables for next iteration */
        f2 = f2 * 0.99hr;
        a2 = a2 + (i % 10) * 0.5k;
        
        /* Ternary with range-dependent result */
        accum_t range_test = (i % 20 < 10) ? accum_min : accum_max;
        if (range_test > 500.0k || range_test < -500.0k) {
            uf1 = uf1 * 0.8uhr;
        }
    }
    
    /* Process through bit-field function */
    struct fixed_bitfield bf = process_fixed_bitfield(accum_max);
    
    /* Compute checksum from all values */
    checksum += (int)(f1 * 1000);
    checksum += (int)(f2 * 1000);
    checksum += (int)(a1 / 10);
    checksum += (int)(a2 / 10);
    checksum += (int)(sat_acc * 100);
    checksum += (int)(fract_acc * 1000);
    checksum += (int)(uf1 * 1000);
    checksum += bf.sign + bf.integer + bf.fraction;
    
    printf("Checksum: %d\n", checksum);
    printf("Range: [%ld.%03ld, %ld.%03ld]\n", 
           (long)accum_min, (long)((accum_min - (long)accum_min) * 1000),
           (long)accum_max, (long)((accum_max - (long)accum_max) * 1000));
    
    return 0;
}
