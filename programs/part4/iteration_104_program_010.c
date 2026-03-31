/* fixed-point-coverage.c
 * Designed to trigger GCC's fixed-point range analysis logic
 * Compile with: gcc -O2 -ffixed-point -fstrict-overflow -o fixed_test fixed-point-coverage.c
 */

#include <stdio.h>
#include <stdint.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef _Sat short _Fract ssfract_t;
typedef _Sat _Fract sfract_sat_t;
typedef _Sat short _Accum ssaccum_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    unsigned int fraction : 8;
    sfract_t f_val;
    saccum_t a_val;
};

/* Function using bit-fields and shifts */
static void process_fixed_bitfield(struct fixed_bitfield *bf, int iterations) {
    volatile sfract_t v_bound = 0.5r;  /* Prevent constant folding */
    saccum_t accumulator = 0.0k;
    
    for (int i = 0; i < iterations; i++) {
        /* Shift operations that may trigger alshift logic */
        unsigned int shifted = bf->fraction << (i % 4);
        shifted = shifted >> 1;
        
        /* Convert to fixed-point */
        sfract_t f_from_shift = (sfract_t)shifted / 256.0r;
        
        /* Range comparison - may trigger sgt/ugt logic */
        if (f_from_shift > v_bound) {
            /* This comparison might invoke range checking */
            bf->f_val = f_from_shift * 0.75r;
        } else if (f_from_shift < -v_bound) {
            bf->f_val = f_from_shift * 1.25r;
        } else {
            /* Mixed precision operation */
            bf->f_val = (sfract_t)((int)f_from_shift + i) * 0.5r;
        }
        
        /* Accumulator with widening range */
        accumulator += bf->a_val * (saccum_t)f_from_shift;
        
        /* Explicit range check against computed bounds */
        saccum_t upper_bound = (saccum_t)(bf->integer * 2) / 256.0k;
        saccum_t lower_bound = -upper_bound * 0.5k;
        
        /* Conditional that may trigger the uncovered comparison logic */
        if (accumulator > upper_bound) {
            accumulator = upper_bound;
        } else if (accumulator < lower_bound) {
            accumulator = lower_bound;
        }
        
        /* Update bit-field values */
        bf->a_val = accumulator;
        bf->fraction = (unsigned int)(accumulator * 256.0k) & 0xFF;
    }
}

/* Main function with comprehensive fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables across different ranges */
    sfract_t f1 = 0.25r;
    sfract_t f2 = -0.75r;
    saccum_t a1 = 10.5k;
    saccum_t a2 = -5.25k;
    laccum_t la1 = 100.123456789lk;
    
    /* Saturating types */
    ssfract_t sf1 = 0.8r;
    ssaccum_t sa1 = 50.0k;
    
    /* Volatile variables to prevent constant folding */
    volatile sfract_t v_f1 = 0.3r;
    volatile saccum_t v_a1 = 20.0k;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {
        .sign = 0,
        .integer = 5,
        .fraction = 128,  /* 0.5 in 8-bit fraction */
        .f_val = 0.5r,
        .a_val = 2.5k
    };
    
    /* Loop with range-widening operations */
    saccum_t accumulator = 0.0k;
    sfract_t f_accum = 0.0r;
    
    for (int i = 0; i < 100; i++) {
        /* Mixed-type arithmetic forcing range calculations */
        saccum_t temp = a1 * (saccum_t)f1;
        
        /* Operation that may overflow */
        temp += a2 * (saccum_t)f2;
        
        /* Conditional with explicit comparison */
        if (temp > (saccum_t)(i * 2)) {
            /* This comparison might trigger the uncovered sgt/ugt logic */
            accumulator += temp * 0.9k;
        } else if (temp < (saccum_t)(-i)) {
            accumulator += temp * 1.1k;
        } else {
            /* Mixed integer/fixed-point operation */
            accumulator += (saccum_t)((int)temp % 16) / 16.0k;
        }
        
        /* Fixed-point multiplication with potential range expansion */
        f_accum += f1 * f2;
        
        /* Ternary operator with range-dependent branches */
        f1 = (f_accum > 0.5r) ? f_accum * 0.8r : 
             (f_accum < -0.5r) ? f_accum * 1.2r : 0.0r;
        
        /* Update with volatile to prevent optimization */
        a1 += v_a1 * 0.1k;
        
        /* Explicit cast and shift simulation */
        unsigned int as_int = (unsigned int)(accumulator * 256.0k);
        as_int = (as_int << 2) | (as_int >> 6);  /* Rotate */
        accumulator = (saccum_t)as_int / 256.0k;
        
        /* Call bit-field processing occasionally */
        if (i % 20 == 0) {
            process_fixed_bitfield(&bf, 5);
            accumulator += bf.a_val;
        }
        
        /* Saturating arithmetic */
        sf1 = sf1 * 1.1r;  /* May saturate */
        sa1 = sa1 + 5.0k;  /* May saturate */
    }
    
    /* Additional range-triggering operations */
    laccum_t wide_range = la1 * 1000.0lk;
    
    /* Explicit range comparison that may hit the uncovered code */
    if (wide_range > 50000.0lk || wide_range < -50000.0lk) {
        wide_range = wide_range / 10.0lk;
    }
    
    /* Mixed precision conditional */
    accum_t mixed_comp = (accumulator > (accum_t)f_accum * 10.0k) ? 
                         accumulator : (accum_t)f_accum * 10.0k;
    
    /* Compute checksum to verify execution */
    uint32_t checksum = 0;
    checksum += (uint32_t)(accumulator * 1000.0k);
    checksum += (uint32_t)(f_accum * 1000.0r);
    checksum += (uint32_t)(wide_range / 100.0lk);
    checksum += (uint32_t)(mixed_comp * 100.0k);
    checksum += bf.fraction;
    checksum += (uint32_t)(sf1 * 1000.0r);
    checksum += (uint32_t)(sa1 * 100.0k);
    
    printf("Checksum: %u\n", checksum);
    printf("Final values: acc=%.3k, f_acc=%.3r, wide=%.6lk\n", 
           accumulator, f_accum, wide_range);
    
    return 0;
}
