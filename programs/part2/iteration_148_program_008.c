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
    result = result >> 2;  /* Shift operation on fixed-point */
    return result;
}

/* Function 2: Accumulator with loop-based range propagation */
static sat_accum_t accumulate_range(int iterations, sat_accum_t base) {
    sat_accum_t total = base;
    for (int i = 0; i < iterations; i++) {
        /* Varying operations that could hit saturation boundaries */
        sat_accum_t increment = (_Accum)i / (_Accum)iterations;
        total = total + increment;
        
        /* Conditional that depends on range analysis */
        if (total > 0.8k) {
            total = total * 0.5k;
        }
    }
    return total;
}

/* Function 3: Fixed-point array reduction with overflow checks */
static sat_fract_t array_reduction(const sat_fract_t* arr, int size) {
    sat_fract_t sum = 0.0r;
    for (int i = 0; i < size; i++) {
        /* Operations designed to approach saturation */
        sum = sum + arr[i];
        
        /* This comparison should trigger the uncovered range check */
        if (sum > 0.95r || sum < -0.95r) {
            /* Force evaluation of saturation boundaries */
            sum = sum * 0.9r;
        }
    }
    return sum;
}

/* Function 4: Mixed-type operations with builtins */
static sat_accum_t mixed_operations_with_builtins(sat_accum_t a, accum_t b) {
    sat_accum_t result = a;
    
    /* Use builtin overflow check */
    int overflow = 0;
    result = __builtin_add_overflow(result, b, &result) ? 
             (a > 0 ? 0.999999k : -0.999999k) : result;
    
    /* Complex expression requiring range analysis */
    result = (result * 2.0k) >> 3;
    
    return result;
}

/* Function 5: Nested conditional with fixed-point comparisons */
static fract_t nested_conditional(fract_t x, fract_t y, int selector) {
    fract_t result;
    
    switch (selector) {
        case 0:
            result = x + y;
            /* This should trigger range comparison logic */
            if (result > 0.8r && result < 0.9r) {
                result = result * 1.1r;
            }
            break;
        case 1:
            result = x - y;
            if (result < -0.8r || result > 0.8r) {
                result = result / 2.0r;
            }
            break;
        case 2:
            result = x * y;
            /* Ternary with fixed-point operands */
            result = (result > 0.5r) ? (result + 0.1r) : (result - 0.1r);
            break;
        default:
            result = x / (y + 0.1r);
    }
    
    return result;
}

/* Function 6: Bit-shift operations that can cause underflow/overflow */
static sat_fract_t shift_operations(sat_fract_t value, int shift_amount) {
    sat_fract_t result = value;
    
    /* Multiple shifts that require careful range analysis */
    for (int i = 0; i < shift_amount; i++) {
        result = result >> 1;
        
        /* Check boundaries - should trigger the uncovered code */
        if (result < 0.01r && result > -0.01r) {
            result = result * 2.0r;
        }
    }
    
    return result;
}

/* Function 7: Inter-procedural range analysis test */
static sat_accum_t interprocedural_test(sat_accum_t start, int steps) {
    sat_accum_t current = start;
    
    for (int i = 0; i < steps; i++) {
        /* Call another function with current value */
        current = complex_sat_operation(current, 0.1k, 0.5k);
        
        /* Operations designed to hit max/min boundaries */
        if (i % 3 == 0) {
            current = current * 1.5k;
        } else if (i % 3 == 1) {
            current = current / 1.5k;
        } else {
            current = current + current;
        }
    }
    
    return current;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = 10;
    int array_size = 8;
    
    /* Use command-line arguments for runtime variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 4) array_size = 8;
        if (array_size > 64) array_size = 64;
    }
    
    /* Initialize fixed-point arrays */
    sat_fract_t fract_array[64];
    sat_accum_t accum_array[64];
    
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (_Fract)i / (_Fract)array_size;
        accum_array[i] = (_Accum)i / (_Accum)array_size;
    }
    
    /* Test 1: Complex saturation operations */
    sat_fract_t test1 = 0.5r;
    for (int i = 0; i < iterations; i++) {
        test1 = complex_sat_operation(test1, 0.3r, 0.8r);
        /* Force near-saturation conditions */
        if (i % 2 == 0) {
            test1 = test1 + 0.4r;
        }
    }
    printf("Test 1 result: %f\n", (float)test1);
    
    /* Test 2: Accumulator with range propagation */
    sat_accum_t test2 = accumulate_range(iterations, 0.2k);
    printf("Test 2 result: %f\n", (float)test2);
    
    /* Test 3: Array reduction */
    sat_fract_t test3 = array_reduction(fract_array, array_size);
    printf("Test 3 result: %f\n", (float)test3);
    
    /* Test 4: Mixed operations with builtins */
    sat_accum_t test4 = mixed_operations_with_builtins(0.7k, 0.4k);
    printf("Test 4 result: %f\n", (float)test4);
    
    /* Test 5: Nested conditionals */
    fract_t test5 = nested_conditional(0.6r, 0.3r, iterations % 4);
    printf("Test 5 result: %f\n", (float)test5);
    
    /* Test 6: Shift operations */
    sat_fract_t test6 = shift_operations(0.8r, iterations % 8 + 1);
    printf("Test 6 result: %f\n", (float)test6);
    
    /* Test 7: Inter-procedural analysis */
    sat_accum_t test7 = interprocedural_test(0.1k, iterations % 5 + 3);
    printf("Test 7 result: %f\n", (float)test7);
    
    /* Final checksum to prevent dead code elimination */
    sat_accum_t checksum = test1 + test2 + test3 + test4 + test5 + test6 + test7;
    
    /* Use inline asm to create hard-to-analyze value flows */
    sat_accum_t volatile_checksum = checksum;
    asm volatile ("" : "+r" (volatile_checksum));
    
    printf("Final checksum: %f\n", (float)volatile_checksum);
    
    /* Additional edge case: Operations at saturation boundaries */
    sat_fract_t max_fract = 0.999999r;
    sat_fract_t min_fract = -0.999999r;
    
    /* These should trigger saturation logic */
    sat_fract_t near_max = max_fract + 0.000001r;
    sat_fract_t near_min = min_fract - 0.000001r;
    
    printf("Near max: %f, Near min: %f\n", (float)near_max, (float)near_min);
    
    /* Test with short fract types for different bit widths */
    sat_short_fract_t short_test = 0.5r;
    for (int i = 0; i < 5; i++) {
        short_test = short_test * 1.2r;
        short_test = short_test >> 1;
    }
    printf("Short fract test: %f\n", (float)short_test);
    
    return 0;
}
