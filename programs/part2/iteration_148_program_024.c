/* test_fixed_point.c - Target coverage for fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef _Sat short _Fract sat_short_fract_t;

/* Test functions with complex fixed-point operations */

/* Function 1: Range analysis with saturation boundaries */
static inline sat_fract_t sat_add_range(sat_fract_t a, sat_fract_t b) {
    /* This should trigger saturation analysis */
    sat_fract_t sum = a + b;
    
    /* Complex expression requiring range analysis */
    sat_fract_t scaled = (sum * 0.5r) + (sum * 0.5r);
    
    /* Conditional based on range analysis */
    if (sum > 0.9r) {
        return scaled * 0.8r;
    } else if (sum < -0.9r) {
        return scaled * (-0.8r);
    }
    
    return scaled;
}

/* Function 2: Loop-based range propagation */
static sat_accum_t loop_range_analysis(int iterations, fract_t base) {
    sat_accum_t total = 0.0k;
    sat_accum_t multiplier = 1.5k;
    
    for (int i = 0; i < iterations; i++) {
        /* Range of 'factor' depends on loop iteration */
        accum_t factor = (accum_t)i * 0.1k + 0.5k;
        
        /* Complex expression requiring precise overflow analysis */
        sat_accum_t term = (base * factor) * multiplier;
        
        /* Shift operation that can cause underflow/overflow */
        if (i % 2 == 0) {
            term = term >> 2;
        } else {
            term = term << 1;
        }
        
        total = total + term;
        
        /* Update multiplier with saturation */
        multiplier = multiplier * 0.9k;
    }
    
    return total;
}

/* Function 3: Array reduction with mixed types */
static sat_fract_t array_reduction(fract_t* arr, int size) {
    sat_fract_t result = 0.0r;
    sat_fract_t running_prod = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Complex expression requiring range analysis */
        sat_fract_t val = arr[i] * 0.8r;
        
        /* Ternary with fixed-point operands */
        val = (val > 0.5r) ? (val * 0.7r) : (val * 1.3r);
        
        /* Addition that may saturate */
        result = result + val;
        
        /* Multiplication that may saturate */
        running_prod = running_prod * (0.9r + val * 0.1r);
    }
    
    /* Final complex expression */
    return (result * 0.5r) + (running_prod * 0.5r);
}

/* Function 4: Using builtins for overflow detection */
static int builtin_overflow_test(sat_accum_t a, sat_accum_t b, sat_accum_t* res) {
    int overflow = 0;
    
    /* Use builtin for overflow detection */
    overflow |= __builtin_add_overflow(a, b, res);
    
    sat_accum_t temp;
    overflow |= __builtin_mul_overflow(*res, 2.0k, &temp);
    
    /* Shift with potential overflow */
    *res = temp >> 3;
    
    return overflow;
}

/* Function 5: Switch statement with fixed-point conditions */
static fract_t switch_range_test(fract_t input) {
    fract_t output;
    
    /* Switch where cases depend on fixed-point comparisons */
    if (input > 0.8r) {
        output = input * 0.6r;
    } else if (input > 0.5r) {
        output = input * 0.8r;
    } else if (input > 0.2r) {
        output = input * 1.2r;
    } else if (input > -0.2r) {
        output = input * 1.5r;
    } else if (input > -0.5r) {
        output = input * 1.8r;
    } else {
        output = input * 2.0r;
    }
    
    /* Additional complex expression */
    output = (output * output) + (output * 0.3r);
    
    return output;
}

/* Function 6: Nested loops with fixed-point induction */
static sat_accum_t nested_loop_test(int outer, int inner) {
    sat_accum_t total = 0.0k;
    
    for (int i = 0; i < outer; i++) {
        sat_accum_t outer_term = (sat_accum_t)i * 0.1k;
        
        for (int j = 0; j < inner; j++) {
            /* Complex expression with multiple operations */
            sat_accum_t inner_term = (sat_accum_t)j * 0.05k;
            
            sat_accum_t product = outer_term * inner_term;
            
            /* Shift that requires range analysis */
            sat_accum_t shifted = product >> (j % 4);
            
            /* Conditional addition */
            if (shifted > 0.2k) {
                total = total + shifted * 0.8k;
            } else {
                total = total - shifted * 0.8k;
            }
        }
        
        /* Update with potential saturation */
        total = total * 0.95k;
    }
    
    return total;
}

