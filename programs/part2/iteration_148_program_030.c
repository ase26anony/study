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

/* Test functions with complex fixed-point operations */

/* Function 1: Range analysis with saturation boundaries */
static inline saccum_t range_boundary_test(saccum_t a, saccum_t b, int shift) {
    /* Operations that should trigger saturation range checks */
    saccum_t temp = a * b;
    temp = temp >> shift;
    
    /* Conditional that depends on range analysis */
    if (temp > 0.5k) {
        return temp + 0.3k;
    } else {
        return temp - 0.3k;
    }
}

/* Function 2: Loop-based range propagation */
static sfract_t loop_range_propagation(int iterations, sfract_t base) {
    sfract_t result = 0.0r;
    sfract_t increment = 0.1r;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        sfract_t temp = base + (i * 0.01r);
        temp = temp * 0.9r;
        
        /* Operation near saturation boundary */
        if (result > 0.8r) {
            result = result + temp * 0.5r;  /* Should saturate */
        } else {
            result = result + temp;
        }
        
        /* Shift operation affecting range */
        base = base >> 1;
    }
    
    return result;
}

/* Function 3: Array reduction with mixed types */
static accum_t array_reduction(const sfract_t* arr, int size) {
    accum_t total = 0.0k;
    accum_t product = 1.0k;
    
    for (int i = 0; i < size; i++) {
        /* Mixed-type operations requiring conversion analysis */
        accum_t converted = (accum_t)arr[i];
        
        /* Operations that could overflow */
        total = total + converted * (i + 1);
        product = product * (converted + 0.1k);
        
        /* Ternary operator with fixed-point operands */
        total = (total > 10.0k) ? total * 0.5k : total * 2.0k;
    }
    
    /* Final range-dependent operation */
    return (total > product) ? total - product : product - total;
}

/* Function 4: Nested conditional range analysis */
static saccum_t nested_conditional(saccum_t x, saccum_t y, int mode) {
    saccum_t result;
    
    switch (mode) {
        case 0:
            /* Multiplication near overflow boundary */
            result = x * y;
            if (result > 0.9k && result < 1.1k) {
                result = result >> 2;
            }
            break;
            
        case 1:
            /* Division with range check */
            result = x / y;
            if (y != 0.0k && result < -0.5k) {
                result = -result;
            }
            break;
            
        case 2:
            /* Complex expression chain */
            result = (x + y) * (x - y);
            result = result >> 1;
            result = result * 2.0k;
            break;
            
        default:
            /* Built-in overflow check */
            saccum_t sum;
            if (__builtin_add_overflow(x, y, &sum)) {
                result = (x > 0) ? 0.999999k : -0.999999k;
            } else {
                result = sum * 0.5k;
            }
    }
    
    return result;
}

/* Function 5: Bit-shift underflow/overflow */
static sfract_t shift_boundary_test(sfract_t value, int shifts[], int count) {
    sfract_t result = value;
    
    for (int i = 0; i < count; i++) {
        /* Shift operations that require precise range analysis */
        if (shifts[i] > 0) {
            result = result << shifts[i];
        } else {
            result = result >> (-shifts[i]);
        }
        
        /* Check for saturation after shift */
        if (result == 0.999999r || result == -0.999999r) {
            /* Reset to middle value */
            result = 0.5r;
        }
    }
    
    return result;
}

/* Function 6: Inter-procedural range propagation */
static inline saccum_t helper_multiply(saccum_t a, saccum_t b) {
    /* Intermediate calculation affecting caller's range */
    saccum_t temp = a * b;
    
    /* Range-dependent operation */
    if (temp > 0.7k) {
        return temp * 0.8k;
    } else if (temp < -0.7k) {
        return temp * 1.2k;
    }
    return temp;
}

static accum_t inter_procedural_test(accum_t x, accum_t y) {
    /* Call helper multiple times */
    saccum_t r1 = helper_multiply(x, 0.5k);
    saccum_t r2 = helper_multiply(y, 0.3k);
    saccum_t r3 = helper_multiply(r1, r2);
    
    /* Final complex expression */
    return (accum_t)(r1 + r2 - r3 * 0.25k);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Use command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 20;
    int test_mode = (argc > 3) ? atoi(argv[3]) % 4 : 0;
    
    /* Initialize fixed-point arrays */
    sfract_t fract_array[100];
    accum_t accum_array[100];
    
    for (int i = 0; i < array_size && i < 100; i++) {
        fract_array[i] = (i % 10) * 0.1r;
        accum_array[i] = (i % 5) * 0.2k - 0.4k;
    }
    
    /* Test 1: Range boundary with saturation */
    saccum_t sat_test = range_boundary_test(0.8k, 0.9k, 1);
    printf("Test 1 result: %f\n", (float)sat_test);
    
    /* Test 2: Loop-based propagation */
    sfract_t loop_result = loop_range_propagation(iterations, 0.5r);
    printf("Test 2 result: %f\n", (float)loop_result);
    
    /* Test 3: Array reduction */
    accum_t reduction_result = array_reduction(fract_array, 
                                              (array_size < 20) ? array_size : 20);
    printf("Test 3 result: %f\n", (float)reduction_result);
    
    /* Test 4: Nested conditionals */
    int shift_pattern[] = {1, -2, 3, -1, 2};
    sfract_t shift_result = shift_boundary_test(0.25r, shift_pattern, 5);
    printf("Test 4 result: %f\n", (float)shift_result);
    
    /* Test 5: Mixed operations with builtins */
    saccum_t builtin_test;
    saccum_t a = 0.999k;
    saccum_t b = 0.5k;
    
    /* Force overflow check */
    if (__builtin_mul_overflow(a, b, &builtin_test)) {
        builtin_test = (a > 0) ? 0.999999k : -0.999999k;
    } else {
        builtin_test = builtin_test * 1.1k;
    }
    printf("Test 5 result: %f\n", (float)builtin_test);
    
    /* Test 6: Inter-procedural analysis */
    accum_t inter_result = inter_procedural_test(0.6k, -0.7k);
    printf("Test 6 result: %f\n", (float)inter_result);
    
    /* Test 7: Switch statement with fixed-point */
    saccum_t switch_result = nested_conditional(0.8k, 0.3k, test_mode);
    printf("Test 7 result: %f\n", (float)switch_result);
    
    /* Test 8: Complex expression chain */
    accum_t complex_result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        complex_result = complex_result + accum_array[i % array_size];
        complex_result = complex_result * 0.95k;
        complex_result = complex_result >> 1;
        
        /* Range-dependent conditional */
        if (complex_result > 0.5k) {
            complex_result = complex_result - 0.1k;
        } else if (complex_result < -0.5k) {
            complex_result = complex_result + 0.1k;
        }
    }
    printf("Test 8 result: %f\n", (float)complex_result);
    
    /* Final checksum to prevent dead code elimination */
    accum_t checksum = sat_test + loop_result + reduction_result + 
                      shift_result + builtin_test + inter_result + 
                      switch_result + complex_result;
    
    printf("Final checksum: %f\n", (float)checksum);
    
    return (checksum != 0.0k) ? 0 : 1;
}
