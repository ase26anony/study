/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8p-7lr;

/* Function with fixed-point operations that should trigger range checking */
static short _Fract process_fixed_point(short _Fract a, short _Fract b, _Accum shift_val) {
    /* Multiplication that may overflow when assigned to narrower type */
    short _Fract mult_result;
    
    /* Force intermediate calculation with wider precision */
    mult_result = a * b;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    _Accum shifted = shift_val << 3;
    
    /* Another multiplication with potential overflow */
    _Accum wide_mult = shifted * 2.5k;
    
    /* Convert to narrower type, potentially triggering saturation check */
    short _Fract narrow_result = (_Fract)wide_mult;
    
    /* Combine results */
    return mult_result + narrow_result;
}

/* Another function with different fixed-point types */
static _Accum accumulate_fixed(_Accum base, unsigned short _Fract factor, int iterations) {
    _Accum result = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed-type operations that require range checking */
        result = result * 1.5k;
        
        /* Left shift with fixed-point */
        result = result << 1;
        
        /* Multiplication with narrower type */
        result = result * (_Accum)factor;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Function that uses array operations */
static void array_fixed_ops(short _Fract arr[], int size) {
    _Accum accumulator = 0.5k;
    
    for (int i = 0; i < size; i++) {
        /* Operation that may exceed bounds */
        _Accum temp = accumulator * (_Accum)arr[i];
        
        /* Left shift */
        temp = temp << 2;
        
        /* Assign to array element with potential overflow */
        arr[i] = (short _Fract)temp;
        
        /* Update accumulator with shift */
        accumulator = accumulator << 1;
        
        /* Prevent optimization */
        asm volatile ("" : : : "memory");
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Initialize various fixed-point arrays */
    short _Fract sf_arr[10];
    _Accum accum_arr[10];
    long _Fract lf_arr[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        sf_arr[i] = (short _Fract)(i * 0.1r);
        accum_arr[i] = (_Accum)(i * 0.2k);
        lf_arr[i] = (long _Fract)(i * 0.05lr);
    }
    
    /* Use volatile source to prevent compile-time computation */
    short _Fract vol_val = (short _Fract)volatile_source;
    
    _Accum total_result = 0.0k;
    
    /* Perform fixed-point operations in loop */
    for (int i = 0; i < iterations; i++) {
        /* Operation 1: Multiplication with potential overflow */
        short _Fract a = sf_arr[i % 10];
        short _Fract b = (short _Fract)(0.9r - (i * 0.05r));
        short _Fract mult_result = a * b;
        
        /* Operation 2: Left shift of _Accum */
        _Accum shift_base = accum_arr[i % 10];
        _Accum shifted = shift_base << (i % 4 + 1);
        
        /* Operation 3: Mixed-type operation */
        long _Fract wide_val = lf_arr[i % 10] * 3.0lr;
        
        /* Convert to narrower type (potential overflow) */
        short _Fract narrow_val = (short _Fract)wide_val;
        
        /* Combine with volatile value */
        short _Fract combined = mult_result + narrow_val + vol_val;
        
        /* Process through function */
        short _Fract processed = process_fixed_point(combined, b, shifted);
        
        /* Accumulate results */
        total_result += (_Accum)processed;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* More complex accumulation */
    _Accum acc_result = accumulate_fixed(total_result, 0.8r, iterations);
    
    /* Array operations */
    array_fixed_ops(sf_arr, iterations);
    
    /* Final computation that mixes everything */
    _Accum final_result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        /* Multiplication that could overflow intermediate */
        _Accum temp = accum_arr[i] * 2.0k;
        
        /* Left shift */
        temp = temp << 3;
        
        /* Multiply by array element */
        temp = temp * (_Accum)sf_arr[i];
        
        final_result += temp;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Make result observable */
    printf("Result: %f\n", (double)final_result);
    
    return 0;
}
