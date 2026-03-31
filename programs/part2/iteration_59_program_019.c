/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract vf1 = 0.5r;
volatile short _Fract vf2 = 0.8r;
volatile _Accum va1 = 0.5k;
volatile _Accum va2 = 0.7k;
volatile long _Fract vlf1 = 0.9lr;

/* Function with fixed-point parameters to force analysis */
_Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* Mixed-type multiplication */
    _Accum temp2 = temp1 << 3;      /* Left shift - FIXED_LSHIFT_EXPR */
    
    /* Narrowing conversion with potential overflow */
    short _Fract narrow = c * a;    /* Could overflow short _Fract range */
    
    /* Another shift operation */
    _Accum temp3 = b << 2;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return temp2 + temp3 + narrow;
}

/* Another function focusing on saturation paths */
short _Fract saturating_multiply(long _Fract a, long _Fract b) {
    /* This multiplication could exceed short _Fract range */
    long _Fract product = a * b;
    
    /* Explicit cast to narrower type - may trigger saturation check */
    short _Fract result = product;
    
    asm volatile ("" : : : "memory");
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 10) iterations = 10;
    
    /* Array of fixed-point values */
    _Accum accum_array[10];
    short _Fract fract_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        accum_array[i] = i * 0.1k;
        fract_array[i] = i * 0.1r;
    }
    
    _Accum total = 0k;
    
    /* Loop with fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Read volatile variables to get unknown-at-compile-time values */
        short _Fract sf1 = vf1;
        short _Fract sf2 = vf2;
        _Accum acc1 = va1;
        _Accum acc2 = va2;
        long _Fract lf1 = vlf1;
        
        /* Operation 1: Multiplication with potential overflow */
        _Accum mult_result = sf1 * acc1;
        
        /* Operation 2: Left shift (FIXED_LSHIFT_EXPR) */
        _Accum shift_result = mult_result << (i % 4 + 1);
        
        /* Operation 3: Mixed operations */
        long _Fract mixed = lf1 * sf2;
        
        /* Operation 4: Narrowing conversion */
        short _Fract narrowed = mixed;  /* May trigger saturation check */
        
        /* Operation 5: Array-based computation */
        _Accum array_op = accum_array[i] * fract_array[i];
        
        /* Operation 6: Another shift */
        array_op = array_op << 2;
        
        /* Call functions that perform fixed-point operations */
        _Accum func_result = process_fixed_point(sf1, acc1, lf1);
        short _Fract sat_result = saturating_multiply(lf1, lf1);
        
        /* Combine results */
        total += shift_result + array_op + func_result + sat_result + narrowed;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
        
        /* Modify array values for next iteration */
        accum_array[i] = accum_array[i] * 0.95k;
        fract_array[i] = fract_array[i] * 0.95r;
    }
    
    /* Use the result to prevent dead code elimination */
    if (total > 10k) {
        printf("Result: %k\n", total);
    } else {
        printf("Result: %k\n", total);
    }
    
    return 0;
}
