/* test_fixed_point.c - Comprehensive test for fixed-point range analysis */
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

/* Test functions that perform fixed-point operations */
static inline sfract_t add_saturated(sfract_t a, sfract_t b) {
    return a + b;  /* This should trigger saturation analysis */
}

static inline saccum_t multiply_saturated(saccum_t a, saccum_t b) {
    return a * b;  /* Multiplication with saturation */
}

static inline fract_t shift_and_add(fract_t base, int shift) {
    /* Complex expression that requires range analysis */
    return (base >> shift) + (base << (4 - shift));
}

static inline accum_t accum_range_test(accum_t a, accum_t b, accum_t c) {
    /* Nested operations that require precise range tracking */
    accum_t temp = (a * b) / c;
    return temp + (temp >> 2) - (temp << 1);
}

/* Function with conditional that depends on fixed-point range */
static int check_overflow_condition(sfract_t x, sfract_t y) {
    sfract_t sum = x + y;
    
    /* This condition should trigger the uncovered if statement */
    if (sum > 0.999r || sum < -0.999r) {
        return 1;  /* Would saturate */
    }
    
    /* Complex condition with multiple comparisons */
    if ((x > 0.5r && y > 0.5r) || (x < -0.5r && y < -0.5r)) {
        return 2;  /* High probability of saturation */
    }
    
    return 0;  /* No saturation expected */
}

/* Loop-based range propagation test */
static saccum_t loop_accumulation(int iterations, fract_t base) {
    saccum_t total = 0.0k;
    fract_t increment = base;
    
    for (int i = 0; i < iterations; i++) {
        /* This loop creates complex range dependencies */
        total = total + (saccum_t)increment;
        increment = increment * 0.95r;  /* Geometric decay */
        
        /* Conditional inside loop affects range */
        if (total > 10.0k) {
            total = total - 5.0k;
        }
    }
    
    return total;
}

/* Array reduction with fixed-point types */
static accum_t array_reduction(const fract_t* arr, int size) {
    accum_t sum = 0.0k;
    accum_t product = 1.0k;
    
    for (int i = 0; i < size; i++) {
        sum = sum + (accum_t)arr[i];
        
        /* Product can overflow/underflow - needs range analysis */
        product = product * (accum_t)arr[i];
        
        /* Conditional that depends on accumulated values */
        if (sum > product) {
            product = product * 0.5k;
        }
    }
    
    /* Final complex expression */
    return (sum + product) / 2.0k;
}

/* Test function using builtins for overflow detection */
static int builtin_overflow_test(sfract_t a, sfract_t b, sfract_t* result) {
    sfract_t temp;
    
    /* Use builtin for overflow detection */
    if (__builtin_add_overflow(a, b, &temp)) {
        *result = (a > 0) ? 0.999r : -0.999r;
        return 1;
    }
    
    *result = temp;
    
    /* Test multiplication overflow */
    if (__builtin_mul_overflow(a, b, &temp)) {
        return 2;
    }
    
    return 0;
}

/* Switch statement based on fixed-point ranges */
static const char* range_category(saccum_t value) {
    /* Categories based on value ranges */
    if (value < -5.0k) return "Very Low";
    if (value < 0.0k) return "Low";
    if (value < 5.0k) return "Medium";
    if (value < 10.0k) return "High";
    return "Very High";
}

