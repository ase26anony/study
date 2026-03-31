/* test_fixed_point.c - Program to exercise fixed-point range analysis */
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

/* Function 1: Basic saturation with addition */
static inline sfract_t sat_add(sfract_t a, sfract_t b) {
    return a + b;  /* Should trigger saturation analysis */
}

/* Function 2: Multiplication with range propagation */
static inline saccum_t sat_mul_range(saccum_t x, saccum_t y) {
    return x * y;  /* Multiplication requires precise range analysis */
}

/* Function 3: Complex expression with shifts */
static inline accum_t shift_expression(accum_t val, int shift) {
    /* This should trigger the shift-related range analysis */
    accum_t result = val;
    if (shift > 0) {
        result = result >> shift;
    } else {
        result = result << (-shift);
    }
    return result * 2.0k;  /* Combine with multiplication */
}

/* Function 4: Conditional based on fixed-point comparison */
static inline int check_overflow(sfract_t a, sfract_t b) {
    /* This condition should trigger the if(a_high.sgt(max_r)...) logic */
    sfract_t sum = a + b;
    if (sum > 0.9r) {
        return 1;  /* Near overflow */
    } else if (sum < -0.9r) {
        return -1; /* Near underflow */
    }
    return 0;  /* Safe range */
}

/* Function 5: Loop-based accumulation with saturation */
static sfract_t accumulate_fract(sfract_t* arr, int n) {
    sfract_t total = 0.0r;
    for (int i = 0; i < n; i++) {
        /* This loop requires inter-iteration range analysis */
        total = total + arr[i];
        /* Force complex range tracking with conditional */
        if (total > 0.8r) {
            total = total * 0.5r;  /* Scale down to avoid saturation */
        }
    }
    return total;
}

/* Function 6: Mixed-type operations */
static accum_t mixed_operations(fract_t a, accum_t b) {
    /* Mixing different fixed-point types requires range conversions */
    accum_t result = (accum_t)a * b;
    
    /* Use ternary operator with fixed-point operands */
    result = (result > 0.5k) ? result * 0.75k : result * 1.25k;
    
    return result;
}

/* Function 7: Built-in overflow checks */
static int builtin_overflow_test(saccum_t* result, saccum_t a, saccum_t b) {
    /* Use GCC builtins for overflow detection */
    return __builtin_add_overflow(a, b, result);
}

/* Function 8: Array reduction with complex indexing */
static saccum_t array_reduction(saccum_t* matrix, int rows, int cols) {
    saccum_t max_val = -1.0k;  /* Minimum value */
    saccum_t min_val = 1.0k;   /* Maximum value */
    
    for (int i = 0; i < rows; i++) {
        saccum_t row_sum = 0.0k;
        for (int j = 0; j < cols; j++) {
            saccum_t val = matrix[i * cols + j];
            row_sum = row_sum + val;
            
            /* Update min/max - requires range tracking */
            if (val > max_val) max_val = val;
            if (val < min_val) min_val = val;
        }
        
        /* Complex condition based on accumulated values */
        if (row_sum > 0.0k && max_val - min_val > 0.5k) {
            row_sum = row_sum * 0.9k;
        }
    }
    
    return max_val - min_val;
}

/* Function 9: Switch statement with fixed-point conditions */
static int fixed_point_switch(sfract_t val) {
    int result = 0;
    
    /* Switch on discretized fixed-point value */
    if (val < -0.5r) {
        result = 1;
    } else if (val < 0.0r) {
        result = 2;
    } else if (val < 0.5r) {
        result = 3;
    } else {
        result = 4;
    }
    
    return result;
}

