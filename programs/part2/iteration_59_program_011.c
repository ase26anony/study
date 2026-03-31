/* fixed-value-coverage.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-value-coverage.c -o fixed-value-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding with volatile variables */
volatile short _Fract v_sf1 = 0.5r;
volatile short _Fract v_sf2 = 0.8r;
volatile _Accum v_acc1 = 0.5k;
volatile _Accum v_acc2 = 0.9k;
volatile long _Fract v_lf1 = 0.95lr;

/* Function to perform fixed-point operations that should trigger
 * the uncovered bounds checking logic */
static _Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c, int shift) {
    /* Multiple operations that create wide intermediate results */
    _Accum temp1;
    _Accum temp2;
    short _Fract narrow_result;
    
    /* Operation 1: Multiplication with potential overflow */
    /* This should trigger fixed-value arithmetic with bounds checking */
    temp1 = (_Accum)a * b;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Operation 2: Left shift (FIXED_LSHIFT_EXPR) */
    /* This directly uses the shift logic in fixed-value.cc */
    temp2 = temp1 << shift;
    
    /* Operation 3: Multiplication with different types */
    /* Forces type conversions and wider intermediate calculations */
    temp1 = temp2 * (_Accum)c;
    
    /* Operation 4: Assignment to narrower type */
    /* This should trigger saturation/overflow checking against max_r/max_s */
    narrow_result = (short _Fract)temp1;
    
    /* Return as _Accum to avoid constant folding */
    return (_Accum)narrow_result + temp2;
}

/* Another function focusing on shift operations */
static _Accum test_shifts(_Accum base, int iterations) {
    _Accum result = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Various shift operations that should trigger the uncovered code */
        result = result << 1;
        asm volatile ("" : : : "memory");
        result = result << 2;
        asm volatile ("" : : : "memory");
        
        /* Mix with multiplication for complex bounds */
        result = result * 0.75k;
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Function using arrays to prevent optimization */
static _Accum process_array(short _Fract arr[], int size) {
    _Accum total = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Complex expression that should trigger bounds checking */
        _Accum temp = (_Accum)arr[i] * 1.5k;
        temp = temp << (i % 4);  /* Variable shift */
        
        /* Assignment that might overflow */
        short _Fract narrowed = (short _Fract)temp;
        
        total += (_Accum)narrowed;
        asm volatile ("" : : : "memory");
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    _Accum final_result = 0.0k;
    
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    printf("Running %d iterations of fixed-point operations\n", iterations);
    
    /* Array of fixed-point values */
    short _Fract sf_array[10];
    for (int i = 0; i < 10; i++) {
        sf_array[i] = (short _Fract)(i * 0.1r);
    }
    
    /* Test 1: Process array with potential overflow */
    final_result += process_array(sf_array, iterations + 3);
    
    /* Test 2: Individual operations with volatile operands */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile variables to prevent constant folding */
        short _Fract a = v_sf1 + (short _Fract)(i * 0.05r);
        _Accum b = v_acc1 * (_Accum)i;
        long _Fract c = v_lf1;
        
        /* Call function that performs complex fixed-point ops */
        _Accum partial = process_fixed_point(a, b, c, i % 3 + 1);
        
        /* More operations to ensure coverage */
        partial = test_shifts(partial, 2);
        
        final_result += partial;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Test 3: Direct operations that should trigger the specific condition */
    /* The condition: if (a_high.sgt (max_r) || (a_high == max_r && a_low.ugt (max_s))) */
    {
        /* Create a value that might approach the maximum bound */
        _Accum large_val = 0.999k;
        
        /* Repeated shifting to potentially exceed bounds */
        for (int i = 0; i < 5; i++) {
            large_val = large_val << 1;
            large_val = large_val * 0.9k;
        }
        
        /* Assignment to narrower type - should trigger bounds check */
        short _Fract test_narrow = (short _Fract)large_val;
        final_result += (_Accum)test_narrow;
    }
    
    /* Test 4: Edge case with values near limits */
    {
        /* Use values that are close to 1.0 to test overflow checking */
        short _Fract near_one = 0.99r;
        _Accum acc_near_one = 0.999k;
        
        /* Multiplication that could overflow */
        _Accum product = (_Accum)near_one * acc_near_one;
        product = product << 2;  /* Shift to potentially exceed bounds */
        
        /* This assignment should trigger the uncovered comparison */
        short _Fract narrowed = (short _Fract)product;
        final_result += (_Accum)narrowed;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %ld (as integer)\n", (long)(final_result * 1000));
    
    return (final_result > 0.0k) ? 0 : 1;
}
