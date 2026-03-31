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

/* Test functions with various fixed-point operations */

/* Function 1: Complex saturated addition with overflow */
static inline sfract_t sat_add_complex(sfract_t a, sfract_t b, sfract_t c) {
    /* This should trigger range analysis for saturation */
    sfract_t sum = a + b;
    /* Conditional based on sum range */
    if (sum > 0.8r) {
        return sum + c;  /* Potential overflow */
    } else {
        return sum - c;  /* Potential underflow */
    }
}

/* Function 2: Multiplication with shift operation */
static inline accum_t mul_shift(accum_t x, int shift) {
    /* Complex expression requiring range analysis */
    accum_t result = x * 2.0k;
    result = result >> shift;
    return result;
}

/* Function 3: Nested saturation operations */
static inline saccum_t nested_saturation(saccum_t base, fract_t multiplier) {
    /* Multiple operations that could saturate */
    saccum_t temp = base * (saccum_t)multiplier;
    temp = temp + 0.5k;
    temp = temp >> 2;
    return temp;
}

/* Function 4: Loop-based range propagation */
static sfract_t loop_range_propagation(int iterations, sfract_t init) {
    sfract_t result = init;
    for (int i = 0; i < iterations; i++) {
        /* Loop variable affects fixed-point calculation */
        fract_t increment = (fract_t)i * 0.01r;
        result = result + (sfract_t)increment;
        
        /* Conditional that depends on accumulated range */
        if (result > 0.9r) {
            result = result - 0.2r;
        }
    }
    return result;
}

/* Function 5: Array reduction with mixed types */
static saccum_t array_reduction(const fract_t* arr, int size) {
    saccum_t sum = 0.0k;
    for (int i = 0; i < size; i++) {
        /* Mixed type operations requiring range analysis */
        saccum_t converted = (saccum_t)arr[i];
        sum = sum + converted * 1.5k;
        
        /* Ternary operator with fixed-point operands */
        sum = (sum > 10.0k) ? 10.0k : sum;
        sum = (sum < -10.0k) ? -10.0k : sum;
    }
    return sum;
}

/* Function 6: Built-in overflow checks */
static int check_overflow(saccum_t* result, saccum_t a, saccum_t b) {
    /* Use builtin for overflow detection */
    return __builtin_add_overflow(a, b, result);
}

/* Function 7: Switch based on fixed-point comparison */
static int fixed_point_switch(sfract_t value) {
    int code = 0;
    /* Switch where cases depend on fixed-point ranges */
    if (value < 0.2r) {
        code = 1;
    } else if (value >= 0.2r && value < 0.5r) {
        code = 2;
    } else if (value >= 0.5r && value < 0.8r) {
        code = 3;
    } else {
        code = 4;  /* This includes saturation case */
    }
    return code;
}

/* Function 8: Complex expression with multiple operations */
static sfract_t complex_expression(sfract_t a, sfract_t b, sfract_t c) {
    /* Expression designed to trigger range analysis */
    sfract_t temp = (a + b) * c;
    temp = temp >> 1;
    temp = temp + 0.1r;
    
    /* This comparison should trigger the uncovered code */
    if (temp > 0.95r || temp < -0.95r) {
        return 0.0r;
    }
    return temp;
}

/* Function 9: Bit-shift operations causing underflow/overflow */
static accum_t shift_operations(accum_t value, int shift_amt) {
    /* Various shift operations */
    accum_t result = value;
    
    /* Left shift - potential overflow */
    result = result << shift_amt;
    
    /* Right shift - potential underflow */
    result = result >> (shift_amt / 2);
    
    /* Arithmetic shift */
    result = result << 1;
    result = result >> 2;
    
    return result;
}

