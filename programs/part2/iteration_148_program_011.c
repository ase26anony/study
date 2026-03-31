/* test_fixed_point.c - Target coverage for fixed-value.cc lines 264-277 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-point type definitions */
typedef _Sat _Fract sat_fract_t;
typedef _Fract fract_t;
typedef _Sat _Accum sat_accum_t;
typedef _Accum accum_t;
typedef _Sat short _Fract sat_short_fract_t;

/* Test functions with complex fixed-point operations */

/* Function 1: Range analysis with saturation boundaries */
static inline sat_fract_t sat_add_with_check(sat_fract_t a, sat_fract_t b) {
    /* This should trigger range analysis for saturation */
    sat_fract_t sum = a + b;
    
    /* Conditional that depends on range analysis */
    if (sum > 0.9r) {
        return sum * 0.5r;
    } else if (sum < -0.9r) {
        return sum * 2.0r;
    }
    return sum;
}

/* Function 2: Multiplication with overflow analysis */
static inline sat_accum_t complex_mult(sat_accum_t x, sat_accum_t y, int shift) {
    /* Multi-step calculation requiring range propagation */
    sat_accum_t prod = x * y;
    
    /* Shift operation that can cause overflow/underflow */
    if (shift > 0) {
        prod = prod >> shift;
    } else if (shift < 0) {
        prod = prod << (-shift);
    }
    
    /* Ternary with fixed-point operands */
    return (prod > 0.5k) ? prod * 0.75k : prod * 1.25k;
}

/* Function 3: Loop-based range analysis */
static sat_accum_t accumulate_array(const sat_fract_t* arr, int n) {
    sat_accum_t total = 0.0k;
    
    /* Loop where induction variable affects fixed-point range */
    for (int i = 0; i < n; i++) {
        /* Complex expression requiring range analysis */
        sat_accum_t scaled = (sat_accum_t)arr[i] * (2.0k - (sat_accum_t)i * 0.1k);
        
        /* Conditional that depends on accumulated range */
        if (total + scaled > 10.0k || total + scaled < -10.0k) {
            total = total * 0.9k;
        } else {
            total = total + scaled;
        }
    }
    return total;
}

/* Function 4: Mixed-type operations */
static fract_t mixed_operations(fract_t a, sat_accum_t b, int iterations) {
    sat_accum_t result = (sat_accum_t)a;
    
    for (int i = 0; i < iterations; i++) {
        /* Operations that require careful range analysis */
        result = result * b;
        
        /* Shift operations on fixed-point */
        if (i % 2 == 0) {
            result = result >> 1;
        } else {
            result = result << 1;
        }
        
        /* Conversion between saturated and unsaturated */
        fract_t temp = (fract_t)(result * 0.5k);
        result = (sat_accum_t)temp * 2.0k;
    }
    
    /* Final range-dependent conditional */
    if (result > 0.8k && (fract_t)result < 0.2r) {
        return (fract_t)(result * 0.5k);
    }
    return (fract_t)result;
}

/* Function 5: Using builtins for overflow detection */
static int detect_overflow(sat_accum_t a, sat_accum_t b, sat_accum_t* res) {
    /* Use builtin for overflow detection with fixed-point */
    int overflow = 0;
    
    /* Multiplication overflow check */
    overflow |= __builtin_mul_overflow((long)a, (long)b, (long*)res);
    
    /* Additional fixed-point specific operation */
    *res = *res >> 2;
    
    /* Complex conditional based on range */
    if (a > 0.7k && b > 0.7k && *res < 0.5k) {
        overflow = 1;  /* Should trigger range analysis */
    }
    
    return overflow;
}

/* Function 6: Switch statement with fixed-point conditions */
static fract_t switch_based(fract_t val) {
    fract_t result = 0.0r;
    
    /* Switch where cases depend on fixed-point comparisons */
    switch ((int)(val * 10.0r)) {
        case 0:  /* val < 0.1r */
            result = val * 2.0r;
            break;
        case 1:  /* 0.1r <= val < 0.2r */
            result = val / 0.5r;
            break;
        case 2:  /* 0.2r <= val < 0.3r */
            result = val + 0.5r;
            break;
        case 3:  /* 0.3r <= val < 0.4r */
            result = val - 0.2r;
            break;
        default:
            result = val * val;
            break;
    }
    
    /* Additional range-dependent operation */
    if (result > 0.9r || result < -0.9r) {
        result = result * 0.5r;
    }
    
    return result;
}

/* Function 7: Array reduction with saturation */
static sat_accum_t array_reduction(sat_short_fract_t* arr, int size) {
    sat_accum_t sum = 0.0k;
    sat_accum_t product = 1.0k;
    
    for (int i = 0; i < size; i++) {
        /* Operations designed to hit saturation boundaries */
        sum = sum + (sat_accum_t)arr[i];
        product = product * (sat_accum_t)arr[i];
        
        /* Conditional that requires precise range analysis */
        if (sum > 5.0k || product < 0.01k) {
            /* Reset to avoid permanent saturation */
            sum = sum * 0.5k;
            product = product * 2.0k;
        }
    }
    
    /* Final expression requiring range analysis */
    return (sum + product) / 2.0k;
}

