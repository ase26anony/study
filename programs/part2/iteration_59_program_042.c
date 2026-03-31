/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract vf1 = 0.5r;
volatile short _Fract vf2 = 0.8r;
volatile _Accum va1 = 0.5k;
volatile _Accum va2 = 1.2k;
volatile long _Fract vlf1 = 0.9lr;
volatile long _Fract vlf2 = 0.95lr;

/* Function with fixed-point parameters to force range checking */
short _Fract process_fract(short _Fract a, short _Fract b, int shift) {
    /* Multiple operations that could overflow */
    short _Fract temp;
    
    /* Multiplication with potential overflow */
    temp = a * b;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    _Accum acc_temp = (_Accum)temp;
    acc_temp = acc_temp << shift;
    
    /* Another multiplication with different types */
    temp = temp * (_Fract)acc_temp;
    
    /* Assign to narrower type to force saturation check */
    short _Fract result = temp;
    
    return result;
}

/* Function using _Accum with shifting */
_Accum process_accum(_Accum a, _Accum b, int shift) {
    _Accum temp;
    
    /* Multiplication that might need range checking */
    temp = a * b;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Left shift - this directly triggers FIXED_LSHIFT_EXPR */
    temp = temp << shift;
    
    /* Another operation with integer promotion */
    temp = temp * 3;
    
    return temp;
}

/* Function that mixes different fixed-point types */
long _Fract mixed_operations(long _Fract a, short _Fract b, int iterations) {
    long _Fract result = 0.0lr;
    _Accum accum_temp;
    
    for (int i = 0; i < iterations; i++) {
        /* Operation that might overflow */
        accum_temp = (_Accum)a * (_Accum)b;
        
        /* Left shift with variable amount */
        accum_temp = accum_temp << (i % 4);
        
        /* Convert back with potential overflow */
        result = result + (long _Fract)accum_temp;
        
        /* Modify values to prevent complete optimization */
        a = a * 0.9lr;
        b = b * 0.8r;
        
        /* Memory barrier to keep operations separate */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 10) iterations = 10;
    
    /* Array of fixed-point values */
    short _Fract sf_array[10];
    _Accum accum_array[10];
    long _Fract lf_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        sf_array[i] = (short _Fract)(i * 0.1r);
        accum_array[i] = (_Accum)(i * 0.2k);
        lf_array[i] = (long _Fract)(i * 0.05lr);
    }
    
    short _Fract result_sf = 0.0r;
    _Accum result_accum = 0.0k;
    long _Fract result_lf = 0.0lr;
    
    /* Perform various fixed-point operations in loops */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 10;
        
        /* Operation 1: Multiplication with volatile operands */
        short _Fract temp1 = vf1 * vf2;
        
        /* Operation 2: Left shift on _Accum */
        _Accum temp2 = va1 << (i % 3 + 1);
        
        /* Operation 3: Process with function call */
        result_sf = process_fract(sf_array[idx], temp1, i % 4);
        
        /* Operation 4: _Accum processing */
        result_accum = process_accum(accum_array[idx], temp2, i % 3);
        
        /* Operation 5: Mixed operations with potential overflow */
        result_lf = mixed_operations(lf_array[idx], sf_array[idx], 3);
        
        /* Operation 6: Narrower assignment that might trigger saturation */
        short _Fract narrow_result = (short _Fract)result_lf;
        
        /* Operation 7: Another shift operation */
        _Accum shifted = result_accum << 2;
        
        /* Operation 8: Multiplication with integer promotion */
        shifted = shifted * (i + 1);
        
        /* Update array values to create data dependencies */
        sf_array[idx] = sf_array[idx] * 0.9r;
        accum_array[idx] = accum_array[idx] * 0.8k;
        lf_array[idx] = lf_array[idx] * 0.95lr;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    volatile short _Fract dummy1 = result_sf;
    volatile _Accum dummy2 = result_accum;
    volatile long _Fract dummy3 = result_lf;
    
    /* Print something to ensure code runs */
    printf("Results: %hd %hd %hd\n", 
           (short)(dummy1 * 1000r),
           (short)(dummy2 * 100k),
           (short)(dummy3 * 1000lr));
    
    return 0;
}
