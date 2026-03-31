/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8p-1lr;  /* 0.5 in long fract */

/* Function with fixed-point parameters that performs operations
   likely to trigger the uncovered bounds checking logic */
static short _Fract process_fixed_point(short _Fract a, short _Fract b, 
                                        _Accum c, int shift_amount) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* FIXED_MULT_P with potential overflow */
    _Accum temp2 = temp1 << shift_amount;  /* FIXED_LSHIFT_EXPR */
    
    /* Force intermediate calculations with wider types */
    long _Fract wide_temp = (long _Fract)a * (long _Fract)b;
    wide_temp = wide_temp << (shift_amount + 1);
    
    /* Assignment to narrower type may trigger saturation checking */
    short _Fract result = temp2;    /* Potential overflow here */
    
    /* Another multiplication that might exceed bounds */
    result = result * a;
    
    return result;
}

/* Another function focusing on shift operations */
static _Accum shift_operations(_Accum base, int iterations) {
    _Accum result = base;
    
    for (int i = 0; i < iterations; i++) {
        /* Left shift that could overflow */
        result = result << 1;
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" : : : "memory");
        
        /* Multiplication after shift */
        result = result * 0.5k;
    }
    
    return result;
}

/* Process array of fixed-point values */
static void process_array(short _Fract *arr, int size) {
    _Accum accumulator = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Operations that mix types and could overflow */
        _Accum temp = arr[i] * 0.8k;
        temp = temp << (i % 4);  /* Variable shift amount */
        
        /* Check for overflow in assignment */
        short _Fract narrowed = temp;
        
        /* Accumulate with potential overflow */
        accumulator = accumulator + temp;
        
        /* Another memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Final operation that might trigger bounds checking */
    short _Fract final_result = accumulator;
    arr[0] = final_result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 5 + 1 : 3;
    
    /* Initialize various fixed-point arrays */
    short _Fract sf_arr[8];
    _Accum accum_arr[8];
    
    /* Initialize with values that could lead to overflow */
    for (int i = 0; i < 8; i++) {
        sf_arr[i] = 0.8r / (i + 1);
        accum_arr[i] = 0.9k * (i + 1);
    }
    
    /* Perform operations likely to trigger the uncovered code */
    short _Fract result1 = 0.5r;
    _Accum result2 = 0.0k;
    
    for (int i = 0; i < iterations; i++) {
        /* Read from volatile to prevent compile-time evaluation */
        long _Fract vol_val = volatile_source + (i * 0.1lr);
        
        /* Convert to shorter types with potential overflow */
        short _Fract short_val = vol_val;
        
        /* Operations that mix types */
        result1 = process_fixed_point(result1, short_val, 
                                     accum_arr[i % 8], i % 3 + 1);
        
        /* Shift operations */
        result2 = shift_operations(accum_arr[i % 8], i + 1);
        
        /* Array processing */
        process_array(sf_arr, 8);
        
        /* Complex expression that might overflow */
        accum_arr[i % 8] = (result1 * result2) << (i % 4);
    }
    
    /* Mix signed and unsigned fixed-point types */
    unsigned short _Fract usf1 = 0.9ur;
    unsigned _Accum uacc1 = 0.8uk;
    
    /* Operations on unsigned types */
    for (int i = 0; i < iterations; i++) {
        usf1 = usf1 * 0.95ur;
        uacc1 = uacc1 << 1;
        
        /* Cross-type operation (signed * unsigned) */
        _Accum mixed = result2 * uacc1;
        mixed = mixed << 2;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result1: %hd, Result2: %ld\n", 
           (short)(result1 * 1000), 
           (long)(result2 * 1000));
    
    /* Print array first element */
    printf("Array[0]: %hd\n", (short)(sf_arr[0] * 1000));
    
    return 0;
}
