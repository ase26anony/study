/* test_fixed_point.c - Comprehensive fixed-point test for GCC coverage */
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

/* Global arrays for range propagation */
static sat_fract g_sat_fract_arr[100];
static sat_accum g_sat_accum_arr[50];
static fract g_fract_arr[100];
static accum g_accum_arr[50];

/* Test 1: Complex fixed-point arithmetic with saturation */
static inline sat_fract test1_sat_add(sat_fract a, sat_fract b, sat_fract c) {
    /* This should trigger saturation boundary checks */
    sat_fract t1 = a + b;
    sat_fract t2 = t1 * c;
    sat_fract t3 = t2 >> 2;  /* Shift operation */
    
    /* Conditional based on range analysis */
    if (t3 > 0.8r) {
        return t3 - 0.1r;
    } else if (t3 < -0.8r) {
        return t3 + 0.1r;
    }
    return t3;
}

/* Test 2: Accumulator with loop-based range propagation */
static sat_accum test2_accum_reduction(int iterations) {
    sat_accum total = 0.0k;
    sat_accum step = 0.1k;
    
    for (int i = 0; i < iterations; i++) {
        /* Operations that may saturate */
        sat_accum temp = total + (step * i);
        
        /* Ternary with fixed-point operands */
        total = (temp > 10.0k) ? 10.0k : 
                (temp < -10.0k) ? -10.0k : temp;
        
        /* Shift operation that requires range analysis */
        if (i % 3 == 0) {
            total = total >> 1;
        }
    }
    return total;
}

/* Test 3: Mixed saturated/unsaturated operations */
static fract test3_mixed_conversion(sat_fract a, fract b) {
    /* Conversion between saturated and unsaturated */
    fract temp = (fract)a + b;
    
    /* Multiplication that may overflow */
    fract result = temp * 1.5r;
    
    /* Use builtin for overflow check */
    fract check;
    if (__builtin_mul_overflow(temp, 1.5r, &check)) {
        /* Should trigger overflow path in range analysis */
        return 0.999999r;  /* Max representable */
    }
    
    /* Conditional based on range */
    return (result > 0.9r) ? result - 0.2r : result;
}

/* Test 4: Array reduction with complex indexing */
static sat_accum test4_array_ops(int start, int end) {
    sat_accum max_val = -100.0k;
    sat_accum min_val = 100.0k;
    
    for (int i = start; i < end && i < 50; i++) {
        /* Complex index calculation */
        int idx = (i * 3) % 50;
        
        /* Operation that requires range analysis */
        g_sat_accum_arr[idx] = g_sat_accum_arr[idx] * 1.1k + 0.05k;
        
        /* Update min/max - triggers comparison logic */
        if (g_sat_accum_arr[idx] > max_val) {
            max_val = g_sat_accum_arr[idx];
        }
        if (g_sat_accum_arr[idx] < min_val) {
            min_val = g_sat_accum_arr[idx];
        }
        
        /* Shift operation */
        if (idx % 4 == 0) {
            g_sat_accum_arr[idx] = g_sat_accum_arr[idx] >> 2;
        }
    }
    
    /* Return range difference */
    return max_val - min_val;
}

/* Test 5: Nested function calls with fixed-point */
static inline sat_fract test5_inner(sat_fract x, sat_fract y) {
    /* Operations near saturation boundaries */
    sat_fract sum = x + y;
    sat_fract prod = sum * 0.9r;
    
    /* This should trigger the uncovered comparison logic */
    if (prod > 0.95r || prod < -0.95r) {
        return prod >> 1;
    }
    return prod;
}

static sat_fract test5_outer(int count) {
    sat_fract result = 0.5r;
    
    for (int i = 0; i < count; i++) {
        sat_fract inc = (i % 2 == 0) ? 0.2r : -0.15r;
        result = test5_inner(result, inc);
        
        /* Additional shift that requires range analysis */
        if (result > 0.8r) {
            result = result >> 1;
        }
    }
    return result;
}

/* Test 6: Switch statement with fixed-point conditions */
static fract test6_switch_demo(fract input) {
    fract output;
    
    /* Switch based on fixed-point range */
    if (input > 0.8r) {
        output = input * 0.5r;
    } else if (input > 0.5r) {
        output = input + 0.1r;
    } else if (input > 0.2r) {
        output = input - 0.05r;
    } else if (input > -0.3r) {
        output = input * 2.0r;  /* May overflow */
    } else {
        output = input >> 2;
    }
    
    /* Additional saturation check */
    if (output > 0.99r) {
        output = 0.99r;
    } else if (output < -0.99r) {
        output = -0.99r;
    }
    
    return output;
}

/* Test 7: Using asm volatile to create complex value flows */
static sat_accum test7_asm_operations(sat_accum a, sat_accum b) {
    sat_accum result;
    
    /* Inline asm with fixed-point constraints */
    asm volatile (
        "add %[res], %[a], %[b]\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b)
    );
    
    /* Operations after asm - compiler must analyze range */
    result = result * 1.5k;
    
    /* Conditional that should trigger the uncovered code */
    if (result > 50.0k || result < -50.0k) {
        result = result >> 3;
    }
    
    return result;
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
        if (array_size > 100) array_size = 100;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        g_fract_arr[i] = (fract)((i % 10) * 0.1r);
        g_sat_fract_arr[i] = (sat_fract)((i % 10) * 0.1r);
    }
    for (int i = 0; i < 50; i++) {
        g_accum_arr[i] = (accum)((i % 20) * 0.5k);
        g_sat_accum_arr[i] = (sat_accum)((i % 20) * 0.5k);
    }
    
    /* Run all tests */
    sat_fract test1_result = test1_sat_add(0.9r, 0.8r, 0.7r);
    sat_accum test2_result = test2_accum_reduction(iterations);
    fract test3_result = test3_mixed_conversion(0.95r, 0.6r);
    sat_accum test4_result = test4_array_ops(0, array_size);
    sat_fract test5_result = test5_outer(iterations);
    fract test6_result = test6_switch_demo(0.75r);
    sat_accum test7_result = test7_asm_operations(10.0k, 20.0k);
    
    /* Additional complex expression to ensure all code is used */
    sat_accum final_checksum = 0.0k;
    for (int i = 0; i < array_size && i < 50; i++) {
        final_checksum = final_checksum + g_sat_accum_arr[i];
        if (i % 3 == 0) {
            final_checksum = final_checksum * 1.01k;
        }
    }
    
    /* Convert to float and print results to prevent dead code elimination */
    printf("Test Results:\n");
    printf("  Test1: %f\n", (float)test1_result);
    printf("  Test2: %f\n", (float)test2_result);
    printf("  Test3: %f\n", (float)test3_result);
    printf("  Test4: %f\n", (float)test4_result);
    printf("  Test5: %f\n", (float)test5_result);
    printf("  Test6: %f\n", (float)test6_result);
    printf("  Test7: %f\n", (float)test7_result);
    printf("  Checksum: %f\n", (float)final_checksum);
    
    /* Additional boundary tests */
    sat_fract boundary_test = 0.999999r;
    boundary_test = boundary_test + 0.000001r;  /* Should saturate */
    printf("  Boundary test: %f (should be 1.0 or saturated)\n", (float)boundary_test);
    
    /* Test negative saturation */
    sat_fract neg_test = -0.999999r;
    neg_test = neg_test - 0.000001r;  /* Should saturate */
    printf("  Negative boundary: %f\n", (float)neg_test);
    
    return 0;
}
