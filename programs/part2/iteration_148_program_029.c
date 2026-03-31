/* test_fixed_point.c - Target coverage for fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract;
typedef _Fract fract;
typedef _Sat _Accum sat_accum;
typedef _Accum accum;
typedef _Sat long _Fract sat_long_fract;
typedef long _Fract long_fract;

/* Test functions with various fixed-point operations */

/* Function 1: Complex saturated addition with range analysis */
static inline sat_fract sat_add_range(sat_fract a, sat_fract b, int shift) {
    /* Operations that require precise overflow analysis */
    sat_fract temp = a + b;
    /* Shift operation that can affect range */
    if (shift > 0) {
        temp = temp >> shift;
    } else if (shift < 0) {
        temp = temp << (-shift);
    }
    return temp;
}

/* Function 2: Multiplication with saturation boundary checks */
static inline sat_accum mul_with_saturation(sat_accum x, sat_accum y, int scale) {
    sat_accum result = x * y;
    /* Force range analysis with conditional shift */
    if (scale != 0) {
        if (result > 0.5k) {
            result = result >> 1;
        } else {
            result = result << 1;
        }
    }
    return result;
}

/* Function 3: Division with overflow protection */
static inline fract safe_divide(fract a, fract b) {
    /* This should trigger range analysis for division */
    if (b == 0.0r) return 0.0r;
    fract result = a / b;
    /* Conditional based on range */
    if (result > 0.9r || result < -0.9r) {
        return (result > 0) ? 0.9r : -0.9r;
    }
    return result;
}

/* Function 4: Loop-based accumulation with saturation */
static sat_accum accumulate_saturated(sat_accum* arr, int n) {
    sat_accum total = 0.0k;
    for (int i = 0; i < n; i++) {
        /* Complex expression requiring range analysis */
        total = total + (arr[i] * (i % 2 ? 0.5k : -0.5k));
        
        /* Conditional that depends on accumulated range */
        if (total > 10.0k || total < -10.0k) {
            total = total * 0.5k;
        }
    }
    return total;
}

/* Function 5: Fixed-point array reduction with mixed types */
static sat_fract array_reduction(fract* arr, int n, int iterations) {
    sat_fract result = 0.0r;
    
    for (int iter = 0; iter < iterations; iter++) {
        sat_fract iter_sum = 0.0r;
        for (int i = 0; i < n; i++) {
            /* Mix saturated and unsaturated operations */
            fract temp = arr[i];
            sat_fract saturated_temp = temp;
            
            /* Operation designed to hit saturation boundaries */
            iter_sum = iter_sum + saturated_temp * (0.999999r / n);
        }
        
        /* Conditional based on range analysis */
        if (iter_sum > 0.95r) {
            result = result + 0.5r;
        } else if (iter_sum < 0.05r) {
            result = result - 0.5r;
        } else {
            result = result + iter_sum;
        }
    }
    
    return result;
}

/* Function 6: Switch statement with fixed-point conditions */
static const char* range_category(sat_accum value) {
    /* This switch should trigger range analysis */
    switch ((value > 0) ? 1 : ((value < 0) ? -1 : 0)) {
        case 1:
            if (value > 0.75k) return "HIGH_POSITIVE";
            else if (value > 0.25k) return "MEDIUM_POSITIVE";
            else return "LOW_POSITIVE";
        case -1:
            if (value < -0.75k) return "HIGH_NEGATIVE";
            else if (value < -0.25k) return "MEDIUM_NEGATIVE";
            else return "LOW_NEGATIVE";
        default:
            return "ZERO";
    }
}

/* Function 7: Using builtins for overflow detection */
static int check_mul_overflow(sat_fract a, sat_fract b, sat_fract* result) {
    /* Use builtin for overflow detection */
    sat_fract tmp;
    int overflow = __builtin_mul_overflow(a, b, &tmp);
    *result = tmp;
    return overflow;
}

/* Function 8: Complex expression with ternary operator */
static sat_accum ternary_range_test(sat_accum a, sat_accum b, int flag) {
    /* Ternary with fixed-point in both branches */
    sat_accum result = (flag > 0) ? 
        (a * 2.0k + b / 2.0k) : 
        (a / 2.0k - b * 2.0k);
    
    /* Additional range-dependent operation */
    return (result > 1.0k) ? (result >> 2) : (result << 2);
}

