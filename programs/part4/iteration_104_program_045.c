/* fixed-point-coverage.c
 * Designed to trigger specific range calculation logic in GCC's fixed-value.cc
 * Compile with: gcc -O2 -ffixed-point -fstrict-overflow -o fixed_test fixed-point-coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef _Sat short _Fract sat_sfract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    unsigned int fraction : 8;
    sfract_t f_val;
};

/* Function using bit-fields and shifts */
static void process_bitfield(struct fixed_bitfield *bf, int shift) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int temp = (bf->integer << shift) | (bf->fraction >> (8 - shift));
    
    /* Convert to fixed-point with potential range checks */
    sfract_t f = (sfract_t)temp / 256.0hr;
    
    /* Conditional with comparison */
    if (f > 0.5hr) {
        bf->f_val = f * 0.75hr;
    } else {
        bf->f_val = f + 0.25hr;
    }
    
    /* Explicit range comparison */
    sfract_t max_val = 0.9hr;
    sfract_t min_val = -0.9hr;
    
    if (bf->f_val > max_val || bf->f_val < min_val) {
        /* Force saturation logic */
        sat_sfract_t sat_val = (_Sat sfract_t)bf->f_val;
        bf->f_val = (sfract_t)sat_val;
    }
}

/* Main function with loops and range calculations */
int main(void) {
    /* Initialize fixed-point variables */
    sfract_t f1 = 0.25hr;
    fract_t f2 = -0.5r;
    accum_t a1 = 100.0lk;
    accum_t a2 = -50.0lk;
    sat_sfract_t sat_f = 0.0hr;
    usfract_t uf = 0.75uhr;
    
    /* Volatile variables to prevent constant folding */
    volatile fract_t v_bound = 0.0r;
    volatile int v_iter = 10;
    
    /* Loop-based range widening */
    accum_t accumulator = 0.0lk;
    fract_t f_accum = 0.0r;
    
    /* Mixed integer/fixed-point context */
    int int_counter = 0;
    unsigned long ul_counter = 0;
    
    /* Main computation loop */
    for (int i = 0; i < v_iter; i++) {
        /* Force range calculations through mixed operations */
        accum_t temp = a1 * (accum_t)i / 10.0lk;
        
        /* Conditional with explicit comparison */
        if (temp > 50.0lk) {
            accumulator += temp - 25.0lk;
        } else if (temp < -50.0lk) {
            accumulator += temp + 25.0lk;
        } else {
            /* Multi-step computation */
            accumulator = accumulator * 1.1lk + temp;
        }
        
        /* Fixed-point with saturation */
        sat_f = (_Sat sfract_t)(sat_f + 0.1hr);
        
        /* Mixed precision operations */
        f_accum = f_accum + (fract_t)((int)i % 4) * 0.125r;
        
        /* Explicit range check that may trigger sgt/ugt logic */
        accum_t max_bound = 1000.0lk;
        accum_t min_bound = -1000.0lk;
        
        if (accumulator > max_bound) {
            accumulator = max_bound;
        } else if (accumulator < min_bound) {
            accumulator = min_bound;
        }
        
        /* Compare with volatile to prevent optimization */
        if (accumulator > (accum_t)v_bound) {
            int_counter++;
        } else {
            ul_counter++;
        }
        
        /* Shift operations that may trigger alshift logic */
        unsigned int shift_val = (i % 8);
        usfract_t shifted = (usfract_t)((unsigned int)uf << shift_val);
        uf = shifted / 256.0uhr;
    }
    
    /* Bit-field operations */
    struct fixed_bitfield bf = {0, 64, 128, 0.0hr};
    for (int i = 0; i < 4; i++) {
        process_bitfield(&bf, i);
        
        /* Range comparison in mixed context */
        int int_val = (int)(bf.f_val * 100);
        if (int_val > 50 || int_val < -50) {
            bf.f_val = (sfract_t)(int_val % 50) / 100.0hr;
        }
    }
    
    /* Additional range boundary tests */
    accum_t test_vals[] = {
        0.0lk, 127.999lk, -128.0lk, 255.0lk, -256.0lk
    };
    
    for (int i = 0; i < 5; i++) {
        accum_t val = test_vals[i];
        accum_t computed = val * 2.0lk - val / 2.0lk;
        
        /* Complex conditional that may trigger the uncovered logic */
        if (computed > 100.0lk && val > 0.0lk) {
            accumulator += computed;
        } else if (computed < -100.0lk && val < 0.0lk) {
            accumulator -= computed;
        }
    }
    
    /* Ternary operations with range implications */
    fract_t result1 = (accumulator > 500.0lk) ? 0.8r : 0.2r;
    fract_t result2 = (accumulator < -500.0lk) ? -0.8r : 
                     (accumulator > 0.0lk) ? result1 : -result1;
    
    /* Final checksum calculation */
    int checksum = (int)(accumulator * 10.0lk) +
                   (int)(f_accum * 100.0r) +
                   (int)(sat_f * 1000.0hr) +
                   (int)(bf.f_val * 100.0hr) +
                   int_counter - (int)ul_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Accumulator: %ld.%03ld\n", 
           (long)(accumulator), 
           (long)((accumulator - (accum_t)(long)accumulator) * 1000lk));
    printf("Fixed accum: %d.%03d\n", 
           (int)(f_accum), 
           (int)((f_accum - (fract_t)(int)f_accum) * 1000r));
    
    return 0;
}
