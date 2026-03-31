/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point type declarations */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Accum laccum;
typedef unsigned short _Fract usfract;
typedef _Sat short _Fract sat_sfract;

/* Bit-field structure with fixed-point */
struct fixed_bitfield {
    unsigned int mantissa : 10;
    unsigned int exponent : 5;
    sfract fixed_val;
};

/* Function using bit-fields and shifts */
static sat_sfract process_fixed_bitfield(struct fixed_bitfield *bf, int shift) {
    /* Operations that may trigger alshift/zext/sext */
    unsigned int temp = bf->mantissa;
    
    /* Shift operations requiring range analysis */
    temp = temp << shift;
    temp = temp >> (shift / 2);
    
    /* Convert to fixed-point with potential overflow */
    sat_sfract result = (_Fract)temp / 512.0r;
    
    /* Explicit comparison that may generate sgt/ugt */
    if (result > 0.5r) {
        result = 0.5r;
    } else if (result < -0.5r) {
        result = -0.5r;
    }
    
    return result;
}

/* Main function with loop-based range widening */
int main(void) {
    /* Declare fixed-point variables spanning different ranges */
    volatile sfract f1 = 0.25r;
    volatile sfract f2 = -0.75r;
    volatile laccum a1 = 100.0k;
    volatile laccum a2 = -50.0k;
    volatile usfract uf1 = 0.8ur;
    
    /* Accumulators for loop operations */
    sfract acc_s = 0.0r;
    laccum acc_l = 0.0k;
    sat_sfract acc_sat = 0.0r;
    
    /* Non-constant bounds to prevent folding */
    volatile int bound1 = 100;
    volatile int bound2 = 50;
    
    /* Bit-field structure */
    struct fixed_bitfield bf = {512, 8, 0.25r};
    
    printf("Starting fixed-point range test...\n");
    
    /* Loop that forces range widening analysis */
    for (int i = 0; i < bound1; i++) {
        /* Mixed precision operations */
        acc_s = acc_s + f1;
        acc_l = acc_l * 1.01k + a2;
        
        /* Saturated arithmetic */
        acc_sat = acc_sat + 0.1r;
        
        /* Conditional with explicit range comparisons */
        if (i > bound2) {
            /* Different operations in different ranges */
            acc_s = acc_s - f2;
            acc_l = acc_l / 2.0k;
        }
        
        /* Complex conditional with multiple comparisons */
        if ((acc_s > 0.9r) || (acc_s < -0.9r)) {
            /* Reset when near saturation */
            acc_s = acc_s * 0.5r;
        }
        
        /* Ternary operator with fixed-point comparison */
        acc_s = (acc_l > 0.0k) ? acc_s + 0.05r : acc_s - 0.05r;
        
        /* Call bit-field function periodically */
        if (i % 10 == 0) {
            sfract bf_result = process_fixed_bitfield(&bf, i % 8);
            acc_s = acc_s + bf_result;
        }
        
        /* Cast between integer and fixed-point */
        int int_val = (int)(acc_l / 10.0k);
        if (int_val > 100) {
            acc_l = acc_l - 20.0k;
        }
        
        /* Explicit overflow check pattern */
        laccum temp = a1 * i;
        if (temp > 1000.0k || temp < -1000.0k) {
            a1 = a1 * 0.9k;
        }
    }
    
    /* Additional fixed-point operations outside loop */
    fract f_array[4] = {0.1r, 0.2r, 0.3r, 0.4r};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Multiplication that may overflow */
            fract prod = f_array[i] * f_array[j];
            
            /* Comparison against computed bounds */
            fract bound = (fract)(i + j) / 8.0r;
            if (prod > bound) {
                f_array[i] = f_array[i] - 0.05r;
            }
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    checksum += (int)(acc_s * 1000.0r);
    checksum += (int)(acc_l / 10.0k);
    checksum += (int)(acc_sat * 1000.0r);
    checksum += (int)(f1 * 1000.0r);
    checksum += (int)(f2 * 1000.0r);
    
    for (int i = 0; i < 4; i++) {
        checksum += (int)(f_array[i] * 1000.0r);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: acc_s=%.3r, acc_l=%.3k, acc_sat=%.3r\n", 
           acc_s, acc_l, acc_sat);
    
    return 0;
}
