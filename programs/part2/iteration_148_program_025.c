/* test_fixed_point.c - Target coverage of fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef _Sat short _Fract sat_short_fract_t;

/* Test functions with different fixed-point operations */

/* Function 1: Complex fixed-point arithmetic with saturation */
static inline sat_fract_t complex_sat_operation(sat_fract_t a, sat_fract_t b, fract_t c) {
    /* This should trigger range analysis for saturation */
    sat_fract_t result = a + b;
    result = result * c;
    
    /* Force conditional based on range analysis */
    if (result > 0.8r) {
        result = result >> 2;  /* Right shift may cause underflow */
    } else {
        result = result << 1;  /* Left shift may cause overflow */
    }
    
    return result;
}

/* Function 2: Accumulator with loop-based range propagation */
static sat_accum_t accumulate_range(int iterations, sat_accum_t base) {
    sat_accum_t total = 0.0k;
    
    /* Loop where induction variable affects fixed-point range */
    for (int i = 0; i < iterations; i++) {
        sat_accum_t increment = base * (sat_accum_t)i;
        
        /* Operation designed to hit saturation boundaries */
        if (i % 2 == 0) {
            total = total + increment;
        } else {
            total = total - increment;
        }
        
        /* Nested conditional to force range analysis */
        if (total > 0.9k || total < -0.9k) {
            total = total / 2.0k;
        }
    }
    
    return total;
}

/* Function 3: Mixed saturation types causing conversions */
static sat_accum_t mixed_saturation_ops(sat_fract_t f1, sat_accum_t a1, fract_t f2) {
    /* Mix saturated and unsaturated types */
    sat_accum_t temp = (sat_accum_t)f1 * a1;
    
    /* Use builtins for overflow detection */
    int overflow = 0;
    sat_accum_t result = __builtin_add_overflow(temp, (sat_accum_t)f2, &result) ? 
                         (temp > 0 ? 0.999999k : -0.999999k) : result;
    
    /* Bit shift operation that requires precise range analysis */
    if (result > 0.5k) {
        result = result >> 3;
    } else if (result < -0.5k) {
        result = result << 2;
    }
    
    return result;
}

/* Function 4: Array reduction with fixed-point values */
static sat_fract_t array_reduction(const sat_fract_t* arr, int size) {
    sat_fract_t sum = 0.0r;
    sat_fract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Operations that can saturate in either direction */
        sum = sum + arr[i];
        product = product * arr[i];
        
        /* Ternary operator with fixed-point operands */
        sat_fract_t adjusted = (sum > 0.7r) ? (product / 2.0r) : (sum * 2.0r);
        
        /* Force range analysis at each iteration */
        if (adjusted > 0.9r) {
            adjusted = 0.9r;
        } else if (adjusted < -0.9r) {
            adjusted = -0.9r;
        }
        
        sum = adjusted;
    }
    
    return sum;
}

/* Function 5: Switch statement based on fixed-point comparisons */
static const char* range_category(sat_accum_t value) {
    /* Switch where cases depend on fixed-point range analysis */
    switch ((int)(value * 10.0k)) {
        case -10 ... -7:
            return "Very Negative";
        case -6 ... -3:
            return "Negative";
        case -2 ... 2:
            return "Near Zero";
        case 3 ... 6:
            return "Positive";
        case 7 ... 10:
            return "Very Positive";
        default:
            /* This should trigger saturation boundary analysis */
            return (value > 0) ? "Saturated Positive" : "Saturated Negative";
    }
}

/* Function 6: Complex expression requiring multi-step range analysis */
static sat_accum_t complex_range_expr(sat_accum_t a, sat_accum_t b, sat_accum_t c) {
    /* Expression designed to exercise the uncovered code path */
    sat_accum_t expr1 = (a * b) >> 4;
    sat_accum_t expr2 = (b * c) << 2;
    sat_accum_t expr3 = (c * a) >> 1;
    
    /* Combined expression that may overflow/underflow */
    sat_accum_t result = expr1 + expr2 - expr3;
    
    /* Conditional that directly matches the uncovered code logic */
    if (result > 0.8k || (result == 0.8k && (expr1 > 0.4k))) {
        result = result / 2.0k;
    }
    
    return result;
}

