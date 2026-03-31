/* test_fixed_point.c - Comprehensive fixed-point test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;
typedef _Sat short _Fract ssfract_t;
typedef _Sat long _Accum slaccum_t;

/* Test function 1: Complex fixed-point arithmetic with saturation */
static inline sfract_t sat_add_mult(sfract_t a, sfract_t b, sfract_t c) {
    /* This should trigger range analysis for saturation */
    sfract_t temp = a + b;
    if (temp > 0.8r) {
        return temp * c;
    } else {
        return (temp + 0.2r) * c;
    }
}

/* Test function 2: Accumulator with shift operations */
static inline saccum_t accum_shift_ops(saccum_t base, int shift) {
    /* Operations that require precise overflow analysis */
    saccum_t result = base;
    
    /* Shift operations that can overflow/underflow */
    if (shift > 0) {
        result = result << shift;
    } else {
        result = result >> (-shift);
    }
    
    /* Multiplication near saturation boundaries */
    result = result * 1.5k;
    
    return result;
}

/* Test function 3: Fixed-point array reduction */
static sfract_t array_reduction(const sfract_t* arr, int size) {
    sfract_t sum = 0.0r;
    for (int i = 0; i < size; i++) {
        /* Conditional addition based on value range */
        if (arr[i] > 0.5r) {
            sum = sum + arr[i] * 0.8r;
        } else {
            sum = sum + arr[i] * 1.2r;
        }
        
        /* Force range analysis with ternary operator */
        sum = (sum > 0.9r) ? 0.9r : sum;
    }
    return sum;
}

/* Test function 4: Mixed-type operations */
static accum_t mixed_type_ops(fract_t a, saccum_t b) {
    /* Mixing saturated and unsaturated types */
    accum_t result = (accum_t)a + (accum_t)b;
    
    /* Complex expression requiring range analysis */
    result = result * 2.0k - 1.0k;
    
    /* Shift operation */
    result = result >> 2;
    
    return result;
}

/* Test function 5: Built-in overflow checks */
static int check_overflow_ops(sfract_t* a, sfract_t* b, sfract_t* result) {
    /* Use builtins for overflow detection */
    int overflow = 0;
    
    /* These should trigger the fixed-value machinery */
    overflow |= __builtin_add_overflow(*a, *b, result);
    
    sfract_t temp;
    overflow |= __builtin_mul_overflow(*a, 1.5r, &temp);
    
    /* Conditional based on overflow results */
    if (!overflow) {
        *result = temp + *b;
    } else {
        *result = 0.5r;  /* Default saturated value */
    }
    
    return overflow;
}

/* Test function 6: Loop with fixed-point induction */
static saccum_t loop_induction_range(int iterations) {
    saccum_t base = 0.5k;
    
    for (int i = 0; i < iterations; i++) {
        /* Induction variable affects fixed-point calculation */
        fract_t scale = (fract_t)i / (fract_t)iterations;
        
        /* Complex expression that depends on loop variable */
        base = base + (saccum_t)(scale * 0.8r) * 0.5k;
        
        /* Conditional that may become constant after range analysis */
        if (base > 0.9k) {
            base = 0.9k;
        } else if (base < -0.9k) {
            base = -0.9k;
        }
    }
    
    return base;
}

/* Test function 7: Switch statement with fixed-point conditions */
static int fixed_point_switch(sfract_t value) {
    int result = 0;
    
    /* Switch on discretized fixed-point value */
    switch ((int)(value * 10.0r)) {
        case 0: result = 1; break;
        case 1: result = 2; break;
        case 2: result = 3; break;
        case 3: result = 4; break;
        case 4: result = 5; break;
        case 5: result = 6; break;
        case 6: result = 7; break;
        case 7: result = 8; break;
        case 8: result = 9; break;
        case 9: result = 10; break;
        default: result = 0; break;
    }
    
    return result;
}

