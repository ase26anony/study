/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x7FFFFFFFFFFFFFFFlr; /* Near max */

/* Function with fixed-point parameters that performs overflow-prone operations */
static short _Fract process_fixed(short _Fract a, short _Fract b, int shift) {
    /* Multiplication that may overflow short _Fract range */
    short _Fract prod = a * b;
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    _Accum shifted = (_Accum)prod << shift;
    
    /* Force intermediate wide calculation */
    long _Fract wide_calc = (long _Fract)a * (long _Fract)b;
    wide_calc = wide_calc << (shift + 1);
    
    /* Assignment to narrower type with potential overflow */
    short _Fract result = (short _Fract)shifted + (short _Fract)(wide_calc >> 8);
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Another function mixing _Accum types with shifts */
static _Accum accumulate_with_shifts(_Accum base, int iterations) {
    _Accum result = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple left shifts that could overflow */
        result = result << 1;
        
        /* Multiplication with potential for wide intermediate */
        result = result * 0.9k;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Process array of fixed-point values */
static void process_array(short _Fract *arr, int size) {
    _Accum accumulator = 0.5k;
    
    for (int i = 0; i < size; i++) {
        /* Operations that may trigger saturation checks */
        short _Fract val = arr[i];
        
        /* Multiplication that could exceed range */
        _Accum temp = (_Accum)val * 1.5k;
        
        /* Left shift */
        temp = temp << 2;
        
        /* Assignment back to narrower type */
        arr[i] = (short _Fract)temp;
        
        /* Accumulate with potential overflow */
        accumulator = accumulator + temp;
        
        /* Prevent optimization */
        if (i % 3 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Final operation that might trigger bounds checking */
    short _Fract final = (short _Fract)accumulator;
    asm volatile ("" : : "r"(final) : "memory");
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations > 10) iterations = 10;
    if (iterations < 2) iterations = 2;
    
    /* Initialize array with fixed-point values */
    short _Fract arr[10];
    for (int i = 0; i < 10; i++) {
        /* Values that may cause overflow when multiplied */
        arr[i] = (short _Fract)(0.7r + i * 0.05r);
    }
    
    /* Process array - this should trigger fixed-value analysis */
    process_array(arr, iterations);
    
    /* Test multiplication with volatile source */
    short _Fract a = 0.8r;
    short _Fract b = (short _Fract)(volatile_source >> 32); /* Extract portion */
    
    /* This multiplication may overflow */
    short _Fract c = process_fixed(a, b, 3);
    
    /* Test _Accum operations with shifts */
    _Accum base_val = 0.25k;
    _Accum acc_result = accumulate_with_shifts(base_val, iterations);
    
    /* Mix types and operations */
    unsigned short _Fract ua = 0.9ur;
    unsigned short _Fract ub = 0.95ur;
    
    /* Unsigned multiplication with shift */
    unsigned _Accum uacc = (unsigned _Accum)ua * (unsigned _Accum)ub;
    uacc = uacc << 1;
    
    /* Convert to signed with potential overflow */
    short _Fract mixed = (short _Fract)uacc + c;
    
    /* More complex expression with multiple operations */
    long _Fract complex_calc = 0.0lr;
    for (int i = 0; i < iterations; i++) {
        long _Fract term = (long _Fract)arr[i % 10] * (long _Fract)acc_result;
        term = term << (i % 4);
        complex_calc = complex_calc + term;
        
        /* Memory barrier every iteration */
        asm volatile ("" : : : "memory");
    }
    
    /* Final assignment to narrower type - may trigger saturation check */
    _Accum final_result = (_Accum)complex_calc + (_Accum)mixed;
    
    /* Print something to prevent dead code elimination */
    printf("Result: %f\n", (double)final_result);
    
    /* Also print array values */
    for (int i = 0; i < 10; i++) {
        printf("%d: %f ", i, (double)arr[i]);
    }
    printf("\n");
    
    return 0;
}
