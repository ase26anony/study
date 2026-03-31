/* test_fixed.c - Comprehensive fixed-point test for GCC coverage */
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

/* Test functions with different fixed-point operations */

/* Function 1: Saturation boundary testing */
static inline sfract_t test_saturation_boundary(sfract_t a, sfract_t b, int shift) {
    /* Operations designed to hit saturation boundaries */
    sfract_t result = a + b;
    
    /* Multiplication near saturation */
    result = result * 0.999999r;
    
    /* Shift operation that could cause overflow/underflow */
    if (shift > 0) {
        /* Simulate shift through multiplication */
        accum_t temp = (accum_t)result * (1 << shift);
        result = (sfract_t)(temp / (1 << 8)); /* Adjust for fract scaling */
    }
    
    return result;
}

/* Function 2: Range propagation in loops */
static saccum_t range_propagation_loop(int iterations, accum_t base) {
    saccum_t total = 0k;
    accum_t multiplier = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        saccum_t term = (multiplier * i) / (iterations * 2k);
        
        /* Conditional based on range analysis */
        if (term > 0.5k && term < 1.5k) {
            total = total + term;
        } else if (term <= 0k) {
            total = total - 0.25k;
        }
        
        /* Update multiplier with saturation */
        multiplier = multiplier * 1.1k;
        if (multiplier > 10k) {
            multiplier = 10k;
        }
    }
    
    return total;
}

/* Function 3: Array reduction with fixed-point */
static fract_t array_reduction(const fract_t* arr, int size) {
    fract_t sum = 0r;
    fract_t product = 1r;
    
    for (int i = 0; i < size; i++) {
        sum = sum + arr[i];
        
        /* Product with range check */
        if (product * arr[i] < 0.1r && product * arr[i] > -0.1r) {
            product = product * arr[i];
        } else {
            product = product * 0.5r;
        }
    }
    
    /* Ternary operator with fixed-point operands */
    return (sum > 0.8r) ? (sum * 0.9r) : (product * 1.1r);
}

/* Function 4: Built-in overflow checks */
static int test_builtin_overflow(sfract_t a, sfract_t b, sfract_t* result) {
    int overflow = 0;
    
    /* Use built-in overflow detection */
    overflow |= __builtin_add_overflow(a, b, result);
    
    sfract_t temp;
    overflow |= __builtin_mul_overflow(*result, a, &temp);
    
    /* Conditional based on overflow detection */
    if (!overflow) {
        *result = temp;
        return 0;
    } else {
        /* Force saturation */
        *result = (a > 0r) ? 0.999999r : -0.999999r;
        return 1;
    }
}

/* Function 5: Switch statement with fixed-point conditions */
static accum_t fixed_point_switch(saccum_t value) {
    accum_t result = 0k;
    
    /* Switch based on fixed-point range */
    if (value < -0.5k) {
        result = value * 2k;
    } else if (value >= -0.5k && value < 0k) {
        result = value + 0.5k;
    } else if (value >= 0k && value < 0.5k) {
        result = value * value;
    } else if (value >= 0.5k && value < 1k) {
        result = 1k / value;
    } else {
        result = value - 1k;
    }
    
    return result;
}

/* Function 6: Nested arithmetic with shifts */
static sfract_t complex_shift_operations(sfract_t a, fract_t b, int shift_amt) {
    /* Convert to accum for shift simulation */
    accum_t temp_a = (accum_t)a * (1 << 8);
    accum_t temp_b = (accum_t)b * (1 << 8);
    
    /* Perform shift operations */
    if (shift_amt > 0) {
        temp_a = temp_a << shift_amt;
        temp_b = temp_b >> shift_amt;
    } else {
        temp_a = temp_a >> (-shift_amt);
        temp_b = temp_b << (-shift_amt);
    }
    
    /* Combine with saturation */
    accum_t combined = temp_a + temp_b;
    
    /* Check for overflow condition similar to uncovered code */
    if (combined > ((accum_t)0.999999r * (1 << 8))) {
        combined = (accum_t)0.999999r * (1 << 8);
    } else if (combined < ((accum_t)-0.999999r * (1 << 8))) {
        combined = (accum_t)-0.999999r * (1 << 8);
    }
    
    return (sfract_t)(combined / (1 << 8));
}

