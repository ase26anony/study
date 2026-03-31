/* Test program to exercise fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef _Sat short _Fract sat_sfract_t;
typedef _Sat _Fract sat_fract_t;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int u1 : 5;
    sfract_t f1 : 8;    /* Fixed-point in bit-field */
    unsigned int u2 : 3;
    fract_t f2 : 16;    /* Larger fixed-point in bit-field */
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_bitfields(volatile sat_sfract_t input) {
    struct fixed_bitfield bf;
    
    /* Initialize with values that require range calculations */
    bf.u1 = 7;
    bf.f1 = 0.25hr;  /* Fixed-point literal */
    bf.u2 = 2;
    bf.f2 = -0.125r;
    
    /* Shift operations that may trigger alshift logic */
    unsigned int shift1 = bf.u1 << 2;  /* Left shift */
    unsigned int shift2 = bf.u2 >> 1;  /* Right shift */
    
    /* Mixed operations with fixed-point */
    bf.f1 = bf.f1 * (sfract_t)shift1;
    bf.f2 = bf.f2 + (fract_t)(shift2 * 0.0625r);
    
    /* Conditional based on shifted values */
    if ((shift1 > 10) && (bf.f1 < 0.5hr)) {
        bf.f1 = bf.f1 + 0.125hr;
    }
    
    /* Explicit range comparison */
    sat_sfract_t temp = input;
    if (temp > 0.75hr) {
        bf.f2 = 0.5r;
    } else if (temp < -0.75hr) {
        bf.f2 = -0.5r;
    }
    
    return bf;
}

/* Main function with loop-based range widening */
int main(void) {
    /* Declare fixed-point variables with various ranges */
    volatile fract_t f_bound = 0.8r;      /* Volatile to prevent constant folding */
    accum_t accumulator = 0.0lk;
    sat_fract_t sat_accum = 0.0r;
    usfract_t unsigned_frac = 0.2uhr;
    
    /* Initialize with values spanning positive/negative ranges */
    fract_t f1 = 0.25r;
    fract_t f2 = -0.5r;
    accum_t a1 = 1.5lk;
    accum_t a2 = -2.25lk;
    
    /* Loop that forces range analysis */
    int i;
    for (i = 0; i < 100; i++) {
        /* Multi-step computations */
        fract_t temp = f1 * f2;
        
        /* Conditional with explicit range comparison */
        if (temp > 0.1r) {
            f1 = f1 - 0.05r;
        } else if (temp < -0.1r) {
            f1 = f1 + 0.05r;
        }
        
        /* Mixed integer/fixed-point operations */
        int int_val = (i % 10) - 5;  /* Range: -5 to 4 */
        accum_t mixed = a1 * (accum_t)int_val;
        
        /* Explicit comparison against computed bounds */
        accum_t bound = (accum_t)(i * 0.01lk);
        if (mixed > bound) {
            a1 = a1 * 0.95lk;
        } else if (mixed < -bound) {
            a1 = a1 * 1.05lk;
        }
        
        /* Accumulate with potential overflow */
        accumulator += mixed;
        sat_accum += (sat_fract_t)(temp * 0.5r);
        
        /* Shift-like operation using multiplication */
        unsigned_frac = unsigned_frac * 2.0uhr;
        if (unsigned_frac > 0.8uhr) {
            unsigned_frac = 0.1uhr;
        }
        
        /* Ternary operator with range implications */
        f2 = (i % 3 == 0) ? f2 * 0.9r : 
             (i % 3 == 1) ? f2 * 1.1r : 
             -f2;
        
        /* Comparison against volatile bound */
        if (accumulator > (accum_t)f_bound) {
            accumulator = accumulator * 0.5lk;
        }
        
        /* Nested conditionals for complex range analysis */
        if (i > 50) {
            if (sat_accum > 0.8r) {
                sat_accum = 0.8r;
            } else if (sat_accum < -0.8r) {
                sat_accum = -0.8r;
            }
        }
    }
    
    /* Call bit-field function */
    struct fixed_bitfield bf_result = process_bitfields(0.6hr);
    
    /* Compute checksum to verify execution */
    int checksum = 0;
    checksum += (int)(accumulator * 100.0lk);
    checksum += (int)(sat_accum * 1000.0r);
    checksum += (int)(f1 * 100.0r);
    checksum += (int)(f2 * 100.0r);
    checksum += (int)(a1 * 100.0lk);
    checksum += (int)(unsigned_frac * 100.0uhr);
    checksum += bf_result.u1 * 10 + bf_result.u2;
    
    printf("Checksum: %d\n", checksum);
    printf("Final values:\n");
    printf("  accumulator: %ld.%03ld\n", 
           (long)accumulator, 
           (long)((accumulator - (long)accumulator) * 1000));
    printf("  sat_accum: %d.%03d\n", 
           (int)sat_accum, 
           (int)((sat_accum - (int)sat_accum) * 1000));
    printf("  f1: %d.%03d\n", 
           (int)f1, 
           (int)((f1 - (int)f1) * 1000));
    
    return 0;
}
