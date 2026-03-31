/* test_fixed_point.c - Program to trigger fixed-point range analysis */
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

/* Test functions with different fixed-point operations */

/* Function 1: Basic saturated addition with range check */
static inline sfract_t sat_add_range_check(sfract_t a, sfract_t b) {
    sfract_t result = a + b;
    /* This condition should trigger range analysis */
    if (result > 0.9r) {
        return 0.9r;
    }
    return result;
}

/* Function 2: Complex multiplication with shifting */
static inline saccum_t complex_mul_shift(saccum_t x, saccum_t y, int shift) {
    saccum_t prod = x * y;
    /* Shift operation that requires range analysis */
    saccum_t shifted = prod >> shift;
    
    /* Conditional based on range */
    if (shifted < -0.5k && shifted > -1.0k) {
        return shifted * 2.0k;
    }
    return shifted;
}

/* Function 3: Loop-based accumulation with saturation */
static sfract_t loop_accumulation(int iterations, fract_t base) {
    sfract_t total = 0.0r;
    fract_t increment = 0.1r;
    
    for (int i = 0; i < iterations; i++) {
        total = total + base + (increment * i);
        /* This addition may saturate */
        if (total >= 0.95r) {
            total = 0.95r;
            break;
        }
    }
    return total;
}

/* Function 4: Array reduction with mixed types */
static saccum_t array_reduction(const accum_t* arr, int size) {
    saccum_t sum = 0.0k;
    for (int i = 0; i < size; i++) {
        /* Mix saturated and unsaturated operations */
        accum_t val = arr[i];
        saccum_t sat_val = val;
        
        /* Complex expression requiring range analysis */
        sum = sum + (sat_val * (i % 2 ? 0.5k : -0.5k));
        
        /* Range check that should trigger the uncovered code */
        if (sum > 10.0k || sum < -10.0k) {
            sum = (sum > 0) ? 10.0k : -10.0k;
        }
    }
    return sum;
}

/* Function 5: Division with overflow protection */
static sfract_t safe_divide(sfract_t a, sfract_t b) {
    /* Check for near-zero denominator */
    if (b > -0.01r && b < 0.01r) {
        return (a > 0) ? 1.0r : -1.0r;
    }
    
    sfract_t result = a / b;
    
    /* Range check that should trigger specific uncovered lines */
    if (result > 0.99r || result < -0.99r) {
        return (result > 0) ? 0.99r : -0.99r;
    }
    return result;
}

/* Function 6: Bit-shift operations on fixed-point */
static saccum_t shift_operations(saccum_t value, int shift_amt) {
    saccum_t result = value;
    
    /* Multiple shift operations */
    if (shift_amt > 0) {
        result = result >> shift_amt;
    } else if (shift_amt < 0) {
        result = result << (-shift_amt);
    }
    
    /* This comparison should trigger range analysis */
    if (result > 5.0k && result < 10.0k) {
        return result * 0.5k;
    }
    return result;
}

/* Function 7: Ternary operator with fixed-point */
static sfract_t ternary_fixed_point(sfract_t a, sfract_t b, int flag) {
    /* Complex ternary expression */
    sfract_t result = flag ? 
        (a + b > 0.5r ? a * 2.0r : b / 2.0r) :
        (a - b < -0.5r ? a * 0.5r : b * 1.5r);
    
    /* Range check */
    if (result > 0.9r || result < -0.9r) {
        return 0.0r;
    }
    return result;
}

/* Function 8: Switch statement with fixed-point */
static accum_t switch_fixed_point(accum_t val) {
    accum_t result = 0.0k;
    
    /* Switch based on fixed-point range */
    if (val < -0.5k) {
        result = val * 2.0k;
    } else if (val >= -0.5k && val < 0.0k) {
        result = val * 1.5k;
    } else if (val >= 0.0k && val < 0.5k) {
        result = val * 1.0k;
    } else {
        result = val * 0.5k;
    }
    
    return result;
}

/* Function 9: Built-in overflow checks */
static int check_overflow(sfract_t a, sfract_t b, sfract_t* result) {
    /* Use built-in overflow check */
    return __builtin_add_overflow(a, b, result);
}

/* Function 10: Nested function calls */
static sfract_t nested_calls(sfract_t a, sfract_t b, sfract_t c) {
    sfract_t t1 = sat_add_range_check(a, b);
    sfract_t t2 = safe_divide(t1, c);
    sfract_t t3 = ternary_fixed_point(t2, a, (t2 > 0.5r));
    
    return t3;
}