/* Function 9: Nested function calls for inter-procedural analysis */
static sat_fract nested_range_propagation(sat_fract a, sat_fract b, int depth) {
    if (depth <= 0) return a + b;
    
    sat_fract half = a * 0.5r;
    sat_fract quarter = b * 0.25r;
    
    /* Recursive call with modified values */
    return nested_range_propagation(half, quarter, depth - 1) * 0.9r;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command-line arguments for variability */
    int array_size = (argc > 1) ? atoi(argv[1]) : 100;
    int iterations = (argc > 2) ? atoi(argv[2]) : 10;
    int seed = (argc > 3) ? atoi(argv[3]) : 42;
    
    srand(seed);
    
    /* Initialize arrays with mixed values */
    fract* fract_array = (fract*)malloc(array_size * sizeof(fract));
    sat_accum* accum_array = (sat_accum*)malloc(array_size * sizeof(sat_accum));
    
    for (int i = 0; i < array_size; i++) {
        /* Generate values that can trigger edge cases */
        float rand_val = (float)rand() / RAND_MAX;
        fract_array[i] = (fract)(rand_val * 2.0 - 1.0);  /* Range: -1.0 to 1.0 */
        accum_array[i] = (sat_accum)(rand_val * 20.0 - 10.0); /* Range: -10.0 to 10.0 */
    }
    
    /* Test 1: Saturated addition with boundary values */
    printf("Test 1: Saturated addition range test\n");
    sat_fract max_fract = 0.999999r;
    sat_fract min_fract = -0.999999r;
    
    /* These should saturate */
    sat_fract test1 = sat_add_range(max_fract, 0.1r, 0);
    sat_fract test2 = sat_add_range(min_fract, -0.1r, 0);
    
    /* Test 2: Multiplication near saturation boundaries */
    printf("Test 2: Multiplication saturation test\n");
    sat_accum test3 = mul_with_saturation(5.0k, 2.5k, 1);
    sat_accum test4 = mul_with_saturation(-5.0k, 2.5k, 1);
    
    /* Test 3: Division with range analysis */
    printf("Test 3: Division range test\n");
    fract test5 = safe_divide(0.8r, 0.9r);
    fract test6 = safe_divide(-0.8r, 0.1r);
    
    /* Test 4: Loop accumulation */
    printf("Test 4: Loop accumulation test\n");
    sat_accum test7 = accumulate_saturated(accum_array, array_size);
    
    /* Test 5: Array reduction */
    printf("Test 5: Array reduction test\n");
    sat_fract test8 = array_reduction(fract_array, array_size, iterations);
    
    /* Test 6: Switch with fixed-point conditions */
    printf("Test 6: Switch statement test\n");
    const char* cat1 = range_category(test3);
    const char* cat2 = range_category(test4);
    
    /* Test 7: Builtin overflow check */
    printf("Test 7: Builtin overflow test\n");
    sat_fract mul_result;
    int overflow = check_mul_overflow(max_fract, max_fract, &mul_result);
    
    /* Test 8: Ternary operator */
    printf("Test 8: Ternary operator test\n");
    sat_accum test9 = ternary_range_test(test3, test4, 1);
    sat_accum test10 = ternary_range_test(test3, test4, -1);
    
    /* Test 9: Nested propagation */
    printf("Test 9: Nested propagation test\n");
    sat_fract test11 = nested_range_propagation(0.8r, 0.6r, 3);
    
    /* Final checksum to prevent dead code elimination */
    sat_accum checksum = 0.0k;
    checksum = checksum + (sat_accum)test1;
    checksum = checksum + (sat_accum)test2;
    checksum = checksum + test3;
    checksum = checksum + test4;
    checksum = checksum + (sat_accum)test5;
    checksum = checksum + (sat_accum)test6;
    checksum = checksum + test7;
    checksum = checksum + (sat_accum)test8;
    checksum = checksum + test9;
    checksum = checksum + test10;
    checksum = checksum + (sat_accum)test11;
    
    /* Print results (converted to float) */
    printf("\nResults (as float):\n");
    printf("test1: %f\n", (float)test1);
    printf("test2: %f\n", (float)test2);
    printf("test3: %f\n", (float)test3);
    printf("test4: %f\n", (float)test4);
    printf("test5: %f\n", (float)test5);
    printf("test6: %f\n", (float)test6);
    printf("test7: %f\n", (float)test7);
    printf("test8: %f\n", (float)test8);
    printf("test9: %f\n", (float)test9);
    printf("test10: %f\n", (float)test10);
    printf("test11: %f\n", (float)test11);
    printf("overflow detected: %d\n", overflow);
    printf("category1: %s, category2: %s\n", cat1, cat2);
    printf("final checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(fract_array);
    free(accum_array);
    
    return 0;
}
