/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.5r;
volatile _Accum volatile_accum = 0.5k;
volatile long _Fract volatile_lf = 0.9lr;

/* Function with fixed-point parameters to force analysis */
_Accum process_fixed(short _Fract a, _Accum b, long _Fract c) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* Mixed-type multiplication */
    _Accum temp2 = temp1 << 3;      /* Left shift - FIXED_LSHIFT_EXPR */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Narrowing conversion with potential overflow */
    short _Fract narrow = c * b;    /* Could overflow short _Fract range */
    
    /* Another shift operation */
    _Accum temp3 = b << 2;
    
    /* Complex expression with multiple operations */
    _Accum result = (temp1 + temp2) * temp3;
    
    /* Assignment to narrower type forcing range check */
    short _Fract final_narrow = result;
    
    return result + (_Accum)final_narrow;
}

/* Array processing with loop-variant values */
void process_array(_Accum *arr, int size) {
    for (int i = 0; i < size; i++) {
        /* Loop-variant shift amounts */
        int shift = i % 8;
        
        /* Fixed-point multiplication with shifting */
        _Accum val = arr[i];
        arr[i] = val * (_Accum)(i + 1);
        
        /* Left shift operation - triggers FIXED_LSHIFT_EXPR */
        if (shift > 0) {
            arr[i] = arr[i] << shift;
        }
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
        
        /* Narrowing conversion in loop */
        short _Fract narrowed = arr[i];
        arr[i] += (_Accum)narrowed;
    }
}

/* Main function with various fixed-point operations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Initialize arrays of different fixed-point types */
    _Accum accum_array[10];
    short _Fract sf_array[10];
    long _Fract lf_array[10];
    
    /* Initialize with values that could cause overflow */
    for (int i = 0; i < 10; i++) {
        accum_array[i] = (_Accum)(0.8k + i * 0.1k);
        sf_array[i] = (short _Fract)(0.7r + i * 0.03r);
        lf_array[i] = (long _Fract)(0.9lr - i * 0.05lr);
    }
    
    /* Process arrays multiple times */
    for (int iter = 0; iter < iterations; iter++) {
        /* Call function with volatile operands */
        _Accum result1 = process_fixed(
            volatile_sf + (short _Fract)(iter * 0.01r),
            volatile_accum * (_Accum)(iter + 1),
            volatile_lf
        );
        
        /* Process array with shifting */
        process_array(accum_array, 10);
        
        /* Complex fixed-point expression */
        for (int i = 0; i < 10; i++) {
            /* Multiplication that could overflow */
            long _Fract wide_result = lf_array[i] * lf_array[(i + 1) % 10];
            
            /* Assignment to narrower type - forces range check */
            sf_array[i] = wide_result;
            
            /* Left shift on _Accum type */
            accum_array[i] = accum_array[i] << 1;
            
            /* Mixed-type operation with integer promotion */
            accum_array[i] = accum_array[i] * (iter + 1);
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Update volatile variables */
        volatile_sf += (short _Fract)0.01r;
        volatile_accum = volatile_accum * 1.1k;
    }
    
    /* Compute sum to prevent dead code elimination */
    _Accum total = 0k;
    for (int i = 0; i < 10; i++) {
        total += accum_array[i] + (_Accum)sf_array[i] + (_Accum)lf_array[i];
    }
    
    /* Print result to ensure side effects */
    printf("Result: %k\n", total);
    
    return 0;
}
