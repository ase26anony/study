/* test_fixed_point.c - Target coverage for fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract;
typedef _Fract fract;
typedef _Sat _Accum sat_accum;
typedef _Accum accum;
typedef _Sat short _Fract sat_short_fract;
typedef short _Fract short_fract;

/* Test functions with different fixed-point operations */

/* Function 1: Complex multiplication with saturation */
static inline sat_accum complex_mult_range(sat_accum a, sat_accum b, int shift) {
    /* This should trigger range analysis for multiplication and shift */
    sat_accum temp = a * b;
    /* Shift operation that requires range analysis */
    if (shift > 0) {
        temp = temp >> shift;
    } else if (shift < 0) {
        temp = temp << (-shift);
    }
    return temp;
}

/* Function 2: Range propagation through addition chain */
static inline sat_fract add_chain_with_overflow(fract a, fract b, fract c) {
    /* Multiple additions that may overflow */
    sat_fract result = a + b;
    result = result + c;
    result = result + a;  /* Potential overflow */
    return result;
}

/* Function 3: Conditional range analysis */
static inline int check_fract_range(fract x, fract y) {
    /* This should trigger the if condition analysis */
    sat_fract sum = x + y;
    
    /* Complex condition that requires range comparison */
    if (sum > 0.9r) {
        return 1;
    } else if (sum < -0.9r) {
        return -1;
    }
    return 0;
}

/* Function 4: Loop-based range accumulation */
static sat_accum loop_accumulation(int iterations, fract base) {
    sat_accum total = 0.0k;
    fract increment = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix of operations that affect range */
        total = total + (sat_accum)increment;
        increment = increment * 0.95r;  /* Decreasing values */
        
        /* Conditional that depends on accumulated range */
        if (total > 10.0k) {
            total = total - 5.0k;
        }
    }
    return total;
}

/* Function 5: Array reduction with saturation */
static sat_fract array_reduction(fract arr[], int size) {
    sat_fract sum = 0.0r;
    for (int i = 0; i < size; i++) {
        sum = sum + arr[i];
        /* Force saturation check */
        if (i % 3 == 0) {
            sum = sum * 1.5r;
        }
    }
    return sum;
}

/* Function 6: Shift operations that trigger specific uncovered code */
static sat_accum shift_boundary_test(accum a, int shift_amt) {
    /* Operations designed to hit the boundary conditions */
    sat_accum result = a;
    
    /* Multiple shifts that may overflow/underflow */
    result = result >> shift_amt;
    result = result << (shift_amt / 2);
    
    /* Ternary with fixed-point operands */
    return (shift_amt > 4) ? (result * 2.0k) : (result / 2.0k);
}

/* Function 7: Mixed-type conversions with range checks */
static sat_fract mixed_conversions(short_fract a, sat_accum b) {
    /* Conversions that require range analysis */
    fract conv_a = a;  /* Widening */
    sat_fract conv_b = (sat_fract)(b / 256.0k);  /* Narrowing with potential overflow */
    
    return conv_a + conv_b;
}

/* Function 8: Using builtins for overflow detection */
static int builtin_overflow_test(sat_fract *result, fract a, fract b) {
    /* Use GCC builtins to trigger internal overflow checks */
    return __builtin_add_overflow(a, b, result);
}

/* Function 9: Switch based on fixed-point comparisons */
static int switch_on_fract(fract value) {
    /* Switch that depends on fixed-point range analysis */
    switch (check_fract_range(value, 0.5r)) {
        case 1:
            return 100;
        case -1:
            return -100;
        default:
            return (value > 0.0r) ? 1 : 0;
    }
}

/* Function 10: Nested loops with complex fixed-point operations */
static sat_accum nested_loop_test(int outer, int inner) {
    sat_accum total = 0.0k;
    
    for (int i = 0; i < outer; i++) {
        fract base = 0.1r * i;
        for (int j = 0; j < inner; j++) {
            /* Complex expression requiring range analysis */
            sat_accum temp = (base * j) >> 2;
            total = total + temp;
            
            /* Conditional that might be optimized based on range */
            if (total > (sat_accum)(i * 10.0k)) {
                total = total / 2.0k;
            }
        }
    }
    return total;
}