/* Function 10: Mixed saturation conversions */
static sfract_t mixed_saturation_conversions(fract_t a, saccum_t b) {
    /* Mix saturated and unsaturated types */
    sfract_t temp1 = (sfract_t)a;
    sfract_t temp2 = (sfract_t)(b * 0.1k);
    
    /* Operation that should trigger saturation logic */
    sfract_t result = temp1 + temp2;
    
    /* Force range analysis with conditional */
    if (result == 1.0r || result == -1.0r) {
        /* At saturation boundaries */
        return result * 0.5r;
    }
    return result;
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
    
    printf("Starting fixed-point tests (iterations=%d, array_size=%d)\n", 
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    fract_t* frac_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    for (int i = 0; i < array_size; i++) {
        frac_array[i] = (fract_t)(i * 0.05r);
    }
    
    /* Test 1: Saturated addition with overflow boundaries */
    printf("\nTest 1: Saturated addition boundaries\n");
    sfract_t max_fract = 0.999999r;
    sfract_t overflow_test = max_fract + 0.1r;  /* Should saturate */
    printf("  Max + 0.1 = %f (as float)\n", (float)overflow_test);
    
    /* Test 2: Complex expressions */
    printf("\nTest 2: Complex expressions\n");
    sfract_t a = 0.7r;
    sfract_t b = 0.3r;
    sfract_t c = 0.5r;
    sfract_t complex_result = complex_expression(a, b, c);
    printf("  Complex expression result = %f\n", (float)complex_result);
    
    /* Test 3: Loop-based range propagation */
    printf("\nTest 3: Loop range propagation\n");
    sfract_t loop_result = loop_range_propagation(iterations, 0.1r);
    printf("  Loop result after %d iterations = %f\n", 
           iterations, (float)loop_result);
    
    /* Test 4: Array reduction */
    printf("\nTest 4: Array reduction\n");
    saccum_t reduction_result = array_reduction(frac_array, array_size);
    printf("  Array reduction result = %f\n", (float)reduction_result);
    
    /* Test 5: Built-in overflow checks */
    printf("\nTest 5: Built-in overflow detection\n");
    saccum_t ovf_result;
    saccum_t val1 = 5.0k;
    saccum_t val2 = 10.0k;
    int has_overflow = check_overflow(&ovf_result, val1, val2);
    printf("  Overflow check: %d, result = %f\n", 
           has_overflow, (float)ovf_result);
    
    /* Test 6: Switch based on fixed-point ranges */
    printf("\nTest 6: Fixed-point switch\n");
    for (int i = 0; i < 5; i++) {
        sfract_t test_val = (sfract_t)(i * 0.25r);
        int switch_code = fixed_point_switch(test_val);
        printf("  Value %f -> code %d\n", (float)test_val, switch_code);
    }
    
    /* Test 7: Multiplication with shifts */
    printf("\nTest 7: Multiplication with shifts\n");
    accum_t shift_test = mul_shift(1.0k, 3);
    printf("  Shift test result = %f\n", (float)shift_test);
    
    /* Test 8: Nested saturation */
    printf("\nTest 8: Nested saturation operations\n");
    saccum_t nested_result = nested_saturation(2.0k, 0.8r);
    printf("  Nested saturation result = %f\n", (float)nested_result);
    
    /* Test 9: Shift operations */
    printf("\nTest 9: Shift operations\n");
    accum_t shift_op_result = shift_operations(0.5k, 4);
    printf("  Shift operations result = %f\n", (float)shift_op_result);
    
    /* Test 10: Mixed saturation conversions */
    printf("\nTest 10: Mixed saturation conversions\n");
    sfract_t mixed_result = mixed_saturation_conversions(0.8r, 5.0k);
    printf("  Mixed conversion result = %f\n", (float)mixed_result);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    accum_t checksum = 0.0k;
    
    /* Include all results in checksum */
    checksum += (accum_t)overflow_test;
    checksum += (accum_t)complex_result;
    checksum += (accum_t)loop_result;
    checksum += reduction_result;
    checksum += ovf_result;
    checksum += (accum_t)shift_test;
    checksum += nested_result;
    checksum += shift_op_result;
    checksum += (accum_t)mixed_result;
    
    printf("  Final checksum = %f\n", (float)checksum);
    
    /* Cleanup */
    free(frac_array);
    
    return 0;
}
