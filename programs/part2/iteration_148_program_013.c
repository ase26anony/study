/* test_fixed_point.c - Comprehensive fixed-point test for GCC coverage */
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

/* Global arrays for inter-procedural analysis */
static sat_fract global_sat_fract_array[100];
static accum global_accum_array[50];
static int array_size = 0;

/* Test function 1: Complex fixed-point arithmetic with saturation */
static inline sat_fract complex_sat_operation(sat_fract a, sat_fract b, fract c) {
    /* Multi-step operation that could overflow */
    sat_fract temp = a + b;
    temp = temp * c;
    temp = temp >> 2;  /* Right shift can cause underflow */
    
    /* Conditional based on range analysis */
    if (temp > 0.8r) {
        return temp - 0.3r;
    } else if (temp < -0.8r) {
        return temp + 0.3r;
    }
    
    /* Ternary with fixed-point operands */
    return (a > b) ? (temp * 1.5r) : (temp / 1.5r);
}

/* Test function 2: Accumulator with loop-based range propagation */
static sat_accum accumulate_range(fract* arr, int n, fract init) {
    sat_accum total = init;
    
    for (int i = 0; i < n; i++) {
        /* Loop induction affects fixed-point range */
        fract multiplier = (fract)i / (fract)n;
        sat_accum increment = arr[i] * multiplier;
        
        /* Operation designed to hit saturation boundaries */
        total = total + increment;
        
        /* Nested condition forcing range analysis */
        if (total > 0.9k) {
            total = total - 0.1k;
        } else if (total < -0.9k) {
            total = total + 0.1k;
        }
    }
    
    return total;
}

/* Test function 3: Fixed-point multiplication with overflow checks */
static inline int safe_multiply(sat_accum* result, sat_accum a, sat_accum b) {
    /* Use builtin for overflow detection */
    int overflow = __builtin_mul_overflow(a, b, result);
    
    /* Force range analysis with conditional */
    if (!overflow) {
        /* Additional operation that might overflow */
        *result = *result * 1.1k;
    } else {
        /* Handle overflow - saturate to max/min */
        *result = (a > 0) ? 0.999999k : -0.999999k;
    }
    
    return overflow;
}

/* Test function 4: Shift operations with range propagation */
static sat_fract shift_operations(fract base, int shift_amount) {
    sat_fract result = base;
    
    /* Multiple shifts that require precise range analysis */
    switch (shift_amount) {
        case 1:
            result = result >> 1;
            break;
        case 2:
            result = result >> 2;
            result = result << 1;  /* Partial recovery */
            break;
        case 3:
            result = result << 1;
            result = result >> 3;
            break;
        default:
            result = result >> shift_amount;
    }
    
    /* Complex expression requiring range analysis */
    result = (result * 0.5r) + (base * 0.25r);
    
    return result;
}

/* Test function 5: Array reduction with mixed types */
static sat_accum array_reduction(sat_fract* arr, int n) {
    sat_accum total = 0.0k;
    sat_accum max_val = -1.0k;
    sat_accum min_val = 1.0k;
    
    for (int i = 0; i < n; i++) {
        /* Convert fract to accum - requires range analysis */
        sat_accum converted = arr[i];
        
        /* Update total with possible saturation */
        total = total + converted;
        
        /* Update min/max - conditions force range analysis */
        if (converted > max_val) {
            max_val = converted;
        }
        if (converted < min_val) {
            min_val = converted;
        }
        
        /* Complex conditional with fixed-point comparison */
        if (total > 0.5k && converted < 0.1r) {
            total = total - 0.05k;
        }
    }
    
    /* Final calculation that could overflow */
    return total + (max_val - min_val) * 0.5k;
}

/* Test function 6: Nested function calls with fixed-point */
static sat_fract nested_operations(fract a, fract b, int depth) {
    if (depth <= 0) {
        return a * b;
    }
    
    /* Recursive calls create complex range dependencies */
    fract half_a = a * 0.5r;
    fract half_b = b * 0.5r;
    
    sat_fract left = nested_operations(half_a, b, depth - 1);
    sat_fract right = nested_operations(a, half_b, depth - 1);
    
    /* Operation that requires precise range analysis */
    return (left + right) * 0.666r;
}

