/* test_fixed_point.c - Program to trigger fixed-value.cc range analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef _Sat long _Fract sat_long_fract_t;

/* Test functions with different fixed-point operations */

/* Function 1: Complex arithmetic with saturation */
static inline sat_fract_t add_with_saturation(sat_fract_t a, sat_fract_t b, sat_fract_t c) {
    /* This should trigger range analysis for saturation */
    sat_fract_t sum = a + b;
    /* Conditional based on range analysis */
    if (sum > 0.9r) {
        return sum * c;
    } else {
        return (sum + c) / 2.0r;
    }
}

/* Function 2: Multiplication with overflow checking */
static inline sat_accum_t multiply_check_overflow(sat_accum_t x, sat_accum_t y, int shift) {
    sat_accum_t result = x * y;
    /* Shift operation that requires range analysis */
    if (shift > 0) {
        result = result >> shift;
    } else if (shift < 0) {
        result = result << (-shift);
    }
    
    /* Check for overflow using builtins */
    sat_accum_t check;
    if (__builtin_mul_overflow(x, y, &check)) {
        return (x > 0 && y > 0) ? 0.999999k : -0.999999k;
    }
    return result;
}

/* Function 3: Loop-based range propagation */
static sat_fract_t accumulate_fractals(fract_t* array, int size) {
    sat_fract_t total = 0.0r;
    for (int i = 0; i < size; i++) {
        /* Complex expression that depends on loop variable */
        fract_t increment = array[i] * (0.1r + (i % 10) * 0.01r);
        
        /* Conditional that forces range analysis */
        if (total + increment > 0.95r || total + increment < -0.95r) {
            /* Use ternary with fixed-point operands */
            total = (total > 0) ? 0.999r : -0.999r;
        } else {
            total += increment;
        }
    }
    return total;
}

/* Function 4: Nested operations with different types */
static accum_t mixed_operations(sat_fract_t a, fract_t b, sat_accum_t c) {
    /* Convert and mix types */
    accum_t result = (accum_t)a * 2.0k;
    result = result + (accum_t)b * 3.0k;
    
    /* Shift operation that can cause underflow/overflow */
    int shift_amount = (a > 0.5r) ? 2 : 4;
    result = result >> shift_amount;
    
    /* Complex conditional */
    if (result > c || (result == c && (accum_t)a * b > 0.1k)) {
        return result * 0.5k;
    }
    return result + c;
}

/* Function 5: Array reduction with saturation boundaries */
static sat_accum_t saturating_array_product(sat_accum_t* arr, int n) {
    if (n <= 0) return 1.0k;
    
    sat_accum_t product = arr[0];
    for (int i = 1; i < n; i++) {
        /* Operations designed to hit saturation */
        sat_accum_t old_product = product;
        product = product * arr[i];
        
        /* Check if we hit saturation */
        if (old_product > 0 && arr[i] > 0 && product <= 0) {
            product = 0.999999k;  /* Max positive */
        } else if (old_product < 0 && arr[i] < 0 && product >= 0) {
            product = -0.999999k; /* Max negative */
        }
        
        /* Additional shift that requires range analysis */
        if (i % 3 == 0) {
            product = product >> 1;
        }
    }
    return product;
}

/* Function 6: Switch based on fixed-point comparisons */
static const char* range_category(sat_fract_t value) {
    /* Switch that depends on range analysis */
    switch ((int)(value * 10.0r)) {
        case -9 ... -5:
            return "Very Negative";
        case -4 ... -1:
            return "Negative";
        case 0:
            return "Zero";
        case 1 ... 4:
            return "Positive";
        case 5 ... 9:
            return "Very Positive";
        default:
            return "Saturated";
    }
}

/* Function 7: Complex expression with multiple operations */
static sat_fract_t complex_fract_expression(sat_fract_t x, sat_fract_t y, int iterations) {
    sat_fract_t result = x;
    for (int i = 0; i < iterations; i++) {
        /* Nested ternary operations */
        result = (result > 0.8r) ? 
                 (result * 0.5r + y * 0.3r) :
                 (result * 1.2r - y * 0.2r);
        
        /* Bit manipulation through asm */
        sat_fract_t temp;
        asm volatile (
            "/* Fixed-point asm block */"
            : "=r" (temp)
            : "0" (result)
        );
        
        /* Force range analysis at boundaries */
        if (result >= 0.999r || result <= -0.999r) {
            result = (result > 0) ? 0.999r : -0.999r;
        }
    }
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int array_size = (argc > 2) ? atoi(argv[2]) : 50;
    
    if (iterations <= 0) iterations = 100;
    if (array_size <= 0) array_size = 50;
    
    printf("Testing fixed-point range analysis with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize arrays with different fixed-point values */
    fract_t fract_array[100];
    sat_accum_t accum_array[100];
    
    for (int i = 0; i < array_size && i < 100; i++) {
        /* Create values that approach saturation */
        fract_array[i] = (fract_t)((i % 20) - 10) * 0.1r;
        accum_array[i] = (sat_accum_t)((i % 30) - 15) * 0.066k;
    }
    
    /* Test 1: Complex arithmetic */
    sat_fract_t test1 = add_with_saturation(0.7r, 0.3r, 0.5r);
    printf("Test 1 result: %f\n", (float)test1);
    
    /* Test 2: Multiplication with overflow */
    sat_accum_t test2 = multiply_check_overflow(0.8k, 0.9k, 2);
    printf("Test 2 result: %f\n", (float)test2);
    
    /* Test 3: Loop accumulation */
    sat_fract_t test3 = accumulate_fractals(fract_array, array_size);
    printf("Test 3 result: %f\n", (float)test3);
    
    /* Test 4: Mixed operations */
    accum_t test4 = mixed_operations(0.6r, 0.4r, 0.5k);
    printf("Test 4 result: %f\n", (float)test4);
    
    /* Test 5: Array product with saturation */
    sat_accum_t test5 = saturating_array_product(accum_array, array_size);
    printf("Test 5 result: %f\n", (float)test5);
    
    /* Test 6: Switch categorization */
    const char* cat1 = range_category(0.75r);
    const char* cat2 = range_category(-0.33r);
    const char* cat3 = range_category(1.1r);  /* Should saturate */
    printf("Test 6: 0.75->%s, -0.33->%s, 1.1->%s\n", cat1, cat2, cat3);
    
    /* Test 7: Complex expression */
    sat_fract_t test7 = complex_fract_expression(0.1r, 0.2r, iterations);
    printf("Test 7 result: %f\n", (float)test7);
    
    /* Final checksum to prevent dead code elimination */
    sat_accum_t checksum = (sat_accum_t)test1 + (sat_accum_t)test2 + 
                          (sat_accum_t)test3 + test4 + test5 + 
                          (sat_accum_t)test7;
    
    /* Additional boundary tests */
    sat_fract_t boundary_test = 0.999999r;
    boundary_test = boundary_test + 0.000001r;  /* Should saturate */
    checksum += (sat_accum_t)boundary_test;
    
    sat_accum_t neg_boundary = -0.999999k;
    neg_boundary = neg_boundary - 0.000001k;  /* Should saturate */
    checksum += neg_boundary;
    
    printf("Final checksum: %f\n", (float)checksum);
    
    /* Use checksum in conditional to force analysis */
    if (checksum > 0.5k || checksum < -0.5k) {
        printf("Checksum magnitude is significant\n");
    }
    
    return 0;
}
