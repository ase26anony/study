/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type definitions */
typedef short _Fract sfract;
typedef _Fract fract;
typedef long _Accum laccum;
typedef unsigned short _Fract usfract;
typedef _Sat short _Fract sat_sfract;

/* Bit-field structure with fixed-point members */
struct fixed_bitfield {
    unsigned int sign : 1;
    unsigned int integer : 7;
    unsigned int fraction : 8;
    sat_sfract sat_value;
};

/* Function using bit-fields and shifts */
static struct fixed_bitfield process_fixed_bits(sfract input) {
    struct fixed_bitfield result;
    volatile sfract temp = input;
    
    /* Force range analysis with shifts */
    unsigned int shifted = ((unsigned int)(temp * 256.0r)) << 4;
    result.sign = (shifted >> 15) & 1;
    result.integer = (shifted >> 8) & 0x7F;
    result.fraction = shifted & 0xFF;
    
    /* This may trigger alshift operations */
    result.sat_value = (sat_sfract)((result.fraction << 4) / 256.0r);
    
    return result;
}

/* Main function with complex fixed-point operations */
int main(void) {
    /* Initialize fixed-point variables across range */
    sfract f1 = 0.5r;
    sfract f2 = -0.25r;
    laccum acc1 = 0.0k;
    laccum acc2 = 0.0k;
    usfract uf1 = 0.75ur;
    volatile sfract bound = 0.8r;  /* Prevent constant folding */
    
    /* Mixed integer/fixed-point variables */
    int int_counter = 0;
    unsigned long ulong_bound = 1000UL;
    
    printf("Starting fixed-point range test...\n");
    
    /* Loop with range widening */
    for (int i = 0; i < 100; i++) {
        /* Multi-step computation forcing range analysis */
        acc1 = acc1 + (laccum)f1 * 0.1k;
        acc2 = acc2 - (laccum)f2 * 0.05k;
        
        /* Explicit range comparisons - may trigger sgt/ugt */
        if (acc1 > (laccum)bound) {
            /* This comparison should invoke range max/min calculation */
            f1 = f1 * 0.9r;
            int_counter++;
        }
        
        if ((sfract)acc2 < -bound) {
            f2 = -f2 * 0.8r;
        }
        
        /* Mixed precision operation with ternary */
        sfract temp = (i % 3 == 0) ? (sfract)(int_counter * 0.01r) : f1;
        
        /* Bit-field operations */
        struct fixed_bitfield bf = process_fixed_bits(temp);
        
        /* Additional comparison with computed bounds */
        laccum max_allowed = (laccum)((i + 1) * 0.02k);
        if (acc1 > max_allowed || acc2 < -max_allowed) {
            /* Force re-evaluation of range bounds */
            acc1 = acc1 * 0.95k;
            acc2 = acc2 * 0.95k;
        }
        
        /* Overflow check with saturating types */
        sat_sfract sat_val = (sat_sfract)(f1 + f2);
        if (sat_val == 1.0r || sat_val == -1.0r) {
            /* Reset if saturated */
            f1 = 0.5r;
            f2 = -0.25r;
        }
        
        /* Complex conditional with multiple comparisons */
        int use_alternative = (acc1 > 0.3k) && (acc2 < -0.1k) && (f1 > 0.2r);
        if (use_alternative) {
            uf1 = (usfract)(uf1 * 0.99ur);
        }
        
        /* Cast to integer for checksum */
        int_counter += (int)(temp * 100.0r);
    }
    
    /* Final computations and checksum */
    laccum final_acc = acc1 + acc2;
    sfract final_frac = f1 + f2;
    
    /* More range comparisons */
    volatile laccum volatile_bound = 0.5k;
    if (final_acc > volatile_bound) {
        final_frac = final_frac * 0.5r;
    }
    
    /* Compute checksum */
    uint32_t checksum = 0;
    checksum += (uint32_t)(final_acc * 1000.0k);
    checksum += (uint32_t)(final_frac * 1000.0r);
    checksum += (uint32_t)(uf1 * 1000.0ur);
    checksum += int_counter;
    
    /* Bit manipulation with fixed-point results */
    unsigned int shifted_checksum = checksum << 4;
    shifted_checksum = shifted_checksum >> 2;
    
    printf("Checksum: %u (shifted: %u)\n", checksum, shifted_checksum);
    printf("Final values: acc1=%.4k, acc2=%.4k, f1=%.4r, f2=%.4r\n", 
           acc1, acc2, f1, f2);
    
    return (checksum > 10000) ? 1 : 0;
}
