/* Test program to trigger fixed-point range analysis logic in GCC */
#include <stdio.h>

/* Fixed-point types with various precisions */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Saturating versions */
typedef short _Sat _Fract ssfract_t;
typedef _Sat _Fract sfract_sat_t;
typedef _Sat _Accum saccum_sat_t;

/* Bit-field structure with fixed-point conversion */
struct bitfield_fixed {
    unsigned int mantissa : 10;
    unsigned int exponent : 5;
    unsigned int sign : 1;
};

/* Function using bit-fields and shifts with fixed-point */
static accum_t process_bitfield(struct bitfield_fixed bf) {
    /* Convert bitfield to fixed-point through shifting */
    unsigned int raw = (bf.mantissa << bf.exponent);
    
    /* Create fixed-point from shifted value */
    accum_t result = (accum_t)raw / (accum_t)(1U << 15);
    
    /* Apply sign */
    if (bf.sign) {
        result = -result;
    }
    
    /* Shift operation that may trigger range analysis */
    result = result * (accum_t)(1 << (bf.exponent % 8));
    
    return result;
}

/* Main test function */
int main(void) {
    volatile saccum_t volatile_bound = 0.5k;  /* Prevent constant folding */
    volatile sfract_t volatile_fract = 0.2r;
    
    /* Initialize fixed-point variables */
    sfract_t f1 = 0.1r;
    sfract_t f2 = -0.3r;
    accum_t a1 = 100.0k;
    accum_t a2 = -50.0k;
    laccum_t la1 = 1000.0lk;
    
    /* Saturating versions */
    saccum_sat_t sat1 = 0.8k;
    saccum_sat_t sat2 = -0.4k;
    
    /* Loop-based range widening */
    accum_t accumulator = 0.0k;
    accum_t product = 1.0k;
    
    /* Use volatile variable as loop bound */
    int iterations = 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed precision operations */
        accumulator += (accum_t)f1 * (accum_t)(i + 1);
        product *= (accum_t)f2 + (accum_t)(i * 0.05k);
        
        /* Explicit range comparisons - may trigger sgt/ugt logic */
        if (accumulator > (accum_t)(i * 0.5k)) {
            /* Operation when above threshold */
            accumulator -= (accum_t)f1 * 2.0k;
        } else if (accumulator < (accum_t)(-i * 0.3k)) {
            /* Operation when below threshold */
            accumulator += (accum_t)f2 * 3.0k;
        }
        
        /* Ternary operator with mixed types */
        a1 = (accumulator > volatile_bound) ? 
             (accum_t)(accumulator * 0.9k) : 
             (accum_t)(accumulator / 0.9k);
             
        /* Check against computed bounds using multiplication */
        accum_t upper_bound = (accum_t)i * 10.0k;
        accum_t lower_bound = -(accum_t)i * 10.0k;
        
        if (a1 > upper_bound) {
            a1 = upper_bound;
        } else if (a1 < lower_bound) {
            a1 = lower_bound;
        }
        
        /* Saturating arithmetic test */
        sat1 += (saccum_sat_t)(0.2k * (saccum_sat_t)i);
        sat2 -= (saccum_sat_t)(0.1k * (saccum_sat_t)i);
        
        /* Check for potential overflow in saturating types */
        if (sat1 > (saccum_sat_t)0.9k || sat1 < (saccum_sat_t)-0.9k) {
            sat1 = (sat1 > 0) ? (saccum_sat_t)0.999k : (saccum_sat_t)-0.999k;
        }
    }
    
    /* Additional fixed-point operations outside loop */
    laccum_t large_accum = la1 * 2.0lk;
    
    /* Complex conditional with multiple comparisons */
    if (large_accum > 500.0lk && large_accum < 1500.0lk) {
        large_accum = (large_accum + la1) / 2.0lk;
    }
    
    /* Bit-field operations */
    struct bitfield_fixed bf = {.mantissa = 512, .exponent = 3, .sign = 0};
    accum_t bf_result = process_bitfield(bf);
    
    /* Mixed integer/fixed-point context */
    unsigned long int_val = 1000UL;
    accum_t mixed_result = (accum_t)int_val / a1;
    
    /* Another range check with shifted bounds */
    accum_t shifted_bound = bf_result * (accum_t)(1 << 4);
    if (mixed_result > shifted_bound) {
        mixed_result = shifted_bound;
    }
    
    /* Compute checksum to verify execution */
    int checksum = 0;
    checksum += (int)(accumulator * 1000k);
    checksum += (int)(product * 1000k);
    checksum += (int)(a1 * 1000k);
    checksum += (int)(large_accum / 10lk);
    checksum += (int)(bf_result * 1000k);
    checksum += (int)(mixed_result * 1000k);
    checksum += (int)(sat1 * 1000k);
    checksum += (int)(sat2 * 1000k);
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with explicit overflow checking */
    accum_t test_val = 0.0k;
    for (int i = 0; i < 20; i++) {
        test_val += 100.0k;
        
        /* This comparison may trigger the specific uncovered logic */
        accum_t max_allowed = (accum_t)(1 << 15) - 1.0k;
        if (test_val > max_allowed) {
            test_val = max_allowed;
        }
    }
    
    printf("Final test_val: %f\n", (double)test_val);
    
    return 0;
}
