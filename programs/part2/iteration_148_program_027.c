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

/* Function 1: Complex saturated addition with range analysis */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b, sfract_t c) {
    /* This should trigger range analysis for saturation */
    sfract_t sum = a + b;
    /* Conditional based on range analysis */
    if (sum > 0.9r) {
        return sum + c;  /* May overflow */
    } else {
        return sum - c;  /* May underflow */
    }
}

/* Function 2: Multiplication with shift causing overflow analysis */
static inline saccum_t mul_shift_overflow(saccum_t x, int shift) {
    /* Multiplication followed by shift - requires precise range analysis */
    saccum_t result = x * 2.0k;
    result = result >> shift;  /* Shift operation on fixed-point */
    return result;
}

/* Function 3: Division with saturation boundary check */
static inline sfract_t div_saturation(sfract_t a, sfract_t b) {
    /* Division near saturation boundaries */
    sfract_t result = a / b;
    /* Ternary operator with fixed-point operands */
    return (result > 0.5r) ? (result * 2.0r) : (result / 2.0r);
}

/* Function 4: Loop-based accumulation with range propagation */
static accum_t loop_accumulation(fract_t* arr, int n) {
    accum_t total = 0.0k;
    for (int i = 0; i < n; i++) {
        /* Induction variable affects fixed-point calculation */
        fract_t multiplier = (fract_t)i / (fract_t)n;
        total = total + (accum_t)(arr[i] * multiplier);
    }
    return total;
}

/* Function 5: Array reduction with saturation */
static sfract_t array_reduction_sat(sfract_t* arr, int n) {
    sfract_t sum = 0.0r;
    for (int i = 0; i < n; i++) {
        sum = sum + arr[i];
        /* Conditional that depends on accumulated range */
        if (sum > 0.8r) {
            sum = sum - 0.3r;  /* Prevent saturation */
        }
    }
    return sum;
}

/* Function 6: Mixed-type operations triggering conversions */
static accum_t mixed_type_ops(fract_t f, accum_t a, saccum_t sa) {
    /* Mix saturated and unsaturated types */
    accum_t result = (accum_t)f * a;
    /* Assignment from saturated to unsaturated requires range check */
    accum_t temp = sa;
    return result + temp;
}

/* Function 7: Switch statement based on fixed-point comparison */
static int fixed_point_switch(sfract_t value) {
    int category = 0;
    /* Switch based on fixed-point range analysis */
    if (value < 0.2r) {
        category = 1;
    } else if (value < 0.5r) {
        category = 2;
    } else if (value < 0.8r) {
        category = 3;
    } else {
        category = 4;  /* Near saturation */
    }
    
    /* Switch statement that depends on category */
    switch (category) {
        case 1: return value * 10.0r;  /* Scale up */
        case 2: return value * 5.0r;   /* Moderate scale */
        case 3: return value * 2.0r;   /* Small scale */
        case 4: return value * 0.5r;   /* Scale down to avoid overflow */
        default: return value;
    }
}

/* Function 8: Built-in overflow checks with fixed-point */
static int builtin_overflow_test(saccum_t* result, saccum_t a, saccum_t b) {
    /* Use builtin for overflow detection */
    return __builtin_add_overflow(a, b, result);
}

/* Function 9: Complex expression requiring multi-step range analysis */
static sfract_t complex_range_expr(sfract_t a, sfract_t b, sfract_t c) {
    /* Nested expressions that require detailed range tracking */
    sfract_t expr1 = (a + b) * c;
    sfract_t expr2 = (a - b) / (c + 0.1r);
    
    /* Conditional that depends on both expressions */
    if (expr1 > expr2) {
        return expr1 * 2.0r - 0.3r;
    } else {
        return expr2 * 0.5r + 0.4r;
    }
}