/* Function 8: Inline assembly with fixed-point constraints */
static sat_accum_t asm_operation(sat_accum_t a, sat_accum_t b) {
    sat_accum_t result;
    
    /* Use inline assembly to create complex value flow */
    asm volatile (
        "/* Complex fixed-point operation */"
        : "=r"(result)
        : "r"(a), "r"(b)
        : "cc"
    );
    
    /* Follow up with compiler-visible operations */
    result = result * 1.5k;
    
    /* Range-dependent conditional */
    if (a > 0.0k && b > 0.0k && result < 0.0k) {
        /* This should trigger the uncovered range check */
        result = -result;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command line arguments for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int array_size = (argc > 2) ? atoi(argv[2]) : 50;
    
    if (iterations <= 0) iterations = 100;
    if (array_size <= 0) array_size = 50;
    
    /* Initialize arrays with fixed-point values */
    sat_fract_t* fract_array = (sat_fract_t*)malloc(array_size * sizeof(sat_fract_t));
    sat_short_fract_t* short_fract_array = (sat_short_fract_t*)malloc(array_size * sizeof(sat_short_fract_t));
    
    if (!fract_array || !short_fract_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with values that will trigger various range conditions */
    for (int i = 0; i < array_size; i++) {
        /* Values designed to test saturation boundaries */
        fract_array[i] = (sat_fract_t)((i % 10) * 0.1r - 0.5r);
        short_fract_array[i] = (sat_short_fract_t)((i % 5) * 0.2r - 0.4r);
    }
    
    /* Test 1: Saturation boundary tests */
    printf("Test 1: Saturation boundaries\n");
    sat_fract_t max_fract = 0.999999r;
    sat_fract_t min_fract = -0.999999r;
    
    /* Operations designed to hit saturation */
    for (int i = 0; i < iterations; i++) {
        max_fract = sat_add_with_check(max_fract, 0.1r);
        min_fract = sat_add_with_check(min_fract, -0.1r);
    }
    printf("  max_fract final: %f\n", (float)max_fract);
    printf("  min_fract final: %f\n", (float)min_fract);
    
    /* Test 2: Complex multiplication with shifts */
    printf("\nTest 2: Complex multiplication\n");
    sat_accum_t accum_result = 0.5k;
    for (int i = 0; i < iterations; i++) {
        accum_result = complex_mult(accum_result, 1.1k, i % 4);
    }
    printf("  accum_result: %f\n", (float)accum_result);
    
    /* Test 3: Array accumulation */
    printf("\nTest 3: Array accumulation\n");
    sat_accum_t array_sum = accumulate_array(fract_array, array_size);
    printf("  array_sum: %f\n", (float)array_sum);
    
    /* Test 4: Mixed operations */
    printf("\nTest 4: Mixed operations\n");
    fract_t mixed_result = mixed_operations(0.3r, 1.5k, iterations % 10);
    printf("  mixed_result: %f\n", (float)mixed_result);
    
    /* Test 5: Overflow detection */
    printf("\nTest 5: Overflow detection\n");
    sat_accum_t overflow_res;
    int has_overflow = detect_overflow(0.8k, 0.9k, &overflow_res);
    printf("  overflow detected: %d, result: %f\n", has_overflow, (float)overflow_res);
    
    /* Test 6: Switch-based operations */
    printf("\nTest 6: Switch-based operations\n");
    fract_t switch_result = 0.0r;
    for (int i = 0; i < array_size; i++) {
        switch_result = switch_based(fract_array[i]);
    }
    printf("  switch_result: %f\n", (float)switch_result);
    
    /* Test 7: Array reduction */
    printf("\nTest 7: Array reduction\n");
    sat_accum_t reduction_result = array_reduction(short_fract_array, array_size);
    printf("  reduction_result: %f\n", (float)reduction_result);
    
    /* Test 8: Assembly operations */
    printf("\nTest 8: Assembly operations\n");
    sat_accum_t asm_result = asm_operation(0.7k, 0.8k);
    printf("  asm_result: %f\n", (float)asm_result);
    
    /* Final checksum to prevent dead code elimination */
    sat_accum_t checksum = 0.0k;
    checksum += (sat_accum_t)max_fract;
    checksum += accum_result;
    checksum += array_sum;
    checksum += (sat_accum_t)mixed_result;
    checksum += overflow_res;
    checksum += (sat_accum_t)switch_result;
    checksum += reduction_result;
    checksum += asm_result;
    
    printf("\nFinal checksum: %f\n", (float)checksum);
    
    /* Cleanup */
    free(fract_array);
    free(short_fract_array);
    
    return 0;
}
