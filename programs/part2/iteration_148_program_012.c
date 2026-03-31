/* test_fixed.c - Comprehensive fixed-point test for GCC range analysis */
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

/* Test functions with complex fixed-point operations */

/* Function 1: Saturation boundary testing */
static inline sfract_t test_saturation_boundary(sfract_t a, sfract_t b, int shift) {
    /* Operations designed to hit saturation boundaries */
    sfract_t result = a + b;
    
    /* Shift operation that can cause overflow/underflow */
    if (shift > 0) {
        /* This shift can overflow */
        result = result << shift;
    } else if (shift < 0) {
        /* This shift can underflow */
        result = result >> (-shift);
    }
    
    /* Multiplication near saturation point */
    result = result * 0.999999r;
    
    /* Conditional based on range analysis */
    if (result > 0.9r && result < 1.0r) {
        /* This branch should be taken for certain ranges */
        result = result + 0.1r;  /* Should saturate */
    }
    
    return result;
}

/* Function 2: Accumulator range propagation */
static inline saccum_t accum_range_propagation(saccum_t base, int iterations) {
    saccum_t total = 0.0k;
    
    /* Loop with fixed-point induction */
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        saccum_t increment = base * (saccum_t)i;
        
        /* Shift that depends on iteration */
        increment = increment >> (i % 4);
        
        /* Addition that may saturate */
        total = total + increment;
        
        /* Conditional that depends on accumulated range */
        if (total > 100.0k || total < -100.0k) {
            /* Reset if out of expected range */
            total = total * 0.5k;
        }
    }
    
    return total;
}

/* Function 3: Mixed-type operations */
static inline accum_t mixed_type_operations(fract_t a, accum_t b, int op) {
    accum_t result;
    
    switch (op) {
        case 0:
            /* Multiplication with different types */
            result = (accum_t)a * b;
            break;
        case 1:
            /* Division with range constraints */
            result = b / (accum_t)(a + 0.1r);
            break;
        case 2:
            /* Complex shift operation */
            result = (accum_t)a * 2.0k;
            result = result >> 3;
            break;
        case 3:
            /* Ternary with fixed-point operands */
            result = (a > 0.5r) ? (b * 1.5k) : (b * 0.5k);
            break;
        default:
            result = 0.0k;
    }
    
    /* Additional range-dependent operation */
    if (result > 50.0k) {
        result = result - 25.0k;
    } else if (result < -50.0k) {
        result = result + 25.0k;
    }
    
    return result;
}

/* Function 4: Array reduction with fixed-point */
static sfract_t array_reduction(const sfract_t* arr, int size) {
    sfract_t sum = 0.0r;
    sfract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Sum that may saturate */
        sum = sum + arr[i];
        
        /* Product that may overflow */
        product = product * arr[i];
        
        /* Conditional based on intermediate range */
        if (sum > 0.8r || product < 0.1r) {
            /* Adjust to stay in range */
            sum = sum * 0.9r;
            product = product * 1.1r;
        }
    }
    
    /* Final combination */
    return (sum + product) * 0.5r;
}

/* Function 5: Built-in overflow checks */
static int builtin_overflow_test(sfract_t a, sfract_t b, sfract_t* res) {
    int overflow = 0;
    
    /* Use built-in overflow detection */
    overflow |= __builtin_add_overflow(a, b, res);
    
    sfract_t temp;
    overflow |= __builtin_mul_overflow(*res, 1.5r, &temp);
    
    /* Shift that might overflow */
    *res = temp << 2;
    
    return overflow;
}

/* Function 6: Nested loops with fixed-point conditions */
static accum_t nested_loop_test(int outer, int inner) {
    accum_t matrix[10][10];
    accum_t total = 0.0k;
    
    /* Initialize matrix with fixed-point values */
    for (int i = 0; i < 10 && i < outer; i++) {
        for (int j = 0; j < 10 && j < inner; j++) {
            matrix[i][j] = (accum_t)(i * j) / 100.0k;
        }
    }
    
    /* Perform matrix operations */
    for (int i = 0; i < outer && i < 10; i++) {
        accum_t row_sum = 0.0k;
        for (int j = 0; j < inner && j < 10; j++) {
            /* Operation that depends on both indices */
            accum_t val = matrix[i][j];
            val = val * (accum_t)(i + 1) / (accum_t)(j + 1);
            
            /* Shift based on position */
            val = val >> ((i + j) % 4);
            
            row_sum = row_sum + val;
            
            /* Range check that might be optimized */
            if (row_sum > 10.0k) {
                row_sum = 5.0k;
            }
        }
        total = total + row_sum;
    }
    
    return total;
}