/* Test function 8: Nested saturation operations */
static sfract_t nested_saturation(sfract_t a, sfract_t b, sfract_t c) {
    /* Multiple operations that can saturate */
    sfract_t temp1 = a + b;
    sfract_t temp2 = temp1 * c;
    sfract_t temp3 = temp2 + 0.3r;
    
    /* This should trigger the specific uncovered code */
    if (temp3 > 0.95r) {
        return 0.95r;
    } else if (temp3 < -0.95r) {
        return -0.95r;
    }
    
    return temp3;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command line arguments for variability */
    int iterations = 10;
    int array_size = 20;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 5) array_size = 20;
    }
    
    printf("Running fixed-point tests with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    sfract_t sat_array[array_size];
    fract_t unsat_array[array_size];
    
    for (int i = 0; i < array_size; i++) {
        fract_t val = (fract_t)i / (fract_t)array_size;
        sat_array[i] = (_Sat _Fract)val;
        unsat_array[i] = val;
    }
    
    /* Test 1: Complex arithmetic */
    sfract_t test1_a = 0.7r;
    sfract_t test1_b = 0.3r;
    sfract_t test1_c = 0.9r;
    sfract_t result1 = sat_add_mult(test1_a, test1_b, test1_c);
    printf("Test 1 result: %f\n", (float)result1);
    
    /* Test 2: Accumulator with shifts */
    saccum_t test2_base = 0.6k;
    int test2_shift = 2;
    saccum_t result2 = accum_shift_ops(test2_base, test2_shift);
    printf("Test 2 result: %f\n", (float)result2);
    
    /* Test 3: Array reduction */
    sfract_t result3 = array_reduction(sat_array, array_size);
    printf("Test 3 result: %f\n", (float)result3);
    
    /* Test 4: Mixed types */
    fract_t test4_a = 0.4r;
    saccum_t test4_b = 0.7k;
    accum_t result4 = mixed_type_ops(test4_a, test4_b);
    printf("Test 4 result: %f\n", (float)result4);
    
    /* Test 5: Built-in overflow */
    sfract_t test5_a = 0.8r;
    sfract_t test5_b = 0.3r;
    sfract_t test5_result;
    int overflow = check_overflow_ops(&test5_a, &test5_b, &test5_result);
    printf("Test 5 result: %f, overflow: %d\n", (float)test5_result, overflow);
    
    /* Test 6: Loop induction */
    saccum_t result6 = loop_induction_range(iterations);
    printf("Test 6 result: %f\n", (float)result6);
    
    /* Test 7: Switch statement */
    sfract_t test7_val = 0.75r;
    int result7 = fixed_point_switch(test7_val);
    printf("Test 7 result: %d\n", result7);
    
    /* Test 8: Nested saturation - specifically targeting uncovered lines */
    sfract_t test8_a = 0.9r;
    sfract_t test8_b = 0.8r;
    sfract_t test8_c = 0.9r;
    sfract_t result8 = nested_saturation(test8_a, test8_b, test8_c);
    printf("Test 8 result: %f\n", (float)result8);
    
    /* Additional stress test: multiple operations in sequence */
    sfract_t final_checksum = 0.0r;
    for (int i = 0; i < iterations; i++) {
        sfract_t val = sat_array[i % array_size];
        
        /* Complex expression designed to trigger range analysis */
        val = val * 1.1r + 0.05r;
        val = val > 0.95r ? 0.95r : val;
        val = val < -0.95r ? -0.95r : val;
        
        /* Shift-like operation using multiplication */
        val = val * 0.5r;
        
        final_checksum = final_checksum + val;
        
        /* Force saturation boundary checks */
        if (final_checksum > 0.99r) {
            final_checksum = 0.99r;
        }
    }
    
    printf("Final checksum: %f\n", (float)final_checksum);
    
    /* Test with assembly to create hard-to-analyze value flows */
    sfract_t asm_input = 0.5r;
    sfract_t asm_output;
    
    /* Use inline assembly with fixed-point constraints */
    asm volatile (
        "/* Fixed-point assembly block */"
        : "=r" (asm_output)
        : "r" (asm_input)
    );
    
    /* Use the result to prevent dead code elimination */
    final_checksum = final_checksum + asm_output * 0.01r;
    
    printf("Final result with assembly: %f\n", (float)final_checksum);
    
    return 0;
}
