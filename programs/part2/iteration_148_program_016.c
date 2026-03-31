/* test_fixed_point.c - Comprehensive fixed-point test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef _Sat long _Fract sat_long_fract_t;

/* Test functions with various fixed-point operations */

/* Function 1: Basic saturation boundary tests */
static inline sat_fract_t test_saturation_boundary(sat_fract_t a, sat_fract_t b) {
    /* Operations that should trigger saturation analysis */
    sat_fract_t sum = a + b;
    sat_fract_t prod = a * b;
    
    /* Conditional based on range analysis */
    if (sum > 0.9r) {
        return sum * 0.5r;
    } else if (prod < 0.1r) {
        return prod + 0.3r;
    }
    
    /* Ternary with fixed-point operands */
    return (a > b) ? (a - b) : (b - a);
}

/* Function 2: Accumulator with shift operations */
static inline sat_accum_t test_accum_shifts(sat_accum_t x, int shift) {
    /* Shift operations requiring range analysis */
    sat_accum_t shifted;
    
    if (shift > 0) {
        shifted = x >> shift;
    } else {
        shifted = x << (-shift);
    }
    
    /* Multiplication that could overflow */
    sat_accum_t scaled = shifted * 2.0k;
    
    /* Nested conditional with fixed-point comparison */
    if (scaled > 10.0k || scaled < -10.0k) {
        return scaled * 0.5k;
    }
    
    return scaled;
}

/* Function 3: Loop-based range propagation */
static sat_accum_t test_loop_range(int iterations, fract_t base) {
    sat_accum_t total = 0.0k;
    
    /* Loop where induction variable affects fixed-point range */
    for (fract_t f = base; f < 0.9r; f += 0.1r) {
        total = total + (sat_accum_t)f;
        
        /* Conditional inside loop */
        if (total > 5.0k) {
            total = total * 0.8k;
        }
    }
    
    /* Additional iterations with variable step */
    for (int i = 0; i < iterations; i++) {
        fract_t increment = (fract_t)i * 0.01r;
        total = total + (sat_accum_t)increment;
    }
    
    return total;
}

/* Function 4: Array reduction with mixed types */
static sat_fract_t test_array_reduction(const fract_t* arr, int size) {
    sat_fract_t sum = 0.0r;
    sat_fract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Operations that require range tracking */
        sum = sum + arr[i];
        product = product * arr[i];
        
        /* Conditional based on accumulated values */
        if (sum > 0.8r || product < 0.2r) {
            /* Reset or adjust values */
            sum = sum * 0.5r;
            product = product * 2.0r;
        }
    }
    
    /* Final conditional with complex expression */
    return (sum + product > 1.5r) ? (sum * 0.7r) : (product * 1.3r);
}

/* Function 5: Using builtins for overflow detection */
static int test_builtin_overflow(sat_accum_t a, sat_accum_t b, sat_accum_t* result) {
    int overflow = 0;
    
    /* Use builtin for overflow detection */
    overflow |= __builtin_add_overflow(a, b, result);
    
    sat_accum_t temp;
    overflow |= __builtin_mul_overflow(*result, 3.0k, &temp);
    
    /* Shift operation that could underflow/overflow */
    *result = temp >> 2;
    
    return overflow;
}

/* Function 6: Switch statement with fixed-point conditions */
static fract_t test_switch_fixed(fract_t value) {
    fract_t result;
    
    /* Switch where cases depend on fixed-point comparisons */
    switch ((int)(value * 10.0r)) {
        case 0: result = value + 0.1r; break;
        case 1: result = value * 2.0r; break;
        case 2: result = value - 0.2r; break;
        case 3: result = value / 2.0r; break;
        default: result = 0.5r; break;
    }
    
    return result;
}

/* Function 7: Complex expression with multiple operations */
static sat_accum_t test_complex_expression(sat_accum_t a, sat_accum_t b, int shift) {
    /* Expression designed to trigger complex range analysis */
    sat_accum_t result = ((a + b) * 3.0k) >> shift;
    
    /* Nested conditional with multiple comparisons */
    if (a > 2.0k && b < -1.0k) {
        result = result * 0.25k;
    } else if (a < -2.0k || b > 1.0k) {
        result = result * 4.0k;
    }
    
    /* Additional operation that could saturate */
    result = result + (a * b);
    
    return result;
}