/* Function 7: Mixed saturation modes */
static void mixed_saturation_test(fract_t input, sat_fract_t* out1, accum_t* out2) {
    /* Convert between saturated and unsaturated */
    sat_fract_t sat_val = input;
    
    /* Operations designed to hit boundaries */
    sat_val = sat_val + 0.6r;
    sat_val = sat_val * 2.0r;  /* Likely saturate for many inputs */
    
    *out1 = sat_val;
    
    /* Convert to accum with different range */
    accum_t accum_val = (accum_t)sat_val * 0.5k;
    
    /* Complex expression requiring range analysis */
    accum_val = (accum_val * accum_val) >> 2;
    
    *out2 = accum_val;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 20;
    
    if (iterations < 1) iterations = 10;
    if (array_size < 5) array_size = 20;
    
    printf("Testing fixed-point range analysis (iterations=%d, array_size=%d)\n", 
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    fract_t* fract_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    for (int i = 0; i < array_size; i++) {
        /* Varying values to trigger different paths */
        fract_array[i] = (fract_t)((i % 10) - 5) * 0.1r;
    }
    
    /* Test 1: Saturation boundary tests */
    printf("\n=== Test 1: Saturation Boundaries ===\n");
    sat_fract_t max_fract = 0.999999r;
    sat_fract_t min_fract = -0.999999r;
    
    /* Operations designed to saturate */
    sat_fract_t test1 = max_fract + 0.1r;
    sat_fract_t test2 = min_fract - 0.1r;
    sat_fract_t test3 = max_fract * 2.0r;
    
    printf("max+0.1: %f\n", (float)test1);
    printf("min-0.1: %f\n", (float)test2);
    printf("max*2.0: %f\n", (float)test3);
    
    /* Test 2: Function calls with range analysis */
    printf("\n=== Test 2: Function Range Analysis ===\n");
    sat_fract_t func_result = sat_add_range(0.7r, 0.3r);  /* Should trigger >0.9r path */
    printf("sat_add_range(0.7, 0.3) = %f\n", (float)func_result);
    
    func_result = sat_add_range(-0.8r, -0.3r);  /* Should trigger <-0.9r path */
    printf("sat_add_range(-0.8, -0.3) = %f\n", (float)func_result);
    
    /* Test 3: Loop-based analysis */
    printf("\n=== Test 3: Loop Range Propagation ===\n");
    sat_accum_t loop_result = loop_range_analysis(iterations, 0.3r);
    printf("loop_range_analysis(%d, 0.3) = %f\n", iterations, (float)loop_result);
    
    /* Test 4: Array reduction */
    printf("\n=== Test 4: Array Reduction ===\n");
    sat_fract_t array_result = array_reduction(fract_array, array_size);
    printf("array_reduction(size=%d) = %f\n", array_size, (float)array_result);
    
    /* Test 5: Builtin overflow detection */
    printf("\n=== Test 5: Builtin Overflow ===\n");
    sat_accum_t overflow_res;
    int had_overflow = builtin_overflow_test(0.8k, 0.7k, &overflow_res);
    printf("builtin_overflow_test: result=%f, overflow=%d\n", 
           (float)overflow_res, had_overflow);
    
    /* Test 6: Switch statement */
    printf("\n=== Test 6: Switch/Conditional ===\n");
    fract_t switch_result = switch_range_test(0.9r);
    printf("switch_range_test(0.9) = %f\n", (float)switch_result);
    
    /* Test 7: Nested loops */
    printf("\n=== Test 7: Nested Loops ===\n");
    sat_accum_t nested_result = nested_loop_test(iterations/2, iterations);
    printf("nested_loop_test = %f\n", (float)nested_result);
    
    /* Test 8: Mixed saturation */
    printf("\n=== Test 8: Mixed Saturation ===\n");
    sat_fract_t mixed_out1;
    accum_t mixed_out2;
    mixed_saturation_test(0.8r, &mixed_out1, &mixed_out2);
    printf("mixed_saturation_test: out1=%f, out2=%f\n", 
           (float)mixed_out1, (float)mixed_out2);
    
    /* Final checksum to prevent dead code elimination */
    printf("\n=== Final Checksum ===\n");
    sat_accum_t checksum = 0.0k;
    
    /* Combine all results */
    checksum = checksum + (sat_accum_t)test1;
    checksum = checksum + (sat_accum_t)test2;
    checksum = checksum + (sat_accum_t)test3;
    checksum = checksum + (sat_accum_t)func_result;
    checksum = checksum + loop_result;
    checksum = checksum + (sat_accum_t)array_result;
    checksum = checksum + overflow_res;
    checksum = checksum + (sat_accum_t)switch_result;
    checksum = checksum + nested_result;
    checksum = checksum + (sat_accum_t)mixed_out1;
    checksum = checksum + mixed_out2;
    
    /* Complex final expression requiring range analysis */
    checksum = (checksum * 0.5k) + (checksum >> 2);
    
    printf("Final checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(fract_array);
    
    return 0;
}