/* Function 10: Inline assembly with fixed-point constraints */
static sfract_t asm_fixed_point(sfract_t a, sfract_t b) {
    sfract_t result;
    /* Assembly that creates hard-to-analyze value flow */
    asm volatile (
        "/* Fixed-point assembly operation */"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    int iterations = 10;
    int array_size = 20;
    
    /* Use command-line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 20;
    }
    
    /* Initialize arrays with different fixed-point values */
    fract_t fract_array[array_size];
    sfract_t sat_fract_array[array_size];
    accum_t accum_array[array_size];
    saccum_t sat_accum_array[array_size];
    
    for (int i = 0; i < array_size; i++) {
        fract_t base_val = (fract_t)i / (fract_t)array_size;
        fract_array[i] = base_val;
        sat_fract_array[i] = base_val;
        accum_array[i] = (accum_t)i * 0.1k;
        sat_accum_array[i] = (saccum_t)i * 0.05k;
    }
    
    /* Variable to accumulate results (prevents dead code elimination) */
    accum_t total_result = 0.0k;
    sfract_t sat_total = 0.0r;
    
    /* Test 1: Loop with fixed-point operations */
    printf("Test 1: Loop accumulation\n");
    for (int iter = 0; iter < iterations; iter++) {
        fract_t loop_val = (fract_t)iter / (fract_t)iterations;
        
        /* Call various test functions */
        sfract_t r1 = sat_add_range(loop_val, 0.7r, 0.3r);
        saccum_t r2 = mul_shift_overflow((saccum_t)loop_val * 10.0k, iter % 4);
        sfract_t r3 = div_saturation(r1, 0.5r + loop_val);
        accum_t r4 = loop_accumulation(fract_array, array_size % (iter + 1) + 1);
        sfract_t r5 = array_reduction_sat(sat_fract_array, array_size % (iter + 1) + 1);
        accum_t r6 = mixed_type_ops(loop_val, accum_array[iter % array_size], 
                                   sat_accum_array[iter % array_size]);
        int r7 = fixed_point_switch(r1);
        
        saccum_t overflow_test;
        int overflow_flag = builtin_overflow_test(&overflow_test, 
                                                 (saccum_t)r1 * 100.0k, 
                                                 (saccum_t)r3 * 50.0k);
        
        sfract_t r9 = complex_range_expr(r1, r3, (sfract_t)loop_val);
        
        /* Accumulate results with different weights */
        total_result += (accum_t)r1 + (accum_t)r2 + r4 + r6 + (accum_t)overflow_test;
        sat_total += r3 + r5 + r9 + (sfract_t)(r7 / 100.0);
        
        /* Print progress occasionally */
        if (iter % (iterations / 5) == 0) {
            printf("  Iteration %d: total_result = %f, sat_total = %f\n", 
                   iter, (float)total_result, (float)sat_total);
        }
    }
    
    /* Test 2: Boundary value tests */
    printf("\nTest 2: Boundary value tests\n");
    
    /* Near saturation boundaries */
    sfract_t near_max = 0.999999r;
    sfract_t near_min = -0.999999r;
    
    /* Operations designed to hit saturation */
    sfract_t sat_test1 = near_max + 0.000001r;  /* Should saturate */
    sfract_t sat_test2 = near_min - 0.000001r;  /* Should saturate */
    saccum_t sat_test3 = (saccum_t)10.0k * 100.0k;  /* Multiplication overflow */
    
    printf("  near_max + 0.000001 = %f\n", (float)sat_test1);
    printf("  near_min - 0.000001 = %f\n", (float)sat_test2);
    printf("  10.0k * 100.0k = %f\n", (float)sat_test3);
    
    total_result += (accum_t)sat_test1 + (accum_t)sat_test2 + sat_test3;
    
    /* Test 3: Shift operations on fixed-point */
    printf("\nTest 3: Shift operations\n");
    for (int shift = 0; shift < 8; shift++) {
        saccum_t shifted = mul_shift_overflow(1.0k, shift);
        total_result += shifted;
        printf("  Shift %d: result = %f\n", shift, (float)shifted);
    }
    
    /* Test 4: Nested function calls */
    printf("\nTest 4: Nested function calls\n");
    sfract_t nested_result = 0.5r;
    for (int i = 0; i < 5; i++) {
        nested_result = complex_range_expr(
            nested_result,
            div_saturation(nested_result, 0.8r),
            sat_add_range(nested_result, 0.2r, 0.1r)
        );
        printf("  Nested call %d: result = %f\n", i, (float)nested_result);
    }
    sat_total += nested_result;
    
    /* Final checksum calculation and output */
    printf("\nFinal results:\n");
    printf("  total_result (accum) = %f\n", (float)total_result);
    printf("  sat_total (sfract) = %f\n", (float)sat_total);
    
    /* Use results to prevent optimization */
    if ((float)total_result > 1000.0 || (float)sat_total > 1.5) {
        printf("  Warning: Possible overflow detected\n");
    }
    
    return 0;
}
