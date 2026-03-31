/* test_fixed_point.c - Comprehensive fixed-point test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;
typedef _Sat long _Fract slfract_t;
typedef long _Fract lfract_t;

/* Test functions with different fixed-point operations */

/* Function 1: Basic saturated addition with range analysis */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b) {
    /* This should trigger range analysis for saturated addition */
    sfract_t result = a + b;
    /* Conditional based on range analysis */
    if (result > 0.9r) {
        return result - 0.1r;
    }
    return result + 0.1r;
}

/* Function 2: Complex multiplication with shift */
static inline saccum_t complex_mul_shift(saccum_t x, int shift) {
    /* Multi-step operation requiring precise range tracking */
    saccum_t temp = x * 2.0k;
    temp = temp >> shift;
    temp = temp * 0.5k;
    
    /* Range-dependent conditional */
    if (temp < -0.5k || temp > 0.5k) {
        return temp * 0.8k;
    }
    return temp * 1.2k;
}

/* Function 3: Loop-based accumulation with saturation */
static sfract_t loop_accumulation(int iterations) {
    sfract_t total = 0.0r;
    fract_t step = 0.1r;
    
    for (fract_t f = 0.1r; f < 0.9r; f = f + step) {
        /* Mixed saturated/unsaturated operations */
        sfract_t saturated_f = f;
        total = total + saturated_f;
        
        /* Conditional that depends on accumulated range */
        if (total > 0.8r) {
            total = total - 0.05r;
        }
    }
    
    /* Additional iterations with variable step */
    for (int i = 0; i < iterations; i++) {
        fract_t var_step = (i % 3 == 0) ? 0.05r : 0.15r;
        total = total + var_step;
    }
    
    return total;
}

/* Function 4: Array reduction with overflow boundaries */
static saccum_t array_reduction(const saccum_t* arr, int size) {
    saccum_t product = 1.0k;
    saccum_t sum = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Operations designed to approach saturation */
        product = product * arr[i];
        sum = sum + arr[i];
        
        /* Range check that should trigger the uncovered code */
        if (product > 0.9k || sum > 0.9k) {
            product = product * 0.5k;
            sum = sum * 0.5k;
        }
    }
    
    /* Ternary operator with fixed-point operands */
    return (product > sum) ? product : sum;
}

/* Function 5: Switch statement with fixed-point conditions */
static fract_t fixed_switch(fract_t input) {
    fract_t result = 0.0r;
    
    /* Switch based on fixed-point comparisons */
    if (input < 0.25r) {
        result = input * 2.0r;
    } else if (input < 0.5r) {
        result = input * 1.5r;
    } else if (input < 0.75r) {
        result = input * 1.2r;
    } else {
        result = input * 0.8r;
    }
    
    return result;
}

/* Function 6: Built-in overflow checks */
static int check_overflow(saccum_t a, saccum_t b, saccum_t* res) {
    /* Use builtin for overflow detection */
    int overflow = 0;
    overflow |= __builtin_add_overflow(a, b, res);
    
    saccum_t temp;
    overflow |= __builtin_mul_overflow(*res, 2.0k, &temp);
    *res = temp;
    
    return overflow;
}

/* Function 7: Mixed-type conversions with range analysis */
static sfract_t mixed_conversions(fract_t f, accum_t a) {
    /* Convert between different fixed-point types */
    sfract_t sf = f;  /* Unsaturated to saturated */
    saccum_t sa = a;  /* Unsaturated to saturated */
    
    /* Mixed-type arithmetic */
    sfract_t result = sf + (sfract_t)(sa * 0.1k);
    
    /* Bit shift operation on fixed-point */
    int shift_amount = (result > 0.5r) ? 2 : 3;
    result = result >> shift_amount;
    
    return result;
}

