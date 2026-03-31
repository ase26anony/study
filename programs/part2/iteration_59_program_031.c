/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8p-7lr;  /* 0.0078125 in hex */

/* Function with fixed-point parameters to force range analysis */
static short _Fract process_fract(short _Fract a, short _Fract b, int shift) {
    /* Intermediate multiplication that may overflow */
    long _Fract temp = (long _Fract)a * (long _Fract)b;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift - triggers FIXED_LSHIFT_EXPR analysis */
    temp = temp << shift;
    
    /* Convert to narrower type - may trigger saturation check */
    short _Fract result = (short _Fract)temp;
    
    return result;
}

/* Another function using _Accum types */
static _Accum process_accum(_Accum a, _Accum b, int shift) {
    /* Multiplication with potential overflow */
    long _Accum temp = (long _Accum)a * (long _Accum)b;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation */
    temp = temp << shift;
    
    /* Narrowing conversion */
    _Accum result = (_Accum)temp;
    
    return result;
}

/* Mix fixed-point with integer promotions */
static long _Fract mixed_operations(short _Fract f, int i, long l) {
    /* These operations cause integer promotion */
    long _Fract r1 = f * i;      /* Fixed * int */
    long _Fract r2 = f * l;      /* Fixed * long */
    long _Fract r3 = r1 << 3;    /* Left shift */
    long _Fract r4 = r2 << 2;    /* Another left shift */
    
    asm volatile ("" : : : "memory");
    
    return r3 + r4;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Initialize arrays of fixed-point values */
    short _Fract sf_array[10];
    _Accum accum_array[10];
    long _Fract lf_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        /* Use volatile source to prevent compile-time evaluation */
        sf_array[i] = (short _Fract)(volatile_source * i);
        accum_array[i] = (_Accum)(volatile_source * i * 10);
        lf_array[i] = (long _Fract)(volatile_source * i * 100);
    }
    
    short _Fract sf_result = 0.0r;
    _Accum accum_result = 0.0k;
    long _Fract lf_result = 0.0lr;
    
    /* Perform fixed-point operations in loop */
    for (int i = 0; i < iterations; i++) {
        int idx1 = i % 10;
        int idx2 = (i + 1) % 10;
        int idx3 = (i + 2) % 10;
        
        /* Multiple fixed-point multiplications */
        sf_result += process_fract(sf_array[idx1], sf_array[idx2], i % 4);
        
        /* Memory barrier between operations */
        asm volatile ("" : : : "memory");
        
        /* _Accum operations */
        accum_result += process_accum(accum_array[idx1], accum_array[idx2], (i + 1) % 4);
        
        /* Mixed operations with integer promotion */
        lf_result += mixed_operations(sf_array[idx3], i, i * 100);
        
        /* Additional left shift operations */
        sf_array[idx1] = sf_array[idx1] << 1;
        accum_array[idx2] = accum_array[idx2] << 2;
        
        /* Narrowing conversions that may overflow */
        if (i % 3 == 0) {
            short _Fract narrow = (short _Fract)lf_array[idx3];
            sf_result += narrow;
        }
    }
    
    /* Force result to be used to prevent dead code elimination */
    volatile short _Fract vsf = sf_result;
    volatile _Accum vacc = accum_result;
    volatile long _Fract vlf = lf_result;
    
    /* Print something to ensure code isn't optimized away entirely */
    printf("Results: %hd %hd %ld\n", 
           (short)(vsf * 1000), 
           (short)(vacc * 1000), 
           (long)(vlf * 1000));
    
    return 0;
}
