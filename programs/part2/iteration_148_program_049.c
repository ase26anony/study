/* test_fixed_point.c - Target coverage for fixed-value.cc lines 264-277 */
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

/* Test functions with various fixed-point operations */

/* Function 1: Complex saturated addition with range analysis */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b, sfract_t c) {
    /* This should trigger range analysis for saturation boundaries */
    sfract_t sum = a + b;
    /* Conditional based on range analysis */
    if (sum > c) {
        return sum - c;
    } else {
        return sum + c;
    }
}

/* Function 2: Multiplication with shift causing overflow analysis */
static inline saccum_t mul_shift_overflow(saccum_t x, int shift) {
    /* Multiplication followed by shift - needs precise range analysis */
    saccum_t scaled = x * 2.0k;
    /* Shift operation that could overflow/underflow */
    if (shift > 0) {
        return scaled >> shift;
    } else {
        return scaled << (-shift);
    }
}

/* Function 3: Division with saturation boundary checks */
static inline sfract_t div_sat_check(sfract_t a, sfract_t b) {
    /* Division near saturation boundaries */
    sfract_t result = a / b;
    
    /* This condition should trigger the uncovered range comparison */
    if (result > 0.999r || result < -0.999r) {
        return (result > 0) ? 0.999r : -0.999r;
    }
    return result;
}

/* Function 4: Loop-based accumulation with range propagation */
static accum_t loop_accumulation(int iterations, fract_t base) {
    accum_t total = 0.0k;
    fract_t increment = 0.1r;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        fract_t scaled = base * (fract_t)i;
        total = total + (accum_t)scaled;
        
        /* Conditional that depends on accumulated range */
        if (total > 10.0k) {
            total = total - 5.0k;
        }
    }
    return total;
}

/* Function 5: Array reduction with mixed saturation */
static sfract_t array_reduction_sat(sfract_t arr[], int size) {
    sfract_t sum = 0.0r;
    sfract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Operations that can saturate */
        sum = sum + arr[i];
        product = product * arr[i];
        
        /* Ternary with fixed-point operands */
        sum = (sum > 0.5r) ? sum * 0.5r : sum * 2.0r;
    }
    
    /* Final expression requiring range analysis */
    return (sum + product) / 2.0r;
}

/* Function 6: Using builtins for overflow detection */
static int builtin_overflow_test(saccum_t a, saccum_t b, saccum_t *result) {
    /* Use overflow builtins with fixed-point */
    int overflow = 0;
    
    /* Multiplication overflow check */
    overflow |= __builtin_mul_overflow(a, b, result);
    
    /* Addition overflow check */
    saccum_t temp;
    overflow |= __builtin_add_overflow(*result, a, &temp);
    *result = temp;
    
    return overflow;
}

/* Function 7: Switch statement based on fixed-point ranges */
static int switch_fixed_range(sfract_t value) {
    /* Switch on discretized fixed-point ranges */
    int category = 0;
    
    if (value < -0.5r) category = 1;
    else if (value < 0.0r) category = 2;
    else if (value < 0.5r) category = 3;
    else category = 4;
    
    switch (category) {
        case 1:
            return -1;
        case 2:
            return (value * 2.0r) > -0.5r ? 0 : -1;
        case 3:
            return (value + 0.5r) < 0.9r ? 1 : 2;
        case 4:
            /* This should trigger saturation boundary checks */
            sfract_t saturated = value + 0.1r;
            return saturated == 0.999r ? 3 : 4;
        default:
            return 0;
    }
}

/* Function 8: Nested function calls with fixed-point propagation */
static sfract_t nested_range_prop(sfract_t a, sfract_t b, sfract_t c) {
    /* Multiple operations that require chained range analysis */
    sfract_t t1 = a * b;
    sfract_t t2 = b * c;
    
    /* Conditional that depends on intermediate ranges */
    if (t1 > t2) {
        return sat_add_range(t1, t2, 0.5r);
    } else {
        return div_sat_check(t1, t2);
    }
}

/* Function 9: Bit-shift operations on fixed-point */
static accum_t shift_fixed_point(accum_t value, int shift1, int shift2) {
    /* Multiple shifts requiring range analysis */
    accum_t shifted = value >> shift1;
    
    /* Conditional shift */
    if (shifted > 0.0k) {
        shifted = shifted << shift2;
    } else {
        shifted = shifted >> shift2;
    }
    
    /* Final range-dependent operation */
    return (shifted * 2.0k) / 3.0k;
}

