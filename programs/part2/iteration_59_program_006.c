/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8p-7lr;

/* Function with fixed-point operations that may overflow */
short _Fract process_fixed(short _Fract a, short _Fract b, int shift) {
    /* These operations should trigger fixed-value analysis */
    short _Fract temp;
    
    /* Multiplication with potential overflow */
    temp = a * b;
    
    /* Left shift - FIXED_LSHIFT_EXPR */
    temp = temp << shift;
    
    /* Another multiplication with different types */
    _Accum acc = 0.5k;
    acc = acc * (_Accum)temp;
    
    /* Shift the accum */
    acc = acc << 1;
    
    /* Convert back with potential overflow */
    short _Fract result = (short _Fract)acc;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Function with saturation-prone operations */
_Accum accumulate_with_shifts(_Accum base, int iterations) {
    _Accum result = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Various shifts that might overflow */
        result = result << 1;
        result = result * 0.9k;
        result = result << 2;
        
        /* Memory barrier between operations */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Main function with loops and array operations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 10) iterations = 10;
    
    /* Array of fixed-point values */
    short _Fract sf_array[10];
    _Accum acc_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        sf_array[i] = (short _Fract)(i * 0.1r);
        acc_array[i] = (_Accum)(i * 0.05k);
    }
    
    /* Volatile read to prevent optimization */
    long _Fract lf_val = volatile_source;
    
    /* Perform operations that should trigger the uncovered code */
    _Accum total = 0k;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix different fixed-point types */
        short _Fract a = sf_array[i];
        short _Fract b = (short _Fract)lf_val;
        
        /* This call should trigger fixed-value arithmetic */
        short _Fract c = process_fixed(a, b, i % 4);
        
        /* Convert to narrower type with potential overflow */
        sf_array[i] = c;
        
        /* Accumulate with shifts */
        _Accum shifted = accumulate_with_shifts(acc_array[i], i + 1);
        
        /* Multiplication that might overflow */
        total = total + (shifted * (_Accum)c);
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Additional overflow-prone operations */
    unsigned short _Fract usf1 = 0.9ur;
    unsigned short _Fract usf2 = 0.95ur;
    
    /* This multiplication might saturate */
    unsigned short _Fract usf_result = usf1 * usf2;
    
    /* Left shift on unsigned fract */
    usf_result = usf_result << 1;
    
    /* More complex expression */
    long _Accum la1 = 0.7lk;
    long _Accum la2 = 0.8lk;
    
    for (int i = 0; i < 3; i++) {
        la1 = la1 * la2;
        la1 = la1 << 2;  /* Multiple shifts */
        
        /* Assignment to narrower type */
        _Accum narrowed = (_Accum)la1;
        total = total + narrowed;
    }
    
    /* Final operation that might trigger saturation check */
    short _Fract final_result;
    if (total > 0.5k) {
        /* This conversion might need saturation */
        final_result = (short _Fract)(total * 0.9k);
    } else {
        final_result = (short _Fract)(total * 1.1k);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %hd (as hex: 0x%04hx)\n", 
           (short)final_result, (unsigned short)final_result);
    
    return 0;
}
