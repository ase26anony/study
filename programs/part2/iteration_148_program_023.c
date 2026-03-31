/* test_fixed_point.c - Program to exercise GCC's fixed-point range analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sfract_t;
typedef _Fract fract_t;
typedef _Sat _Accum saccum_t;
typedef _Accum accum_t;
typedef _Sat long _Fract slfract_t;
typedef long _Fract lfract_t;

/* Test functions with various fixed-point operations */

/* Function 1: Complex saturated addition with range-dependent branching */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b, fract_t threshold) {
    sfract_t sum = a + b;
    /* This condition should trigger range analysis */
    if (sum > threshold) {
        return sum - (threshold / 2);
    }
    return sum + (threshold / 4);
}

/* Function 2: Multiplication with shift causing potential overflow */
static inline saccum_t mult_with_shift(saccum_t x, saccum_t y, int shift) {
    saccum_t prod = x * y;
    /* Shift operation that requires range analysis */
    if (shift > 0) {
        prod = prod >> shift;
    } else {
        prod = prod << (-shift);
    }
    return prod;
}

/* Function 3: Loop-based accumulation with saturation */
static sfract_t loop_accumulation(int iterations, fract_t base) {
    sfract_t total = 0.0r;
    fract_t increment = base;
    
    /* Loop where range analysis is needed for the induction variable */
    for (int i = 0; i < iterations; i++) {
        total = total + increment;
        increment = increment * 0.9r;  /* Decreasing step */
        
        /* Conditional that depends on accumulated range */
        if (total > 0.8r) {
            total = total - 0.3r;
        }
    }
    return total;
}

/* Function 4: Array reduction with mixed saturation */
static accum_t array_reduction(const fract_t* arr, int size) {
    accum_t sum = 0.0k;
    saccum_t sat_sum = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Mix saturated and unsaturated operations */
        accum_t temp = (accum_t)arr[i] * 2.0k;
        sum = sum + temp;
        
        /* This addition should trigger saturation analysis */
        sat_sum = sat_sum + (_Sat _Accum)temp;
        
        /* Range-dependent operation */
        if (i % 3 == 0) {
            sum = sum >> 1;
        } else if (i % 3 == 1) {
            sat_sum = sat_sum << 1;
        }
    }
    
    /* Final range check */
    if (sat_sum > sum) {
        return (accum_t)sat_sum;
    }
    return sum;
}

/* Function 5: Nested ternary with fixed-point operations */
static fract_t ternary_operation(fract_t a, fract_t b, int mode) {
    /* Complex ternary expression requiring range analysis */
    return (mode == 0) ? (a + b) :
           (mode == 1) ? (a * b) :
           (mode == 2) ? (a - b) :
           (a / (b != 0.0r ? b : 0.5r));
}

/* Function 6: Using builtins for overflow detection */
static int check_overflow(saccum_t* result, saccum_t a, saccum_t b) {
    /* Use builtin for overflow detection */
    return __builtin_add_overflow(a, b, result);
}

/* Function 7: Switch statement with fixed-point conditions */
static fract_t switch_based(fract_t val, int option) {
    fract_t result = 0.0r;
    
    switch (option) {
        case 0:
            result = val * 0.25r;
            /* Force range analysis for comparison */
            if (result > 0.5r) result = 0.5r;
            break;
        case 1:
            result = val / 0.75r;
            if (result < -0.5r) result = -0.5r;
            break;
        case 2:
            result = val + val;
            /* This comparison should trigger the target code */
            if (result > 0.999999r) result = 0.999999r;
            break;
        case 3:
            result = val - 0.5r;
            if (result < 0.0r) result = 0.0r;
            break;
        default:
            result = val;
    }
    
    return result;
}