/* Function 10: Mixed-type operations triggering conversions */
static accum_t mixed_type_ops(fract_t f, accum_t a, hfract_t h) {
    /* Operations with different fixed-point types */
    accum_t result = (accum_t)f + a;
    
    /* Conversion with potential range issues */
    result = result * (accum_t)h;
    
    /* Check for overflow in mixed-type expression */
    if (result > (accum_t)10.0r * a) {
        return a;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Use command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int array_size = (argc > 2) ? atoi(argv[2]) : 50;
    float init_val = (argc > 3) ? atof(argv[3]) : 0.5;
    
    printf("Testing fixed-point range analysis (iterations=%d, array_size=%d)\n", 
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    sfract_t sat_array[100];
    fract_t unsat_array[100];
    
    for (int i = 0; i < array_size && i < 100; i++) {
        float val = init_val * (i + 1) / array_size;
        sat_array[i] = (sfract_t)val;
        unsat_array[i] = (fract_t)val;
    }
    
    /* Test 1: Saturated addition with range checks */
    sfract_t test1_a = 0.8r;
    sfract_t test1_b = 0.3r;
    sfract_t test1_c = 0.9r;
    sfract_t result1 = sat_add_range(test1_a, test1_b, test1_c);
    printf("Test 1 result: %f\n", (float)result1);
    
    /* Test 2: Multiplication with overflow analysis */
    saccum_t test2_val = 0.7k;
    int test2_shift = 3;
    saccum_t result2 = mul_shift_overflow(test2_val, test2_shift);
    printf("Test 2 result: %f\n", (float)result2);
    
    /* Test 3: Division near saturation boundaries */
    sfract_t test3_a = 0.99r;
    sfract_t test3_b = 0.5r;
    sfract_t result3 = div_sat_check(test3_a, test3_b);
    printf("Test 3 result: %f\n", (float)result3);
    
    /* Test 4: Loop accumulation */
    fract_t test4_base = 0.05r;
    accum_t result4 = loop_accumulation(iterations, test4_base);
    printf("Test 4 result: %f\n", (float)result4);
    
    /* Test 5: Array reduction */
    sfract_t result5 = array_reduction_sat(sat_array, 
                                          (array_size < 100) ? array_size : 100);
    printf("Test 5 result: %f\n", (float)result5);
    
    /* Test 6: Builtin overflow detection */
    saccum_t test6_a = 0.8k;
    saccum_t test6_b = 1.2k;
    saccum_t result6;
    int overflow = builtin_overflow_test(test6_a, test6_b, &result6);
    printf("Test 6 result: %f (overflow: %d)\n", (float)result6, overflow);
    
    /* Test 7: Switch based on fixed-point ranges */
    sfract_t test7_vals[] = {-0.7r, -0.3r, 0.3r, 0.8r};
    for (int i = 0; i < 4; i++) {
        int category = switch_fixed_range(test7_vals[i]);
        printf("Test 7[%d]: value=%f -> category=%d\n", 
               i, (float)test7_vals[i], category);
    }
    
    /* Test 8: Nested range propagation */
    sfract_t result8 = nested_range_prop(0.6r, 0.7r, 0.8r);
    printf("Test 8 result: %f\n", (float)result8);
    
    /* Test 9: Shift operations */
    accum_t test9_val = 0.25k;
    accum_t result9 = shift_fixed_point(test9_val, 2, 1);
    printf("Test 9 result: %f\n", (float)result9);
    
    /* Test 10: Mixed-type operations */
    fract_t test10_f = 0.4r;
    accum_t test10_a = 0.6k;
    hfract_t test10_h = 0.8r;
    accum_t result10 = mixed_type_ops(test10_f, test10_a, test10_h);
    printf("Test 10 result: %f\n", (float)result10);
    
    /* Final checksum to prevent dead code elimination */
    accum_t checksum = (accum_t)result1 + (accum_t)result2 + (accum_t)result3 +
                      result4 + (accum_t)result5 + result6 + 
                      (accum_t)result8 + result9 + result10;
    
    printf("Final checksum: %f\n", (float)checksum);
    
    /* Additional stress test: operations designed to hit saturation boundaries */
    printf("\nSaturation boundary tests:\n");
    
    /* Should saturate at maximum */
    sfract_t max_sat = 0.999999r;
    sfract_t max_plus = max_sat + 0.1r;
    printf("Max saturation test: %f + 0.1 = %f\n", 
           (float)max_sat, (float)max_plus);
    
    /* Should saturate at minimum */
    sfract_t min_sat = -0.999999r;
    sfract_t min_minus = min_sat - 0.1r;
    printf("Min saturation test: %f - 0.1 = %f\n", 
           (float)min_sat, (float)min_minus);
    
    /* Multiplication causing overflow */
    saccum_t large_accum = 0.9k;
    saccum_t scaled = large_accum * 2.0k;
    printf("Multiplication overflow test: %f * 2.0 = %f\n",
           (float)large_accum, (float)scaled);
    
    /* Division causing underflow */
    sfract_t small_fract = 0.001r;
    sfract_t divided = small_fract / 10.0r;
    printf("Division underflow test: %f / 10.0 = %f\n",
           (float)small_fract, (float)divided);
    
    return 0;
}