/* Function 8: Nested loops with complex conditions */
static saccum_t nested_loop_analysis(int outer, int inner) {
    saccum_t matrix[10][10];
    saccum_t total = 0.0k;
    
    /* Initialize matrix with values near saturation boundaries */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = (i * 0.1k) * (j * 0.1k);
        }
    }
    
    /* Nested reduction that requires inter-iteration range analysis */
    for (int i = 0; i < outer && i < 10; i++) {
        saccum_t row_sum = 0.0k;
        for (int j = 0; j < inner && j < 10; j++) {
            row_sum = row_sum + matrix[i][j];
            
            /* Condition that depends on accumulating range */
            if (row_sum > 0.8k) {
                row_sum = row_sum * 0.9k;
            }
        }
        total = total + row_sum;
    }
    
    return total;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Use command-line arguments for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    int array_size = (argc > 2) ? atoi(argv[2]) : 8;
    
    if (iterations < 1) iterations = 1;
    if (array_size < 4) array_size = 4;
    if (array_size > 20) array_size = 20;
    
    printf("Running fixed-point tests with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize test arrays */
    saccum_t accum_array[20];
    fract_t fract_array[20];
    
    for (int i = 0; i < array_size; i++) {
        accum_array[i] = (i % 2 == 0) ? 0.3k : 0.7k;
        fract_array[i] = (i % 3 == 0) ? 0.2r : 0.6r;
    }
    
    /* Test 1: Basic saturated operations */
    sfract_t test1_a = 0.8r;
    sfract_t test1_b = 0.3r;
    sfract_t test1_result = sat_add_range(test1_a, test1_b);
    printf("Test 1 - Saturated add: %.6f\n", (float)test1_result);
    
    /* Test 2: Complex multiplication with shifts */
    saccum_t test2_base = 0.4k;
    saccum_t test2_result = complex_mul_shift(test2_base, 2);
    printf("Test 2 - Complex mul/shift: %.6f\n", (float)test2_result);
    
    /* Test 3: Loop accumulation */
    sfract_t test3_result = loop_accumulation(iterations);
    printf("Test 3 - Loop accumulation: %.6f\n", (float)test3_result);
    
    /* Test 4: Array reduction */
    saccum_t test4_result = array_reduction(accum_array, array_size);
    printf("Test 4 - Array reduction: %.6f\n", (float)test4_result);
    
    /* Test 5: Switch with fixed-point */
    fract_t test5_input = 0.35r;
    fract_t test5_result = fixed_switch(test5_input);
    printf("Test 5 - Fixed-point switch: %.6f\n", (float)test5_result);
    
    /* Test 6: Built-in overflow checks */
    saccum_t test6_res;
    int test6_overflow = check_overflow(0.7k, 0.3k, &test6_res);
    printf("Test 6 - Overflow check: result=%.6f, overflow=%d\n", 
           (float)test6_res, test6_overflow);
    
    /* Test 7: Mixed conversions */
    sfract_t test7_result = mixed_conversions(0.4r, 0.8k);
    printf("Test 7 - Mixed conversions: %.6f\n", (float)test7_result);
    
    /* Test 8: Nested loops */
    saccum_t test8_result = nested_loop_analysis(iterations, iterations);
    printf("Test 8 - Nested loops: %.6f\n", (float)test8_result);
    
    /* Final checksum to prevent dead code elimination */
    float checksum = 0.0f;
    checksum += (float)test1_result;
    checksum += (float)test2_result;
    checksum += (float)test3_result;
    checksum += (float)test4_result;
    checksum += (float)test5_result;
    checksum += (float)test6_res;
    checksum += (float)test7_result;
    checksum += (float)test8_result;
    
    printf("Final checksum: %.6f\n", checksum);
    
    /* Additional edge case: Direct saturation boundary test */
    sfract_t max_fract = 0.999999r;
    sfract_t saturated = max_fract + 0.1r;
    printf("Saturation test: %.6f + 0.1 = %.6f\n", 
           (float)max_fract, (float)saturated);
    
    /* Test with inline assembly to create hard-to-analyze flows */
    accum_t asm_input = 0.5k;
    accum_t asm_output;
    asm volatile (
        "/* Fixed-point operation in assembly */"
        : "=r" (asm_output)
        : "r" (asm_input)
    );
    printf("Assembly test result: %.6f\n", (float)asm_output);
    
    return 0;
}
