/* test_fixed_point.c - Target coverage of fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;

/* Test functions with complex fixed-point operations */

/* Function 1: Range analysis with saturation boundaries */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b) {
    /* This should trigger saturation analysis */
    sfract_t sum = a + b;
    
    /* Complex expression requiring range analysis */
    sfract_t scaled = (sum * 0.5r) >> 2;
    
    /* Conditional based on range analysis */
    if (sum > 0.9r) {
        return scaled + 0.1r;
    } else if (sum < -0.9r) {
        return scaled - 0.1r;
    }
    return scaled;
}

/* Function 2: Loop-based range propagation */
static saccum_t loop_accumulate(fract_t* arr, int n) {
    saccum_t total = 0.0k;
    
    /* Loop where induction variable affects fixed-point range */
    for (int i = 0; i < n; i++) {
        /* Complex expression requiring range analysis */
        saccum_t scaled = (saccum_t)arr[i] * (2.0k + (saccum_t)i * 0.1k);
        
        /* Shift operation that can cause overflow/underflow */
        if (i % 2 == 0) {
            scaled = scaled >> (i % 4);
        } else {
            scaled = scaled << (i % 3);
        }
        
        total = total + scaled;
        
        /* Conditional that depends on range analysis */
        if (total > 100.0k || total < -100.0k) {
            total = total * 0.5k;
        }
    }
    return total;
}

/* Function 3: Mixed saturation types and conversions */
static sfract_t mixed_saturation_ops(sfract_t a, fract_t b, saccum_t c) {
    /* Mix saturated and unsaturated types */
    sfract_t temp1 = a + (sfract_t)b;
    saccum_t temp2 = (saccum_t)temp1 * c;
    
    /* Shift with potential overflow */
    temp2 = temp2 >> 3;
    
    /* Back to fract with possible saturation */
    sfract_t result = (sfract_t)(temp2 * 0.1k);
    
    /* Ternary operator with fixed-point operands */
    return (result > 0.5r) ? result : result + 0.1r;
}

/* Function 4: Using builtins for overflow detection */
static int builtin_overflow_test(sfract_t* a, sfract_t* b, sfract_t* res) {
    /* Use builtin for overflow detection */
    int overflow = 0;
    
    /* Multiple overflow checks */
    overflow |= __builtin_add_overflow(*a, *b, res);
    
    sfract_t temp = *res;
    overflow |= __builtin_mul_overflow(&temp, &temp, res);
    
    /* Shift operation after multiplication */
    *res = *res >> 2;
    
    return overflow;
}

/* Function 5: Switch statement with fixed-point conditions */
static fract_t switch_fixed_point(sfract_t val) {
    fract_t result = 0.0r;
    
    /* Switch based on fixed-point comparisons */
    switch ((int)(val * 10.0r)) {
        case 0 ... 3:  /* 0.0 to 0.3 */
            result = val * 2.0r;
            break;
        case 4 ... 6:  /* 0.4 to 0.6 */
            result = val / 2.0r;
            break;
        case 7 ... 10: /* 0.7 to 1.0 */
            result = 1.0r - val;
            break;
        default:
            result = val;
    }
    
    /* Additional range-dependent operation */
    if (result > 0.8r) {
        result = result >> 1;
    }
    
    return result;
}

/* Function 6: Array reduction with fixed-point */
static saccum_t array_reduction(sfract_t* arr, int size) {
    saccum_t prod = 1.0k;
    saccum_t sum = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Product that can overflow */
        prod = prod * (saccum_t)arr[i];
        
        /* Sum with saturation */
        sum = sum + (saccum_t)arr[i];
        
        /* Conditional based on product range */
        if (prod > 10.0k || prod < -10.0k) {
            prod = prod * 0.5k;
        }
    }
    
    /* Final complex expression */
    return (prod + sum) / (saccum_t)size;
}

/* Function 7: Inline asm to create hard-to-analyze flows */
static sfract_t asm_fixed_point(sfract_t a, sfract_t b) {
    sfract_t result;
    
    /* Inline asm with fixed-point constraints */
    asm volatile (
        "/* Fixed-point operation with hard-to-analyze flow */"
        : "=r" (result)
        : "r" (a), "r" (b)
    );
    
    /* Follow up with compiler-visible operations */
    result = result * 0.5r;
    result = result >> 1;
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command-line args for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int array_size = (argc > 2) ? atoi(argv[2]) : 50;
    
    if (iterations <= 0) iterations = 100;
    if (array_size <= 0) array_size = 50;
    
    /* Initialize arrays */
    sfract_t* sf_arr = (sfract_t*)malloc(array_size * sizeof(sfract_t));
    fract_t* f_arr = (fract_t*)malloc(array_size * sizeof(fract_t));
    
    if (!sf_arr || !f_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with values that will trigger range analysis */
    for (int i = 0; i < array_size; i++) {
        /* Values near saturation boundaries */
        fract_t base_val = (fract_t)((i % 10) * 0.1r);
        sf_arr[i] = (sfract_t)base_val;
        f_arr[i] = base_val;
        
        /* Add some negative values */
        if (i % 3 == 0) {
            sf_arr[i] = -sf_arr[i];
            f_arr[i] = -f_arr[i];
        }
    }
    
    /* Test 1: Saturation boundary tests */
    printf("Test 1: Saturation boundaries\n");
    sfract_t sat_test = 0.999999r;
    for (int i = 0; i < iterations; i++) {
        sat_test = sat_add_range(sat_test, 0.000001r);
        
        /* Force different code paths */
        if (i % 5 == 0) {
            sat_test = -sat_test;
        }
    }
    printf("  Result: %f\n", (float)sat_test);
    
    /* Test 2: Loop accumulation */
    printf("Test 2: Loop accumulation\n");
    saccum_t accum_result = loop_accumulate(f_arr, array_size);
    printf("  Result: %f\n", (float)accum_result);
    
    /* Test 3: Mixed operations */
    printf("Test 3: Mixed saturation types\n");
    sfract_t mixed_result = mixed_saturation_ops(
        0.8r, 0.7r, 5.0k);
    printf("  Result: %f\n", (float)mixed_result);
    
    /* Test 4: Builtin overflow */
    printf("Test 4: Builtin overflow detection\n");
    sfract_t a = 0.9r, b = 0.8r, res;
    int overflow = builtin_overflow_test(&a, &b, &res);
    printf("  Overflow: %d, Result: %f\n", overflow, (float)res);
    
    /* Test 5: Switch statement */
    printf("Test 5: Switch with fixed-point\n");
    fract_t switch_result = switch_fixed_point(0.5r);
    printf("  Result: %f\n", (float)switch_result);
    
    /* Test 6: Array reduction */
    printf("Test 6: Array reduction\n");
    saccum_t reduction_result = array_reduction(sf_arr, array_size);
    printf("  Result: %f\n", (float)reduction_result);
    
    /* Test 7: Inline asm */
    printf("Test 7: Inline assembly\n");
    sfract_t asm_result = asm_fixed_point(0.6r, 0.4r);
    printf("  Result: %f\n", (float)asm_result);
    
    /* Final checksum to prevent dead code elimination */
    saccum_t checksum = (saccum_t)sat_test + 
                       accum_result + 
                       (saccum_t)mixed_result + 
                       (saccum_t)res + 
                       (saccum_t)switch_result + 
                       reduction_result + 
                       (saccum_t)asm_result;
    
    printf("\nFinal checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(sf_arr);
    free(f_arr);
    
    return 0;
}
