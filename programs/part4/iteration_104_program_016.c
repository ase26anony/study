/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point types with different precisions */
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
    sfract_t fval;
    saccum_t aval;
};

/* Function using bit-fields and shifts */
static void process_bitfield(struct fixed_bitfield *bf, int shift) {
    volatile unsigned int mask = (1u << shift) - 1;
    
    /* Operations that may trigger range calculations */
    bf->fraction = (bf->fraction << shift) | (bf->integer >> (8 - shift));
    bf->integer = (bf->integer << shift) & 0x7F;
    
    /* Fixed-point operations with shifted values */
    sfract_t temp = bf->fval;
    for (int i = 0; i < shift; i++) {
        /* Force range widening through repeated operations */
        temp = temp * (sfract_t)0.5hr;
        if (temp > (sfract_t)0.25hr) {
            bf->fval = temp + (sfract_t)0.125hr;
        } else {
            bf->fval = temp - (sfract_t)0.125hr;
        }
    }
    
    /* Mixed integer/fixed-point comparison */
    unsigned int frac_part = bf->fraction;
    if ((accum_t)frac_part > bf->aval * (accum_t)0.9k) {
        bf->aval = bf->aval + (saccum_t)0.1k;
    }
}

int main(void) {
    /* Initialize with values spanning positive, negative, and saturated ranges */
    volatile sfract_t sf1 = 0.5hr;
    volatile sfract_t sf2 = -0.75hr;
    volatile accum_t acc1 = 100.5k;
    volatile accum_t acc2 = -50.25k;
    volatile ssfract_t ssf1 = 0.9hr;
    volatile ssaccum_t ssa1 = 200.0k;
    
    /* Saturating fixed-point values */
    sfract_sat_t sat_f1 = 1.0hr;  /* Will saturate */
    sfract_sat_t sat_f2 = -1.0hr; /* Will saturate */
    
    struct fixed_bitfield bf = {
        .sign = 0,
        .integer = 42,
        .fraction = 128,
        .fval = 0.3hr,
        .aval = 50.0k
    };
    
    /* Loop-based range widening */
    accum_t accumulator = 0.0k;
    sfract_t f_accumulator = 0.0hr;
    
    /* Non-constant bounds to prevent constant folding */
    volatile int bound1 = 100;
    volatile int bound2 = -100;
    
    for (int i = 0; i < 100; i++) {
        /* Multi-step computations forcing range evaluation */
        accum_t temp_acc = acc1 * (accum_t)i / (accum_t)bound1;
        
        /* Explicit range comparisons - may trigger sgt/ugt operations */
        if (temp_acc > (accum_t)bound2 && temp_acc < (accum_t)bound1) {
            accumulator += temp_acc;
            
            /* Nested conditional with mixed types */
            if ((sfract_t)(i % 10) * 0.1hr > f_accumulator) {
                f_accumulator = f_accumulator + (sfract_t)0.05hr;
            } else {
                f_accumulator = f_accumulator - (sfract_t)0.03hr;
            }
        }
        
        /* Saturating arithmetic operations */
        sat_f1 = sat_f1 + (sfract_sat_t)0.2hr;
        sat_f2 = sat_f2 - (sfract_sat_t)0.15hr;
        
        /* Overflow-prone multiplication */
        ssa1 = ssa1 * (ssaccum_t)1.1k;
        
        /* Ternary operator with fixed-point comparison */
        sf1 = (sf1 > sf2) ? sf1 * (sfract_t)0.9hr : sf2 * (sfract_t)1.1hr;
        
        /* Periodic bitfield processing */
        if (i % 7 == 0) {
            process_bitfield(&bf, (i % 4) + 1);
        }
        
        /* Complex conditional with multiple comparisons */
        if (acc1 > accumulator || 
            (acc1 == accumulator && acc2.ugt(accumulator))) {
            acc1 = acc1 - (accum_t)0.5k;
        }
        
        /* Force range analysis across iterations */
        if (i > 50) {
            acc2 = acc2 + (accum_t)(i * 0.01k);
        }
    }
    
    /* Additional fixed-point operations outside loop */
    laccum_t large_acc = (laccum_t)accumulator * 2.0lk;
    lfract_t large_frac = (lfract_t)f_accumulator / 2.0lr;
    
    /* Mixed precision comparisons */
    if (large_acc > (laccum_t)acc1 && large_frac < (lfract_t)sf1) {
        large_acc = large_acc - (laccum_t)10.0lk;
    }
    
    /* Compute checksum */
    unsigned int checksum = 0;
    checksum += (unsigned int)(accumulator * 1000k);
    checksum += (unsigned int)(f_accumulator * 1000hr);
    checksum += (unsigned int)(large_acc * 100lk);
    checksum += (unsigned int)(large_frac * 1000lr);
    checksum += bf.integer * 100 + bf.fraction;
    
    printf("Checksum: %u\n", checksum);
    printf("Final values: acc=%.3k, f_acc=%.3hr\n", accumulator, f_accumulator);
    
    return 0;
}