/* Test function 7: Using asm to create hard-to-analyze values */
static fract asm_fixed_point(fract input) {
    fract result;
    
    /* Inline asm with fixed-point constraints */
    asm volatile (
        "/* Complex fixed-point operation */"
        : "=r" (result)
        : "r" (input)
        : "cc"
    );
    
    /* Follow up with compiler-visible operations */
    result = result * 0.75r;
    
    return result;
}

/* Test function 8: Boundary value testing */
static void test_boundaries(void) {
    /* Values at or near saturation boundaries */
    sat_fract max_fract = 0.999999r;
    sat_fract min_fract = -0.999999r;
    sat_accum max_accum = 0.999999999k;
    sat_accum min_accum = -0.999999999k;
    
    /* Operations designed to trigger saturation logic */
    sat_fract test1 = max_fract + 0.000001r;  /* Should saturate */
    sat_fract test2 = min_fract - 0.000001r;  /* Should saturate */
    sat_accum test3 = max_accum * 1.1k;       /* Should saturate */
    sat_accum test4 = min_accum * 1.1k;       /* Should saturate */
    
    /* Store results to prevent optimization */
    global_sat_fract_array[0] = test1;
    global_sat_fract_array[1] = test2;
    global_accum_array[0] = test3;
    global_accum_array[1] = test4;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command-line arguments for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_len = (argc > 2) ? atoi(argv[2]) : 20;
    fract base_value = (argc > 3) ? (fract)atof(argv[3]) : 0.5r;
    
    if (iterations < 1) iterations = 1;
    if (array_len < 5) array_len = 5;
    if (array_len > 100) array_len = 100;
    
    array_size = array_len;
    
    /* Initialize arrays with varying values */
    sat_fract* sat_array = (sat_fract*)malloc(array_len * sizeof(sat_fract));
    fract* fract_array = (fract*)malloc(array_len * sizeof(fract));
    
    if (!sat_array || !fract_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < array_len; i++) {
        /* Create pattern of values */
        fract val = (fract)i / (fract)array_len;
        sat_array[i] = val;
        fract_array[i] = val * base_value;
    }
    
    /* Run various test functions */
    sat_accum total_accum = 0.0k;
    sat_fract total_fract = 0.0r;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Test 1: Complex operations */
        sat_fract result1 = complex_sat_operation(
            sat_array[iter % array_len],
            sat_array[(iter + 1) % array_len],
            fract_array[iter % array_len]
        );
        
        /* Test 2: Accumulation with range propagation */
        sat_accum result2 = accumulate_range(fract_array, array_len, base_value);
        
        /* Test 3: Multiplication with overflow checks */
        sat_accum mult_result;
        safe_multiply(&mult_result, result2, (sat_accum)base_value);
        
        /* Test 4: Shift operations */
        sat_fract result4 = shift_operations(
            fract_array[iter % array_len],
            iter % 4
        );
        
        /* Test 5: Array reduction */
        sat_accum result5 = array_reduction(sat_array, array_len);
        
        /* Test 6: Nested operations */
        sat_fract result6 = nested_operations(
            fract_array[iter % array_len],
            fract_array[(iter + 2) % array_len],
            3
        );
        
        /* Test 7: ASM operations */
        fract result7 = asm_fixed_point(fract_array[iter % array_len]);
        
        /* Combine results - complex expression requiring range analysis */
        total_accum = total_accum + mult_result + result5;
        total_fract = total_fract + result1 + result4 + result6 + result7;
        
        /* Conditional that depends on fixed-point range */
        if (total_accum > 0.8k || total_fract > 0.8r) {
            total_accum = total_accum * 0.9k;
            total_fract = total_fract * 0.9r;
        }
    }
    
    /* Test boundary conditions */
    test_boundaries();
    
    /* Final checksum calculation */
    sat_accum final_checksum = total_accum + (sat_accum)total_fract;
    
    /* Add array contributions */
    for (int i = 0; i < array_len; i++) {
        final_checksum = final_checksum + (sat_accum)sat_array[i];
        final_checksum = final_checksum + (sat_accum)fract_array[i];
    }
    
    /* Add global array contributions */
    for (int i = 0; i < 4; i++) {
        if (i < 2) final_checksum = final_checksum + (sat_accum)global_sat_fract_array[i];
        if (i < 2) final_checksum = final_checksum + global_accum_array[i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %f\n", (float)final_checksum);
    printf("Total accum: %f\n", (float)total_accum);
    printf("Total fract: %f\n", (float)total_fract);
    
    /* Cleanup */
    free(sat_array);
    free(fract_array);
    
    return 0;
}
