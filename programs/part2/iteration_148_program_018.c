/* test_fixed.c - Comprehensive fixed-point test for GCC coverage */
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

/* Test function 1: Complex fixed-point arithmetic with saturation */
static inline saccum_t complex_sat_math(saccum_t a, saccum_t b, saccum_t c) {
    /* This should trigger range analysis for multiplication and addition */
    saccum_t temp = (a * b) >> 3;
    temp = temp + c;
    
    /* Force conditional based on range analysis */
    if (temp > 0.5k) {
        return temp >> 1;
    } else {
        return temp << 1;
    }
}

/* Test function 2: Fixed-point array reduction with overflow */
static sfract_t array_reduction(const sfract_t* arr, int n) {
    sfract_t sum = 0.0r;
    sfract_t product = 1.0r;
    
    for (int i = 0; i < n; i++) {
        /* Operations that can saturate */
        sum = sum + arr[i];
        product = product * arr[i];
        
        /* Conditional that depends on range */
        if (sum > 0.8r) {
            sum = sum - 0.3r;
        }
    }
    
    /* Mix operations that require range analysis */
    return (sum + product) >> 2;
}

/* Test function 3: Nested fixed-point operations in loops */
static accum_t nested_loop_analysis(int iterations) {
    accum_t result = 0.0k;
    fract_t step = 0.1r;
    
    for (int i = 0; i < iterations; i++) {
        fract_t inner = 0.5r;
        
        for (int j = 0; j < 5; j++) {
            /* Operations that approach saturation */
            inner = inner * 1.1r;
            result = result + (accum_t)inner;
            
            /* Conditional with fixed-point comparison */
            if (inner > 0.9r) {
                inner = 0.1r;
            }
        }
        
        step = step * 1.05r;
        if (step > 0.5r) {
            step = 0.1r;
        }
    }
    
    return result;
}

/* Test function 4: Using builtins for overflow detection */
static int builtin_overflow_test(sfract_t a, sfract_t b, sfract_t* res) {
    sfract_t tmp;
    int overflow;
    
    /* Use builtin for overflow detection */
    overflow = __builtin_add_overflow(a, b, &tmp);
    if (!overflow) {
        *res = tmp;
    } else {
        /* Handle saturation manually */
        *res = (a > 0) ? 0.999999r : -0.999999r;
    }
    
    return overflow;
}

/* Test function 5: Switch statement with fixed-point conditions */
static fract_t switch_fixed_point(fract_t val) {
    /* Switch on discretized fixed-point value */
    int category;
    
    if (val < 0.25r) category = 0;
    else if (val < 0.5r) category = 1;
    else if (val < 0.75r) category = 2;
    else category = 3;
    
    switch (category) {
        case 0:
            return val * 2.0r;
        case 1:
            return val / 2.0r;
        case 2:
            return (val + 0.25r) >> 1;
        case 3:
            return (val - 0.25r) << 1;
        default:
            return val;
    }
}

/* Test function 6: Ternary operator with fixed-point */
static sfract_t ternary_operations(sfract_t a, sfract_t b, int flag) {
    /* Complex ternary expressions */
    sfract_t result = flag ? 
        (a > b ? a + b : a - b) :
        (a < b ? a * b : a / b);
    
    /* Nested ternary */
    return (result > 0.5r) ? 
        (result >> 1) : 
        ((result < -0.5r) ? (result << 1) : result);
}

/* Test function 7: Mixed saturated/unsaturated operations */
static void mixed_saturation_ops(void) {
    fract_t unsat = 0.7r;
    sfract_t sat = 0.8r;
    
    /* Mix types - requires range analysis for conversion */
    sfract_t mixed = sat + (sfract_t)unsat;
    
    /* Operations near boundaries */
    for (int i = 0; i < 10; i++) {
        unsat = unsat * 1.1r;
        sat = sat + 0.1r;
        
        /* Assignment that may saturate */
        sfract_t test = (sfract_t)unsat;
        
        /* Force analysis of both branches */
        if (test > sat) {
            sat = test;
        } else {
            unsat = (fract_t)sat;
        }
    }
}

