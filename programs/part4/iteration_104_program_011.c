/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    unsigned int fraction : 8;
    volatile sfract_t f_val;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf) {
    /* Shift operations that may trigger alshift logic */
    unsigned int shifted = (bf.integer << 3) | (bf.fraction >> 5);
    
    /* Convert to fixed-point with explicit range */
    sfract_t result = (sfract_t)shifted / 256.0hr;
    
    /* Conditional based on shifted value */
    if (shifted > 0x7F) {
        bf.f_val = 0.5hr;
    } else if (shifted < 0x10) {
        bf.f_val = -0.5hr;
    } else {
        bf.f_val = result;
    }
    
    return bf;
}

/* Main test function */
int main(void) {
    /* Initialize fixed-point variables across range */
    sfract_t sf1 = 0.25hr;
    sfract_t sf2 = -0.75hr;
    accum_t acc1 = 100.0k;
    accum_t acc2 = -50.0k;
    sat_fract_t sat_f = 0.9hr;
    sat_accum_t sat_acc = 300.0rk;
    
    /* Volatile bounds to prevent constant folding */
    volatile accum_t volatile_bound = 200.0k;
    volatile sfract_t volatile_fract = 0.0hr;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {0, 64, 128, 0.0hr};
    
    /* Loop-based range widening */
    accum_t accumulator = 0.0k;
    sfract_t fract_accum = 0.0hr;
    
    /* Mixed integer/fixed-point context */
    unsigned long iteration_count = 100;
    int int_counter = 0;
    
    for (unsigned long i = 0; i < iteration_count; i++) {
        /* Multi-step computations forcing range evaluation */
        accumulator += acc1;
        fract_accum += sf1;
        
        /* Explicit range comparisons - may trigger sgt/ugt */
        if (accumulator > volatile_bound) {
            /* This comparison should invoke range checking */
            accumulator = volatile_bound;
            int_counter++;
        }
        
        /* Another comparison with different bounds */
        if (accumulator.sgt (1000.0k) || (accumulator == 1000.0k && 
            fract_accum.ugt (0.5hr))) {
            /* Force evaluation of max/min bounds */
            accumulator -= acc2;
        }
        
        /* Mixed precision operations */
        int int_val = (int)(accumulator / 10.0k);
        sfract_t scaled = (sfract_t)int_val / 100.0hr;
        
        /* Conditional with ternary operator */
        fract_accum = (scaled > 0.2hr) ? scaled : -scaled;
        
        /* Saturation operations */
        sat_f += 0.1hr;
        sat_acc += 50.0rk;
        
        /* Multiplication forcing overflow checking */
        if (i % 10 == 0) {
            accumulator *= 1.1k;
            fract_accum *= 1.5hr;
        }
        
        /* Process bit-field every 20 iterations */
        if (i % 20 == 0) {
            bf.integer = (unsigned int)(accumulator / 10.0k) & 0x7F;
            bf.fraction = (unsigned int)(fract_accum * 256.0hr) & 0xFF;
            bf = process_fixed_bitfield(bf);
            volatile_fract = bf.f_val;
        }
        
        /* Complex conditional with multiple comparisons */
        if ((accumulator > 500.0k && fract_accum < -0.3hr) ||
            (accumulator < -300.0k && fract_accum > 0.7hr)) {
            /* Reset accumulators when crossing complex bounds */
            accumulator = 0.0k;
            fract_accum = 0.0hr;
        }
    }
    
    /* Additional explicit comparisons outside loop */
    laccum_t large_accum = accumulator * 2.0lk;
    if (large_accum.sgt (5000.0lk)) {
        large_accum = 5000.0lk;
    }
    
    /* Compute checksum */
    int checksum = 0;
    checksum += (int)(accumulator * 10.0k);
    checksum += (int)(fract_accum * 1000.0hr);
    checksum += (int)(large_accum / 10.0lk);
    checksum += (int)(sat_f * 100.0hr);
    checksum += (int)(sat_acc / 10.0rk);
    checksum += int_counter;
    
    /* Print result to verify execution */
    printf("Checksum: %d\n", checksum);
    printf("Final accumulator: %ld\n", (long)(accumulator * 1000.0k));
    printf("Final fract_accum: %d\n", (int)(fract_accum * 1000.0hr));
    
    return 0;
}
