/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    unsigned int fraction : 8;
    sfract_t sf_val;
    fract_t f_val;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bitfield(struct fixed_bitfield bf, int shift) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int temp = (bf.integer << shift) | (bf.fraction >> (8 - shift));
    
    /* Convert to fixed-point with potential range issues */
    bf.sf_val = (_Fract)temp / 256.0r;
    
    /* Shift operations that require range analysis */
    bf.fraction = (bf.fraction << shift) | (bf.integer >> (7 - shift));
    bf.integer = temp & 0x7F;
    
    return bf;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables across different ranges */
    volatile sfract_t sf1 = 0.5hr;
    volatile sfract_t sf2 = -0.25hr;
    volatile fract_t f1 = 0.75r;
    volatile fract_t f2 = -0.125r;
    volatile accum_t a1 = 100.0lk;
    volatile accum_t a2 = -50.0lk;
    volatile ufract_t uf1 = 0.8ur;
    
    /* Saturated types for overflow scenarios */
    sat_fract_t sat_f = 0.9r;
    sat_accum_t sat_a = 1000.0lk;
    
    /* Accumulators for loop operations */
    accum_t accum_pos = 0.0lk;
    accum_t accum_neg = 0.0lk;
    fract_t fract_acc = 0.0r;
    
    /* Mixed integer/fixed-point variables */
    int int_counter = 0;
    unsigned long ulong_bound = 1000;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {
        .sign = 0,
        .integer = 64,
        .fraction = 128,
        .sf_val = 0.25hr,
        .f_val = 0.5r
    };
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Force non-constant bounds with volatile */
        volatile int bound = (i % 10) + 5;
        
        /* Complex arithmetic that may overflow */
        accum_pos += a1 * (_Accum)i / 50.0lk;
        accum_neg += a2 * (_Accum)(i - 50) / 25.0lk;
        
        /* Fixed-point multiplication with saturation check */
        fract_acc = fract_acc * f1 + sf1 * sf2;
        
        /* Conditional blocks with explicit range comparisons */
        if (accum_pos > (_Accum)bound * 10.0lk) {
            /* This comparison may trigger sgt/ugt logic */
            accum_pos = (_Accum)bound * 10.0lk;
            int_counter++;
        }
        
        if (accum_neg < -(_Accum)bound * 5.0lk) {
            /* Negative bound comparison */
            accum_neg = -(_Accum)bound * 5.0lk;
            int_counter--;
        }
        
        /* Mixed-type comparison */
        if ((_Accum)int_counter > accum_pos / 2.0lk) {
            /* Force conversion and range analysis */
            fract_acc = fract_acc * 0.9r;
        }
        
        /* Ternary operator with fixed-point */
        sf1 = (accum_pos > 0.0lk) ? 0.7hr : -0.3hr;
        
        /* Saturated arithmetic that needs overflow checking */
        sat_f = sat_f * 1.1r;
        sat_a = sat_a + 50.0lk;
        
        /* Bit-field operations every 10 iterations */
        if (i % 10 == 0) {
            bf = process_fixed_bitfield(bf, (i / 10) % 4);
            
            /* Use bit-field values in fixed-point operations */
            accum_pos += (_Accum)bf.integer;
            fract_acc += (_Fract)bf.fraction / 256.0r;
        }
        
        /* Explicit cast to integer for range analysis */
        unsigned int temp_uint = (unsigned int)(accum_pos * 10.0lk);
        if (temp_uint > ulong_bound) {
            accum_pos = (_Accum)ulong_bound / 10.0lk;
        }
        
        /* Shift-like operation using multiplication */
        f2 = f2 * 0.5r;  /* Equivalent to right shift for fixed-point */
        
        /* Check for near-zero (range boundary) */
        if (f2 > -0.01r && f2 < 0.01r) {
            f2 = (i % 2) ? 0.1r : -0.1r;
        }
    }
    
    /* Process bit-field one more time */
    bf = process_fixed_bitfield(bf, 2);
    
    /* Compute checksum to verify execution */
    long checksum = 0;
    checksum += (long)(accum_pos * 1000.0lk);
    checksum += (long)(accum_neg * 1000.0lk);
    checksum += (long)(fract_acc * 1000.0r);
    checksum += (long)(sat_f * 1000.0r);
    checksum += (long)(sat_a / 10.0lk);
    checksum += bf.integer;
    checksum += bf.fraction;
    checksum += int_counter;
    
    printf("Checksum: %ld\n", checksum);
    printf("Final values:\n");
    printf("  accum_pos: %Lf\n", (long double)accum_pos);
    printf("  accum_neg: %Lf\n", (long double)accum_neg);
    printf("  fract_acc: %Lf\n", (long double)fract_acc);
    printf("  sat_f: %Lf\n", (long double)sat_f);
    printf("  sat_a: %Lf\n", (long double)sat_a);
    
    return 0;
}