/* Ternary operations with fixed-point */
static sfract_t ternary_operation(sfract_t a, sfract_t b, int flag) {
    /* Complex ternary expression */
    return flag ? 
           (a > b ? a + 0.1r : b - 0.1r) :
           (a < b ? a * 1.1r : b * 0.9r);
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
    
    printf("Testing fixed-point range analysis with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize test arrays */
    fract_t* frac_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    if (!frac_array) return 1;
    
    /* Fill array with values that will trigger various range conditions */
    for (int i = 0; i < array_size; i++) {
        /* Mix of values that can cause overflow/underflow */
        frac_array[i] = (fract_t)((i % 10) * 0.1r);
        if (i % 3 == 0) frac_array[i] = -frac_array[i];
    }
    
    /* Test 1: Basic saturation operations */
    printf("\nTest 1: Saturation boundary tests\n");
    sfract_t max_fract = 0.999999r;
    sfract_t min_fract = -0.999999r;
    
    /* Operations designed to hit saturation boundaries */
    sfract_t test1 = add_saturated(max_fract, 0.5r);
    sfract_t test2 = add_saturated(min_fract, -0.5r);
    
    printf("  max + 0.5 = %f (as float)\n", (float)test1);
    printf("  min - 0.5 = %f (as float)\n", (float)test2);
    
    /* Test 2: Multiplication saturation */
    saccum_t accum1 = 5.0k;
    saccum_t accum2 = 3.0k;
    saccum_t product = multiply_saturated(accum1, accum2);
    printf("  5.0k * 3.0k = %f (as float)\n", (float)product);
    
    /* Test 3: Overflow condition checking */
    printf("\nTest 3: Overflow condition analysis\n");
    int cond1 = check_overflow_condition(0.8r, 0.9r);  /* Should saturate */
    int cond2 = check_overflow_condition(0.3r, 0.4r);  /* Should not saturate */
    printf("  Condition 0.8r+0.9r: %d\n", cond1);
    printf("  Condition 0.3r+0.4r: %d\n", cond2);
    
    /* Test 4: Loop-based accumulation */
    printf("\nTest 4: Loop accumulation test\n");
    saccum_t loop_result = loop_accumulation(iterations, 0.5r);
    printf("  Loop result after %d iterations: %f\n", iterations, (float)loop_result);
    
    /* Test 5: Array reduction */
    printf("\nTest 5: Array reduction\n");
    accum_t reduction_result = array_reduction(frac_array, array_size);
    printf("  Array reduction result: %f\n", (float)reduction_result);
    
    /* Test 6: Builtin overflow detection */
    printf("\nTest 6: Builtin overflow detection\n");
    sfract_t builtin_result;
    int overflow_code = builtin_overflow_test(0.9r, 0.8r, &builtin_result);
    printf("  Builtin test result: code=%d, value=%f\n", 
           overflow_code, (float)builtin_result);
    
    /* Test 7: Range categorization */
    printf("\nTest 7: Range categorization\n");
    const char* cat1 = range_category(-7.5k);
    const char* cat2 = range_category(2.3k);
    const char* cat3 = range_category(12.0k);
    printf("  -7.5k -> %s\n", cat1);
    printf("  2.3k -> %s\n", cat2);
    printf("  12.0k -> %s\n", cat3);
    
    /* Test 8: Ternary operations */
    printf("\nTest 8: Ternary operations\n");
    sfract_t ternary1 = ternary_operation(0.6r, 0.4r, 1);
    sfract_t ternary2 = ternary_operation(0.3r, 0.7r, 0);
    printf("  Ternary 1: %f\n", (float)ternary1);
    printf("  Ternary 2: %f\n", (float)ternary2);
    
    /* Test 9: Complex shift operations */
    printf("\nTest 9: Shift operations\n");
    fract_t shift_test = shift_and_add(0.75r, 2);
    printf("  Shift test result: %f\n", (float)shift_test);
    
    /* Test 10: Accumulator range test */
    printf("\nTest 10: Accumulator range test\n");
    accum_t accum_test = accum_range_test(2.5k, 1.5k, 0.75k);
    printf("  Accumulator test: %f\n", (float)accum_test);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    accum_t checksum = (accum_t)test1 + (accum_t)test2 + product + 
                      (accum_t)loop_result + reduction_result + 
                      (accum_t)builtin_result + (accum_t)ternary1 +
                      (accum_t)ternary2 + (accum_t)shift_test + accum_test;
    
    printf("  Total checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(frac_array);
    
    return 0;
}
