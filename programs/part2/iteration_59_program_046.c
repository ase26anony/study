/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract v_sf = 0.5r;
volatile _Accum v_acc = 0.5k;
volatile long _Fract v_lf = 0.7lr;

/* Function with fixed-point operations that should trigger range checking */
static _Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* FIXED_MULT_P with different types */
    _Accum temp2 = temp1 << 3;      /* FIXED_LSHIFT_EXPR - triggers shift logic */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* More operations with potential overflow */
    long _Fract temp3 = c * c;      /* Multiplication with same type */
    _Accum temp4 = temp2 + (_Accum)temp3;
    
    /* Another shift operation */
    temp4 = temp4 << 2;
    
    /* Convert to narrower type - may trigger saturation check */
    short _Fract result_sf = (short _Fract)temp4;
    
    /* Return as _Accum */
    return (_Accum)result_sf;
}

/* Another function focusing on shift operations */
static void fixed_shift_operations(int iterations) {
    _Accum acc_array[10];
    short _Fract sf_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        acc_array[i] = (_Accum)(i * 0.1k);
        sf_array[i] = (short _Fract)(i * 0.05r);
    }
    
    /* Perform operations in loop */
    for (int i = 0; i < iterations && i < 10; i++) {
        /* Left shift operations that should trigger the uncovered code */
        _Accum shifted = acc_array[i] << (i + 1);
        
        /* Multiplication that might overflow */
        short _Fract multiplied = sf_array[i] * sf_array[(i + 1) % 10];
        
        /* Convert with potential overflow */
        _Accum converted = (_Accum)multiplied;
        
        /* Combine results */
        acc_array[i] = shifted + converted;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
}

/* Function that mixes fixed-point with integer promotions */
static long _Fract mixed_operations(unsigned int n) {
    /* Start with a fixed-point value */
    long _Fract base = 0.8lr;
    
    /* Multiply by integer - causes promotion */
    long _Fract result = base * (long _Fract)n;
    
    /* Shift operation */
    result = result << 2;
    
    /* Another multiplication that could overflow */
    result = result * 0.95lr;
    
    /* Convert to narrower type - may trigger bounds checking */
    short _Fract narrowed = (short _Fract)result;
    
    /* Return widened again */
    return (long _Fract)narrowed;
}

int main(int argc, char *argv[]) {
    int iterations = argc > 1 ? atoi(argv[1]) % 10 : 5;
    if (iterations <= 0) iterations = 3;
    
    /* Use volatile variables as sources */
    short _Fract sf1 = v_sf;
    _Accum acc1 = v_acc;
    long _Fract lf1 = v_lf;
    
    /* Call functions with fixed-point operations */
    _Accum result1 = process_fixed_point(sf1, acc1, lf1);
    
    /* Perform more operations in main */
    fixed_shift_operations(iterations);
    
    /* Mixed operations with loop-variant values */
    long _Fract total = 0.0lr;
    for (int i = 0; i < iterations; i++) {
        total += mixed_operations(i + 1);
        asm volatile ("" : : : "memory");
    }
    
    /* Create a scenario with potential overflow */
    short _Fract narrow_array[5];
    for (int i = 0; i < 5; i++) {
        /* Operations that could exceed short _Fract range */
        long _Fract wide_val = 0.9lr * (i + 1) * 0.5lr;
        
        /* Left shift - triggers FIXED_LSHIFT_EXPR */
        wide_val = wide_val << 1;
        
        /* Convert to narrower type - may trigger saturation logic */
        narrow_array[i] = (short _Fract)wide_val;
    }
    
    /* Compute a final result to prevent dead code elimination */
    _Accum final_result = result1 + (_Accum)total + (_Accum)narrow_array[iterations % 5];
    
    /* Print to ensure side effects */
    printf("Result: %ld\n", (long)(final_result * 1000k));
    
    return 0;
}
