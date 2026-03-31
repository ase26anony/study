/* test_fixed.c - Program to trigger fixed-point range analysis in GCC */
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

/* Test functions with different fixed-point operations */

/* Function 1: Complex saturated addition with range-dependent branching */
static inline sfract_t sat_add_range(sfract_t a, sfract_t b, fract_t threshold) {
    sfract_t sum = a + b;
    /* This should trigger range analysis for the comparison */
    if (sum > threshold) {
        return sum - (threshold / 2);
    } else {
        return sum + (threshold / 2);
    }
}

/* Function 2: Multiplication with shift causing potential overflow */
static inline saccum_t mul_shift_overflow(saccum_t x, int shift) {
    saccum_t result = x * 2.0k;
    /* Shift operation that requires range analysis */
    result = result >> shift;
    return result;
}

/* Function 3: Nested conditional with fixed-point operations */
static inline fract_t conditional_chain(fract_t a, fract_t b, fract_t c) {
    fract_t temp;
    
    /* Complex conditional that depends on ranges */
    if (a > 0.5r && b < 0.3r) {
        temp = a * b;
    } else if (a + b > 0.8r) {
        temp = a - b;
    } else {
        temp = a / (b + 0.1r);
    }
    
    /* Ternary operator with fixed-point */
    return (c > 0.0r) ? temp * c : temp / (c - 0.1r);
}

/* Function 4: Loop-based range propagation */
static accum_t loop_range_propagation(int iterations, fract_t base) {
    accum_t total = 0.0k;
    fract_t increment = 0.1r;
    
    for (fract_t f = base; f < 0.9r; f += increment) {
        /* Operations that require analyzing ranges across iterations */
        total = total + (accum_t)f * 2.0k;
        
        /* Conditional that becomes compile-time constant after range analysis */
        if (f > 0.7r) {
            total = total >> 1;
        }
    }
    
    return total;
}

/* Function 5: Array reduction with saturation */
static sfract_t array_saturation_reduction(sfract_t arr[], int size) {
    sfract_t sum = 0.0r;
    
    for (int i = 0; i < size; i++) {
        /* This addition should trigger saturation analysis */
        sum = sum + arr[i];
        
        /* Complex expression requiring range tracking */
        if (i % 2 == 0) {
            sum = sum * 0.9r;
        } else {
            sum = sum / 0.9r;
        }
    }
    
    return sum;
}

/* Function 6: Mixed-type operations causing conversions */
static accum_t mixed_type_operations(fract_t a, accum_t b, sfract_t c) {
    /* Mixing saturated and unsaturated types */
    accum_t result = (accum_t)a + b;
    
    /* Operations that require range checks during conversion */
    sfract_t saturated = (sfract_t)(a * 0.5r);
    result = result * (accum_t)saturated;
    
    /* Using builtins for overflow detection */
    sfract_t overflow_test;
    if (__builtin_add_overflow_p(a, c, (sfract_t)0.0r)) {
        result = result >> 2;
    }
    
    return result;
}

/* Function 7: Switch statement based on fixed-point ranges */
static int switch_on_fixed_range(sfract_t value) {
    int result = 0;
    
    /* Switch where cases depend on fixed-point comparisons */
    switch ((int)(value * 10.0r)) {
        case 0 ... 3:  /* 0.0 - 0.3 */
            result = 1;
            break;
        case 4 ... 6:  /* 0.4 - 0.6 */
            result = 2;
            break;
        case 7 ... 9:  /* 0.7 - 0.9 */
            result = 3;
            break;
        default:       /* Saturated values */
            result = 4;
            break;
    }
    
    return result;
}

/* Function 8: Explicit saturation boundary testing */
static void test_saturation_boundaries(void) {
    sfract_t max_fract = 0.999999r;
    sfract_t min_fract = -0.999999r;
    
    /* Operations designed to hit saturation */
    sfract_t sat1 = max_fract + 0.1r;      /* Should saturate to max */
    sfract_t sat2 = min_fract - 0.1r;      /* Should saturate to min */
    
    /* Multiplication causing overflow */
    sfract_t sat3 = max_fract * 1.5r;      /* Should saturate */
    
    /* Use results to prevent dead code elimination */
    volatile sfract_t dummy __attribute__((unused)) = sat1 + sat2 + sat3;
}

/* Function 9: Bit-shift operations on fixed-point */
static accum_t shift_operations(accum_t value, int shift_amt) {
    accum_t result = value;
    
    /* Various shift operations requiring range analysis */
    if (shift_amt > 0) {
        result = result << shift_amt;
    } else if (shift_amt < 0) {
        result = result >> (-shift_amt);
    }
    
    /* Combined shift and arithmetic */
    result = (result * 2.0k) >> 3;
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int iterations = 10;
    int array_size = 8;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 8;
    }
    
    /* Initialize arrays */
    sfract_t sat_array[array_size];
    fract_t unsat_array[array_size];
    
    for (int i = 0; i < array_size; i++) {
        sat_array[i] = (sfract_t)(i * 0.125r);
        unsat_array[i] = (fract_t)(i * 0.125r);
    }
    
    /* Test 1: Saturated addition with range branching */
    sfract_t test1 = 0.0r;
    for (int i = 0; i < iterations; i++) {
        test1 = sat_add_range(test1, 0.2r, 0.5r);
    }
    
    /* Test 2: Multiplication with shift overflow */
    saccum_t test2 = 0.5k;
    for (int i = 1; i <= 4; i++) {
        test2 = mul_shift_overflow(test2, i);
    }
    
    /* Test 3: Conditional chain */
    fract_t test3 = conditional_chain(0.6r, 0.2r, 0.4r);
    
    /* Test 4: Loop range propagation */
    accum_t test4 = loop_range_propagation(iterations, 0.1r);
    
    /* Test 5: Array saturation reduction */
    sfract_t test5 = array_saturation_reduction(sat_array, array_size);
    
    /* Test 6: Mixed type operations */
    accum_t test6 = mixed_type_operations(0.7r, 1.5k, 0.3r);
    
    /* Test 7: Switch on fixed range */
    int test7 = switch_on_fixed_range(test5);
    
    /* Test 8: Saturation boundaries */
    test_saturation_boundaries();
    
    /* Test 9: Shift operations */
    accum_t test9 = shift_operations(1.0k, 2);
    
    /* Final checksum calculation using all results */
    accum_t checksum = (accum_t)test1 + test2 + (accum_t)test3 + test4 + 
                      (accum_t)test5 + test6 + (accum_t)test7 + test9;
    
    /* Convert to float and print to prevent dead code elimination */
    printf("Checksum: %f\n", (float)checksum);
    printf("Test results: %f %f %f %f %f %f %d %f\n",
           (float)test1, (float)test2, (float)test3, (float)test4,
           (float)test5, (float)test6, test7, (float)test9);
    
    return 0;
}
