/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point types */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Accum accum_t;
typedef unsigned short _Fract usfract_t;
typedef _Sat short _Fract sat_sfract_t;

/* Bit-field structure with fixed-point */
struct mixed_data {
    unsigned int flag : 1;
    unsigned int count : 7;
    sfract_t value : 8;  /* Fixed-point in bit-field */
    unsigned int pad : 16;
};

/* Function using bit-fields and shifts */
static void process_bitfield(struct mixed_data *data, int iterations) {
    volatile sfract_t volatile_bound = 0.5hr;
    
    for (int i = 0; i < iterations; i++) {
        /* Shift operations that may trigger alshift logic */
        unsigned int temp = data->count << (i % 4);
        data->count = temp & 0x7F;
        
        /* Fixed-point operation in bit-field context */
        sfract_t old_val = data->value;
        data->value = old_val * 0.75hr;
        
        /* Range comparison with volatile to prevent constant folding */
        if (data->value > volatile_bound) {
            data->value = 0.25hr;
        } else if (data->value < -volatile_bound) {
            data->value = -0.25hr;
        }
        
        /* Explicit comparison that may generate sgt/ugt */
        int cmp_result = (data->value > 0.0hr) ? 1 : 
                        ((data->value < 0.0hr) ? -1 : 0);
        data->flag = (cmp_result > 0);
    }
}

int main(void) {
    /* Initialize fixed-point variables across range */
    fract_t f1 = 0.25r;
    fract_t f2 = -0.5r;
    accum_t a1 = 100.0lk;
    accum_t a2 = -50.0lk;
    sat_sfract_t sat1 = 0.75hr;
    usfract_t uf1 = 0.8uhr;
    
    /* Volatile bounds to prevent constant folding */
    volatile accum_t volatile_max = 1000.0lk;
    volatile accum_t volatile_min = -1000.0lk;
    
    /* Accumulators for range widening */
    accum_t acc1 = 0.0lk;
    accum_t acc2 = 0.0lk;
    fract_t fract_acc = 0.0r;
    
    /* Mixed integer/fixed-point variables */
    int int_counter = 0;
    unsigned long ulong_bound = 100;
    
    /* Bit-field structure */
    struct mixed_data data = {0, 1, 0.5hr, 0};
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Multi-step computations forcing range analysis */
        acc1 = acc1 + a1 * (i % 10) / 10lk;
        acc2 = acc2 - a2 * (i % 5) / 5lk;
        
        /* Fixed-point multiplication with saturation risk */
        sat1 = sat1 * 1.1hr;
        
        /* Range comparisons triggering bound calculations */
        if (acc1 > volatile_max) {
            acc1 = volatile_max;
        } else if (acc1 < volatile_min) {
            acc1 = volatile_min;
        }
        
        /* Complex conditional with mixed types */
        if ((acc1 > 500.0lk) && (int_counter < (int)ulong_bound)) {
            fract_acc = fract_acc + f1;
            int_counter++;
        } else if ((acc1 < -500.0lk) || (int_counter >= 50)) {
            fract_acc = fract_acc - f2;
            int_counter--;
        }
        
        /* Explicit comparison that may lower to sgt/ugt */
        int range_check = (acc1 > acc2) ? 1 : 0;
        if (range_check) {
            /* Force conversion path */
            unsigned long temp = (unsigned long)(acc1 * 10lk);
            ulong_bound = temp % 200;
        }
        
        /* Ternary operator with fixed-point comparison */
        fract_t selector = (acc1 > 0.0lk) ? f1 : f2;
        fract_acc = fract_acc * selector;
        
        /* Periodic overflow-inducing operation */
        if (i % 7 == 0) {
            a1 = a1 * 1.5lk;
            a2 = a2 * 1.2lk;
        }
    }
    
    /* Process bit-field structure */
    process_bitfield(&data, 50);
    
    /* Additional mixed-type comparisons */
    for (int i = 0; i < 20; i++) {
        /* Cast between integer and fixed-point */
        int int_val = (int)(acc1 / 10lk);
        unsigned int uint_val = (unsigned int)(uf1 * 100);
        
        /* Comparisons in mixed contexts */
        if ((int_val > 0) && (uint_val < 50)) {
            uf1 = uf1 * 0.9uhr;
        }
        
        /* Shift operations with fixed-point derived values */
        unsigned int shift_amt = uint_val % 16;
        unsigned int shifted = int_val << shift_amt;
        
        /* Range check that may trigger sext/zext logic */
        if (shifted > 0x7FFF) {
            uf1 = 0.1uhr;
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    checksum += (int)(acc1 * 10lk);
    checksum += (int)(acc2 * 10lk);
    checksum += (int)(fract_acc * 100r);
    checksum += (int)(sat1 * 100hr);
    checksum += (int)(uf1 * 100uhr);
    checksum += data.count;
    checksum += data.flag;
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: acc1=%.3Lk, acc2=%.3Lk, fract_acc=%.3r\n", 
           (long double)acc1, (long double)acc2, (double)fract_acc);
    
    return 0;
}
