/* test_fixed.c - Program to trigger fixed-point range analysis */
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

/* Test functions with complex fixed-point operations */

/* Function 1: Saturation boundary testing */
static inline sfract_t test_saturation_boundary(sfract_t a, sfract_t b) {
    /* Operations designed to hit saturation boundaries */
    sfract_t sum = a + b;
    sfract_t prod = a * b;
    
    /* This addition may saturate */
    sfract_t result = sum + prod;
    
    /* Conditional based on range analysis */
    if (result > 0.9r) {
        return result * 0.5r;
    } else if (result < -0.9r) {
        return result * 2.0r;
    }
    
    return result;
}

/* Function 2: Accumulator with shift operations */
static inline saccum_t test_accum_shifts(saccum_t x, int shift) {
    /* Complex shift operations requiring range analysis */
    saccum_t shifted;
    
    if (shift > 0) {
        shifted = x >> shift;
    } else {
        shifted = x << (-shift);
    }
    
    /* Multiplication that may overflow */
    saccum_t scaled = shifted * 2.0k;
    
    /* Ternary with fixed-point operands */
    return (scaled > 10.0k) ? scaled * 0.8k : scaled * 1.2k;
}

/* Function 3: Loop-based range propagation */
static sfract_t test_loop_range(int iterations, sfract_t start) {
    sfract_t total = 0.0r;
    sfract_t current = start;
    
    /* Loop where induction variable affects fixed-point range */
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        sfract_t increment = current * (0.1r + (i % 3) * 0.05r);
        
        /* Conditional that depends on accumulated range */
        if (total + increment > 0.95r) {
            total = 0.95r;
            break;
        } else if (total + increment < -0.95r) {
            total = -0.95r;
            break;
        }
        
        total += increment;
        current *= 0.9r;
    }
    
    return total;
}

/* Function 4: Array reduction with mixed types */
static accum_t test_array_reduction(const fract_t* arr, int size) {
    accum_t total = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Mixed-type operations requiring conversion analysis */
        accum_t scaled = (accum_t)arr[i] * 2.0k;
        
        /* Shift operation that may underflow/overflow */
        if (i % 2 == 0) {
            scaled = scaled >> 1;
        } else {
            scaled = scaled << 1;
        }
        
        total += scaled;
        
        /* Conditional based on intermediate range */
        if (total > 100.0k || total < -100.0k) {
            total = (total > 0) ? 100.0k : -100.0k;
        }
    }
    
    return total;
}

/* Function 5: Switch statement with fixed-point conditions */
static sfract_t test_switch_range(sfract_t val) {
    sfract_t result = 0.0r;
    
    /* Switch where cases depend on fixed-point comparisons */
    if (val > 0.8r) {
        result = val * 0.5r;
    } else if (val > 0.6r) {
        result = val * 0.7r;
    } else if (val > 0.4r) {
        result = val * 0.9r;
    } else if (val > 0.2r) {
        result = val * 1.1r;
    } else {
        result = val * 1.3r;
    }
    
    /* Additional overflow-prone operation */
    result = result + result * 0.5r;
    
    return result;
}

/* Function 6: Using builtins for overflow detection */
static int test_builtin_overflow(sfract_t a, sfract_t b, sfract_t* res) {
    /* Use overflow builtins with fixed-point types */
    int overflow = 0;
    
    /* These should trigger range analysis */
    overflow |= __builtin_add_overflow(a, b, res);
    
    sfract_t temp;
    overflow |= __builtin_mul_overflow(*res, 2.0r, &temp);
    
    *res = temp;
    return overflow;
}

