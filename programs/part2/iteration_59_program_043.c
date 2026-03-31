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

/* Function with fixed-point operations that may overflow */
short _Fract process_fract(short _Fract a, short _Fract b, int shift) {
    /* Multiple operations to create complex intermediate values */
    short _Fract temp;
    
    /* Multiplication that might overflow intermediate representation */
    temp = a * b;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift on fixed-point (FIXED_LSHIFT_EXPR) */
    /* This directly triggers the shift logic in fixed-value.cc */
    if (shift > 0) {
        temp = temp << shift;
    }
    
    /* Another multiplication */
    temp = temp * a;
    
    asm volatile ("" : : : "memory");
    
    return temp;
}

/* Function using _Accum types with shifting */
_Accum process_accum(_Accum a, _Accum b, int shift) {
    _Accum result;
    
    /* Multiplication with potential for wide intermediate */
    result = a * b;
    
    asm volatile ("" : : : "memory");
    
    /* Left shift - this should trigger the uncovered code path */
    if (shift > 0) {
        result = result << shift;
    }
    
    /* Additional operation to prevent simplification */
    result = result + (a / 4k);
    
    return result;
}

/* Function that forces narrowing conversion with potential overflow */
short _Fract narrow_conversion(long _Fract a, long _Fract b) {
    /* Multiplication that could exceed short _Fract range */
    long _Fract wide_result = a * b;
    
    asm volatile ("" : : : "memory");
    
    /* Narrowing conversion - may trigger saturation checking */
    short _Fract narrow_result = (short _Fract)wide_result;
    
    return narrow_result;
}

/* Main function with loops and array operations */
int main(int argc, char *argv[]) {
    int i, iterations;
    
    /* Use argc to make loop count non-constant */
    iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 100) iterations = 100;
    
    /* Array of fixed-point values */
    short _Fract sf_array[10];
    _Accum accum_array[10];
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 10; i++) {
        sf_array[i] = (short _Fract)(i * 0.1r);
        accum_array[i] = (_Accum)(i * 0.2k);
    }
    
    short _Fract sf_result = 0.0r;
    _Accum accum_result = 0.0k;
    
    /* Loop with fixed-point operations */
    for (i = 0; i < iterations; i++) {
        int idx = i % 10;
        int shift = (i % 3) + 1;  /* Shift values 1-3 */
        
        /* Mix operations with volatile and non-volatile values */
        short _Fract temp1 = process_fract(sf_array[idx], vf1, shift);
        short _Fract temp2 = process_fract(vf2, sf_array[(idx + 1) % 10], shift);
        
        asm volatile ("" : : : "memory");
        
        /* Accumulate results */
        sf_result = sf_result + temp1 * temp2;
        
        /* Process _Accum types */
        _Accum atemp1 = process_accum(accum_array[idx], va1, shift);
        _Accum atemp2 = process_accum(va2, accum_array[(idx + 5) % 10], shift + 1);
        
        accum_result = accum_result + atemp1 * atemp2;
        
        /* Force narrowing conversion that may overflow */
        if (i % 2 == 0) {
            short _Fract narrow = narrow_conversion(vlf1, vlf2);
            sf_result = sf_result + narrow;
        }
        
        /* Modify array values to create loop-variant behavior */
        sf_array[idx] = sf_array[idx] * 0.9r;
        accum_array[idx] = accum_array[idx] * 1.1k;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: sf_result = %f, accum_result = %f\n", 
           (double)sf_result, (double)accum_result);
    
    return 0;
}