/* Main test driver */
int main(int argc, char *argv[]) {
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
    
    /* Initialize arrays */
    fract *fract_array = (fract*)malloc(array_size * sizeof(fract));
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = (fract)((i % 10) * 0.1r);
    }
    
    /* Test 1: Complex multiplication with boundary values */
    printf("Test 1: Complex multiplication range\n");
    sat_accum max_accum = 0.999999999k;  /* Near maximum */
    sat_accum result1 = complex_mult_range(max_accum, 2.0k, 1);
    printf("  Result1: %f\n", (float)result1);
    
    /* Test 2: Addition chain */
    printf("\nTest 2: Addition chain overflow test\n");
    fract a = 0.7r, b = 0.8r, c = -0.9r;
    sat_fract result2 = add_chain_with_overflow(a, b, c);
    printf("  Result2: %f\n", (float)result2);
    
    /* Test 3: Range checking */
    printf("\nTest 3: Range condition analysis\n");
    int range_check = check_fract_range(0.8r, 0.3r);
    printf("  Range check result: %d\n", range_check);
    
    /* Test 4: Loop accumulation */
    printf("\nTest 4: Loop-based accumulation\n");
    sat_accum result4 = loop_accumulation(iterations, 0.5r);
    printf("  Loop result: %f\n", (float)result4);
    
    /* Test 5: Array reduction */
    printf("\nTest 5: Array reduction with saturation\n");
    sat_fract result5 = array_reduction(fract_array, array_size);
    printf("  Array sum: %f\n", (float)result5);
    
    /* Test 6: Shift boundary tests */
    printf("\nTest 6: Shift boundary conditions\n");
    accum test_val = 0.5k;
    sat_accum result6 = shift_boundary_test(test_val, 3);
    printf("  Shift result: %f\n", (float)result6);
    
    /* Test 7: Mixed conversions */
    printf("\nTest 7: Mixed type conversions\n");
    short_fract short_val = 0.9r;
    sat_accum accum_val = 100.0k;
    sat_fract result7 = mixed_conversions(short_val, accum_val);
    printf("  Conversion result: %f\n", (float)result7);
    
    /* Test 8: Builtin overflow */
    printf("\nTest 8: Builtin overflow detection\n");
    sat_fract overflow_result;
    int overflow_flag = builtin_overflow_test(&overflow_result, 0.9r, 0.2r);
    printf("  Overflow: %d, Result: %f\n", overflow_flag, (float)overflow_result);
    
    /* Test 9: Switch statement */
    printf("\nTest 9: Switch on fixed-point comparison\n");
    int switch_result = switch_on_fract(0.6r);
    printf("  Switch result: %d\n", switch_result);
    
    /* Test 10: Nested loops */
    printf("\nTest 10: Nested loop complex operations\n");
    sat_accum result10 = nested_loop_test(5, iterations);
    printf("  Nested loop result: %f\n", (float)result10);
    
    /* Final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    sat_accum final_checksum = result1 + (sat_accum)result2 + result4 + 
                              (sat_accum)result5 + result6 + (sat_accum)result7 +
                              (sat_accum)overflow_result + result10;
    
    /* Additional complex expression for range analysis */
    for (int i = 0; i < 5; i++) {
        final_checksum = final_checksum * 0.9k;
        final_checksum = final_checksum + (i * 0.1k);
        
        /* Force evaluation of boundary condition */
        if (final_checksum > 5.0k && final_checksum < 10.0k) {
            final_checksum = final_checksum >> 1;
        }
    }
    
    printf("Final checksum: %f\n", (float)final_checksum);
    
    /* Cleanup */
    free(fract_array);
    
    return 0;
}
