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
static inline sat_fract_t complex_sat_add(sat_fract_t a, sat_fract_t b, sat_fract_t c) {
    /* This should trigger range analysis for saturation */
    sat_fract_t tmp = a + b;
    if (tmp > 0.8r) {
        tmp = tmp * 0.5r;
    }
    return tmp + c;
}

/* Function 2: Fixed-point multiplication with overflow analysis */
static inline sat_accum_t accum_multiply(sat_accum_t x, sat_accum_t y, int shift) {
    sat_accum_t result = x * y;
    /* Shift operation that requires range analysis */
    if (shift > 0) {
        result = result >> shift;
    } else if (shift < 0) {
        result = result << (-shift);
    }
    return result;
}

/* Function 3: Fixed-point division with range propagation */
static inline fract_t fract_divide(fract_t a, fract_t b) {
    if (b == 0.0r) return 0.0r;
    fract_t result = a / b;
    /* Conditional based on range */
    if (result > 0.9r && result < 1.1r) {
        return result * 0.5r;
    }
    return result;
}

/* Function 4: Loop-based fixed-point accumulation */
static sat_accum_t loop_accumulation(int iterations, fract_t base) {
    sat_accum_t total = 0.0k;
    for (fract_t f = base; f < 0.9r; f += 0.1r) {
        total = total + (_Accum)f;
        /* Nested condition that depends on accumulated value */
        if (total > 5.0k) {
            total = total * 0.8k;
        }
    }
    
    /* Additional loop with different step */
    for (int i = 0; i < iterations; i++) {
        fract_t increment = (fract_t)i * 0.01r;
        total = total + (_Accum)increment;
    }
    return total;
}

/* Function 5: Fixed-point array reduction */
static sat_fract_t array_reduction(const fract_t* arr, int size) {
    sat_fract_t sum = 0.0r;
    sat_fract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        sum = sum + arr[i];
        product = product * arr[i];
        
        /* Conditional that depends on intermediate range */
        if (sum > 0.5r && product < 0.1r) {
            sum = sum - 0.1r;
        }
    }
    
    /* Ternary operator with fixed-point operands */
    return (sum > product) ? sum : product;
}

/* Function 6: Mixed-type operations triggering conversions */
static sat_accum_t mixed_operations(sat_fract_t f, sat_accum_t a, int shift) {
    /* Mix different fixed-point types */
    sat_accum_t result = (sat_accum_t)f + a;
    
    /* Shift operation that can overflow */
    if (shift > 0 && shift < 16) {
        result = result >> shift;
    }
    
    /* Multiplication near saturation boundaries */
    result = result * 1.5k;
    
    return result;
}

/* Function 7: Switch statement based on fixed-point comparisons */
static int fixed_point_switch(sat_fract_t value) {
    int result = 0;
    
    /* Switch where cases depend on fixed-point range analysis */
    if (value < 0.2r) {
        result = 1;
    } else if (value >= 0.2r && value < 0.5r) {
        result = 2;
    } else if (value >= 0.5r && value < 0.8r) {
        result = 3;
    } else {
        result = 4;
    }
    
    return result;
}

/* Function 8: Using builtins for overflow detection */
static int builtin_overflow_test(sat_fract_t a, sat_fract_t b, sat_fract_t* res) {
    /* Use overflow builtins with fixed-point */
    int overflow = 0;
    sat_fract_t tmp;
    
    /* Simulate overflow check */
    tmp = a + b;
    if (tmp == 1.0r || tmp == -1.0r) {
        overflow = 1;
    }
    
    *res = tmp;
    return overflow;
}

/* Function 9: Complex expression requiring range analysis */
static sat_accum_t complex_range_expr(sat_accum_t x, sat_accum_t y, int n) {
    sat_accum_t result = 0.0k;
    
    for (int i = 0; i < n; i++) {
        /* Complex expression that should trigger the uncovered code */
        result = result + (x * y) >> (i % 4);
        
        /* Nested condition */
        if (result > 10.0k) {
            result = result / 2.0k;
        } else if (result < -10.0k) {
            result = result * (-0.5k);
        }
    }
    
    return result;
}

