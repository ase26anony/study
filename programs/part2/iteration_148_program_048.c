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
typedef _Sat short _Accum ssaccum_t;

/* Test functions with various fixed-point operations */

/* Function 1: Saturation boundary testing */
static inline sfract_t test_saturation_boundary(sfract_t a, sfract_t b) {
    /* Operations that should trigger saturation analysis */
    sfract_t sum = a + b;
    sfract_t prod = a * b;
    
    /* Conditional based on range analysis */
    if (sum > 0.999r) {
        return 0.999r;
    } else if (prod < -0.999r) {
        return -0.999r;
    }
    
    /* Ternary with fixed-point operands */
    return (sum > 0.5r) ? sum : prod;
}

/* Function 2: Accumulator range propagation */
static inline saccum_t test_accum_range(saccum_t base, int iterations) {
    saccum_t result = base;
    
    /* Loop with fixed-point induction */
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        result = result * 1.5k + 0.25k;
        
        /* Shift operation that can cause overflow */
        if (i % 2 == 0) {
            result = result >> 2;
        } else {
            result = result << 1;
        }
    }
    
    return result;
}

/* Function 3: Mixed-type operations */
static inline accum_t test_mixed_types(fract_t a, accum_t b) {
    /* Mixed precision operations */
    accum_t scaled_a = (accum_t)a * 10.0k;
    accum_t result = scaled_a + b;
    
    /* Division with potential overflow */
    if (b != 0.0k) {
        result = result / b;
    }
    
    /* Built-in overflow check */
    int overflow = 0;
    accum_t temp = __builtin_add_overflow(scaled_a, b, &result) ? 0.0k : result;
    
    return overflow ? 0.0k : result;
}

/* Function 4: Array reduction with fixed-point */
static sfract_t test_array_reduction(const sfract_t* arr, int size) {
    sfract_t total = 0.0r;
    sfract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Operations that can saturate */
        total = total + arr[i];
        product = product * arr[i];
        
        /* Conditional based on intermediate range */
        if (total > 0.8r) {
            total = total - 0.1r;
        }
        
        /* Nested ternary with fixed-point */
        product = (product < 0.0r) ? -product : product;
    }
    
    /* Final range-dependent operation */
    return (total > product) ? total : product;
}

/* Function 5: Complex control flow with fixed-point */
static saccum_t test_complex_control(saccum_t start, int steps) {
    saccum_t current = start;
    
    switch (steps % 4) {
        case 0:
            current = current * 2.0k;
            break;
        case 1:
            current = current / 2.0k;
            break;
        case 2:
            current = current + current;
            break;
        case 3:
            current = current - start;
            break;
    }
    
    /* Loop with fixed-point condition */
    while (current > 0.0k && steps > 0) {
        current = current * 0.9k;
        steps--;
        
        /* Inline assembly to create hard-to-analyze values */
        asm volatile ("" : "+r" (current) : : "memory");
    }
    
    return current;
}

/* Function 6: Boundary value testing */
static void test_boundary_values(void) {
    /* Values at or near saturation boundaries */
    sfract_t max_fract = 0.999999r;
    sfract_t min_fract = -0.999999r;
    saccum_t max_accum = 9223372036854775.807k;  /* Near max */
    saccum_t min_accum = -9223372036854775.807k; /* Near min */
    
    /* Operations designed to hit boundaries */
    sfract_t test1 = max_fract + 0.000001r;  /* Should saturate */
    sfract_t test2 = min_fract - 0.000001r;  /* Should saturate */
    saccum_t test3 = max_accum * 1.1k;       /* Should saturate */
    saccum_t test4 = min_accum / 0.5k;       /* Should saturate */
    
    /* Use results to prevent dead code elimination */
    volatile sfract_t v1 = test1;
    volatile sfract_t v2 = test2;
    volatile saccum_t v3 = test3;
    volatile saccum_t v4 = test4;
}

/* Function 7: Nested function calls with fixed-point */
static accum_t nested_calls(accum_t x, int depth) {
    if (depth <= 0) {
        return x;
    }
    
    /* Recursive calls with different operations */
    accum_t a = nested_calls(x * 0.5k, depth - 1);
    accum_t b = nested_calls(x + 1.0k, depth - 1);
    
    /* Range-dependent operation */
    if (a > b) {
        return a - b;
    } else {
        return b - a;
    }
}

/* Main test driver */
int main(int argc, char* argv[]) {
    /* Use command-line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 20;
    
    if (iterations <= 0) iterations = 10;
    if (array_size <= 0) array_size = 20;
    if (array_size > 100) array_size = 100;
    
    printf("Testing fixed-point range analysis with iterations=%d, array_size=%d\n",
           iterations, array_size);
    
    /* Initialize fixed-point arrays */
    sfract_t fract_array[100];
    accum_t accum_array[100];
    
    for (int i = 0; i < array_size; i++) {
        fract_array[i] = ((sfract_t)i / array_size) - 0.5r;
        accum_array[i] = ((accum_t)i * 100.0k) / array_size;
    }
    
    /* Run various tests */
    sfract_t fract_result = 0.0r;
    saccum_t accum_result = 0.0k;
    accum_t mixed_result = 0.0k;
    
    /* Test 1: Saturation boundaries */
    test_boundary_values();
    
    /* Test 2: Array reduction */
    fract_result = test_array_reduction(fract_array, array_size);
    
    /* Test 3: Accumulator range */
    accum_result = test_accum_range(0.5k, iterations);
    
    /* Test 4: Mixed types */
    mixed_result = test_mixed_types(0.7r, 50.0k);
    
    /* Test 5: Complex control flow */
    saccum_t control_result = test_complex_control(100.0k, iterations);
    
    /* Test 6: Nested calls */
    accum_t nested_result = nested_calls(10.0k, 3);
    
    /* Test 7: Direct saturation tests */
    for (int i = 0; i < iterations; i++) {
        sfract_t a = ((sfract_t)i / iterations) * 2.0r - 1.0r;
        sfract_t b = 0.5r;
        fract_result = fract_result + test_saturation_boundary(a, b);
    }
    
    /* Final checksum calculation using all results */
    accum_t final_checksum = (accum_t)fract_result + accum_result + mixed_result 
                           + control_result + nested_result;
    
    /* Convert to float for printing (prevents dead code elimination) */
    printf("Results (as float):\n");
    printf("  fract_result: %f\n", (float)fract_result);
    printf("  accum_result: %f\n", (float)accum_result);
    printf("  mixed_result: %f\n", (float)mixed_result);
    printf("  control_result: %f\n", (float)control_result);
    printf("  nested_result: %f\n", (float)nested_result);
    printf("  final_checksum: %f\n", (float)final_checksum);
    
    /* Additional overflow tests using builtins */
    {
        sfract_t of_a = 0.9r;
        sfract_t of_b = 0.9r;
        sfract_t of_result;
        int overflow = __builtin_add_overflow(of_a, of_b, &of_result);
        printf("  Overflow test: %d (result: %f)\n", overflow, (float)of_result);
    }
    
    return 0;
}
