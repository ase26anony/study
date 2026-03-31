/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>
#include <stdint.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;
typedef _Sat _Fract sat_fract_t;
typedef _Sat _Accum sat_accum_t;

/* Bit-field structure with fixed-point */
struct mixed_data {
    unsigned int flag : 3;
    sfract_t value : 13;  /* Bit-field with fixed-point */
    unsigned int count : 16;
};

/* Function using bit-fields and shifts */
static sat_accum_t process_bitfield(struct mixed_data *data, int iterations) {
    volatile sat_accum_t accumulator = 0.5k;
    volatile sfract_t threshold = 0.25r;
    
    for (int i = 0; i < iterations; i++) {
        /* Shift operations that may trigger range calculations */
        unsigned int shifted = data->count << (data->flag + 1);
        
        /* Fixed-point operation with shift context */
        sfract_t scaled = (sfract_t)(shifted % 256) / 256.0r;
        
        /* Conditional with explicit range comparison */
        if (scaled > threshold) {
            /* This comparison may trigger sgt/ugt operations */
            accumulator += (sat_accum_t)scaled * 0.1k;
            
            /* Additional range check */
            if (accumulator > 0.8k) {
                accumulator = 0.8k;  /* Simulate saturation */
            }
        } else if (scaled < -threshold) {
            accumulator -= (sat_accum_t)scaled * 0.05k;
            
            /* Lower bound check */
            if (accumulator < -0.7k) {
                accumulator = -0.7k;
            }
        }
        
        /* Update bit-field with shift */
        data->count = (data->count + 1) & 0xFFFF;
        data->flag = (data->flag + 1) % 7;
    }
    
    return accumulator;
}

int main(void) {
    /* Initialize fixed-point variables spanning different ranges */
    volatile fract_t f_pos = 0.75r;
    volatile fract_t f_neg = -0.5r;
    volatile accum_t a_pos = 100.5k;
    volatile accum_t a_neg = -200.25k;
    volatile sat_fract_t sf_sat = 0.9r;
    volatile sat_accum_t sa_sat = 300.0k;  /* Will saturate */
    
    /* Accumulators for loop computations */
    accum_t accum1 = 0.0k;
    accum_t accum2 = 0.0k;
    laccum_t long_accum = 0.0lk;
    
    /* Mixed integer/fixed-point variables */
    unsigned long int_mask = 0xFF;
    int shift_count = 3;
    
    /* Bit-field structure */
    struct mixed_data data = {1, 0.5r, 1000};
    
    /* Loop with range-widening operations */
    for (int i = 0; i < 100; i++) {
        /* Multi-step computation forcing range analysis */
        accum_t temp = accum1 * f_pos + accum2 * f_neg;
        
        /* Explicit range comparison - may trigger uncovered logic */
        if (temp > a_pos) {
            accum1 = a_pos;
            /* Ternary operator with mixed types */
            accum2 = (i & 1) ? (accum_t)(int_mask >> shift_count) / 256.0k 
                             : (accum_t)(int_mask << shift_count) / 65536.0k;
        } else if (temp < a_neg) {
            accum1 = a_neg;
            accum2 = -accum2;
        } else {
            /* Complex expression with multiple operations */
            accum1 = temp + (accum_t)i / 1000.0k;
            accum2 = accum2 * 0.99k - (accum_t)(i % 10) / 50.0k;
        }
        
        /* Saturation checks */
        if (sf_sat > 0.95r) {
            sf_sat = 0.95r;
        }
        
        /* Mixed integer/fixed-point operation in conditional */
        unsigned long shifted = int_mask << (i % 5);
        if ((accum_t)(shifted & 0xFF) / 256.0k > 0.5k) {
            long_accum += (laccum_t)accum1 * 0.01lk;
        }
        
        /* Force non-constant bounds with volatile */
        volatile int bound = i % 20;
        if (accum1 > (accum_t)bound / 10.0k) {
            accum1 = accum1 * 0.9k;
        }
        
        /* Update integer mask with shift */
        int_mask = (int_mask * 3 + 1) & 0xFFFFFFFF;
    }
    
    /* Process bit-field function */
    sat_accum_t result = process_bitfield(&data, 50);
    
    /* Additional fixed-point operations outside loop */
    lfract_t lf1 = 0.999999r;  /* Near upper bound */
    lfract_t lf2 = -0.999999r; /* Near lower bound */
    
    /* Comparisons that may trigger range logic */
    if (lf1 > 0.9999r && lf2 < -0.9999r) {
        lf1 = lf1 * 0.5r;
        lf2 = lf2 * 0.5r;
    }
    
    /* Compute checksum to verify execution */
    uint32_t checksum = 0;
    checksum += (uint32_t)(accum1 * 1000.0k);
    checksum += (uint32_t)(accum2 * 1000.0k);
    checksum += (uint32_t)(long_accum * 1000.0lk);
    checksum += (uint32_t)(result * 1000.0k);
    checksum += (uint32_t)(lf1 * 1000000.0r);
    checksum += (uint32_t)(lf2 * 1000000.0r);
    checksum += data.count;
    checksum += data.flag;
    
    printf("Checksum: %u\n", checksum);
    printf("Final values: accum1=%.3k, accum2=%.3k, long_accum=%.3lk\n", 
           accum1, accum2, long_accum);
    printf("Bitfield result: %.3k, count=%u, flag=%u\n", 
           result, data.count, data.flag);
    
    return 0;
}
