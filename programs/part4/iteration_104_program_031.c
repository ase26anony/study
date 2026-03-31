/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    ufract_t fraction;  /* unsigned fixed-point */
    volatile accum_t accum;  /* volatile to prevent constant folding */
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed(struct fixed_bitfield bf) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int temp = bf.integer;
    
    /* Shift operations that require range analysis */
    temp = temp << (bf.sign ? 2 : 3);
    
    /* Convert back to fixed-point with potential overflow */
    bf.fraction = (ufract_t)(temp & 0xFF) / 256.0r;
    
    /* Mixed integer/fixed-point comparison */
    if ((int)bf.accum > (temp >> 1)) {
        bf.accum = bf.accum * 0.5k;
    }
    
    return bf;
}

int main(void) {
    /* Initialize fixed-point variables across different ranges */
    sfract_t sf1 = 0.5hr;
    sfract_t sf2 = -0.75hr;
    fract_t f1 = 0.125r;
    fract_t f2 = 0.875r;
    accum_t acc1 = 100.0k;
    accum_t acc2 = -50.0k;
    sat_fract_t sat1 = 0.9r;
    
    /* Volatile variables to prevent constant folding */
    volatile fract_t v_bound = 0.5r;
    volatile accum_t v_acc_bound = 1000.0k;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {
        .sign = 0,
        .integer = 64,
        .fraction = 0.25ur,
        .accum = 500.0k
    };
    
    /* Loop-based range widening */
    int checksum = 0;
    
    for (int i = 0; i < 100; i++) {
        /* Multi-step computations forcing range evaluation */
        
        /* 1. Addition with potential overflow */
        acc1 = acc1 + (accum_t)(i * 0.1k);
        
        /* 2. Multiplication with range checking */
        f1 = f1 * f2;
        
        /* 3. Explicit range comparisons (may trigger sgt/ugt) */
        if (acc1 > v_acc_bound) {
            /* This comparison should invoke range analysis */
            acc1 = acc1 * 0.9k;
        }
        
        /* 4. Ternary operator with mixed types */
        fract_t temp = (i % 2) ? f1 : f2;
        
        /* 5. Comparison against computed bounds */
        accum_t bound = (accum_t)(i * 10.0k);
        if (acc2 < bound && bound > 0.0k) {
            acc2 = acc2 + 5.0k;
        }
        
        /* 6. Saturated arithmetic */
        sat1 = sat1 + 0.2r;
        
        /* 7. Bit-field operations every 10 iterations */
        if (i % 10 == 0) {
            bf.integer = (bf.integer + 1) & 0x7F;
            bf = process_fixed(bf);
            
            /* Mixed comparison triggering conversions */
            if ((int)bf.accum > (int)(acc1 / 10.0k)) {
                bf.accum = bf.accum - 10.0k;
            }
        }
        
        /* 8. Complex conditional with multiple comparisons */
        if ((acc1 > 0.0k && f1 > v_bound) || 
            (acc2 < 0.0k && sf1 < -0.5hr)) {
            sf2 = sf2 * 0.8hr;
        }
        
        /* 9. Shift-like behavior using multiplication */
        if (i > 50) {
            /* Simulate shift through multiplication */
            f2 = f2 * 0.5r;  /* Right shift equivalent */
        }
        
        /* Accumulate checksum */
        checksum += (int)(acc1 * 0.01k) + (int)(f1 * 100.0r);
    }
    
    /* Additional range boundary tests */
    
    /* Test near maximum bounds */
    accum_t max_test = 9223372036854775.807k;  /* Near LONG_ACCUM_MAX */
    if (max_test > 9000000000000000.0k) {
        max_test = max_test * 0.99k;
    }
    
    /* Test near minimum bounds */
    accum_t min_test = -9223372036854775.807k; /* Near LONG_ACCUM_MIN */
    if (min_test < -9000000000000000.0k) {
        min_test = min_test / 2.0k;
    }
    
    /* Mixed-type comparisons in final computation */
    long int_result = (long)acc1 + (long)acc2 + (int)(f1 * 1000.0r);
    
    /* Final checksum computation with overflow check */
    if (int_result > 1000000L) {
        checksum += (int_result >> 3);
    } else {
        checksum += int_result;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: acc1=%.3Lfk, acc2=%.3Lfk, f1=%.3r\n", 
           (long double)acc1, (long double)acc2, (double)f1);
    
    return 0;
}
