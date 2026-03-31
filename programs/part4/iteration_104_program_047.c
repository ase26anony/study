/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef _Sat short _Fract ssfract_t;
typedef _Sat _Fract sfract_sat_t;
typedef _Sat long _Fract slfract_t;

/* Bit-field structure with fixed-point */
struct mixed_data {
    unsigned int flag : 3;
    sfract_t value : 16;  /* Fixed-point in bit-field */
    unsigned int count : 8;
};

/* Function using bit-fields and shifts */
static accum_t process_bitfield(struct mixed_data *data, int shift) {
    volatile accum_t result = 0.5k;  /* Prevent constant folding */
    
    /* Shift operations that may trigger alshift logic */
    unsigned int temp = data->count << shift;
    
    /* Convert to fixed-point with potential range checks */
    accum_t converted = (accum_t)temp / 256.0k;
    
    /* Mix with bit-field fixed-point value */
    result = result + (accum_t)data->value * converted;
    
    /* Explicit range comparison */
    if (result > 1.0k) {
        result = 1.0k;
    } else if (result < -1.0k) {
        result = -1.0k;
    }
    
    return result;
}

/* Loop-based range widening */
static accum_t accumulate_range(int iterations) {
    volatile accum_t acc = 0.0k;  /* Non-constant bounds */
    fract_t step = 0.1r;
    saccum_t small_acc = 0.0hk;
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed precision operations */
        acc = acc + (accum_t)step * 2.0k;
        small_acc = small_acc + 0.05hk;
        
        /* Conditional with explicit range check */
        if (acc > (accum_t)small_acc * 10.0k) {
            /* This comparison may trigger sgt/ugt logic */
            acc = acc - 0.5k;
        }
        
        /* Multiplication that can overflow */
        fract_t temp = step * step;
        if (temp > 0.5r) {
            step = 0.01r;
        }
        
        /* Ternary operator with range comparison */
        acc = (acc > 5.0k) ? 5.0k : acc;
        acc = (acc < -5.0k) ? -5.0k : acc;
    }
    
    return acc;
}

/* Function with saturating arithmetic */
static ssfract_t saturating_operations(void) {
    ssfract_t a = 0.7hr;
    ssfract_t b = 0.6hr;
    ssfract_t result;
    
    /* Operations that may saturate */
    result = a + b;  /* May saturate near 1.0 */
    result = result * a;
    result = result - b;
    
    /* Explicit comparison against bounds */
    if (result > 0.9hr) {
        result = 0.9hr;
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize fixed-point variables */
    fract_t f1 = 0.5r;
    fract_t f2 = -0.3r;
    accum_t a1 = 2.5k;
    accum_t a2 = -1.8k;
    lfract_t lf1 = 0.999999lr;
    
    /* Bit-field test */
    struct mixed_data data = {2, 0.5hr, 32};
    accum_t bitfield_result = process_bitfield(&data, 2);
    
    /* Convert to integer for checksum */
    checksum += (int)(bitfield_result * 1000.0k);
    
    /* Loop with range widening */
    for (int outer = 0; outer < 3; outer++) {
        accum_t loop_result = accumulate_range(10 + outer);
        checksum += (int)(loop_result * 100.0k);
        
        /* Mixed integer/fixed-point operations */
        unsigned int int_val = 100 + outer * 50;
        accum_t mixed = a1 * (accum_t)int_val / 1000.0k;
        
        /* Explicit range comparison with non-constant */
        volatile accum_t volatile_bound = 1.5k + outer * 0.1k;
        if (mixed > volatile_bound) {
            mixed = volatile_bound;
        }
        
        checksum += (int)(mixed * 1000.0k);
        
        /* Saturating operations */
        ssfract_t sat_result = saturating_operations();
        checksum += (int)(sat_result * 1000);
    }
    
    /* Complex expression with multiple operations */
    accum_t complex_expr = a1 * f1 + a2 * f2;
    complex_expr = complex_expr / 2.0k;
    
    /* Range check that may trigger the uncovered logic */
    if (complex_expr > 10.0k || complex_expr < -10.0k) {
        complex_expr = 0.0k;
    }
    
    checksum += (int)(complex_expr * 1000.0k);
    
    /* Test with long fixed-point */
    laccum_t long_acc = 100000.0lk;
    for (int i = 0; i < 5; i++) {
        long_acc = long_acc * 0.9lk;
        
        /* Comparison that may need extended precision range check */
        if (long_acc < 50000.0lk) {
            long_acc = 50000.0lk;
        }
    }
    checksum += (int)(long_acc / 1000.0lk);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
