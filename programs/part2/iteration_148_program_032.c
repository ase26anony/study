/* test_fixed_point.c - Target coverage for fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef _Sat long _Fract sat_long_fract_t;

/* Test functions with complex fixed-point operations */

/* Function 1: Range analysis with saturation boundaries */
static inline sat_fract_t sat_add_with_check(sat_fract_t a, sat_fract_t b) {
    /* This should trigger range analysis for saturation */
    sat_fract_t result = a + b;
    
    /* Conditional that depends on range analysis */
    if (a + b > 0.999999r) {
        return 0.999999r;  /* Explicit saturation */
    }
    return result;
}

/* Function 2: Multiplication with overflow analysis */
static inline sat_accum_t mul_with_overflow(sat_accum_t x, sat_accum_t y) {
    /* Complex expression requiring range analysis */
    sat_accum_t temp = (x * y) >> 3;
    
    /* Nested operations that need precise range tracking */
    if (x > 0k && y > 0k) {
        temp = temp + (x >> 2) + (y >> 2);
    }
    return temp;
}

/* Function 3: Loop-based range propagation */
static sat_accum_t loop_accumulation(int iterations, fract_t base) {
    sat_accum_t total = 0k;
    
    /* Loop where induction variable affects fixed-point range */
    for (fract_t f = base; f < base + 0.5r; f += 0.1r) {
        total = total + (_Accum)f;
        
        /* Conditional that forces range evaluation */
        if (total > 10k) {
            total = 10k;  /* Manual saturation */
        }
    }
    
    /* Additional loop with different step */
    for (int i = 0; i < iterations; i++) {
        fract_t increment = 0.01r * i;
        total = total + (_Accum)increment;
    }
    
    return total;
}

/* Function 4: Array reduction with mixed types */
static sat_fract_t array_reduction(fract_t* arr, int size) {
    sat_fract_t sum = 0r;
    
    for (int i = 0; i < size; i++) {
        /* Complex expression that could overflow */
        sum = sum + arr[i] * 0.5r;
        
        /* Ternary operator with fixed-point operands */
        sum = (sum > 0.9r) ? 0.9r : sum;
    }
    
    return sum;
}

/* Function 5: Shift operations that can underflow/overflow */
static sat_accum_t shift_operations(sat_accum_t val, int shift) {
    sat_accum_t result = val;
    
    /* Multiple shift operations */
    result = result >> shift;
    result = result << (shift / 2);
    
    /* Conditional based on shifted value */
    if (result < -5k || result > 5k) {
        result = (result > 0k) ? 5k : -5k;
    }
    
    return result;
}

/* Function 6: Using builtins for overflow detection */
static int builtin_overflow_test(accum_t a, accum_t b, accum_t* res) {
    /* Use builtin overflow check */
    int overflow = __builtin_mul_overflow(a, b, res);
    
    if (!overflow) {
        /* Additional operation if no overflow */
        *res = *res >> 2;
    }
    
    return overflow;
}

/* Function 7: Switch statement with fixed-point conditions */
static int switch_fixed_point(sat_fract_t val) {
    int result = 0;
    
    /* Switch that depends on fixed-point comparisons */
    switch ((int)(val * 10)) {
        case 0 ... 3:
            result = 1;
            break;
        case 4 ... 6:
            result = 2;
            break;
        case 7 ... 10:
            result = 3;
            break;
        default:
            result = 0;
    }
    
    return result;
}

/* Function 8: Complex expression tree */
static sat_accum_t complex_expression(sat_accum_t a, sat_accum_t b, sat_accum_t c) {
    /* Deep expression tree requiring range analysis */
    sat_accum_t result = ((a * b) + (c << 2)) >> 1;
    result = result - (a >> 2) + (b >> 3);
    
    /* Nested conditional */
    if (a > 0k) {
        result = result * 0.75k;
    } else {
        result = result / 0.75k;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
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
    
    /* Initialize fixed-point arrays */
    fract_t* fract_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    accum_t* accum_array = (accum_t*)malloc(array_size * sizeof(accum_t));
    
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (fract_t)(i * 0.05r);
        accum_array[i] = (accum_t)(i * 0.1k);
    }
    
    /* Test 1: Saturation boundary tests */
    printf("Test 1: Saturation boundaries\n");
    sat_fract_t max_fract = 0.999999r;
    sat_fract_t test1 = sat_add_with_check(max_fract, 0.1r);
    printf("  sat_add_with_check(0.999999r, 0.1r) = %f\n", (float)test1);
    
    /* Test 2: Multiplication overflow */
    printf("\nTest 2: Multiplication overflow\n");
    sat_accum_t test2 = mul_with_overflow(2.5k, 3.0k);
    printf("  mul_with_overflow(2.5k, 3.0k) = %f\n", (float)test2);
    
    /* Test 3: Loop accumulation */
    printf("\nTest 3: Loop accumulation\n");
    sat_accum_t test3 = loop_accumulation(iterations, 0.2r);
    printf("  loop_accumulation(%d, 0.2r) = %f\n", iterations, (float)test3);
    
    /* Test 4: Array reduction */
    printf("\nTest 4: Array reduction\n");
    sat_fract_t test4 = array_reduction(fract_array, array_size);
    printf("  array_reduction(array, %d) = %f\n", array_size, (float)test4);
    
    /* Test 5: Shift operations */
    printf("\nTest 5: Shift operations\n");
    sat_accum_t test5 = shift_operations(2.0k, 3);
    printf("  shift_operations(2.0k, 3) = %f\n", (float)test5);
    
    /* Test 6: Builtin overflow */
    printf("\nTest 6: Builtin overflow detection\n");
    accum_t overflow_res;
    int overflow = builtin_overflow_test(1.5k, 2.0k, &overflow_res);
    printf("  builtin_overflow_test(1.5k, 2.0k) = %f, overflow=%d\n", 
           (float)overflow_res, overflow);
    
    /* Test 7: Switch with fixed-point */
    printf("\nTest 7: Switch statement\n");
    int test7 = switch_fixed_point(0.75r);
    printf("  switch_fixed_point(0.75r) = %d\n", test7);
    
    /* Test 8: Complex expression */
    printf("\nTest 8: Complex expression\n");
    sat_accum_t test8 = complex_expression(1.0k, 2.0k, 3.0k);
    printf("  complex_expression(1.0k, 2.0k, 3.0k) = %f\n", (float)test8);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    sat_accum_t checksum = 0k;
    
    /* Mix all results */
    checksum = checksum + (_Accum)test1;
    checksum = checksum + test2;
    checksum = checksum + test3;
    checksum = checksum + (_Accum)test4;
    checksum = checksum + test5;
    checksum = checksum + overflow_res;
    checksum = checksum + (_Accum)test7;
    checksum = checksum + test8;
    
    /* Additional complex expression for checksum */
    for (int i = 0; i < iterations; i++) {
        checksum = checksum + (_Accum)(i * 0.01r);
        
        /* Conditional that depends on range */
        if (checksum > 20k) {
            checksum = checksum - 5k;
        }
    }
    
    printf("  Final checksum = %f\n", (float)checksum);
    
    /* Cleanup */
    free(fract_array);
    free(accum_array);
    
    return 0;
}
