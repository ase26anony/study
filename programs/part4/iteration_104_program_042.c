/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point types with various precisions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;
typedef short _Sat _Fract ssfract_t;
typedef _Sat _Fract sfract_sat_t;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf1 : 7;
    unsigned int ubf2 : 9;
    signed int sbf1 : 8;
    signed int sbf2 : 6;
};

/* Function using bit-fields and shifts with fixed-point */
static accum_t process_with_bitfields(struct mixed_bf *bf, accum_t base) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int shift1 = bf->ubf1 << 3;  /* Potential zext */
    signed int shift2 = bf->sbf1 >> 2;    /* Potential sext */
    
    /* Mix with fixed-point */
    accum_t result = base;
    result += (accum_t)shift1 * 0.125k;  /* 1/8 */
    result -= (accum_t)shift2 * 0.25k;   /* 1/4 */
    
    /* Conditional based on shifted values */
    if (shift1 > 40) {
        result = result * 1.5k;
    }
    if (shift2 < -10) {
        result = result * 0.75k;
    }
    
    return result;
}

int main(void) {
    /* Initialize fixed-point variables across range */
    sfract_t sf1 = 0.5hr;
    sfract_t sf2 = -0.25hr;
    fract_t f1 = 0.75r;
    fract_t f2 = -0.5r;
    lfract_t lf1 = 0.875lr;
    lfract_t lf2 = -0.125lr;
    
    saccum_t sa1 = 10.5hk;
    saccum_t sa2 = -5.25hk;
    accum_t a1 = 100.75k;
    accum_t a2 = -50.5k;
    laccum_t la1 = 1000.875lk;
    laccum_t la2 = -500.125lk;
    
    /* Saturated versions */
    ssfract_t ssf1 = 0.9hr;
    sfract_sat_t sfs1 = -0.8r;
    
    /* Volatile bounds to prevent constant folding */
    volatile accum_t volatile_bound = 50.0k;
    volatile sfract_t volatile_sf_bound = 0.7hr;
    
    /* Accumulators for loop */
    accum_t accum_main = 0.0k;
    fract_t accum_frac = 0.0r;
    laccum_t accum_long = 0.0lk;
    
    struct mixed_bf bf_data = {32, 256, -32, 16};
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Mixed arithmetic forcing range calculations */
        accum_main += a1 * (accum_t)i * 0.01k;
        accum_frac = accum_frac + f1 * (fract_t)(i % 10) * 0.1r;
        accum_long = accum_long - la2 * (laccum_t)(i / 3) * 0.05lk;
        
        /* Explicit comparisons triggering sgt/ugt logic */
        if (accum_main > (accum_t)i * 10.0k) {
            accum_main = accum_main * 0.9k;
        }
        
        if (accum_frac < (fract_t)(i * 0.01r)) {
            accum_frac = accum_frac + 0.05r;
        }
        
        /* Comparison with volatile (non-constant) bounds */
        if (accum_main > volatile_bound) {
            accum_main = volatile_bound * 0.8k;
        }
        
        if (accum_frac < volatile_sf_bound) {
            accum_frac = volatile_sf_bound;
        }
        
        /* Ternary operator with mixed types */
        accum_main = (i % 3 == 0) ? accum_main * 1.1k : accum_main * 0.95k;
        
        /* Cast between integer and fixed-point in condition */
        int int_temp = (int)(accum_main * 0.1k);
        if (int_temp > 100) {
            accum_main = 100.0k;
        }
        
        /* Call bit-field function periodically */
        if (i % 7 == 0) {
            accum_main = process_with_bitfields(&bf_data, accum_main);
        }
        
        /* Saturation boundary checks */
        ssf1 = ssf1 + 0.05hr;
        sfs1 = sfs1 - 0.03r;
        
        /* Multiplication that may overflow */
        sa1 = sa1 * 1.2hk;
        sa2 = sa2 * 0.8hk;
        
        /* Check against computed bounds */
        accum_t temp_bound = (accum_t)i * 2.5k;
        if (accum_main > temp_bound && accum_main < temp_bound * 2.0k) {
            accum_main = (accum_main + temp_bound) * 0.5k;
        }
    }
    
    /* Additional fixed-point operations outside loop */
    /* Complex expression forcing range analysis */
    accum_t complex_expr = (a1 * a2) + (accum_main * 3.0k) - (accum_t)(la1 * 0.1lk);
    
    /* More comparisons */
    if (complex_expr > 0.0k && complex_expr < 1000.0k) {
        complex_expr = complex_expr * 0.5k;
    }
    
    /* Mixed-type shift operations */
    unsigned int shift_op = (unsigned int)(complex_expr * 100.0k);
    shift_op = shift_op << 4;  /* May trigger zext logic */
    
    signed int shift_op_signed = (signed int)(complex_expr);
    shift_op_signed = shift_op_signed >> 2;  /* May trigger sext logic */
    
    /* Final checksum calculation */
    long checksum = 0;
    checksum += (long)(accum_main * 1000.0k);
    checksum += (long)(accum_frac * 10000.0r);
    checksum += (long)(accum_long * 100.0lk);
    checksum += (long)(complex_expr * 100.0k);
    checksum += shift_op;
    checksum += shift_op_signed;
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