/* Test function 8: Bit-shift with underflow/overflow */
static saccum_t shift_boundary_test(saccum_t val, int shift) {
    /* Shifts that can cause overflow/underflow */
    saccum_t result;
    
    if (shift > 0) {
        result = val << shift;
        
        /* Check for overflow */
        if (result == 0k && val != 0k) {
            /* Underflow occurred */
            return -1.0k;
        }
    } else {
        result = val >> (-shift);
        
        /* Check for overflow */
        if ((result > 0k && val < 0k) || (result < 0k && val > 0k)) {
            /* Sign changed - overflow */
            return (val > 0) ? 0.999999k : -0.999999k;
        }
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    int iterations = 10;
    int array_size = 20;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 5) array_size = 20;
    }
    
    printf("Running fixed-point tests with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    sfract_t* sat_array = (sfract_t*)malloc(array_size * sizeof(sfract_t));
    fract_t* unsat_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    
    for (int i = 0; i < array_size; i++) {
        /* Varying values to trigger different paths */
        unsat_array[i] = (fract_t)((i % 10) * 0.1r);
        sat_array[i] = (sfract_t)(((i % 7) + 3) * 0.1r);
    }
    
    /* Test 1: Complex saturated math */
    saccum_t acc1 = 0.3k;
    saccum_t acc2 = 0.7k;
    saccum_t acc3 = -0.2k;
    
    for (int i = 0; i < iterations; i++) {
        acc1 = complex_sat_math(acc1, acc2, acc3);
        acc2 = acc2 * 1.1k;
        acc3 = acc3 + 0.05k;
    }
    printf("Test 1 result: %f\n", (float)acc1);
    
    /* Test 2: Array reduction */
    sfract_t red_result = array_reduction(sat_array, array_size);
    printf("Test 2 result: %f\n", (float)red_result);
    
    /* Test 3: Nested loops */
    accum_t nested_result = nested_loop_analysis(iterations);
    printf("Test 3 result: %f\n", (float)nested_result);
    
    /* Test 4: Builtin overflow */
    sfract_t builtin_res;
    int overflow_count = 0;
    for (int i = 0; i < array_size - 1; i++) {
        overflow_count += builtin_overflow_test(
            sat_array[i], sat_array[i+1], &builtin_res);
    }
    printf("Test 4: Overflow detected in %d/%d operations\n", 
           overflow_count, array_size - 1);
    
    /* Test 5: Switch statement */
    fract_t switch_result = 0.0r;
    for (int i = 0; i < array_size; i++) {
        switch_result = switch_fixed_point(unsat_array[i]);
    }
    printf("Test 5 result: %f\n", (float)switch_result);
    
    /* Test 6: Ternary operations */
    sfract_t ternary_result = 0.5r;
    for (int i = 0; i < iterations; i++) {
        ternary_result = ternary_operations(
            ternary_result, sat_array[i % array_size], i % 2);
    }
    printf("Test 6 result: %f\n", (float)ternary_result);
    
    /* Test 7: Mixed saturation (void function, no result) */
    mixed_saturation_ops();
    printf("Test 7 completed\n");
    
    /* Test 8: Shift boundary */
    saccum_t shift_val = 0.8k;
    saccum_t shift_result = 0k;
    for (int i = 1; i <= 8; i++) {
        shift_result = shift_boundary_test(shift_val, i);
        shift_val = shift_val / 2.0k;
    }
    printf("Test 8 result: %f\n", (float)shift_result);
    
    /* Final checksum to prevent dead code elimination */
    accum_t checksum = (accum_t)acc1 + 
                      (accum_t)red_result + 
                      (accum_t)nested_result + 
                      (accum_t)switch_result + 
                      (accum_t)ternary_result + 
                      shift_result;
    
    printf("Final checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(sat_array);
    free(unsat_array);
    
    return 0;
}
