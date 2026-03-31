/* fixed-value-coverage.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-value-coverage.c -o fixed-value-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding with volatile reads */
volatile short _Fract v_sf = 0.7r;
volatile _Accum v_acc = 0.5k;
volatile long _Fract v_lf = 0.9lr;

/* Function with fixed-point operations that should trigger range checking */
static _Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c) {
    /* Multiple operations to create complex intermediate results */
    _Accum temp1 = a * b;           /* FIXED_MULT_P with different types */
    _Accum temp2 = temp1 << 3;      /* FIXED_LSHIFT_EXPR - should trigger shift logic */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* More operations with potential overflow */
    long _Fract temp3 = c * c;      /* Square - could overflow */
    _Accum temp4 = temp2 + (_Accum)temp3;
    
    /* Another shift with different amount */
    temp4 = temp4 << 2;
    
    /* Force saturation check by assigning to narrower type */
    short _Fract narrow_result = (short _Fract)temp4;
    
    return (_Accum)narrow_result + temp2;
}

/* Another function focusing on multiplication overflow cases */
static short _Fract test_multiplication_overflow(short _Fract a, short _Fract b) {
    /* This multiplication might overflow short _Fract range */
    short _Fract result = a * b;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Chain operations */
    result = result * result;
    result = result << 1;  /* Left shift on fract */
    
    return result;
}

/* Test array operations with loops */
static void array_fixed_point_operations(int iterations) {
    _Accum arr_acc[10];
    short _Fract arr_sf[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        arr_acc[i] = (_Accum)(i * 0.1k);
        arr_sf[i] = (short _Fract)((i % 5) * 0.2r);
    }
    
    /* Perform operations in loop with loop-variant values */
    _Accum sum = 0k;
    for (int i = 0; i < iterations && i < 10; i++) {
        /* Mix operations that could overflow */
        _Accum temp = arr_acc[i] * (_Accum)arr_sf[i];
        
        /* Left shift - should trigger FIXED_LSHIFT_EXPR logic */
        temp = temp << (i % 4 + 1);
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" : : : "memory");
        
        /* Potential overflow when converting to narrower type */
        short _Fract narrow = (short _Fract)temp;
        
        sum += (_Accum)narrow;
        
        /* Modify array values to create data dependencies */
        arr_acc[(i + 1) % 10] = temp * 0.5k;
    }
    
    /* Use the result to prevent dead code elimination */
    volatile _Accum sink = sum;
    (void)sink;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    printf("Running fixed-point coverage test with %d iterations\n", iterations);
    
    /* Read volatile values to prevent compile-time computation */
    short _Fract sf1 = v_sf;
    _Accum acc1 = v_acc;
    long _Fract lf1 = v_lf;
    
    /* Test case 1: Complex fixed-point processing */
    _Accum result1 = process_fixed_point(sf1, acc1, lf1);
    
    /* Test case 2: Multiplication overflow focus */
    short _Fract sf2 = (short _Fract)(0.9r);
    short _Fract result2 = test_multiplication_overflow(sf1, sf2);
    
    /* Test case 3: Array operations with loops */
    array_fixed_point_operations(iterations);
    
    /* Test case 4: Direct operations that should trigger the uncovered code */
    /* These operations are designed to create the specific conditions for
       the max_r/max_s and min_r/min_s initialization and comparison */
    
    /* Create a value that might exceed bounds when shifted */
    _Accum large_val = 0.999k;  /* Close to maximum */
    
    /* Multiple shifts to create wide intermediate results */
    for (int i = 0; i < iterations; i++) {
        /* Shift operation that should trigger the uncovered logic */
        large_val = large_val << 1;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Multiplication that could overflow */
        large_val = large_val * (_Accum)0.8k;
        
        /* Try to assign to narrower type to force range check */
        short _Fract test_narrow = (short _Fract)large_val;
        
        /* Use result */
        result1 += (_Accum)test_narrow;
    }
    
    /* Test case 5: Mixed-width operations */
    /* These are likely to trigger the specific comparison logic */
    unsigned short _Fract usf1 = 0.8ur;
    signed _Accum sacc1 = 0.7k;
    
    /* Operation that creates need for bounds checking */
    signed _Accum mixed_result = (_Accum)usf1 * sacc1;
    mixed_result = mixed_result << 4;  /* Significant left shift */
    
    /* Force potential overflow situation */
    short _Fract final_narrow;
    if (iterations % 2) {
        final_narrow = (short _Fract)mixed_result;
    } else {
        final_narrow = (short _Fract)(mixed_result * 2k);
    }
    
    /* Use results to prevent optimization */
    volatile _Accum final_sink = result1 + (_Accum)result2 + (_Accum)final_narrow;
    (void)final_sink;
    
    printf("Test completed (compile with -fsat-conversion for coverage)\n");
    
    return 0;
}
