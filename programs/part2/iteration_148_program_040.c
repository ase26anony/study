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

/* Test functions with complex fixed-point operations */

/* Function 1: Range analysis with saturated addition */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b, fract_t c) {
    /* This should trigger range analysis for saturation */
    sfract_t sum = a + b;
    
    /* Conditional that depends on range analysis */
    if (sum > 0.9r) {
        /* Force evaluation of max_r/max_s comparison */
        return sum * c;
    } else if (sum < -0.9r) {
        return sum / c;
    }
    
    /* Ternary with fixed-point operands */
    return (c > 0.5r) ? (sum + 0.1r) : (sum - 0.1r);
}

/* Function 2: Multiplication with overflow analysis */
static inline saccum_t mult_with_overflow(saccum_t x, saccum_t y, int shift) {
    /* Complex expression requiring range analysis */
    saccum_t prod = x * y;
    
    /* Shift operation that can cause overflow/underflow */
    if (shift > 0) {
        prod = prod >> shift;
    } else if (shift < 0) {
        prod = prod << (-shift);
    }
    
    /* Nested condition based on product range */
    if (prod > 10.0k) {
        return 10.0k;
    } else if (prod < -10.0k) {
        return -10.0k;
    }
    
    return prod;
}

/* Function 3: Loop-based range propagation */
static accum_t loop_range_propagation(int iterations, fract_t base) {
    accum_t total = 0.0k;
    fract_t step = 0.1r;
    
    /* Loop where induction variable affects fixed-point range */
    for (int i = 0; i < iterations; i++) {
        fract_t scaled = base * (fract_t)i;
        
        /* Condition that depends on accumulated range */
        if (scaled > 0.5r) {
            total += (accum_t)scaled * 2.0k;
        } else {
            total += (accum_t)scaled * 0.5k;
        }
        
        /* Modify base based on iteration */
        base = base + step;
        if (base > 0.9r) {
            base = 0.1r;
        }
    }
    
    return total;
}

/* Function 4: Array reduction with saturation */
static sfract_t array_saturation(sfract_t arr[], int size) {
    sfract_t sum = 0.0r;
    
    for (int i = 0; i < size; i++) {
        /* This addition can saturate */
        sum = sum + arr[i];
        
        /* Conditional that forces range evaluation */
        if (sum > 0.95r || sum < -0.95r) {
            /* Reset to middle of range */
            sum = 0.0r;
        }
    }
    
    return sum;
}

/* Function 5: Mixed-type operations */
static accum_t mixed_type_operations(hfract_t h, fract_t f, accum_t a) {
    /* Conversions between different fixed-point types */
    sfract_t s1 = (_Sat _Fract)h;
    fract_t f1 = (fract_t)h;
    
    /* Complex expression with shifts */
    accum_t result = (accum_t)s1 * a;
    result = result + (accum_t)f * 100.0k;
    
    /* Shift that requires precise range analysis */
    if (result > 1000.0k) {
        result = result >> 2;
    } else if (result < -1000.0k) {
        result = result << 1;
    }
    
    return result;
}

/* Function 6: Switch based on fixed-point comparison */
static int switch_fixed_point(saccum_t val) {
    int result = 0;
    
    /* Switch where cases depend on fixed-point range */
    switch ((val > 5.0k) ? 1 : (val < -5.0k) ? 2 : 0) {
        case 0:
            result = (int)(val * 10.0k);
            break;
        case 1:
            result = 100 + (int)((val - 5.0k) * 2.0k);
            break;
        case 2:
            result = -100 + (int)((val + 5.0k) * 2.0k);
            break;
    }
    
    return result;
}

/* Function 7: Using builtins for overflow detection */
static int builtin_overflow_test(accum_t a, accum_t b, accum_t *res) {
    /* Use builtin for overflow detection */
    int overflow = __builtin_mul_overflow(a, b, res);
    
    if (!overflow) {
        /* Try addition overflow */
        accum_t sum;
        overflow = __builtin_add_overflow(*res, a, &sum);
        if (!overflow) {
            *res = sum;
        }
    }
    
    return overflow;
}

