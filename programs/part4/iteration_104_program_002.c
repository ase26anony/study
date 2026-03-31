/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef unsigned _Fract ufract_t;

/* Saturating fixed-point types */
typedef _Sat short _Fract sat_sfract_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat long _Accum sat_accum_t;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf : 8;
    int sbf : 8;
    unsigned int : 16;
};

/* Function using bit-fields and shifts with fixed-point */
static void bitfield_fixed_ops(struct mixed_bf *bf, fract_t f, accum_t a) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int shift_val = bf->ubf & 0x7;  /* Limit shift to 0-7 */
    
    /* Shift operations that require range analysis */
    unsigned int shifted = bf->ubf << shift_val;
    int signed_shifted = bf->sbf << (shift_val % 7);
    
    /* Mix with fixed-point values */
    fract_t f_scaled = f;
    for (int i = 0; i < (shift_val % 4); i++) {
        f_scaled = f_scaled * 0.5r;
    }
    
    /* Conditional based on shifted values */
    if ((shifted > 100) || (signed_shifted < -50)) {
        bf->ubf = shifted >> 2;
        bf->sbf = signed_shifted >> 1;
    }
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables across range */
    volatile sfract_t sf1 = 0.5hr;
    volatile sfract_t sf2 = -0.25hr;
    volatile fract_t f1 = 0.75r;
    volatile fract_t f2 = -0.5r;
    volatile accum_t a1 = 100.0lk;
    volatile accum_t a2 = -50.0lk;
    volatile usfract_t usf1 = 0.8ur;
    volatile ufract_t uf1 = 0.9ur;
    
    /* Saturating types */
    sat_sfract_t sat_sf = 0.9hr;
    sat_accum_t sat_a = 200.0lk;
    
    /* Accumulators for loop */
    accum_t acc1 = 0.0lk;
    accum_t acc2 = 0.0lk;
    fract_t f_acc = 0.0r;
    
    /* Bit-field structure */
    struct mixed_bf bf = { .ubf = 42, .sbf = -20 };
    
    /* Loop with range-widening operations */
    volatile int iterations = 10;
    for (int i = 0; i < iterations; i++) {
        /* Mixed arithmetic forcing range calculations */
        acc1 = acc1 + a1 * (accum_t)(i * 0.1lk);
        acc2 = acc2 - a2 * (accum_t)((i % 3) * 0.05lk);
        
        /* Fixed-point multiplication with overflow potential */
        f_acc = f_acc + f1 * (fract_t)(i * 0.01r);
        
        /* Saturating operations */
        sat_sf = sat_sf * 1.1hr;
        sat_a = sat_a + 25.0lk;
        
        /* Explicit range comparisons - may trigger sgt/ugt logic */
        if (acc1 > 50.0lk) {
            /* Reduce if exceeds threshold */
            acc1 = acc1 * 0.8lk;
        }
        
        if (acc2 < -30.0lk) {
            /* Increase if below threshold */
            acc2 = acc2 + 10.0lk;
        }
        
        /* Complex conditional with multiple comparisons */
        if ((f_acc > 0.5r) && (f_acc < 0.9r)) {
            f_acc = f_acc * 0.5r;
        } else if (f_acc <= -0.3r) {
            f_acc = f_acc + 0.1r;
        }
        
        /* Mixed-type comparison */
        if ((accum_t)f_acc > acc1 * 0.1lk) {
            f_acc = f_acc - 0.05r;
        }
        
        /* Ternary operator with range check */
        acc1 = (acc1 > 100.0lk) ? 100.0lk : acc1;
        acc2 = (acc2 < -100.0lk) ? -100.0lk : acc2;
        
        /* Update bit-fields */
        bf.ubf = (bf.ubf + i) & 0xFF;
        bf.sbf = (bf.sbf - (i % 5)) & 0xFF;
        
        /* Call function with bit-field operations */
        bitfield_fixed_ops(&bf, f_acc, acc1);
        
        /* Additional shift operations on bit-fields */
        unsigned int temp = bf.ubf;
        for (int j = 0; j < 3; j++) {
            temp = temp << 1;
            if (temp > 200) {
                temp = temp >> 2;
            }
        }
        bf.ubf = temp & 0xFF;
    }
    
    /* Post-loop computations with range checks */
    accum_t final_acc = acc1 + acc2;
    
    /* More explicit comparisons that may hit uncovered lines */
    if (final_acc > 75.0lk) {
        final_acc = 75.0lk;
    } else if (final_acc < -75.0lk) {
        final_acc = -75.0lk;
    }
    
    /* Mixed unsigned/signed fixed-point comparison */
    if ((ufract_t)(f_acc > 0 ? f_acc : -f_acc) > usf1) {
        f_acc = f_acc * 0.8r;
    }
    
    /* Compute checksum for verification */
    int checksum = 0;
    checksum += (int)(final_acc * 10.0lk);
    checksum += (int)(f_acc * 100.0r);
    checksum += (int)(sat_sf * 100.0hr);
    checksum += (int)(sat_a * 0.1lk);
    checksum += bf.ubf;
    checksum += bf.sbf;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional test cases in separate scopes */
    {
        /* Test with explicit overflow boundaries */
        sat_accum_t sat_test = 0.0lk;
        for (int i = 0; i < 20; i++) {
            sat_test = sat_test + 50.0lk;
            /* This should saturate */
            if (sat_test == sat_test + 0.0lk) {
                /* Force comparison */
            }
        }
    }
    
    {
        /* Test with very small values */
        fract_t tiny = 0.001r;
        for (int i = 0; i < 100; i++) {
            tiny = tiny * 1.01r;
            if (tiny > 0.1r) {
                tiny = 0.001r;
            }
        }
    }
    
    return 0;
}