/* Function 8: Inline assembly with fixed-point constraints */
static sat_fract_t test_asm_fixed(sat_fract_t a, sat_fract_t b) {
    sat_fract_t result;
    
    /* Assembly that creates hard-to-analyze value flow */
    asm volatile (
        "/* Fixed-point assembly operation */"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Follow up with C operations for range analysis */
    result = result * 0.75r;
    
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Use command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 20;
    
    if (iterations < 1) iterations = 10;
    if (array_size < 5) array_size = 5;
    if (array_size > 100) array_size = 100;
    
    printf("Testing fixed-point operations with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    fract_t* fract_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    if (!fract_array) return 1;
    
    /* Fill array with values that will trigger various conditions */
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (fract_t)((i % 10) * 0.1r);
    }
    
    /* Test 1: Saturation boundary tests */
    sat_fract_t sat_result = 0.0r;
    for (int i = 0; i < iterations; i++) {
        sat_fract_t a = (sat_fract_t)((i % 5) * 0.25r);
        sat_fract_t b = (sat_fract_t)(0.8r - (i % 3) * 0.3r);
        sat_result = sat_result + test_saturation_boundary(a, b);
    }
    printf("Test 1 - Saturation result: %f\n", (float)sat_result);
    
    /* Test 2: Accumulator with shifts */
    sat_accum_t accum_result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        int shift = (i % 7) - 3;  /* Range -3 to 3 */
        sat_accum_t val = (sat_accum_t)((i % 4) * 2.5k - 3.0k);
        accum_result = accum_result + test_accum_shifts(val, shift);
    }
    printf("Test 2 - Accumulator result: %f\n", (float)accum_result);
    
    /* Test 3: Loop-based range propagation */
    fract_t base_value = (iterations % 2) ? 0.1r : 0.2r;
    sat_accum_t loop_result = test_loop_range(iterations, base_value);
    printf("Test 3 - Loop result: %f\n", (float)loop_result);
    
    /* Test 4: Array reduction */
    sat_fract_t array_result = test_array_reduction(fract_array, array_size);
    printf("Test 4 - Array reduction result: %f\n", (float)array_result);
    
    /* Test 5: Builtin overflow detection */
    sat_accum_t builtin_result;
    sat_accum_t a = 5.0k;
    sat_accum_t b = -3.0k;
    int overflow = test_builtin_overflow(a, b, &builtin_result);
    printf("Test 5 - Builtin overflow: %d, result: %f\n", overflow, (float)builtin_result);
    
    /* Test 6: Switch with fixed-point */
    fract_t switch_result = 0.0r;
    for (int i = 0; i < array_size; i++) {
        switch_result = switch_result + test_switch_fixed(fract_array[i]);
    }
    printf("Test 6 - Switch result: %f\n", (float)switch_result);
    
    /* Test 7: Complex expressions */
    sat_accum_t complex_result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        sat_accum_t x = (sat_accum_t)((i % 6) * 1.5k - 3.0k);
        sat_accum_t y = (sat_accum_t)((i % 4) * 2.0k - 3.0k);
        int shift = i % 5;
        complex_result = complex_result + test_complex_expression(x, y, shift);
    }
    printf("Test 7 - Complex expression result: %f\n", (float)complex_result);
    
    /* Test 8: Assembly with fixed-point */
    sat_fract_t asm_result = 0.0r;
    for (int i = 0; i < iterations && i < array_size; i++) {
        asm_result = asm_result + test_asm_fixed(fract_array[i], fract_array[(i + 1) % array_size]);
    }
    printf("Test 8 - Assembly result: %f\n", (float)asm_result);
    
    /* Final checksum calculation to prevent dead code elimination */
    sat_accum_t final_checksum = (sat_accum_t)sat_result 
                               + accum_result 
                               + loop_result 
                               + (sat_accum_t)array_result 
                               + builtin_result 
                               + (sat_accum_t)switch_result 
                               + complex_result 
                               + (sat_accum_t)asm_result;
    
    printf("Final checksum: %f\n", (float)final_checksum);
    
    /* Cleanup */
    free(fract_array);
    
    return 0;
}
