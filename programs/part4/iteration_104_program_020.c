/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int f_part : 8;    /* fractional part */
    unsigned int i_part : 8;    /* integer part */
    unsigned int sign : 1;      /* sign bit */
    unsigned int pad : 15;      /* padding */
};

/* Function using bit-fields and shifts */
static void bitfield_fixed_ops(struct mixed_bf *bf, fract_t *result) {
    volatile fract_t v1 = 0.25r;
    volatile fract_t v2 = -0.5r;
    
    /* Operations that may trigger alshift/sext/zext */
    unsigned int temp = bf->f_part;
    temp = temp << (bf->i_part & 0x3);  /* Variable shift */
    
    /* Convert to fixed-point with potential range checks */
    *result = (fract_t)((temp * v1) + v2);
    
    /* Explicit range comparison */
    if (*result > 0.75r) {
        *result = 0.75r;
    } else if (*result < -0.75r) {
        *result = -0.75r;
    }
}

int main(void) {
    /* Initialize fixed-point variables across range */
    volatile fract_t f1 = 0.25r;
    volatile fract_t f2 = -0.5r;
    volatile accum_t a1 = 100.0k;
    volatile accum_t a2 = -50.0k;
    sat_fract_t sf1 = 0.8r;
    sat_accum_t sa1 = 200.0k;
    
    /* Accumulators for loop */
    fract_t f_acc = 0.0r;
    accum_t a_acc = 0.0k;
    int checksum = 0;
    
    /* Loop-based range widening */
    for (int i = 0; i < 100; i++) {
        /* Mixed precision operations */
        f_acc += f1 * (fract_t)(i % 10);
        a_acc += a1 * (accum_t)(i % 20);
        
        /* Explicit range comparisons - may trigger sgt/ugt */
        volatile fract_t bound1 = 0.5r + (fract_t)(i * 0.01r);
        volatile fract_t bound2 = -0.5r - (fract_t)(i * 0.01r);
        
        if (f_acc > bound1) {
            /* Operation when above upper bound */
            f_acc = bound1 - 0.1r;
            a_acc += 10.0k;
        } else if (f_acc < bound2) {
            /* Operation when below lower bound */
            f_acc = bound2 + 0.1r;
            a_acc -= 10.0k;
        }
        
        /* Ternary operator with mixed types */
        fract_t temp = (i & 1) ? f1 : f2;
        f_acc = (f_acc * temp > 0.9r) ? 0.9r : f_acc * temp;
        
        /* Saturated operations */
        sf1 = sf1 + 0.1r;
        sa1 = sa1 * 1.05k;
        
        /* Cast to integer for conditional */
        int int_val = (int)(a_acc * 0.01k);
        if (int_val > 100) {
            a_acc = 100.0k;
        } else if (int_val < -100) {
            a_acc = -100.0k;
        }
    }
    
    /* Bit-field operations */
    struct mixed_bf bf = {.f_part = 128, .i_part = 4, .sign = 0};
    fract_t bf_result;
    bitfield_fixed_ops(&bf, &bf_result);
    
    /* Additional overflow-prone operations */
    for (int i = 0; i < 50; i++) {
        /* Multiplication near limits */
        accum_t test = a_acc * 1.1k;
        
        /* Explicit comparison against computed bounds */
        accum_t upper_bound = 150.0k + (accum_t)i;
        accum_t lower_bound = -150.0k - (accum_t)i;
        
        if (test > upper_bound) {
            a_acc = upper_bound / 1.1k;
        } else if (test < lower_bound) {
            a_acc = lower_bound / 1.1k;
        } else {
            a_acc = test;
        }
        
        /* Shift-like operations using multiplication */
        fract_t shifted = f_acc * 2.0r;  /* Equivalent to left shift */
        if (shifted > 0.95r) {
            f_acc = 0.95r / 2.0r;
        } else {
            f_acc = shifted;
        }
    }
    
    /* Compute checksum */
    checksum += (int)(f_acc * 1000.0r);
    checksum += (int)(a_acc * 100.0k);
    checksum += (int)(bf_result * 1000.0r);
    checksum += (int)(sf1 * 1000.0r);
    checksum += (int)(sa1 * 100.0k);
    
    printf("Checksum: %d\n", checksum);
    
    /* Final explicit comparisons */
    volatile accum_t final_check = 500.0k;
    if (a_acc * 2.0k > final_check) {
        printf("Final overflow check triggered\n");
    }
    
    return 0;
}