/* Function 7: Using asm to create hard-to-analyze value flows */
static sat_fract_t asm_fixed_point(sat_fract_t a, sat_fract_t b) {
    sat_fract_t result;
    
    /* Inline asm with fixed-point constraints */
    asm volatile (
        "/* Fixed-point operation with hard-to-analyze flow */"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Follow up with operations that require range analysis */
    result = result * 1.5r;
    
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Use command-line arguments for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 20;
    
    if (iterations < 1) iterations = 1;
    if (array_size < 5) array_size = 5;
    if (array_size > 100) array_size = 100;
    
    printf("Testing fixed-point range analysis with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    sat_fract_t* fract_array = (sat_fract_t*)malloc(array_size * sizeof(sat_fract_t));
    sat_accum_t* accum_array = (sat_accum_t*)malloc(array_size * sizeof(sat_accum_t));
    
    /* Fill arrays with values designed to trigger edge cases */
    for (int i = 0; i < array_size; i++) {
        /* Mix of values near saturation boundaries */
        fract_array[i] = (sat_fract_t)((i % 10) * 0.1r);
        accum_array[i] = (sat_accum_t)(((i % 7) - 3) * 0.3k);
    }
    
    /* Force some boundary values */
    fract_array[0] = 0.999999r;  /* Near max */
    fract_array[1] = -0.999999r; /* Near min */
    accum_array[0] = 0.999999k;
    accum_array[1] = -0.999999k;
    
    /* Test 1: Complex saturation operations */
    sat_fract_t test1_result = 0.0r;
    for (int i = 0; i < iterations; i++) {
        sat_fract_t a = (i % 2 == 0) ? 0.7r : -0.7r;
        sat_fract_t b = (i % 3 == 0) ? 0.3r : -0.3r;
        fract_t c = 0.5r;
        
        test1_result = test1_result + complex_sat_operation(a, b, c);
    }
    
    /* Test 2: Accumulator with range propagation */
    sat_accum_t test2_result = accumulate_range(iterations, 0.2k);
    
    /* Test 3: Mixed saturation types */
    sat_accum_t test3_result = mixed_saturation_ops(0.8r, 0.6k, 0.4r);
    
    /* Test 4: Array reduction */
    sat_fract_t test4_result = array_reduction(fract_array, array_size);
    
    /* Test 5: Switch-based categorization */
    const char* category = range_category(test2_result);
    
    /* Test 6: Complex range expression */
    sat_accum_t test6_result = complex_range_expr(0.7k, 0.8k, -0.6k);
    
    /* Test 7: ASM-based operations */
    sat_fract_t test7_result = asm_fixed_point(0.5r, 0.3r);
    
    /* Final checksum calculation to prevent dead code elimination */
    sat_accum_t checksum = (sat_accum_t)test1_result + test2_result + test3_result 
                         + (sat_accum_t)test4_result + test6_result 
                         + (sat_accum_t)test7_result;
    
    /* Print results (converted to float) */
    printf("Results:\n");
    printf("  Test1 (complex sat): %f\n", (float)test1_result);
    printf("  Test2 (accumulator): %f\n", (float)test2_result);
    printf("  Test3 (mixed types): %f\n", (float)test3_result);
    printf("  Test4 (array reduction): %f\n", (float)test4_result);
    printf("  Test5 (category): %s\n", category);
    printf("  Test6 (complex expr): %f\n", (float)test6_result);
    printf("  Test7 (asm): %f\n", (float)test7_result);
    printf("  Final checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(fract_array);
    free(accum_array);
    
    return 0;
}
