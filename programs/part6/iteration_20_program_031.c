/* test_fixed_point_ranges.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume_result(const void *ptr, int size) {
    asm volatile("" : : "r"(ptr), "r"(size) : "memory");
}

/* Helper to create complex expressions */
__attribute__((noinline, noipa))
long _Accum compute_product(long _Accum a, long _Accum b, int shift) {
    /* This should trigger range analysis for multiplication */
    long _Accum temp = a * b;
    
    /* Mix with integer promotion */
    int i_temp = (int)(temp * 100k);
    
    /* Conditional expression that depends on ranges */
    long _Accum result = (i_temp > 0) ? (temp << shift) : (temp >> (-shift));
    
    return result;
}

int main(void) {
    /* Array to accumulate results */
    long _Accum results[8] = {0k};
    
    /* Initialize with volatile seeds to prevent constant folding */
    int seed1 = vi;
    unsigned int seed2 = vu;
    long seed3 = vl;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 8; i++) {
        /* Create values near boundaries for different fixed-point types */
        
        /* 1. Signed accumulative types near maximum */
        long _Accum la_max = 0.999999999k;  /* Very close to max */
        long _Accum la_min = -0.999999999k; /* Very close to min */
        
        /* Operations that could overflow */
        long _Accum product1 = la_max * la_max;  /* Should exceed 1.0 */
        long _Accum product2 = la_min * la_min;  /* Should exceed 1.0 */
        
        /* 2. Unsigned fractional types */
        unsigned _Fract uf_max = 0.9999999ur;  /* Close to 1.0 */
        unsigned _Fract uf_min = 0.0000001ur;  /* Close to 0.0 */
        
        /* Operations near overflow boundary */
        unsigned _Fract sum_uf = uf_max + uf_min;  /* Could overflow to 0 */
        
        /* 3. Signed fractional types */
        _Fract sf_max = 0.9999999r;   /* Close to 1.0 */
        _Fract sf_min = -0.9999999r;  /* Close to -1.0 */
        
        /* Complex expression mixing types */
        _Fract mixed = (_Fract)uf_max + sf_min;
        
        /* 4. Left shift operations that could overflow */
        _Accum a_shift = 0.5k;
        int shift_amount = (seed1 + i) % 8;
        
        /* This shift could overflow depending on shift_amount */
        _Accum shifted = a_shift << shift_amount;
        
        /* 5. Multiplication with integer promotion */
        int int_val = (seed2 + i * 100) % 256;
        _Accum from_int = (_Accum)int_val * 0.01k;
        
        /* 6. Nested operations to create complex range analysis */
        long _Accum complex_expr = compute_product(la_max, from_int, shift_amount);
        
        /* 7. Conditional assignment based on overflow-like check */
        /* Simulate overflow check by comparing with saturated constants */
        long _Accum final_val;
        if (complex_expr > 0.999999k) {
            /* Near overflow - adjust */
            final_val = 0.999999k;
        } else if (complex_expr < -0.999999k) {
            /* Near underflow - adjust */
            final_val = -0.999999k;
        } else {
            final_val = complex_expr;
        }
        
        /* 8. Mix with bitwise operations through integer casts */
        int temp_int = (int)(final_val * 1000000k);
        temp_int = temp_int ^ (seed3 & 0xFF);
        final_val = (long _Accum)temp_int / 1000000k;
        
        /* Store result */
        results[i] = final_val;
        
        /* Update seeds to vary computations */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed3 = (seed3 * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Additional edge case: Very large left shift */
    _Accum base = 0.0009765625k;  /* 2^-10 */
    _Accum large_shifted = base << 20;  /* Could overflow to max */
    
    /* Force range analysis for conversion */
    short _Fract sf_array[4];
    for (int i = 0; i < 4; i++) {
        /* Create values that might overflow during conversion */
        _Accum source = (i % 2 == 0) ? 1.5k : -1.5k;
        short _Fract converted = (short _Fract)source;  /* Conversion overflow */
        sf_array[i] = converted;
    }
    
    /* Mix all results */
    long _Accum final_result = 0k;
    for (int i = 0; i < 8; i++) {
        final_result += results[i];
    }
    final_result += (long _Accum)large_shifted;
    
    /* Prevent dead code elimination */
    consume_result(results, sizeof(results));
    consume_result(&final_result, sizeof(final_result));
    consume_result(sf_array, sizeof(sf_array));
    
    /* Return hash of results */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= (int)(results[i] * 1000000k);
    }
    return hash & 0xFF;
}
