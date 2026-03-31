/* test_fixed.c - Fixed-point range analysis stress test */
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

/* Function 1: Complex saturated addition with overflow */
static inline sfract_t sat_add_complex(sfract_t a, sfract_t b, sfract_t c) {
    /* This should trigger range analysis for saturation */
    sfract_t sum = a + b;
    /* Conditional that depends on range analysis */
    if (sum > 0.8r) {
        return sum + c;  /* Potential overflow */
    } else {
        return sum - c;  /* Potential underflow */
    }
}

/* Function 2: Multiplication with shift - requires precise range analysis */
static inline saccum_t mul_shift_range(saccum_t x, int shift) {
    /* Complex expression requiring range tracking */
    saccum_t temp = x * 2.0k;
    /* Shift operation that affects range */
    if (shift > 0) {
        temp = temp >> shift;
    } else {
        temp = temp << (-shift);
    }
    return temp * 0.5k;
}

/* Function 3: Loop-based accumulation with saturation */
static sfract_t loop_accumulate(sfract_t start, sfract_t step, int iterations) {
    sfract_t total = 0.0r;
    for (int i = 0; i < iterations; i++) {
        sfract_t val = start + (step * i);
        /* This addition can saturate depending on ranges */
        total = total + val;
        /* Conditional branch based on fixed-point comparison */
        if (total > 0.95r) {
            total = 0.95r;  /* Manual clamping */
        }
    }
    return total;
}

/* Function 4: Array reduction with mixed types */
static accum_t array_reduction(const fract_t* arr, int size) {
    accum_t result = 0.0k;
    for (int i = 0; i < size; i++) {
        /* Mixed-type arithmetic requiring conversion analysis */
        accum_t scaled = (accum_t)arr[i] * 1.5k;
        result = result + scaled;
        
        /* Ternary operator with fixed-point operands */
        result = (result > 10.0k) ? 10.0k : 
                (result < -10.0k) ? -10.0k : result;
    }
    return result;
}

/* Function 5: Using builtins for overflow detection */
static int builtin_overflow_test(sfract_t a, sfract_t b, sfract_t* res) {
    /* Use builtin for overflow detection */
    return __builtin_add_overflow(a, b, res);
}

/* Function 6: Switch statement based on fixed-point ranges */
static int range_based_switch(saccum_t value) {
    switch ((int)(value * 10.0k)) {
        case 0 ... 3:  /* 0.0 to 0.3 */
            return 1;
        case 4 ... 6:  /* 0.4 to 0.6 */
            return 2;
        case 7 ... 10: /* 0.7 to 1.0 */
            return 3;
        default:
            return 0;
    }
}

/* Function 7: Nested loops with fixed-point induction */
static saccum_t nested_loop_test(int outer, int inner) {
    saccum_t matrix[10][10];
    saccum_t total = 0.0k;
    
    /* Initialize with values that may saturate */
    for (int i = 0; i < outer && i < 10; i++) {
        for (int j = 0; j < inner && j < 10; j++) {
            matrix[i][j] = (i * 0.1k) + (j * 0.05k);
        }
    }
    
    /* Process with operations that require range analysis */
    for (int i = 0; i < outer && i < 10; i++) {
        saccum_t row_sum = 0.0k;
        for (int j = 0; j < inner && j < 10; j++) {
            /* Operation that can overflow */
            row_sum = row_sum + (matrix[i][j] * 1.2k);
        }
        total = total + row_sum;
    }
    
    return total;
}

