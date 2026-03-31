/* Test program to exercise GCC's fixed-point range analysis logic */
#include <stdio.h>

/* Fixed-point type definitions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef _Sat short _Fract ssfract_t;
typedef _Sat _Fract sfract_sat_t;
typedef _Sat long _Fract slfract_t;

/* Bit-field structure with fixed-point */
struct mixed_data {
    unsigned int flag : 1;
    unsigned int count : 7;
    sfract_t value : 8;  /* Fixed-point in bit-field */
    unsigned int pad : 16;
};

/* Function using bit-fields and shifts */
static struct mixed_data process_bitfield(volatile sfract_t input) {
    struct mixed_data result;
    result.flag = (input > 0.5hr) ? 1 : 0;
    result.count = 0;
    
    /* Shift operations that may trigger range calculations */
    unsigned int temp = (unsigned int)(input * 256.0hr);
    temp = temp << 3;  /* Left shift */
    result.count = temp >> 4;  /* Right shift */
    
    /* Fixed-point operation in bit-field context */
    result.value = input * 0.75hr;
    
    return result;
}

/* Main test function */
int main(void) {
    volatile sfract_t v1 = 0.25hr;   /* Prevent constant folding */
    volatile sfract_t v2 = -0.75hr;
    volatile accum_t a1 = 100.0k;
    volatile accum_t a2 = -50.0k;
    
    /* Loop-based range widening */
    sfract_sat_t sat_acc = 0.0hr;
    accum_t range_acc = 0.0k;
    laccum_t wide_acc = 0.0lk;
    
    int checksum = 0;
    
    /* Loop that forces range analysis */
    for (int i = 0; i < 100; i++) {
        /* Mixed precision operations */
        sfract_t temp = v1 * (sfract_t)(i * 0.01hr);
        
        /* Conditional with explicit range comparison */
        if (temp > 0.5hr) {
            sat_acc += 0.1hr;
            /* Check for saturation */
            if (sat_acc == 1.0hr) {
                sat_acc = 0.9hr;  /* Reset before saturation */
            }
        } else if (temp < -0.5hr) {
            sat_acc -= 0.1hr;
            if (sat_acc == -1.0hr) {
                sat_acc = -0.9hr;
            }
        }
        
        /* Accumulator with widening range */
        range_acc += a1 * (accum_t)(i * 0.1k);
        
        /* Explicit range check against computed bounds */
        accum_t upper_bound = a1 * 2.0k;
        accum_t lower_bound = a2 * 3.0k;
        
        if (range_acc > upper_bound) {
            range_acc = upper_bound;
        } else if (range_acc < lower_bound) {
            range_acc = lower_bound;
        }
        
        /* Mixed integer/fixed-point context */
        wide_acc += (laccum_t)range_acc * (laccum_t)(i % 10);
        
        /* Ternary operator with range comparison */
        fract_t select = (range_acc > 0.0k) ? 0.7hr : -0.3hr;
        checksum += (int)(select * 100.0hr);
        
        /* Modify volatile variables to change bounds */
        v1 += 0.01hr;
        if (v1 > 0.9hr) v1 = 0.25hr;
        
        a1 += 1.0k;
        if (a1 > 200.0k) a1 = 100.0k;
    }
    
    /* Bit-field operations */
    struct mixed_data bf_result = process_bitfield(sat_acc);
    checksum += bf_result.count;
    checksum += (int)(bf_result.value * 100.0hr);
    
    /* Final range comparisons */
    accum_t final_max = a1 * 10.0k;
    accum_t final_min = a2 * 5.0k;
    
    /* Multiple comparison expressions */
    if (range_acc > final_max || range_acc < final_min) {
        range_acc = (range_acc > final_max) ? final_max : final_min;
    }
    
    /* Mixed-type conditional */
    int use_alt = (wide_acc > 10000.0lk) ? 1 : 0;
    if (use_alt && (range_acc > 0.0k)) {
        wide_acc /= 2.0lk;
    }
    
    /* Compute final checksum */
    checksum += (int)(sat_acc * 1000.0hr);
    checksum += (int)(range_acc);
    checksum += (int)(wide_acc % 1000);
    checksum += use_alt * 37;
    
    printf("Checksum: %d\n", checksum);
    printf("Final values: sat_acc=%.4hr, range_acc=%.4k, wide_acc=%.4lk\n",
           sat_acc, range_acc, wide_acc);
    
    return 0;
}
