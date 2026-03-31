/* test_fixed_point.c - Designed to trigger fixed-value.cc lines 264-277 */
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

/* Function 1: Complex saturated addition with range analysis */
static inline sat_fract_t add_with_saturation(sat_fract_t a, sat_fract_t b, sat_fract_t c) {
    /* This should trigger range analysis for saturation */
    sat_fract_t sum = a + b;
    /* Conditional that depends on range analysis */
    if (sum > 0.9r) {
        return sum + c;
    } else {
        return sum - c;
    }
}

/* Function 2: Multiplication with shift causing overflow analysis */
static inline sat_accum_t multiply_shift(sat_accum_t x, int shift) {
    /* Operations that require precise overflow analysis */
    sat_accum_t result = x * 2.0k;
    result = result >> shift;
    result = result * 0.75k;
    
    /* Range-dependent conditional */
    if (result < -0.5k || result > 0.5k) {
        return result >> 1;
    }
    return result;
}

/* Function 3: Loop-based accumulation with saturation */
static sat_accum_t accumulate_array(const sat_fract_t* arr, int n) {
    sat_accum_t total = 0.0k;
    for (int i = 0; i < n; i++) {
        /* This addition can saturate */
        total = total + (_Accum)arr[i];
        
        /* Conditional that forces range analysis */
        if (total > 0.95k) {
            total = total * 0.9k;
        } else if (total < -0.95k) {
            total = total * 0.9k;
        }
    }
    return total;
}

/* Function 4: Mixed-type operations triggering conversions */
static sat_fract_t mixed_operations(fract_t a, sat_fract_t b, sat_short_fract_t c) {
    /* Mixing saturated and unsaturated types */
    sat_fract_t result = a + b;
    
    /* Shift operation requiring range analysis */
    result = result << 2;
    
    /* Ternary with fixed-point operands */
    return (result > 0.5r) ? result * c : result / c;
}

/* Function 5: Using builtins for overflow detection */
static int detect_overflow(sat_accum_t* result, sat_accum_t a, sat_accum_t b) {
    /* Use builtin for overflow detection */
    return __builtin_add_overflow(a, b, result);
}

/* Function 6: Complex expression requiring range propagation */
static sat_fract_t complex_range_expr(sat_fract_t base, int iterations) {
    sat_fract_t current = base;
    for (int i = 0; i < iterations; i++) {
        /* Operations that can saturate in different directions */
        current = current * 1.1r;
        current = current + 0.05r;
        
        /* Switch based on fixed-point comparison */
        switch ((int)(current * 10.0r)) {
            case 0 ... 3:
                current = current - 0.1r;
                break;
            case 4 ... 6:
                current = current * 0.9r;
                break;
            case 7 ... 10:
                current = current + 0.05r;
                break;
            default:
                current = 0.5r;
        }
    }
    return current;
}

/* Function 7: Array reduction with saturation boundaries */
static sat_accum_t array_reduction(const sat_accum_t* arr, int size) {
    sat_accum_t max_val = -1.0k;
    sat_accum_t min_val = 1.0k;
    sat_accum_t sum = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Update max/min - these can trigger saturation logic */
        if (arr[i] > max_val) max_val = arr[i];
        if (arr[i] < min_val) min_val = arr[i];
        
        /* Sum with potential saturation */
        sat_accum_t old_sum = sum;
        sum = sum + arr[i];
        
        /* Check if saturation occurred */
        if (old_sum > 0.8k && arr[i] > 0.2k && sum <= old_sum) {
            /* Likely saturated */
            sum = 0.999999k;
        }
    }
    
    /* Range-dependent return */
    return (max_val - min_val > 0.5k) ? sum : sum / 2.0k;
}

/* Function 8: Nested function calls for inter-procedural analysis */
static sat_fract_t nested_calls(sat_fract_t x, int depth) {
    if (depth <= 0) return x;
    
    sat_fract_t half = x * 0.5r;
    sat_fract_t quarter = half * 0.5r;
    
    /* Recursive calls with different arguments */
    sat_fract_t a = nested_calls(half, depth - 1);
    sat_fract_t b = nested_calls(quarter, depth - 1);
    
    /* Operation that requires range analysis */
    sat_fract_t result = a + b;
    
    /* Force analysis of the conditional */
    if (result > 0.75r) {
        return result - 0.1r;
    } else if (result < 0.25r) {
        return result + 0.1r;
    }
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Use command line argument for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize arrays with different fixed-point values */
    sat_fract_t fract_array[20];
    sat_accum_t accum_array[20];
    
    for (int i = 0; i < 20; i++) {
        fract_array[i] = (_Fract)(i * 0.05r);
        accum_array[i] = (_Accum)((i - 10) * 0.1k);
    }
    
    /* Force values near saturation boundaries */
    fract_array[0] = 0.999999r;  /* Near max */
    fract_array[1] = -0.999999r; /* Near min */
    accum_array[0] = 0.999999k;
    accum_array[1] = -0.999999k;
    
    /* Test 1: Complex saturated addition */
    sat_fract_t test1 = 0.0r;
    for (int i = 0; i < iterations; i++) {
        test1 = add_with_saturation(test1, 0.1r, 0.05r);
    }
    
    /* Test 2: Multiplication with shifts */
    sat_accum_t test2 = 0.5k;
    for (int i = 0; i < iterations % 10; i++) {
        test2 = multiply_shift(test2, i % 4);
    }
    
    /* Test 3: Array accumulation */
    sat_accum_t test3 = accumulate_array(fract_array, 20);
    
    /* Test 4: Mixed operations */
    sat_fract_t test4 = mixed_operations(0.3r, 0.7r, 0.9r);
    
    /* Test 5: Overflow detection */
    sat_accum_t overflow_result;
    int had_overflow = detect_overflow(&overflow_result, 0.8k, 0.3k);
    
    /* Test 6: Complex range expression */
    sat_fract_t test6 = complex_range_expr(0.1r, iterations % 5);
    
    /* Test 7: Array reduction */
    sat_accum_t test7 = array_reduction(accum_array, 20);
    
    /* Test 8: Nested calls */
    sat_fract_t test8 = nested_calls(0.8r, 3);
    
    /* Additional tests with asm to create hard-to-analyze flows */
    sat_accum_t asm_test = 0.0k;
    for (int i = 0; i < 5; i++) {
        /* Use asm to manipulate fixed-point values */
        asm volatile (
            "add %[val], %[val], %[inc]"
            : [val] "+r" (asm_test)
            : [inc] "r" (0.1k)
            : "cc"
        );
    }
    
    /* Combine results to prevent dead code elimination */
    sat_accum_t final_result = 
        (_Accum)test1 + test2 + test3 + (_Accum)test4 + 
        overflow_result + (_Accum)test6 + test7 + (_Accum)test8 + asm_test;
    
    /* Print result (convert to float for display) */
    printf("Final result: %f\n", (float)final_result);
    printf("Had overflow: %d\n", had_overflow);
    
    /* Additional print to ensure all values are used */
    printf("Test values: %f %f %f %f %f %f\n", 
           (float)test1, (float)test2, (float)test3,
           (float)test4, (float)test6, (float)test8);
    
    return 0;
}
