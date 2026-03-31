/* test_fixed_point.c - Target coverage for fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract;
typedef _Fract fract;
typedef _Sat _Accum sat_accum;
typedef _Accum accum;
typedef _Sat short _Fract sat_short_fract;
typedef short _Fract short_fract;

/* Test functions with different fixed-point operations */

/* Function 1: Complex saturated addition with range analysis */
static inline sat_fract add_with_saturation(sat_fract a, sat_fract b, sat_fract c) {
    /* This should trigger range analysis for saturation boundaries */
    sat_fract sum = a + b;
    /* Conditional based on range analysis */
    if (sum > 0.9r) {
        return sum + c;
    } else {
        return sum - c;
    }
}

/* Function 2: Multiplication with shift causing overflow analysis */
static inline sat_accum mul_with_shift(sat_accum x, int shift) {
    /* Multiplication followed by shift - requires precise range analysis */
    sat_accum result = x * 2.0k;
    result = result >> shift;
    return result;
}

/* Function 3: Division with saturation boundary check */
static inline sat_fract divide_with_check(fract a, fract b) {
    /* Division that might overflow */
    sat_fract result = a / b;
    
    /* This condition should trigger the uncovered comparison logic */
    if (result > 0.95r || result < -0.95r) {
        return result * 0.5r;
    }
    return result;
}

/* Function 4: Loop-based accumulation with range propagation */
static sat_accum accumulate_array(const sat_fract* arr, int size) {
    sat_accum total = 0.0k;
    for (int i = 0; i < size; i++) {
        /* Complex expression requiring range analysis */
        total = total + (sat_accum)arr[i] * (0.5k + (i % 2 ? 0.25k : -0.25k));
        
        /* Conditional that depends on accumulated range */
        if (total > 10.0k || total < -10.0k) {
            total = total * 0.9k;
        }
    }
    return total;
}

/* Function 5: Nested ternary with fixed-point operations */
static sat_fract ternary_operation(sat_fract a, sat_fract b, int mode) {
    /* Complex ternary expression requiring range analysis */
    return mode > 0 ? 
           (a + b > 0.8r ? a * 1.2r : b * 1.2r) :
           (a - b < -0.8r ? a * 0.8r : b * 0.8r);
}

/* Function 6: Using builtins for overflow detection */
static int check_mul_overflow(sat_accum a, sat_accum b, sat_accum* res) {
    /* This builtin should trigger fixed-point range analysis */
    return __builtin_mul_overflow(a, b, res);
}

/* Function 7: Switch statement based on fixed-point ranges */
static const char* range_category(sat_fract val) {
    /* Switch on discretized ranges */
    switch ((int)(val * 10.0r)) {
        case -10 ... -5: return "Very Negative";
        case -4 ... -1: return "Negative";
        case 0: return "Zero";
        case 1 ... 4: return "Positive";
        case 5 ... 10: return "Very Positive";
        default: return "Out of Range";
    }
}

/* Function 8: Bit-shift operations causing underflow/overflow */
static sat_accum shift_operations(sat_accum val, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Alternating shifts that can cause overflow */
        if (i % 2 == 0) {
            val = val << 1;
        } else {
            val = val >> 2;
        }
        
        /* This comparison should trigger the uncovered code */
        if (val > 100.0k || val < -100.0k) {
            val = val * 0.5k;
        }
    }
    return val;
}

/* Function 9: Mixed saturation conversions */
static sat_fract mixed_conversions(fract a, sat_fract b, sat_accum c) {
    /* Mixing different fixed-point types */
    sat_fract result = a + b;
    result = result + (sat_fract)(c * 0.1k);
    
    /* Boundary check that requires range analysis */
    if (result >= 0.999r || result <= -0.999r) {
        return result * 0.99r;
    }
    return result;
}

/* Function 10: Complex expression tree */
static sat_accum complex_expression(sat_accum a, sat_accum b, sat_accum c) {
    /* Deep expression tree requiring extensive range analysis */
    sat_accum result = (a + b) * c - (a * b) / c + (b - a) >> 2;
    
    /* This is the key comparison that should trigger lines 264-277 */
    if (result > 50.0k || (result == 50.0k && (a + b) > 100.0k)) {
        return result * 0.8k;
    }
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    int array_size = argc > 2 ? atoi(argv[2]) : 50;
    
    printf("Testing fixed-point range analysis (iterations=%d, array_size=%d)\n", 
           iterations, array_size);
    
    /* Initialize arrays with varying fixed-point values */
    sat_fract* fract_array = (sat_fract*)malloc(array_size * sizeof(sat_fract));
    sat_accum* accum_array = (sat_accum*)malloc(array_size * sizeof(sat_accum));
    
    for (int i = 0; i < array_size; i++) {
        /* Generate values that approach saturation boundaries */
        fract_array[i] = (sat_fract)((i % 10) * 0.1r - 0.5r);
        accum_array[i] = (sat_accum)((i % 20) * 1.0k - 10.0k);
    }
    
    sat_accum total_accum = 0.0k;
    sat_fract total_fract = 0.0r;
    
    /* Perform various tests to trigger range analysis */
    for (int iter = 0; iter < iterations; iter++) {
        /* Test 1: Complex expressions */
        sat_accum a = (sat_accum)(iter * 0.1k);
        sat_accum b = (sat_accum)((iter % 3) * 0.5k);
        sat_accum c = complex_expression(a, b, 2.0k);
        total_accum += c;
        
        /* Test 2: Array accumulation with range propagation */
        if (iter % 10 == 0) {
            sat_accum arr_sum = accumulate_array(fract_array, array_size);
            total_accum += arr_sum;
        }
        
        /* Test 3: Multiplication overflow checks */
        sat_accum mul_result;
        if (check_mul_overflow(a, b, &mul_result)) {
            total_accum += mul_result * 0.5k;
        }
        
        /* Test 4: Shift operations */
        sat_accum shifted = shift_operations(a, 5);
        total_accum += shifted;
        
        /* Test 5: Mixed type conversions */
        sat_fract f1 = (sat_fract)(iter * 0.01r);
        sat_fract f2 = (sat_fract)((iter % 7) * 0.15r - 0.5r);
        sat_fract mixed = mixed_conversions(f1, f2, a);
        total_fract += mixed;
        
        /* Test 6: Ternary operations */
        sat_fract ternary_result = ternary_operation(f1, f2, iter % 2);
        total_fract += ternary_result;
        
        /* Test 7: Division with boundary checks */
        if (iter % 3 == 0) {
            sat_fract div_result = divide_with_check(f1, f2 + 0.1r);
            total_fract += div_result;
        }
        
        /* Test 8: Range categorization */
        const char* category = range_category(f1);
        if (strcmp(category, "Very Positive") == 0) {
            total_fract += 0.1r;
        }
    }
    
    /* Final checksum to prevent dead code elimination */
    sat_accum final_result = total_accum + (sat_accum)total_fract;
    
    /* Convert to float for printing */
    printf("Final result (as float): %f\n", (float)final_result);
    printf("Fract total: %f, Accum total: %f\n", 
           (float)total_fract, (float)total_accum);
    
    /* Cleanup */
    free(fract_array);
    free(accum_array);
    
    return 0;
}
