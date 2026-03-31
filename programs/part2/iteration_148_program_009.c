/* test_fixed_point.c - Trigger fixed-value.cc range analysis logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;

/* Test functions with complex fixed-point operations */

/* Function 1: Range analysis with saturation boundaries */
static inline sat_fract_t sat_add_with_check(sat_fract_t a, sat_fract_t b) {
    /* This should trigger saturation range checks */
    sat_fract_t sum = a + b;
    
    /* Conditional that depends on range analysis */
    if (sum > 0.999999r) {
        return 0.999999r;
    } else if (sum < -0.999999r) {
        return -0.999999r;
    }
    return sum;
}

/* Function 2: Multiplication with shift operations */
static inline sat_accum_t complex_accum_op(sat_accum_t x, int shift) {
    /* Operations that require precise overflow analysis */
    sat_accum_t y = x * 2.0k;
    
    /* Shift operation that can cause underflow/overflow */
    if (shift > 0) {
        y = y >> shift;
    } else {
        y = y << (-shift);
    }
    
    /* Ternary operator with fixed-point operands */
    return (y > 10.0k) ? 10.0k : (y < -10.0k) ? -10.0k : y;
}

/* Function 3: Loop-based range propagation */
static sat_accum_t accumulate_fract_array(const fract_t* arr, int n) {
    sat_accum_t total = 0.0k;
    
    /* Loop where induction variable affects fixed-point range */
    for (int i = 0; i < n; i++) {
        /* Complex expression requiring range analysis */
        sat_accum_t term = (sat_accum_t)arr[i] * (sat_accum_t)i;
        
        /* Conditional that forces range evaluation */
        if (term > 5.0k || term < -5.0k) {
            total += term / 2.0k;
        } else {
            total += term;
        }
    }
    
    return total;
}

/* Function 4: Using builtins for overflow detection */
static int check_mul_overflow(sat_fract_t a, sat_fract_t b, sat_fract_t* res) {
    /* Use builtin for overflow detection */
    sat_fract_t tmp;
    int overflow = __builtin_mul_overflow(a, b, &tmp);
    
    if (!overflow) {
        *res = tmp;
    } else {
        /* Handle overflow - should trigger saturation logic */
        *res = (a > 0 && b > 0) ? 0.999999r : -0.999999r;
    }
    
    return overflow;
}

/* Function 5: Switch statement with fixed-point comparisons */
static const char* range_category(sat_accum_t val) {
    /* Switch that depends on fixed-point range analysis */
    if (val > 50.0k) return "HIGH";
    if (val > 20.0k) return "MEDIUM";
    if (val > 0.0k) return "LOW_POS";
    if (val == 0.0k) return "ZERO";
    if (val > -20.0k) return "LOW_NEG";
    return "VERY_LOW";
}

/* Function 6: Nested operations with mixed types */
static sat_accum_t mixed_type_operations(fract_t f, sat_accum_t a) {
    /* Mix saturated and non-saturated types */
    sat_accum_t result = (sat_accum_t)f * a;
    
    /* Complex expression with multiple operations */
    result = (result + a) / 2.0k;
    
    /* Shift operation that requires range analysis */
    result = result >> 2;
    
    return result;
}

/* Function 7: Array reduction with saturation */
static sat_accum_t saturating_array_sum(sat_accum_t* arr, int n) {
    sat_accum_t sum = 0.0k;
    
    for (int i = 0; i < n; i++) {
        /* Addition that may saturate */
        sat_accum_t new_sum = sum + arr[i];
        
        /* Check for saturation - forces range analysis */
        if (new_sum <= sum && arr[i] > 0.0k) {
            /* Positive overflow */
            sum = 32767.999999k; /* Max _Sat _Accum */
        } else if (new_sum >= sum && arr[i] < 0.0k) {
            /* Negative overflow */
            sum = -32768.000000k; /* Min _Sat _Accum */
        } else {
            sum = new_sum;
        }
    }
    
    return sum;
}

/* Main test function */
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
    
    /* Initialize arrays */
    fract_t fract_array[array_size];
    sat_accum_t accum_array[array_size];
    
    /* Fill arrays with values that will trigger range analysis */
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (fract_t)((i % 10) * 0.1r);
        accum_array[i] = (sat_accum_t)((i - array_size/2) * 100.0k);
    }
    
    sat_accum_t total_result = 0.0k;
    
    /* Test 1: Loop with fixed-point operations */
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary inputs to avoid constant propagation */
        fract_t base_fract = (fract_t)((iter % 5) * 0.2r);
        sat_accum_t base_accum = (sat_accum_t)(iter * 50.0k);
        
        /* Call functions that trigger range analysis */
        sat_fract_t sf = sat_add_with_check(base_fract, 0.8r);
        
        sat_accum_t ca = complex_accum_op(base_accum, iter % 4);
        
        sat_accum_t af = accumulate_fract_array(fract_array, 
                            array_size > 10 ? 10 : array_size);
        
        sat_fract_t mul_res;
        int overflow = check_mul_overflow(sf, 0.9r, &mul_res);
        
        const char* category = range_category(ca);
        
        sat_accum_t mixed = mixed_type_operations(sf, ca);
        
        /* Combine results - complex expression requiring analysis */
        sat_accum_t iter_result = ca + af + mixed;
        
        /* Conditional based on fixed-point comparison */
        if (iter_result > 1000.0k) {
            iter_result = iter_result / 2.0k;
        } else if (iter_result < -1000.0k) {
            iter_result = iter_result * (-0.5k);
        }
        
        total_result += iter_result;
    }
    
    /* Test 2: Array reduction with saturation */
    sat_accum_t array_sum = saturating_array_sum(accum_array, array_size);
    total_result += array_sum;
    
    /* Test 3: Edge case operations to trigger specific uncovered lines */
    sat_accum_t max_val = 32767.999999k;  /* Near max _Sat _Accum */
    sat_accum_t min_val = -32768.000000k; /* Near min _Sat _Accum */
    
    /* Operations designed to hit saturation boundaries */
    sat_accum_t near_overflow = max_val * 1.1k;
    sat_accum_t near_underflow = min_val * 1.1k;
    
    /* Shift operations that require range analysis */
    sat_accum_t shifted = max_val >> 1;
    shifted = shifted << 2;
    
    /* Complex conditional with fixed-point comparisons */
    if (near_overflow > max_val || 
        (near_overflow == max_val && shifted > max_val)) {
        total_result += max_val;
    }
    
    if (near_underflow < min_val) {
        total_result += min_val;
    }
    
    /* Final checksum and output to prevent dead code elimination */
    printf("Total result: %f\n", (float)total_result);
    printf("Array sum: %f\n", (float)array_sum);
    
    /* Use results to affect return value */
    if (total_result > 0.0k) {
        return 0;
    } else {
        return 1;
    }
}