/* Function 10: Inline assembly with fixed-point constraints */
static sat_fract_t asm_fixed_point(sat_fract_t a, sat_fract_t b) {
    sat_fract_t result;
    
    /* Use inline assembly to create hard-to-analyze value flow */
    asm volatile (
        "/* Fixed-point assembly block */"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Force additional operations */
    result = result * 0.5r;
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    int iterations = 10;
    int array_size = 8;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 8;
        if (array_size > 20) array_size = 20;
    }
    
    printf("Testing fixed-point range analysis (iterations=%d, array_size=%d)\n", 
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    fract_t fract_array[20];
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (fract_t)i * 0.1r;
    }
    
    /* Test 1: Complex saturation addition */
    sat_fract_t sf1 = 0.7r;
    sat_fract_t sf2 = 0.3r;
    sat_fract_t sf3 = 0.1r;
    sat_fract_t result1 = complex_sat_add(sf1, sf2, sf3);
    printf("Test 1 result: %f\n", (float)result1);
    
    /* Test 2: Accumulator multiplication with shifts */
    sat_accum_t sa1 = 2.5k;
    sat_accum_t sa2 = 1.5k;
    sat_accum_t result2 = accum_multiply(sa1, sa2, 2);
    printf("Test 2 result: %f\n", (float)result2);
    
    /* Test 3: Division with range checks */
    fract_t f1 = 0.8r;
    fract_t f2 = 0.9r;
    fract_t result3 = fract_divide(f1, f2);
    printf("Test 3 result: %f\n", (float)result3);
    
    /* Test 4: Loop accumulation */
    sat_accum_t result4 = loop_accumulation(iterations, 0.1r);
    printf("Test 4 result: %f\n", (float)result4);
    
    /* Test 5: Array reduction */
    sat_fract_t result5 = array_reduction(fract_array, array_size);
    printf("Test 5 result: %f\n", (float)result5);
    
    /* Test 6: Mixed operations */
    sat_accum_t result6 = mixed_operations(0.7r, 3.0k, 3);
    printf("Test 6 result: %f\n", (float)result6);
    
    /* Test 7: Switch based on fixed-point */
    int switch_result = fixed_point_switch(0.6r);
    printf("Test 7 switch result: %d\n", switch_result);
    
    /* Test 8: Builtin-like overflow check */
    sat_fract_t overflow_res;
    int overflow = builtin_overflow_test(0.9r, 0.2r, &overflow_res);
    printf("Test 8 overflow: %d, result: %f\n", overflow, (float)overflow_res);
    
    /* Test 9: Complex range expression */
    sat_accum_t result9 = complex_range_expr(1.5k, 2.0k, iterations);
    printf("Test 9 result: %f\n", (float)result9);
    
    /* Test 10: Assembly-influenced operations */
    sat_fract_t result10 = asm_fixed_point(0.4r, 0.6r);
    printf("Test 10 result: %f\n", (float)result10);
    
    /* Final checksum to prevent dead code elimination */
    sat_accum_t checksum = 0.0k;
    checksum = checksum + (_Accum)result1;
    checksum = checksum + result2;
    checksum = checksum + (_Accum)result3;
    checksum = checksum + result4;
    checksum = checksum + (_Accum)result5;
    checksum = checksum + result6;
    checksum = checksum + (_Accum)switch_result;
    checksum = checksum + (_Accum)overflow_res;
    checksum = checksum + result9;
    checksum = checksum + (_Accum)result10;
    
    printf("Final checksum: %f\n", (float)checksum);
    
    /* Additional edge case tests */
    
    /* Test saturation boundaries */
    sat_fract_t max_fract = 0.999999r;
    sat_fract_t saturated = max_fract + 0.1r;
    printf("Saturation test: %f + 0.1 = %f\n", (float)max_fract, (float)saturated);
    
    /* Test negative saturation */
    sat_fract_t min_fract = -0.999999r;
    sat_fract_t neg_saturated = min_fract - 0.1r;
    printf("Negative saturation: %f - 0.1 = %f\n", (float)min_fract, (float)neg_saturated);
    
    /* Test shift operations that could trigger the uncovered code */
    sat_accum_t shift_test = 5.0k;
    for (int i = 0; i < 5; i++) {
        shift_test = shift_test >> 1;
        printf("Shift iteration %d: %f\n", i, (float)shift_test);
    }
    
    /* Complex nested condition that should trigger range analysis */
    sat_accum_t x = 0.0k;
    for (int i = 0; i < iterations; i++) {
        x = x + 0.5k;
        if (x > 2.0k && x < 4.0k) {
            x = x * 0.75k;
        } else if (x >= 4.0k) {
            x = x - 1.0k;
        }
    }
    printf("Final x after complex conditions: %f\n", (float)x);
    
    return 0;
}
