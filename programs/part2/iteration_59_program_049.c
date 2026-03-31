/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract vf1 = 0.5r;
volatile short _Fract vf2 = 0.8r;
volatile _Accum va1 = 0.5k;
volatile _Accum va2 = 0.7k;
volatile int vi1 = 2;

/* Function with fixed-point operations that may overflow */
static long _Fract process_fixed(short _Fract a, short _Fract b, _Accum c, int shift) {
    /* Multiple operations that could overflow */
    short _Fract t1 = a * b;           /* FIXED_MULT_P */
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    _Accum t2 = c << shift;            /* FIXED_LSHIFT_EXPR */
    
    /* Another multiplication with potential overflow */
    long _Fract t3 = (long _Fract)a * (long _Fract)b;
    
    asm volatile ("" : : : "memory");
    
    /* Mixed-type operation causing promotion */
    _Accum t4 = c * shift;             /* Integer promotion */
    
    /* Convert to narrower type - may trigger saturation check */
    short _Fract result = (short _Fract)t3;
    
    /* Return something to prevent dead code elimination */
    return (long _Fract)t1 + (long _Fract)t2 + t3 + (long _Fract)t4;
}

/* Another function focusing on shift operations */
static _Accum shift_operations(_Accum base, int iterations) {
    _Accum result = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Left shift that could overflow */
        result = result << 1;          /* FIXED_LSHIFT_EXPR */
        
        /* Multiplication that could overflow */
        result = result * 1.5k;
        
        asm volatile ("" : : : "memory");
        
        /* Check for potential overflow to narrower type */
        if (i % 2 == 0) {
            short _Fract narrowed = (short _Fract)result;
            /* Use the result to prevent elimination */
            result = result * 0.9k + (_Accum)narrowed;
        }
    }
    
    return result;
}

/* Process array of fixed-point values */
static void process_array(short _Fract *arr, int size) {
    long _Fract accumulator = 0.0lr;
    
    for (int i = 0; i < size; i++) {
        /* Operations that might overflow */
        short _Fract val = arr[i];
        
        /* Multiplication with potential overflow */
        short _Fract prod = val * val;
        
        asm volatile ("" : : : "memory");
        
        /* Left shift */
        _Accum shifted = (_Accum)val << (i % 4);
        
        /* Mixed operation */
        long _Fract mixed = (long _Fract)val * i;
        
        /* Assignment to narrower type - may trigger bounds check */
        if (i % 3 == 0) {
            short _Fract narrowed = (short _Fract)mixed;
            accumulator += (long _Fract)narrowed;
        }
        
        accumulator += (long _Fract)prod + (long _Fract)shifted + mixed;
    }
    
    /* Use accumulator to prevent dead code elimination */
    volatile long _Fract sink = accumulator;
    (void)sink;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Initialize array with pattern */
    short _Fract arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = (short _Fract)(i * 0.1r);
    }
    
    /* Call functions with volatile operands to prevent constant folding */
    long _Fract result1 = process_fixed(vf1, vf2, va1, vi1);
    
    asm volatile ("" : : : "memory");
    
    _Accum result2 = shift_operations(va2, iterations);
    
    asm volatile ("" : : : "memory");
    
    process_array(arr, iterations + 3);
    
    /* More complex expressions that could trigger overflow checking */
    for (int i = 0; i < iterations; i++) {
        /* Operations that create wide intermediate results */
        long _Fract a = 0.9lr;
        long _Fract b = 0.95lr;
        
        /* This multiplication could overflow when assigned to short _Fract */
        short _Fract c = (short _Fract)(a * b);
        
        /* Left shift on _Accum */
        _Accum d = 0.5k;
        d = d << (i + 1);              /* FIXED_LSHIFT_EXPR */
        
        /* Mixed-type operation */
        _Accum e = d * argc;           /* Integer promotion */
        
        /* Assignment to narrower type */
        short _Fract f = (short _Fract)e;
        
        /* Use results to prevent elimination */
        result1 += (long _Fract)c + (long _Fract)f;
        
        asm volatile ("" : : : "memory");
    }
    
    /* Print something to ensure code isn't optimized away */
    printf("Result: %ld\n", (long)(result1 * 1000lr));
    
    return 0;
}