/* Function 7: Mixed-type operations */
static void mixed_type_operations(sfract_t* s_out, fract_t* f_out, 
                                  saccum_t sa_in, accum_t a_in) {
    /* Mix saturated and unsaturated types */
    *s_out = (sfract_t)((fract_t)sa_in * 0.75r);
    *f_out = (fract_t)((sfract_t)a_in + 0.25r);
    
    /* Additional mixing */
    accum_t temp = (accum_t)*s_out * 2k;
    *s_out = (sfract_t)(temp / 2k);
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 20;
    int seed = (argc > 3) ? atoi(argv[3]) : 42;
    
    /* Initialize fixed-point arrays */
    fract_t f_array[array_size];
    sfract_t sf_array[array_size];
    
    srand(seed);
    for (int i = 0; i < array_size; i++) {
        /* Generate values in range [-0.9, 0.9] */
        float rand_val = (float)rand() / RAND_MAX * 1.8f - 0.9f;
        f_array[i] = (fract_t)rand_val;
        sf_array[i] = (sfract_t)rand_val;
    }
    
    /* Test 1: Saturation boundary */
    sfract_t sat_result = 0r;
    for (int i = 0; i < iterations; i++) {
        sfract_t a = (sfract_t)(0.5r + (i * 0.05r));
        sfract_t b = (sfract_t)(0.3r - (i * 0.02r));
        sat_result = sat_result + test_saturation_boundary(a, b, i % 4);
    }
    
    /* Test 2: Range propagation */
    accum_t base_val = (accum_t)((seed % 100) / 100.0);
    saccum_t range_result = range_propagation_loop(iterations, base_val);
    
    /* Test 3: Array reduction */
    fract_t reduction_result = array_reduction(f_array, array_size);
    
    /* Test 4: Built-in overflow */
    sfract_t builtin_result;
    int overflow_count = 0;
    for (int i = 0; i < array_size / 2; i++) {
        overflow_count += test_builtin_overflow(
            sf_array[i], 
            sf_array[array_size - i - 1], 
            &builtin_result
        );
    }
    
    /* Test 5: Switch-based operations */
    accum_t switch_result = 0k;
    for (int i = 0; i < array_size; i++) {
        saccum_t val = (saccum_t)((accum_t)f_array[i]);
        switch_result = switch_result + fixed_point_switch(val);
    }
    
    /* Test 6: Complex shift operations */
    sfract_t shift_result = 0r;
    for (int i = 0; i < iterations; i++) {
        shift_result = shift_result + 
            complex_shift_operations(
                sf_array[i % array_size],
                f_array[(i + 1) % array_size],
                (i % 7) - 3  /* Shift from -3 to 3 */
            );
    }
    
    /* Test 7: Mixed-type operations */
    sfract_t mixed_sf_result = 0r;
    fract_t mixed_f_result = 0r;
    for (int i = 0; i < iterations; i++) {
        saccum_t sa_val = (saccum_t)(i * 0.1k);
        accum_t a_val = (accum_t)((array_size - i) * 0.05k);
        mixed_type_operations(&mixed_sf_result, &mixed_f_result, sa_val, a_val);
    }
    
    /* Final checksum calculation to prevent dead code elimination */
    accum_t checksum = 0k;
    checksum = checksum + (accum_t)sat_result;
    checksum = checksum + (accum_t)range_result;
    checksum = checksum + (accum_t)reduction_result;
    checksum = checksum + (accum_t)builtin_result;
    checksum = checksum + switch_result;
    checksum = checksum + (accum_t)shift_result;
    checksum = checksum + (accum_t)mixed_sf_result;
    checksum = checksum + (accum_t)mixed_f_result;
    
    /* Print results (converted to float) */
    printf("Test Results:\n");
    printf("  Saturation test: %f\n", (float)sat_result);
    printf("  Range propagation: %f\n", (float)range_result);
    printf("  Array reduction: %f\n", (float)reduction_result);
    printf("  Overflow count: %d\n", overflow_count);
    printf("  Switch operations: %f\n", (float)switch_result);
    printf("  Shift operations: %f\n", (float)shift_result);
    printf("  Mixed-type SF: %f, F: %f\n", 
           (float)mixed_sf_result, (float)mixed_f_result);
    printf("  Final checksum: %f\n", (float)checksum);
    
    return (checksum != 0k) ? 0 : 1;
}