/* Function 7: Inline assembly with fixed-point */
static sfract_t asm_fixed_test(sfract_t a, sfract_t b) {
    sfract_t result;
    
    /* Use inline assembly to create hard-to-analyze value flow */
    asm volatile (
        "/* Fixed-point operation in assembly */"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Follow up with C operations for range analysis */
    result = result * 0.75r;
    
    /* Conditional that depends on assembly result */
    if (result > 0.0r) {
        result = result + 0.25r;
    } else {
        result = result - 0.25r;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Use command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Initialize arrays with fixed-point values */
    sfract_t sfract_array[20];
    fract_t fract_array[20];
    saccum_t saccum_array[10];
    
    /* Fill arrays with values that test boundaries */
    for (int i = 0; i < 20; i++) {
        /* Values near saturation boundaries */
        sfract_array[i] = (sfract_t)((i % 19) / 19.0r);
        fract_array[i] = (fract_t)((i % 17) / 17.0r);
        
        if (i < 10) {
            saccum_array[i] = (saccum_t)((i - 5) * 20.0k);
        }
    }
    
    /* Test 1: Saturation boundary testing */
    sfract_t sat_result = 0.0r;
    for (int i = 0; i < iterations; i++) {
        sat_result = test_saturation_boundary(
            sfract_array[i % 20],
            0.5r,
            (i % 3) - 1  /* shifts: -1, 0, 1 */
        );
    }
    
    /* Test 2: Accumulator range propagation */
    saccum_t accum_result = accum_range_propagation(10.0k, iterations % 20);
    
    /* Test 3: Mixed-type operations */
    accum_t mixed_result = 0.0k;
    for (int i = 0; i < 5; i++) {
        mixed_result = mixed_result + mixed_type_operations(
            fract_array[i],
            (accum_t)saccum_array[i % 10],
            i % 4
        );
    }
    
    /* Test 4: Array reduction */
    sfract_t reduction_result = array_reduction(sfract_array, 20);
    
    /* Test 5: Built-in overflow checks */
    sfract_t overflow_result;
    int had_overflow = builtin_overflow_test(0.9r, 0.8r, &overflow_result);
    
    /* Test 6: Nested loops */
    accum_t nested_result = nested_loop_test(iterations % 10, (iterations + 3) % 10);
    
    /* Test 7: Assembly test */
    sfract_t asm_result = asm_fixed_test(0.3r, 0.7r);
    
    /* Combine all results into a checksum to prevent dead code elimination */
    accum_t final_checksum = (accum_t)sat_result 
                           + accum_result 
                           + mixed_result 
                           + (accum_t)reduction_result 
                           + (accum_t)overflow_result 
                           + nested_result 
                           + (accum_t)asm_result;
    
    /* Print results (converted to float for readability) */
    printf("Test Results:\n");
    printf("  Saturation test: %f\n", (float)sat_result);
    printf("  Accumulator test: %f\n", (float)accum_result);
    printf("  Mixed-type test: %f\n", (float)mixed_result);
    printf("  Array reduction: %f\n", (float)reduction_result);
    printf("  Overflow detected: %d (result: %f)\n", had_overflow, (float)overflow_result);
    printf("  Nested loop test: %f\n", (float)nested_result);
    printf("  Assembly test: %f\n", (float)asm_result);
    printf("  Final checksum: %f\n", (float)final_checksum);
    
    /* Use checksum in a conditional to ensure all code is needed */
    if (final_checksum > 1000.0k || final_checksum < -1000.0k) {
        printf("Warning: Checksum out of expected range!\n");
    }
    
    return (final_checksum != 0.0k) ? 0 : 1;
}
