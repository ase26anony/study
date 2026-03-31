/* test_fixed_point.c - Target coverage of fixed-value.cc lines 264-277 */
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

/* Global arrays for range propagation */
static sat_fract_t global_fract_array[100];
static sat_accum_t global_accum_array[50];

/* Test function 1: Complex fixed-point arithmetic with saturation */
static inline sat_fract_t complex_saturating_add(sat_fract_t a, sat_fract_t b, sat_fract_t c) {
    /* Nested operations that may overflow */
    sat_fract_t temp = a + b;
    temp = temp * c;
    
    /* Shift operation that requires range analysis */
    int shift_amount = 2;
    temp = temp >> shift_amount;
    
    /* Conditional based on range */
    if (temp > 0.8r) {
        return temp - 0.1r;
    } else if (temp < -0.8r) {
        return temp + 0.1r;
    }
    return temp;
}

/* Test function 2: Accumulator with loop-based range analysis */
static inline sat_accum_t accumulate_range(int iterations, fract_t base) {
    sat_accum_t total = 0.0k;
    sat_accum_t multiplier = 1.5k;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex expression requiring range analysis */
        sat_accum_t term = (base * i) * multiplier;
        term = term >> (i % 4);  /* Variable shift */
        
        /* This addition may saturate */
        total = total + term;
        
        /* Range-dependent conditional */
        if (total > 100.0k) {
            total = total / 2.0k;
        } else if (total < -100.0k) {
            total = total * 0.75k;
        }
    }
    return total;
}

/* Test function 3: Fixed-point array reduction with boundary conditions */
static sat_fract_t array_reduction(sat_fract_t* arr, int size) {
    sat_fract_t sum = 0.0r;
    sat_fract_t product = 1.0r;
    
    for (int i = 0; i < size; i++) {
        /* Operations designed to hit saturation boundaries */
        sum = sum + arr[i];
        product = product * arr[i];
        
        /* Critical: This comparison triggers the range analysis */
        if (sum > 0.95r || product < -0.95r) {
            /* Reset to avoid permanent saturation */
            sum = sum * 0.5r;
            product = product * 0.5r;
        }
        
        /* Ternary with fixed-point operands */
        sat_fract_t adjustment = (i % 2 == 0) ? 0.01r : -0.01r;
        arr[i] = arr[i] + adjustment;
    }
    
    /* Final range-dependent operation */
    return (sum > product) ? sum : product;
}

/* Test function 4: Mixed-type operations with builtins */
static sat_accum_t mixed_operations_with_builtins(fract_t a, accum_t b) {
    sat_accum_t result = 0.0k;
    
    /* Use builtins for overflow detection */
    sat_accum_t temp1, temp2;
    int overflow1, overflow2;
    
    /* These builtins should trigger the fixed-value machinery */
    overflow1 = __builtin_add_overflow(a, b, &temp1);
    overflow2 = __builtin_mul_overflow(a, b, &temp2);
    
    if (overflow1 || overflow2) {
        /* Handle overflow - this path uses range analysis */
        result = (temp1 + temp2) / 2.0k;
    } else {
        result = temp1 * temp2;
    }
    
    /* Shift operation that may underflow/overflow */
    result = result >> 3;
    result = result << 2;
    
    return result;
}

/* Test function 5: Switch statement with fixed-point conditions */
static fract_t switch_based_on_fixed_point(sat_fract_t value) {
    fract_t result = 0.0r;
    
    /* Switch on discretized fixed-point value */
    switch ((int)(value * 10)) {
        case 0:  /* value in [0.0, 0.1) */
            result = value * 2.0r;
            break;
        case 1:  /* value in [0.1, 0.2) */
            result = value / 0.5r;
            break;
        case 2:  /* value in [0.2, 0.3) */
            result = value + 0.5r;
            break;
        case 9:  /* value in [0.9, 1.0] - near saturation */
            result = value * 1.1r;  /* May saturate */
            break;
        default:
            result = value - 0.1r;
    }
    
    return result;
}

/* Test function 6: Inline assembly with fixed-point constraints */
static sat_accum_t asm_fixed_point_flow(sat_accum_t x, sat_accum_t y) {
    sat_accum_t result;
    
    /* Assembly that creates hard-to-analyze value flow */
    asm volatile (
        "add %[res], %[x], %[y]\n\t"
        "asr %[res], %[res], #2\n\t"
        : [res] "=r" (result)
        : [x] "r" (x), [y] "r" (y)
        : "cc"
    );
    
    /* Post-assembly range-dependent operation */
    if (result > 50.0k) {
        result = result - 25.0k;
    } else if (result < -50.0k) {
        result = result + 25.0k;
    }
    
    return result;
}

/* Initialize arrays with boundary values */
static void init_arrays(void) {
    /* Fill with values near saturation boundaries */
    for (int i = 0; i < 100; i++) {
        if (i % 10 == 0) {
            global_fract_array[i] = 0.999999r;  /* Near max */
        } else if (i % 10 == 5) {
            global_fract_array[i] = -0.999999r; /* Near min */
        } else {
            global_fract_array[i] = ((i % 20) - 10) * 0.1r;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        global_accum_array[i] = (i - 25) * 10.0k;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Use command-line arguments for runtime variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int array_size = (argc > 2) ? atoi(argv[2]) : 20;
    
    if (iterations < 1) iterations = 1;
    if (array_size < 1) array_size = 1;
    if (array_size > 100) array_size = 100;
    
    printf("Testing fixed-point range analysis (iterations=%d, array_size=%d)\n", 
           iterations, array_size);
    
    /* Initialize test data */
    init_arrays();
    
    /* Test 1: Complex saturating operations */
    sat_fract_t test1_a = 0.7r;
    sat_fract_t test1_b = 0.6r;
    sat_fract_t test1_c = 0.8r;
    sat_fract_t result1 = complex_saturating_add(test1_a, test1_b, test1_c);
    printf("Test 1 result: %f\n", (float)result1);
    
    /* Test 2: Loop-based accumulation */
    sat_accum_t result2 = accumulate_range(iterations, 0.3r);
    printf("Test 2 result: %f\n", (float)result2);
    
    /* Test 3: Array reduction with boundary checks */
    sat_fract_t local_array[50];
    memcpy(local_array, global_fract_array, sizeof(local_array[0]) * 50);
    sat_fract_t result3 = array_reduction(local_array, array_size);
    printf("Test 3 result: %f\n", (float)result3);
    
    /* Test 4: Mixed operations with builtins */
    sat_accum_t result4 = mixed_operations_with_builtins(0.9r, 100.0k);
    printf("Test 4 result: %f\n", (float)result4);
    
    /* Test 5: Switch statement */
    fract_t test5_values[] = {0.05r, 0.15r, 0.25r, 0.95r};
    for (int i = 0; i < 4; i++) {
        fract_t result5 = switch_based_on_fixed_point(test5_values[i]);
        printf("Test 5[%d] result: %f\n", i, (float)result5);
    }
    
    /* Test 6: Assembly flow */
    sat_accum_t result6 = asm_fixed_point_flow(30.0k, 40.0k);
    printf("Test 6 result: %f\n", (float)result6);
    
    /* Final checksum to prevent dead code elimination */
    sat_accum_t checksum = 0.0k;
    checksum = checksum + result1;
    checksum = checksum + result2;
    checksum = checksum + result3;
    checksum = checksum + result4;
    checksum = checksum + result6;
    
    printf("Final checksum: %f\n", (float)checksum);
    
    return 0;
}