/* Main test function */
int main(int argc, char* argv[]) {
    /* Use command line arguments for variability */
    int iterations = argc > 1 ? atoi(argv[1]) : 10;
    int array_size = argc > 2 ? atoi(argv[2]) : 20;
    int shift_amount = argc > 3 ? atoi(argv[3]) : 3;
    
    printf("Testing fixed-point range analysis with:\n");
    printf("  iterations: %d\n", iterations);
    printf("  array_size: %d\n", array_size);
    printf("  shift_amount: %d\n\n", shift_amount);
    
    /* Initialize arrays */
    accum_t accum_array[100];
    fract_t fract_array[100];
    
    for (int i = 0; i < array_size && i < 100; i++) {
        accum_array[i] = (accum_t)((i - array_size/2) * 0.1k);
        fract_array[i] = (fract_t)(i * 0.01r);
    }
    
    /* Test 1: Basic saturated operations */
    sfract_t s1 = 0.7r;
    sfract_t s2 = 0.3r;
    sfract_t sum1 = sat_add_range_check(s1, s2);
    printf("Test 1 - Saturated add: %.4f + %.4f = %.4f\n", 
           (float)s1, (float)s2, (float)sum1);
    
    /* Test 2: Multiplication with shift */
    saccum_t acc1 = 2.5k;
    saccum_t acc2 = 1.5k;
    saccum_t prod1 = complex_mul_shift(acc1, acc2, shift_amount);
    printf("Test 2 - Complex mul/shift: %.4f * %.4f >> %d = %.4f\n",
           (float)acc1, (float)acc2, shift_amount, (float)prod1);
    
    /* Test 3: Loop accumulation */
    sfract_t loop_result = loop_accumulation(iterations, 0.05r);
    printf("Test 3 - Loop accumulation (%d iters): %.4f\n",
           iterations, (float)loop_result);
    
    /* Test 4: Array reduction */
    saccum_t reduce_result = array_reduction(accum_array, 
                                           array_size < 100 ? array_size : 100);
    printf("Test 4 - Array reduction: %.4f\n", (float)reduce_result);
    
    /* Test 5: Division */
    sfract_t div_result = safe_divide(0.8r, 0.4r);
    printf("Test 5 - Safe division: %.4f / %.4f = %.4f\n",
           0.8f, 0.4f, (float)div_result);
    
    /* Test 6: Shift operations */
    saccum_t shift_result = shift_operations(8.0k, shift_amount);
    printf("Test 6 - Shift operations: %.4f >> %d = %.4f\n",
           8.0f, shift_amount, (float)shift_result);
    
    /* Test 7: Ternary operator */
    sfract_t ternary_result = ternary_fixed_point(0.6r, 0.3r, 1);
    printf("Test 7 - Ternary operator: %.4f\n", (float)ternary_result);
    
    /* Test 8: Switch-like behavior */
    accum_t switch_result = switch_fixed_point(0.3k);
    printf("Test 8 - Switch fixed-point: %.4f\n", (float)switch_result);
    
    /* Test 9: Built-in overflow check */
    sfract_t overflow_result;
    int overflow_occurred = check_overflow(0.9r, 0.2r, &overflow_result);
    printf("Test 9 - Overflow check: %d (result: %.4f)\n",
           overflow_occurred, (float)overflow_result);
    
    /* Test 10: Nested calls */
    sfract_t nested_result = nested_calls(0.4r, 0.3r, 0.8r);
    printf("Test 10 - Nested calls: %.4f\n", (float)nested_result);
    
    /* Final checksum to prevent dead code elimination */
    float checksum = 0.0f;
    checksum += (float)sum1;
    checksum += (float)prod1;
    checksum += (float)loop_result;
    checksum += (float)reduce_result;
    checksum += (float)div_result;
    checksum += (float)shift_result;
    checksum += (float)ternary_result;
    checksum += (float)switch_result;
    checksum += (float)overflow_result;
    checksum += (float)nested_result;
    
    printf("\nFinal checksum: %.6f\n", checksum);
    
    /* Additional edge case tests */
    printf("\nEdge case tests:\n");
    
    /* Test near saturation boundaries */
    sfract_t near_max = 0.999999r;
    sfract_t small_add = 0.000001r;
    sfract_t saturated_sum = near_max + small_add;
    printf("Saturation test: %.6f + %.6f = %.6f\n",
           (float)near_max, (float)small_add, (float)saturated_sum);
    
    /* Test underflow */
    sfract_t near_min = -0.999999r;
    sfract_t small_sub = -0.000001r;
    sfract_t underflow_sum = near_min + small_sub;
    printf("Underflow test: %.6f + %.6f = %.6f\n",
           (float)near_min, (float)small_sub, (float)underflow_sum);
    
    /* Test with asm to create complex value flow */
    sfract_t asm_value = 0.5r;
    asm volatile (
        "/* dummy asm to prevent optimization */"
        : "+r" (asm_value)
        :
        : "memory"
    );
    
    /* Complex expression that should trigger the specific uncovered lines */
    sfract_t complex_expr = (asm_value * 2.0r) / 0.3r;
    if (complex_expr > 0.9r || complex_expr < -0.9r) {
        printf("Complex expression triggered range check: %.4f\n", 
               (float)complex_expr);
    }
    
    return 0;
}
