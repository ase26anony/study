/* test_fixed_point.c - Target coverage for fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef short _Fract short_fract_t;
typedef _Sat short _Fract sat_short_fract_t;

/* Test function 1: Complex fixed-point arithmetic with saturation */
static inline sat_fract_t complex_sat_operation(sat_fract_t a, sat_fract_t b, int shift) {
    /* Operations that require range analysis */
    sat_fract_t temp = a + b;
    temp = temp * 0.75r;
    
    /* Shift operation that can cause overflow/underflow */
    if (shift > 0) {
        temp = temp << shift;
    } else if (shift < 0) {
        temp = temp >> (-shift);
    }
    
    /* Conditional based on range */
    if (temp > 0.8r) {
        return temp * 0.9r;
    } else if (temp < -0.8r) {
        return temp * 1.1r;
    }
    
    return temp;
}

/* Test function 2: Accumulator with loop-based range propagation */
static sat_accum_t accumulate_range(int iterations, fract_t base) {
    sat_accum_t total = 0.0k;
    sat_accum_t multiplier = 1.5k;
    
    for (int i = 0; i < iterations; i++) {
        /* Range changes with each iteration */
        fract_t increment = base * (fract_t)i;
        total = total + (sat_accum_t)increment * multiplier;
        
        /* Force saturation check */
        if (total > 10.0k) {
            multiplier = 0.5k;
        } else if (total < -10.0k) {
            multiplier = 2.0k;
        }
    }
    
    return total;
}

/* Test function 3: Array reduction with mixed types */
static sat_fract_t array_reduction(const fract_t* arr, int size) {
    sat_fract_t sum = 0.0r;
    sat_fract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Complex expression requiring range analysis */
        sat_fract_t val = arr[i];
        
        /* Operations near saturation boundaries */
        if (i % 2 == 0) {
            sum = sum + val * 0.999r;  /* Near upper bound */
        } else {
            sum = sum - val * 0.999r;  /* Near lower bound */
        }
        
        product = product * (0.5r + val * 0.5r);
        
        /* Ternary with fixed-point operands */
        sum = (sum > 0.9r) ? sum * 0.8r : sum * 1.2r;
    }
    
    /* Final operation that could trigger the uncovered code */
    return (sum + product) / 2.0r;
}

/* Test function 4: Using builtins for overflow detection */
static int check_overflow_operations(accum_t* result1, accum_t* result2) {
    accum_t a = 0.7k;
    accum_t b = 0.8k;
    accum_t c = 0.0k;
    
    /* Use overflow builtins */
    int overflow1 = __builtin_add_overflow(a, b, &c);
    *result1 = c;
    
    accum_t d = 0.9k;
    accum_t e = 0.0k;
    int overflow2 = __builtin_mul_overflow(a, d, &e);
    *result2 = e;
    
    /* Conditional based on overflow results */
    if (overflow1 || overflow2) {
        return -1;
    }
    
    return 0;
}

/* Test function 5: Switch statement with fixed-point conditions */
static fract_t switch_based_operation(fract_t input, int mode) {
    fract_t output;
    
    switch (mode) {
        case 0:
            output = input * 0.25r;
            break;
        case 1:
            output = input * 0.5r;
            break;
        case 2:
            output = input * 0.75r;
            break;
        case 3:
            /* Near saturation boundary */
            output = input * 0.999r;
            break;
        default:
            output = input;
    }
    
    /* Additional range-dependent operation */
    if (output > 0.8r) {
        output = output >> 1;
    } else if (output < -0.8r) {
        output = output << 1;
    }
    
    return output;
}

