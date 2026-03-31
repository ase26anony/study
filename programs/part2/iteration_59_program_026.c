/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8p-3lr;  /* 0.0625 */

/* Function with fixed-point operations that should trigger range checking */
static short _Fract process_fixed_point(short _Fract a, short _Fract b, int shift) {
    /* Multiplication that may overflow */
    short _Fract mul_result = a * b;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    _Accum shifted = (_Accum)mul_result << shift;
    
    /* Another multiplication with wider intermediate */
    long _Fract wide_mul = (long _Fract)a * (long _Fract)b;
    
    /* Convert to narrower type, potentially triggering saturation check */
    short _Fract narrow_result = (short _Fract)wide_mul;
    
    /* Mix operations to create complex expression */
    _Accum complex_result = (_Accum)narrow_result * (_Accum)shifted;
    
    /* Final conversion that may need bounds checking */
    return (short _Fract)complex_result;
}

/* Another function with different fixed-point types */
static _Accum accumulate_fixed(_Accum base, unsigned short _Fract multiplier, int iterations) {
    _Accum result = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Operation that could overflow */
        result = result * (_Accum)multiplier;
        
        /* Left shift on fixed-point */
        result = result << 1;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Mix with integer promotion */
        result = result * (i + 1);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop count non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 5 + 1 : 3;
    
    /* Initialize arrays of fixed-point values */
    short _Fract sf_array[5] = {
        0.5hr, 0.8hr, -0.3hr, 0.9hr, -0.7hr
    };
    
    _Accum accum_array[5] = {
        0.5k, 1.2k, -0.8k, 2.5k, -1.5k
    };
    
    /* Read from volatile to prevent compile-time evaluation */
    long _Fract lf_base = volatile_source;
    
    /* Process fixed-point operations in loop */
    short _Fract total = 0.0hr;
    
    for (int i = 0; i < iterations; i++) {
        /* Operations that should trigger fixed-value analysis */
        
        /* 1. Multiplication with potential overflow */
        short _Fract a = sf_array[i];
        short _Fract b = sf_array[(i + 1) % 5];
        short _Fract mul_result = a * b;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* 2. Left shift operation */
        _Accum shifted = (_Accum)mul_result << (i + 1);
        
        /* 3. Conversion to narrower type */
        short _Fract converted = (short _Fract)shifted;
        
        /* 4. Complex expression with mixed types */
        _Accum temp = (_Accum)converted * accum_array[i];
        
        /* 5. Another left shift */
        temp = temp << 2;
        
        /* 6. Final conversion that may need saturation check */
        total += (short _Fract)temp;
        
        /* Call function with fixed-point operations */
        total += process_fixed_point(a, b, i);
    }
    
    /* More operations with different fixed-point types */
    unsigned short _Fract usf = 0.9uhr;
    _Accum accum_result = accumulate_fixed(1.0k, usf, iterations);
    
    /* Operations that specifically target the uncovered code path */
    /* These values are chosen to potentially exceed bounds */
    short _Fract large_val = 0.99hr;
    short _Fract another_large = 0.98hr;
    
    /* Multiplication that could overflow short _Fract range */
    long _Fract wide_result = (long _Fract)large_val * (long _Fract)another_large;
    
    /* Conversion that should trigger bounds checking */
    short _Fract potentially_overflowing = (short _Fract)wide_result;
    
    /* Left shift on the result */
    _Accum shifted_overflow = (_Accum)potentially_overflowing << 3;
    
    /* Final conversion */
    short _Fract final_result = (short _Fract)shifted_overflow;
    
    /* Use results to prevent dead code elimination */
    total += final_result;
    
    /* Print result to ensure side effects */
    printf("Result: %f\n", (double)total);
    printf("Accum result: %f\n", (double)accum_result);
    
    return 0;
}