/* Function 8: Complex expression with multiple operations */
static saccum_t complex_expression(saccum_t x, saccum_t y, int n) {
    saccum_t result = x;
    
    for (int i = 0; i < n; i++) {
        /* Chain of operations requiring precise range tracking */
        result = (result * y) >> 2;
        result = result + (x / (i + 2));
        
        /* Conditional that depends on accumulated range */
        if (result > 100.0k || result < -100.0k) {
            result = result >> 1;
        }
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Use command line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 20;
    int test_mode = (argc > 3) ? atoi(argv[3]) % 4 : 0;
    
    /* Initialize fixed-point arrays */
    fract_t frac_array[50];
    accum_t accum_array[50];
    
    for (int i = 0; i < 50; i++) {
        frac_array[i] = (fract_t)((i % 10) * 0.1r);
        accum_array[i] = (accum_t)((i % 20) * 0.05k);
    }
    
    /* Test 1: Saturated operations near boundaries */
    sfract_t sat_max = 0.999999r;
    sfract_t sat_min = -0.999999r;
    
    printf("Test 1 - Saturation boundaries:\n");
    sfract_t test1 = sat_max + 0.1r;  /* Should saturate */
    sfract_t test2 = sat_min - 0.1r;  /* Should saturate */
    printf("  sat_max + 0.1r = %f (as float)\n", (float)test1);
    printf("  sat_min - 0.1r = %f (as float)\n", (float)test2);
    
    /* Test 2: Range-dependent branching */
    printf("\nTest 2 - Range-dependent branching:\n");
    fract_t threshold = 0.7r;
    for (int i = 0; i < 5; i++) {
        fract_t a = (fract_t)(i * 0.2r);
        fract_t b = (fract_t)(0.3r);
        sfract_t result = sat_add_range(a, b, threshold);
        printf("  sat_add_range(%f, 0.3r, 0.7r) = %f\n", 
               (float)a, (float)result);
    }
    
    /* Test 3: Multiplication with shifts */
    printf("\nTest 3 - Multiplication with shifts:\n");
    saccum_t accum_val = 50.0k;
    for (int shift = -2; shift <= 2; shift++) {
        saccum_t result = mult_with_shift(accum_val, 2.0k, shift);
        printf("  50.0k * 2.0k >> %d = %f\n", shift, (float)result);
    }
    
    /* Test 4: Loop accumulation */
    printf("\nTest 4 - Loop accumulation (%d iterations):\n", iterations);
    sfract_t loop_result = loop_accumulation(iterations, 0.2r);
    printf("  loop_accumulation result = %f\n", (float)loop_result);
    
    /* Test 5: Array reduction */
    printf("\nTest 5 - Array reduction (size %d):\n", array_size);
    accum_t reduce_result = array_reduction(frac_array, array_size);
    printf("  array_reduction result = %f\n", (float)reduce_result);
    
    /* Test 6: Ternary operations */
    printf("\nTest 6 - Ternary operations:\n");
    for (int mode = 0; mode < 4; mode++) {
        fract_t a = 0.6r;
        fract_t b = 0.4r;
        fract_t result = ternary_operation(a, b, mode);
        printf("  ternary_operation(0.6r, 0.4r, mode=%d) = %f\n", 
               mode, (float)result);
    }
    
    /* Test 7: Overflow detection with builtins */
    printf("\nTest 7 - Overflow detection:\n");
    saccum_t of_result;
    saccum_t large_val = 100.0k;
    int overflow = check_overflow(&of_result, large_val, large_val);
    printf("  Overflow check 100.0k + 100.0k: overflow=%d, result=%f\n",
           overflow, (float)of_result);
    
    /* Test 8: Switch-based operations */
    printf("\nTest 8 - Switch-based operations (mode=%d):\n", test_mode);
    fract_t switch_val = 0.8r;
    fract_t switch_result = switch_based(switch_val, test_mode);
    printf("  switch_based(0.8r, %d) = %f\n", test_mode, (float)switch_result);
    
    /* Test 9: Complex expressions */
    printf("\nTest 9 - Complex expressions:\n");
    saccum_t complex_result = complex_expression(10.0k, 1.5k, iterations % 5 + 1);
    printf("  complex_expression(10.0k, 1.5k, %d) = %f\n", 
           iterations % 5 + 1, (float)complex_result);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    accum_t checksum = 0.0k;
    
    /* Mix all results into checksum */
    checksum += (accum_t)test1;
    checksum += (accum_t)test2;
    checksum += (accum_t)loop_result;
    checksum += reduce_result;
    checksum += (accum_t)switch_result;
    checksum += complex_result;
    
    /* Additional complex fixed-point computation */
    for (int i = 0; i < iterations % 10 + 1; i++) {
        checksum = checksum * 1.1k;
        checksum = checksum >> 1;
        
        /* Force range analysis with conditional */
        if (checksum > 1000.0k) {
            checksum = checksum / 2.0k;
        } else if (checksum < -1000.0k) {
            checksum = checksum * (-0.5k);
        }
    }
    
    printf("  Final checksum = %f\n", (float)checksum);
    
    return 0;
}
