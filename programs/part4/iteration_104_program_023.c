/* Fixed-point range analysis test targeting GCC's fixed-value.cc uncovered lines */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point types with different precisions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef _Sat short _Fract ssfract_t;
typedef _Sat _Accum saccum_t;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf1 : 8;
    signed int sbf1 : 8;
    unsigned int ubf2 : 12;
    unsigned int : 4;  /* padding */
};

/* Function using bit-fields and shifts with fixed-point */
static accum_t bitfield_fixed_ops(struct mixed_bf *bf, accum_t base) {
    volatile accum_t result = base;
    
    /* Shift operations that may trigger alshift logic */
    unsigned int shift1 = bf->ubf1 >> 2;
    unsigned int shift2 = bf->ubf2 << 1;
    
    /* Convert to fixed-point with different scaling */
    sfract_t f1 = shift1 * 0.1r;
    accum_t a1 = shift2 * 0.01k;
    
    /* Mixed precision operations */
    result += (accum_t)f1 * 0.5k;
    result += a1;
    
    /* Explicit range comparison */
    if (result > 0.8k) {
        result = 0.8k;
    } else if (result < -0.8k) {
        result = -0.8k;
    }
    
    /* Ternary operator with range check */
    accum_t temp = (result > 0.5k) ? (result * 0.9k) : (result * 1.1k);
    
    /* Check against computed bounds */
    accum_t upper_bound = 0.75k;
    accum_t lower_bound = -0.75k;
    
    if (temp > upper_bound || temp < lower_bound) {
        temp = (temp > 0) ? upper_bound : lower_bound;
    }
    
    return temp;
}

int main(void) {
    /* Initialize fixed-point variables across range */
    volatile sfract_t sf1 = 0.5r;
    volatile sfract_t sf2 = -0.3r;
    volatile accum_t acc1 = 0.0k;
    volatile accum_t acc2 = 0.0k;
    volatile laccum_t lacc = 0.0lk;
    
    /* Saturating fixed-point */
    volatile ssfract_t ssf1 = 0.7r;
    volatile saccum_t sacc1 = 0.0k;
    
    /* Initialize bit-field structure */
    struct mixed_bf bf = {32, -16, 2048};
    
    /* Loop-based range widening */
    for (int i = 0; i < 100; i++) {
        /* Multi-step computations */
        acc1 += sf1 * 0.01k;
        acc2 += sf2 * 0.02k;
        
        /* Multiplication that may overflow */
        lacc = acc1 * acc2 * 0.5lk;
        
        /* Saturating operations */
        ssf1 = ssf1 * 0.99r;
        sacc1 += ssf1 * 0.1k;
        
        /* Explicit range comparisons - may trigger sgt/ugt */
        accum_t max_bound = 0.9k - (i * 0.001k);
        accum_t min_bound = -0.9k + (i * 0.001k);
        
        /* Conditional with range checks */
        if (acc1 > max_bound) {
            acc1 = max_bound;
        } else if (acc1 < min_bound) {
            acc1 = min_bound;
        }
        
        /* Nested comparisons */
        if (acc2 > 0.5k || (acc2 == 0.5k && lacc > 0.0lk)) {
            acc2 *= 0.95k;
        }
        
        /* Mixed integer/fixed-point context */
        unsigned int ui = i * 37;
        fract_t ftemp = (ui & 0xFF) * 0.0039r;  /* 1/256 */
        
        /* Cast and compare */
        if ((int)(ftemp * 100) > 50) {
            acc1 -= 0.01k;
        }
        
        /* Call bitfield function periodically */
        if (i % 10 == 0) {
            acc1 = bitfield_fixed_ops(&bf, acc1);
        }
        
        /* Shift operations in mixed context */
        unsigned int shift_val = (ui >> 3) & 0x1F;
        accum_t shifted = (accum_t)shift_val * 0.01k;
        
        /* Compare shifted value */
        if (shifted > 0.3k) {
            acc2 += 0.005k;
        }
    }
    
    /* Additional overflow-triggering operations */
    accum_t large_acc = 0.999k;
    for (int i = 0; i < 10; i++) {
        large_acc *= large_acc;  /* May approach saturation */
        
        /* Direct comparison that may trigger the uncovered logic */
        accum_t limit = 0.5k;
        if (large_acc > limit && large_acc < 0.9k) {
            large_acc = 0.5k * large_acc + 0.25k;
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    checksum += (int)(acc1 * 1000);
    checksum += (int)(acc2 * 1000);
    checksum += (int)(lacc * 1000);
    checksum += (int)(sacc1 * 1000);
    checksum += (int)(large_acc * 1000);
    
    /* Print to prevent optimization */
    printf("Checksum: %d\n", checksum);
    printf("acc1: %.5k, acc2: %.5k, lacc: %.5lk\n", 
           (accum_t)acc1, (accum_t)acc2, (laccum_t)lacc);
    
    return 0;
}
