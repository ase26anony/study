/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned _Fract ufract_t;
typedef _Sat _Fract sat_fract_t;

/* Bit-field structure for mixed operations */
struct mixed_bf {
    unsigned int ubf : 8;
    signed int sbf : 8;
    unsigned int : 16;
};

/* Function using bit-fields and shifts with fixed-point */
static void bitfield_fixed_ops(struct mixed_bf *bf, fract_t *result) {
    /* Operations that may trigger alshift/zext/sext logic */
    unsigned int temp = bf->ubf;
    
    /* Shift operations that require range analysis */
    temp = temp << 3;  /* Left shift - may trigger alshift logic */
    temp = temp >> 1;  /* Right shift */
    
    /* Convert to fixed-point with potential range checks */
    *result = (fract_t)(temp / 256.0f);
    
    /* More shifts with different bit widths */
    bf->sbf = bf->sbf << 2;
    bf->ubf = bf->ubf >> 1;
}

/* Main function with the core logic */
int main(void) {
    /* Declare and initialize fixed-point variables */
    volatile sfract_t sf1 = 0.5hr;
    volatile fract_t f1 = 0.25r;
    volatile accum_t a1 = 1000.0lk;
    volatile ufract_t uf1 = 0.75ur;
    volatile sat_fract_t sat1 = 0.5r;
    
    /* Additional variables for mixed operations */
    int int_var = 100;
    unsigned long ulong_var = 1000;
    
    /* Accumulators for loop operations */
    accum_t accum = 0.0lk;
    fract_t fract_accum = 0.0r;
    
    /* Volatile bounds to prevent constant folding */
    volatile fract_t volatile_bound = 0.8r;
    volatile accum_t volatile_accum_bound = 500.0lk;
    
    /* Bit-field structure */
    struct mixed_bf bf = {128, -64};
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Mixed integer/fixed-point operations */
        accum += (accum_t)int_var * 0.01lk;
        fract_accum += (fract_t)(i % 10) * 0.1r;
        
        /* Operations that may trigger overflow checks */
        sf1 = sf1 * 0.9hr;
        f1 = f1 + 0.05r;
        a1 = a1 - 15.0lk;
        uf1 = uf1 * 0.99ur;
        
        /* Saturated operations */
        sat1 = sat1 + 0.3r;
        
        /* Explicit range comparisons - may trigger sgt/ugt logic */
        if (accum > volatile_accum_bound) {
            /* This comparison may invoke the uncovered range logic */
            accum = accum * 0.5lk;
        }
        
        if (f1 > volatile_bound) {
            /* Another potential trigger for range comparisons */
            f1 = f1 - 0.2r;
        }
        
        /* Ternary operator with mixed types */
        accum = (accum > 1000.0lk) ? 1000.0lk : 
                (accum < -1000.0lk) ? -1000.0lk : accum;
        
        /* Complex conditional with multiple comparisons */
        if ((accum > 800.0lk && f1 > 0.5r) || 
            (uf1 < 0.2ur && sf1 > 0.1hr)) {
            int_var += 5;
        }
        
        /* Cast operations in conditional context */
        if ((int)accum > int_var) {
            int_var = (int)accum;
        }
        
        /* Bit-field operations every 10 iterations */
        if (i % 10 == 0) {
            bitfield_fixed_ops(&bf, &f1);
        }
        
        /* Shift operations on integers affecting fixed-point */
        ulong_var = ulong_var << 1;
        if (ulong_var > 10000) {
            ulong_var = 1000;
            accum += (accum_t)(ulong_var) * 0.001lk;
        }
    }
    
    /* Additional explicit comparisons outside loop */
    accum_t test_val = accum * 2.0lk;
    if (test_val > 2000.0lk || test_val < -2000.0lk) {
        accum = accum / 2.0lk;
    }
    
    /* More complex range checking */
    fract_t f_test = f1 * 2.0r;
    if (f_test > 1.0r) {
        f1 = 1.0r;
    } else if (f_test < -1.0r) {
        f1 = -1.0r;
    }
    
    /* Compute checksum */
    int checksum = 0;
    checksum += (int)(accum * 100.0lk);
    checksum += (int)(f1 * 1000.0r);
    checksum += (int)(sf1 * 1000.0r);
    checksum += (int)(a1 * 0.1lk);
    checksum += (int)(uf1 * 1000.0r);
    checksum += (int)(sat1 * 1000.0r);
    checksum += int_var;
    checksum += (int)ulong_var;
    checksum += bf.ubf + bf.sbf;
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: accum=%Lfk, f1=%fr, sf1=%hr, a1=%Lfk, uf1=%ur, sat1=%fr\n",
           (long double)accum, (double)f1, (double)sf1, 
           (long double)a1, (double)uf1, (double)sat1);
    
    return 0;
}
