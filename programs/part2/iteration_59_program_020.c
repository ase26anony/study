/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>

/* Memory barrier to prevent optimization */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Fixed-point type combinations */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;

/* Function with fixed-point operations that may overflow */
static accum_t process_fixed_point(sfract_t a, sfract_t b, int shift) {
    /* These operations should trigger fixed-value analysis */
    accum_t result;
    
    /* Multiplication with potential overflow */
    accum_t temp1 = (accum_t)a * (accum_t)b;
    MEMORY_BARRIER();
    
    /* Left shift - FIXED_LSHIFT_EXPR */
    accum_t temp2 = temp1 << shift;
    MEMORY_BARRIER();
    
    /* Another multiplication to widen intermediate */
    result = temp2 * (accum_t)0.5k;
    MEMORY_BARRIER();
    
    return result;
}

/* Function that forces saturation checking */
static sfract_t narrow_conversion(laccum_t wide_val) {
    /* This conversion should trigger range checking */
    sfract_t narrow;
    
    /* Explicit cast to narrower type - may need saturation */
    narrow = (sfract_t)wide_val;
    MEMORY_BARRIER();
    
    return narrow;
}

/* Complex fixed-point computation with loops */
static void compute_fixed_array(volatile sfract_t *input, sfract_t *output, int size, int shift) {
    for (int i = 0; i < size; i++) {
        /* Read volatile to prevent constant folding */
        sfract_t val1 = input[i];
        MEMORY_BARRIER();
        
        /* Use loop-variant index to prevent optimization */
        sfract_t val2 = (sfract_t)(0.1r * i);
        
        /* Multiplication that may overflow intermediate */
        accum_t intermediate = (accum_t)val1 * (accum_t)val2;
        MEMORY_BARRIER();
        
        /* Left shift operation */
        intermediate = intermediate << (shift + i % 3);
        MEMORY_BARRIER();
        
        /* Convert back with potential saturation */
        output[i] = narrow_conversion(intermediate);
        MEMORY_BARRIER();
    }
}

/* Main test function */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? 4 : 6;
    
    /* Array of fixed-point values */
    volatile sfract_t input[8];
    sfract_t output[8];
    
    /* Initialize with values that may cause overflow */
    for (int i = 0; i < 8; i++) {
        /* Use values close to bounds */
        input[i] = (sfract_t)(0.9r - 0.05r * i);
    }
    
    /* Perform multiple fixed-point operations */
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary shift amount based on iteration */
        int shift = 2 + (iter % 3);
        
        /* Process array */
        compute_fixed_array(input, output, 8, shift);
        MEMORY_BARRIER();
        
        /* Additional direct operations */
        for (int i = 0; i < 8; i++) {
            /* Mix different fixed-point types */
            lfract_t lval = (lfract_t)output[i];
            accum_t aval = (accum_t)lval * (accum_t)0.8k;
            MEMORY_BARRIER();
            
            /* Left shift with varying amounts */
            aval = aval << ((i + shift) % 5);
            MEMORY_BARRIER();
            
            /* Narrow conversion that may need saturation */
            output[i] = narrow_conversion(aval);
            MEMORY_BARRIER();
        }
    }
    
    /* Use results to prevent dead code elimination */
    sfract_t sum = 0r;
    for (int i = 0; i < 8; i++) {
        sum += output[i];
    }
    
    /* Print something to ensure code runs */
    printf("Result: %f\n", (double)sum);
    
    return 0;
}

/* Additional test functions to increase coverage */
static void test_mixed_operations(void) {
    /* Mixed integer and fixed-point operations */
    int int_val = 100;
    sfract_t sf_val = 0.7r;
    accum_t acc_val;
    
    /* Integer promotion with fixed-point */
    acc_val = (accum_t)sf_val * (accum_t)int_val;
    MEMORY_BARRIER();
    
    /* Multiple shifts */
    acc_val = acc_val << 3;
    MEMORY_BARRIER();
    acc_val = acc_val << 1;
    MEMORY_BARRIER();
    
    /* Convert with potential overflow */
    sfract_t result = (sfract_t)acc_val;
    (void)result; /* Suppress unused warning */
}

/* Call from main to ensure it's used */
void run_additional_tests(void) {
    test_mixed_operations();
    
    /* Test with _Accum types specifically */
    saccum_t sacc1 = 0.5k;
    saccum_t sacc2 = 0.9k;
    saccum_t sacc_result;
    
    /* Multiplication and shift */
    sacc_result = sacc1 * sacc2;
    MEMORY_BARRIER();
    sacc_result = sacc_result << 4;  /* This may overflow */
    MEMORY_BARRIER();
    
    /* Convert to narrower type */
    sfract_t narrow_result = (sfract_t)sacc_result;
    (void)narrow_result;
}
