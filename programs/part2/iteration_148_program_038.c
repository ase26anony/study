/* test_fixed_point.c - Comprehensive fixed-point test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;
typedef short _Fract hfract_t;
typedef _Sat short _Fract shfract_t;

/* Test functions with different fixed-point operations */

/* Function 1: Range analysis with saturated addition */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b, fract_t c) {
    /* Complex expression that requires range analysis */
    sfract_t result = a + b;
    /* This may trigger saturation logic */
    result = result + c;
    /* Conditional based on range */
    if (result > 0.9r) {
        return result * 0.5r;
    }
    return result * 1.5r;
}

/* Function 2: Multiplication with overflow checking */
static inline saccum_t mul_with_overflow(saccum_t x, saccum_t y) {
    saccum_t prod = x * y;
    /* This should trigger range analysis for multiplication */
    if (prod < -0.5k || prod > 0.5k) {
        return prod >> 2;
    }
    return prod << 1;
}

/* Function 3: Division and shift operations */
static inline accum_t div_and_shift(accum_t a, accum_t b, int shift) {
    accum_t result = a / b;
    /* Shift operation requiring range analysis */
    if (shift > 0) {
        result = result >> shift;
    } else {
        result = result << (-shift);
    }
    return result;
}

/* Function 4: Loop-based range propagation */
static sfract_t loop_range_propagation(sfract_t start, int iterations) {
    sfract_t total = 0.0r;
    for (sfract_t f = start; f < 0.9r && iterations > 0; f += 0.1r, iterations--) {
        total = total + f;
        /* Complex expression inside loop */
        total = total * 0.95r;
    }
    return total;
}

/* Function 5: Array reduction with mixed types */
static saccum_t array_reduction(const sfract_t* arr, int size) {
    saccum_t sum = 0.0k;
    for (int i = 0; i < size; i++) {
        /* Mix saturated and unsaturated operations */
        accum_t temp = (accum_t)arr[i];
        sum = sum + (saccum_t)(temp * 2.0k);
        
        /* Conditional that depends on range */
        if (sum > 0.8k) {
            sum = sum - 0.3k;
        }
    }
    return sum;
}

/* Function 6: Ternary operator with fixed-point */
static fract_t ternary_range(fract_t a, fract_t b, int flag) {
    /* Both branches have different range implications */
    return flag ? (a * b) : (a / b);
}

/* Function 7: Switch statement with fixed-point conditions */
static int switch_fixed_point(fract_t value) {
    int result = 0;
    switch ((int)(value * 10.0r)) {
        case 0: result = 1; break;
        case 1: result = 2; break;
        case 2: result = 3; break;
        case 3: result = 4; break;
        default: result = 5; break;
    }
    return result;
}

/* Function 8: Builtin overflow checks */
static int builtin_overflow_test(saccum_t* result, saccum_t a, saccum_t b) {
    /* Use GCC builtins for overflow detection */
    return __builtin_add_overflow(a, b, result);
}

/* Function 9: Complex nested expressions */
static saccum_t complex_nested(saccum_t a, saccum_t b, saccum_t c) {
    /* Expression designed to trigger complex range analysis */
    return ((a + b) * c) >> 3 + ((a - b) / c) << 2;
}

/* Function 10: Boundary value testing */
static sfract_t boundary_test(sfract_t val) {
    /* Operations designed to hit saturation boundaries */
    sfract_t max_val = 0.999999r;
    sfract_t min_val = -0.999999r;
    
    /* These may trigger the uncovered saturation logic */
    sfract_t test1 = val + max_val;
    sfract_t test2 = val + min_val;
    sfract_t test3 = test1 * test2;
    
    return test3;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    int iterations = 10;
    int array_size = 20;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 20;
    }
    
    /* Initialize fixed-point arrays */
    sfract_t* sfract_arr = (sfract_t*)malloc(array_size * sizeof(sfract_t));
    fract_t* fract_arr = (fract_t*)malloc(array_size * sizeof(fract_t));
    
    if (!sfract_arr || !fract_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with values that will trigger range analysis */
    for (int i = 0; i < array_size; i++) {
        fract_arr[i] = (fract_t)((i % 10) * 0.1r);
        sfract_arr[i] = (sfract_t)((i % 7) * 0.15r - 0.5r);
    }
    
    /* Test 1: Saturated addition with range analysis */
    sfract_t sat_result = 0.0r;
    for (int i = 0; i < iterations; i++) {
        sat_result = sat_add_range(sfract_arr[i % array_size], 
                                  0.7r, 
                                  fract_arr[(i + 1) % array_size]);
    }
    
    /* Test 2: Multiplication overflow */
    saccum_t mul_result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        mul_result = mul_with_overflow(0.8k, -0.6k);
        mul_result = mul_with_overflow(mul_result, 0.9k);
    }
    
    /* Test 3: Division and shifts */
    accum_t div_result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        div_result = div_and_shift(1.0k, 0.5k + (i % 3) * 0.1k, i % 5);
    }
    
    /* Test 4: Loop range propagation */
    sfract_t loop_result = loop_range_propagation(0.1r, iterations);
    
    /* Test 5: Array reduction */
    saccum_t reduce_result = array_reduction(sfract_arr, array_size);
    
    /* Test 6: Ternary operator */
    fract_t ternary_result = 0.0r;
    for (int i = 0; i < iterations; i++) {
        ternary_result = ternary_range(fract_arr[i % array_size], 
                                      0.5r, 
                                      i % 2);
    }
    
    /* Test 7: Switch statement */
    int switch_result = 0;
    for (int i = 0; i < iterations; i++) {
        switch_result += switch_fixed_point(fract_arr[i % array_size]);
    }
    
    /* Test 8: Builtin overflow */
    saccum_t builtin_result;
    int overflow_flag = 0;
    for (int i = 0; i < iterations; i++) {
        overflow_flag |= builtin_overflow_test(&builtin_result, 
                                              0.9k, 
                                              0.2k * (i % 4));
    }
    
    /* Test 9: Complex nested expressions */
    saccum_t nested_result = complex_nested(0.7k, -0.3k, 0.5k);
    
    /* Test 10: Boundary testing */
    sfract_t boundary_result = boundary_test(0.999r);
    
    /* Final checksum calculation to prevent dead code elimination */
    accum_t checksum = (accum_t)sat_result 
                     + (accum_t)mul_result 
                     + div_result 
                     + (accum_t)loop_result 
                     + reduce_result 
                     + (accum_t)ternary_result 
                     + (accum_t)switch_result 
                     + (accum_t)builtin_result 
                     + nested_result 
                     + (accum_t)boundary_result;
    
    /* Print results (converted to float) */
    printf("Test Results:\n");
    printf("  Saturated addition: %f\n", (float)sat_result);
    printf("  Multiplication: %f\n", (float)mul_result);
    printf("  Division/shift: %f\n", (float)div_result);
    printf("  Loop propagation: %f\n", (float)loop_result);
    printf("  Array reduction: %f\n", (float)reduce_result);
    printf("  Ternary operator: %f\n", (float)ternary_result);
    printf("  Switch result: %d\n", switch_result);
    printf("  Overflow flag: %d\n", overflow_flag);
    printf("  Nested expression: %f\n", (float)nested_result);
    printf("  Boundary test: %f\n", (float)boundary_result);
    printf("  Final checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(sfract_arr);
    free(fract_arr);
    
    return 0;
}