/* Function 10: Nested function calls with fixed-point args */
static accum_t nested_calls(accum_t x, int depth) {
    if (depth <= 0) return x;
    
    /* Recursive pattern with arithmetic */
    accum_t half = x * 0.5k;
    accum_t quarter = nested_calls(half, depth - 1);
    
    return x + quarter;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Initialize arrays with different fixed-point types */
    sfract_t sf_arr[20];
    fract_t f_arr[20];
    saccum_t sa_arr[25];
    accum_t a_arr[25];
    
    /* Initialize with values that will trigger various range conditions */
    for (int i = 0; i < 20; i++) {
        /* Create values near saturation boundaries */
        fract_t base = (fract_t)((i - 10) * 0.1r);
        sf_arr[i] = (_Sat _Fract)base;
        f_arr[i] = base;
        
        if (i < 25) {
            accum_t abase = (accum_t)((i - 12) * 0.2k);
            sa_arr[i] = (_Sat _Accum)abase;
            a_arr[i] = abase;
        }
    }
    
    /* Force values to saturation boundaries */
    sf_arr[0] = 0.999999r;  /* Near maximum */
    sf_arr[1] = -0.999999r; /* Near minimum */
    sa_arr[0] = 0.999999999k;  /* Near maximum */
    sa_arr[1] = -0.999999999k; /* Near minimum */
    
    /* Test 1: Basic saturation operations */
    printf("Test 1: Basic saturation\n");
    sfract_t sat_result = sat_add(sf_arr[0], 0.5r);
    printf("  sat_add(0.999999r, 0.5r) = %f (as float)\n", (float)sat_result);
    
    /* Test 2: Multiplication range analysis */
    printf("\nTest 2: Multiplication range\n");
    saccum_t mul_result = sat_mul_range(sa_arr[5], 2.0k);
    printf("  sat_mul_range(%fk, 2.0k) = %fk\n", (float)sa_arr[5], (float)mul_result);
    
    /* Test 3: Shift expressions */
    printf("\nTest 3: Shift expressions\n");
    accum_t shift_result = shift_expression(a_arr[10], 3);
    printf("  shift_expression(%fk, 3) = %fk\n", (float)a_arr[10], (float)shift_result);
    
    /* Test 4: Conditional overflow checks */
    printf("\nTest 4: Conditional overflow checks\n");
    int overflow_status = check_overflow(sf_arr[18], sf_arr[19]);
    printf("  check_overflow(%fr, %fr) = %d\n", 
           (float)sf_arr[18], (float)sf_arr[19], overflow_status);
    
    /* Test 5: Loop accumulation */
    printf("\nTest 5: Loop accumulation\n");
    sfract_t accum_result = accumulate_fract(sf_arr, 20);
    printf("  accumulate_fract(array, 20) = %f\n", (float)accum_result);
    
    /* Test 6: Mixed-type operations */
    printf("\nTest 6: Mixed-type operations\n");
    accum_t mixed_result = mixed_operations(f_arr[5], a_arr[5]);
    printf("  mixed_operations(%fr, %fk) = %fk\n", 
           (float)f_arr[5], (float)a_arr[5], (float)mixed_result);
    
    /* Test 7: Built-in overflow */
    printf("\nTest 7: Built-in overflow detection\n");
    saccum_t builtin_result;
    int has_overflow = builtin_overflow_test(&builtin_result, sa_arr[0], 0.1k);
    printf("  builtin_overflow_test(%fk, 0.1k) = %d, result = %fk\n",
           (float)sa_arr[0], has_overflow, (float)builtin_result);
    
    /* Test 8: Array reduction */
    printf("\nTest 8: Array reduction\n");
    saccum_t reduction_result = array_reduction(sa_arr, 5, 5);
    printf("  array_reduction(5x5 matrix) = %fk\n", (float)reduction_result);
    
    /* Test 9: Switch with fixed-point */
    printf("\nTest 9: Fixed-point switch\n");
    int switch_result = fixed_point_switch(sf_arr[10]);
    printf("  fixed_point_switch(%fr) = %d\n", (float)sf_arr[10], switch_result);
    
    /* Test 10: Nested calls */
    printf("\nTest 10: Nested function calls\n");
    accum_t nested_result = nested_calls(1.0k, 3);
    printf("  nested_calls(1.0k, 3) = %fk\n", (float)nested_result);
    
    /* Complex final checksum to prevent dead code elimination */
    printf("\nFinal checksum calculation:\n");
    accum_t checksum = 0.0k;
    
    /* Use command-line iterations for runtime variability */
    for (int i = 0; i < iterations; i++) {
        /* Complex expression that requires full range analysis */
        accum_t term = (accum_t)i * 0.01k;
        
        /* Force evaluation of saturation boundaries */
        if (i % 3 == 0) {
            term = term + 0.5k;
        } else if (i % 3 == 1) {
            term = term - 0.5k;
        }
        
        /* Use shift operation */
        term = term >> (i % 4);
        
        /* Conditional assignment based on range */
        term = (term > 0.0k) ? term * 0.9k : term * 1.1k;
        
        checksum = checksum + term;
    }
    
    /* Mix in array values */
    for (int i = 0; i < 20 && i < iterations; i++) {
        checksum = checksum + (accum_t)sf_arr[i];
        checksum = checksum + a_arr[i % 25];
    }
    
    printf("  Final checksum after %d iterations: %fk\n", 
           iterations, (float)checksum);
    
    return 0;
}