/* Test function 6: Nested loops with fixed-point induction */
static sat_accum_t nested_loop_test(int outer, int inner) {
    sat_accum_t matrix[10][10];
    sat_accum_t total = 0.0k;
    
    /* Initialize with values near boundaries */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            fract_t frac_val = (fract_t)(i * j) / 100.0r;
            matrix[i][j] = (sat_accum_t)frac_val * 10.0k;
        }
    }
    
    /* Perform operations that require range analysis */
    for (int i = 0; i < outer && i < 10; i++) {
        sat_accum_t row_sum = 0.0k;
        for (int j = 0; j < inner && j < 10; j++) {
            /* Complex expression */
            row_sum = row_sum + matrix[i][j] * (sat_accum_t)((i + j) % 3);
            
            /* Force potential saturation */
            if (row_sum > 50.0k) {
                row_sum = row_sum * 0.5k;
            }
        }
        total = total + row_sum;
    }
    
    return total;
}

/* Test function 7: Inline assembly with fixed-point constraints */
static accum_t asm_fixed_point_operation(accum_t a, accum_t b) {
    accum_t result;
    
    /* Use inline assembly to create hard-to-analyze value flow */
    asm volatile (
        "/* Fixed-point operation */"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Additional operations to ensure range analysis */
    result = result * 1.5k;
    result = result >> 2;
    
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    int iterations = 100;
    int array_size = 50;
    
    /* Use command-line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 50;
        if (array_size > 1000) array_size = 1000;
    }
    
    printf("Testing fixed-point range analysis with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize arrays */
    fract_t* fract_array = (fract_t*)malloc(array_size * sizeof(fract_t));
    if (!fract_array) return 1;
    
    for (int i = 0; i < array_size; i++) {
        /* Values distributed across range */
        fract_array[i] = (fract_t)((i % 100) - 50) / 100.0r;
    }
    
    /* Test 1: Complex saturation operations */
    sat_fract_t test1_result = 0.0r;
    for (int i = 0; i < iterations; i++) {
        sat_fract_t a = (sat_fract_t)(i % 100) / 100.0r;
        sat_fract_t b = (sat_fract_t)((i * 3) % 100) / 100.0r;
        test1_result = test1_result + complex_sat_operation(a, b, i % 4 - 2);
    }
    
    /* Test 2: Accumulator with range propagation */
    sat_accum_t test2_result = accumulate_range(iterations, 0.01r);
    
    /* Test 3: Array reduction */
    sat_fract_t test3_result = array_reduction(fract_array, array_size);
    
    /* Test 4: Overflow builtins */
    accum_t overflow_result1, overflow_result2;
    int overflow_status = check_overflow_operations(&overflow_result1, &overflow_result2);
    
    /* Test 5: Switch-based operations */
    fract_t test5_result = 0.0r;
    for (int i = 0; i < array_size; i++) {
        test5_result = test5_result + switch_based_operation(fract_array[i], i % 5);
    }
    
    /* Test 6: Nested loops */
    sat_accum_t test6_result = nested_loop_test(iterations / 10, iterations / 20);
    
    /* Test 7: Assembly operations */
    accum_t test7_result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        accum_t a = (accum_t)i / 100.0k;
        accum_t b = (accum_t)(i * 2) / 100.0k;
        test7_result = test7_result + asm_fixed_point_operation(a, b);
    }
    
    /* Final checksum calculation using all results */
    accum_t final_checksum = (accum_t)test1_result + test2_result + 
                            (accum_t)test3_result + overflow_result1 + 
                            overflow_result2 + (accum_t)test5_result + 
                            test6_result + test7_result;
    
    /* Convert to float for printing (avoiding dead code elimination) */
    printf("Test results:\n");
    printf("  Test1 (complex sat): %f\n", (float)test1_result);
    printf("  Test2 (accumulator): %f\n", (float)test2_result);
    printf("  Test3 (array reduc): %f\n", (float)test3_result);
    printf("  Test4 (overflow1): %f, overflow2: %f, status: %d\n", 
           (float)overflow_result1, (float)overflow_result2, overflow_status);
    printf("  Test5 (switch): %f\n", (float)test5_result);
    printf("  Test6 (nested): %f\n", (float)test6_result);
    printf("  Test7 (asm): %f\n", (float)test7_result);
    printf("  Final checksum: %f\n", (float)final_checksum);
    
    free(fract_array);
    
    /* Return based on checksum to ensure all code is executed */
    return (final_checksum != 0.0k) ? 0 : 1;
}