/* Function 7: Complex nested expressions */
static saccum_t test_nested_expressions(saccum_t a, saccum_t b, saccum_t c) {
    /* Deeply nested expression requiring extensive range analysis */
    saccum_t result = ((a * b) >> 2) + ((b * c) >> 3) - ((a * c) >> 4);
    
    /* Multiple conditional checks */
    if (result > 50.0k) {
        result = ((result - 25.0k) * 0.8k) >> 1;
    } else if (result < -50.0k) {
        result = ((result + 25.0k) * 0.8k) << 1;
    }
    
    /* Final saturation check */
    if (result > 100.0k) result = 100.0k;
    if (result < -100.0k) result = -100.0k;
    
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Use command-line arguments for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Initialize fixed-point arrays */
    const int ARRAY_SIZE = 20;
    fract_t fract_array[ARRAY_SIZE];
    sfract_t sfract_array[ARRAY_SIZE];
    
    /* Fill arrays with values that will trigger range analysis */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Values designed to test boundary conditions */
        fract_t base_val = (fract_t)((i - ARRAY_SIZE/2) * 0.05r);
        fract_array[i] = base_val;
        sfract_array[i] = (_Sat _Fract)base_val;
    }
    
    /* Test 1: Saturation boundaries */
    printf("Test 1 - Saturation Boundaries:\n");
    for (int i = 0; i < 5; i++) {
        sfract_t a = (sfract_t)(0.8r + i * 0.05r);
        sfract_t b = (sfract_t)(0.3r - i * 0.05r);
        sfract_t result = test_saturation_boundary(a, b);
        printf("  test_saturation_boundary(%.3f, %.3f) = %.3f\n", 
               (float)a, (float)b, (float)result);
    }
    
    /* Test 2: Accumulator shifts */
    printf("\nTest 2 - Accumulator Shifts:\n");
    saccum_t accum = 5.0k;
    for (int shift = -3; shift <= 3; shift++) {
        saccum_t result = test_accum_shifts(accum, shift);
        printf("  test_accum_shifts(%.3f, %d) = %.3f\n",
               (float)accum, shift, (float)result);
    }
    
    /* Test 3: Loop-based range propagation */
    printf("\nTest 3 - Loop Range Propagation:\n");
    sfract_t loop_result = test_loop_range(iterations, 0.5r);
    printf("  test_loop_range(%d, 0.5) = %.3f\n", 
           iterations, (float)loop_result);
    
    /* Test 4: Array reduction */
    printf("\nTest 4 - Array Reduction:\n");
    accum_t array_result = test_array_reduction(fract_array, ARRAY_SIZE);
    printf("  test_array_reduction(array, %d) = %.3f\n",
           ARRAY_SIZE, (float)array_result);
    
    /* Test 5: Switch with fixed-point conditions */
    printf("\nTest 5 - Switch Statements:\n");
    for (sfract_t val = 0.1r; val < 1.0r; val += 0.2r) {
        sfract_t switch_result = test_switch_range(val);
        printf("  test_switch_range(%.3f) = %.3f\n",
               (float)val, (float)switch_result);
    }
    
    /* Test 6: Builtin overflow detection */
    printf("\nTest 6 - Builtin Overflow:\n");
    sfract_t builtin_res;
    int overflow = test_builtin_overflow(0.7r, 0.6r, &builtin_res);
    printf("  test_builtin_overflow(0.7, 0.6) = %.3f, overflow=%d\n",
           (float)builtin_res, overflow);
    
    /* Test 7: Nested expressions */
    printf("\nTest 7 - Nested Expressions:\n");
    saccum_t nested_result = test_nested_expressions(10.0k, 20.0k, 30.0k);
    printf("  test_nested_expressions(10.0, 20.0, 30.0) = %.3f\n",
           (float)nested_result);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal Checksum Calculation:\n");
    
    /* Complex checksum using all test results */
    accum_t checksum = (accum_t)loop_result + 
                      (accum_t)array_result + 
                      (accum_t)nested_result;
    
    /* Additional fixed-point operations for checksum */
    for (int i = 0; i < iterations % 10; i++) {
        checksum = checksum * 1.1k;
        checksum = checksum >> 1;
    }
    
    printf("  Final checksum: %.6f\n", (float)checksum);
    
    return 0;
}