/* Function 8: Bit-shift operations on fixed-point */
static accum_t shift_operations(accum_t base, int* shifts, int count) {
    accum_t result = base;
    for (int i = 0; i < count; i++) {
        /* Various shift operations requiring range analysis */
        if (shifts[i] > 0) {
            result = result >> shifts[i];
        } else if (shifts[i] < 0) {
            result = result << (-shifts[i]);
        }
        
        /* Multiplication that interacts with shifted range */
        result = result * 0.75k;
    }
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use command line arguments for variability */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    int array_size = (argc > 2) ? atoi(argv[2]) : 8;
    
    if (iterations < 1) iterations = 1;
    if (array_size < 4) array_size = 4;
    if (array_size > 20) array_size = 20;
    
    printf("Running fixed-point tests with iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Initialize test arrays */
    fract_t frac_array[20];
    accum_t accum_array[20];
    int shift_array[] = {1, -2, 3, -1, 2, -3};
    
    for (int i = 0; i < array_size; i++) {
        frac_array[i] = (fract_t)(i * 0.1r);
        accum_array[i] = (accum_t)(i * 0.05k);
    }
    
    /* Test 1: Saturated addition with overflow conditions */
    sfract_t s1 = 0.8r;
    sfract_t s2 = 0.3r;
    sfract_t s3 = 0.4r;
    sfract_t result1 = sat_add_complex(s1, s2, s3);
    printf("Test 1 - Saturated add: %f\n", (float)result1);
    
    /* Test 2: Multiplication with shift */
    saccum_t acc1 = 0.7k;
    saccum_t result2 = mul_shift_range(acc1, 2);
    printf("Test 2 - Mul shift: %f\n", (float)result2);
    
    /* Test 3: Loop accumulation */
    sfract_t result3 = loop_accumulate(0.1r, 0.15r, iterations);
    printf("Test 3 - Loop accumulate: %f\n", (float)result3);
    
    /* Test 4: Array reduction */
    accum_t result4 = array_reduction(frac_array, array_size);
    printf("Test 4 - Array reduction: %f\n", (float)result4);
    
    /* Test 5: Builtin overflow detection */
    sfract_t overflow_result;
    int overflow = builtin_overflow_test(0.9r, 0.2r, &overflow_result);
    printf("Test 5 - Overflow detection: %d, result: %f\n", 
           overflow, (float)overflow_result);
    
    /* Test 6: Switch based on ranges */
    int switch_result = range_based_switch(0.45k);
    printf("Test 6 - Range switch: %d\n", switch_result);
    
    /* Test 7: Nested loops */
    saccum_t result7 = nested_loop_test(iterations, array_size % 10);
    printf("Test 7 - Nested loops: %f\n", (float)result7);
    
    /* Test 8: Shift operations */
    accum_t result8 = shift_operations(1.0k, shift_array, 
                                      sizeof(shift_array)/sizeof(shift_array[0]));
    printf("Test 8 - Shift operations: %f\n", (float)result8);
    
    /* Complex final calculation using all results */
    accum_t final_checksum = (accum_t)result1 + (accum_t)result2 + 
                            (accum_t)result3 + result4 + 
                            (accum_t)overflow_result + (accum_t)switch_result +
                            result7 + result8;
    
    /* Use asm to prevent optimization */
    asm volatile ("" : "+r" (final_checksum));
    
    printf("Final checksum: %f\n", (float)final_checksum);
    
    /* Additional edge case tests */
    
    /* Test near saturation boundaries */
    sfract_t near_max = 0.999999r;
    sfract_t small_add = 0.000001r;
    sfract_t saturated = near_max + small_add;
    printf("Saturation test: %f + %f = %f\n", 
           (float)near_max, (float)small_add, (float)saturated);
    
    /* Test underflow */
    sfract_t near_min = -0.999999r;
    sfract_t small_sub = 0.000001r;
    sfract_t underflow = near_min - small_sub;
    printf("Underflow test: %f - %f = %f\n", 
           (float)near_min, (float)small_sub, (float)underflow);
    
    /* Mixed width operations */
    hfract_t hf = 0.5r;
    fract_t f = 0.3r;
    fract_t mixed_result = hf + f;
    printf("Mixed width: %f + %f = %f\n", 
           (float)hf, (float)f, (float)mixed_result);
    
    return 0;
}