/* Function 8: Complex nested expressions */
static saccum_t complex_nested_expr(saccum_t x, int n) {
    /* Very complex expression requiring deep range analysis */
    saccum_t result = x;
    
    for (int i = 0; i < n; i++) {
        /* Nested ternary operations */
        result = (result > 0.0k) ? 
                 ((result * 1.5k) >> 1) : 
                 ((result * 0.75k) << 1);
                 
        /* Additional conditional */
        if (i % 2 == 0) {
            result = result + (saccum_t)(i * 0.1k);
        } else {
            result = result - (saccum_t)(i * 0.05k);
        }
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = 10;
    int array_size = 20;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 5) array_size = 20;
    }
    
    /* Initialize arrays */
    sfract_t sat_array[array_size];
    fract_t unsat_array[array_size];
    
    for (int i = 0; i < array_size; i++) {
        fract_t val = (fract_t)i / (fract_t)array_size;
        sat_array[i] = (_Sat _Fract)val;
        unsat_array[i] = val;
    }
    
    /* Test 1: Saturated addition with range analysis */
    printf("Test 1: Saturated addition range analysis\n");
    sfract_t test1_a = 0.8r;
    sfract_t test1_b = 0.7r;  /* Sum > 1.0, should saturate */
    fract_t test1_c = 0.5r;
    
    sfract_t result1 = sat_add_range(test1_a, test1_b, test1_c);
    printf("  Result 1: %f\n", (float)result1);
    
    /* Test 2: Multiplication with overflow */
    printf("\nTest 2: Multiplication overflow analysis\n");
    saccum_t test2_x = 5.0k;
    saccum_t test2_y = 3.0k;
    int test2_shift = 1;
    
    saccum_t result2 = mult_with_overflow(test2_x, test2_y, test2_shift);
    printf("  Result 2: %f\n", (float)result2);
    
    /* Test 3: Loop-based range propagation */
    printf("\nTest 3: Loop range propagation\n");
    fract_t test3_base = 0.3r;
    accum_t result3 = loop_range_propagation(iterations, test3_base);
    printf("  Result 3: %f (iterations: %d)\n", (float)result3, iterations);
    
    /* Test 4: Array saturation */
    printf("\nTest 4: Array saturation test\n");
    sfract_t result4 = array_saturation(sat_array, array_size);
    printf("  Result 4: %f\n", (float)result4);
    
    /* Test 5: Mixed-type operations */
    printf("\nTest 5: Mixed-type operations\n");
    hfract_t test5_h = 0.5hr;
    fract_t test5_f = 0.7r;
    accum_t test5_a = 2.5k;
    
    accum_t result5 = mixed_type_operations(test5_h, test5_f, test5_a);
    printf("  Result 5: %f\n", (float)result5);
    
    /* Test 6: Switch based on fixed-point */
    printf("\nTest 6: Switch with fixed-point comparison\n");
    saccum_t test6_val = 7.5k;
    int result6 = switch_fixed_point(test6_val);
    printf("  Result 6: %d\n", result6);
    
    /* Test 7: Builtin overflow detection */
    printf("\nTest 7: Builtin overflow detection\n");
    accum_t test7_a = 100.0k;
    accum_t test7_b = 50.0k;
    accum_t test7_res;
    
    int overflow = builtin_overflow_test(test7_a, test7_b, &test7_res);
    printf("  Overflow: %d, Result: %f\n", overflow, (float)test7_res);
    
    /* Test 8: Complex nested expressions */
    printf("\nTest 8: Complex nested expressions\n");
    saccum_t test8_x = 1.0k;
    saccum_t result8 = complex_nested_expr(test8_x, iterations);
    printf("  Result 8: %f\n", (float)result8);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    accum_t checksum = (accum_t)result1 + (accum_t)result2 + result3 + 
                      (accum_t)result4 + result5 + (accum_t)result6 + 
                      test7_res + result8;
    
    printf("Total checksum: %f\n", (float)checksum);
    
    /* Additional edge case tests */
    printf("\nEdge case tests:\n");
    
    /* Test near saturation boundaries */
    sfract_t edge1 = 0.999999r;
    sfract_t edge2 = 0.000001r;
    sfract_t edge_sum = edge1 + edge2;  /* Should saturate */
    printf("Edge sum (0.999999 + 0.000001): %f\n", (float)edge_sum);
    
    /* Test negative saturation */
    sfract_t neg_edge1 = -0.999999r;
    sfract_t neg_edge2 = -0.000001r;
    sfract_t neg_edge_sum = neg_edge1 + neg_edge2;  /* Should saturate */
    printf("Negative edge sum: %f\n", (float)neg_edge_sum);
    
    /* Test shift operations */
    saccum_t shift_test = 0.5k;
    for (int i = 0; i < 5; i++) {
        shift_test = shift_test >> 1;
        printf("Shift test iteration %d: %f\n", i, (float)shift_test);
    }
    
    return 0;
}
