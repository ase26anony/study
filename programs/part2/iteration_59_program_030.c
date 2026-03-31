/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8p-7lr;  /* 0.0078125 */

/* Function with fixed-point operations that may overflow */
short _Fract process_fixed(short _Fract a, short _Fract b, int shift) {
    /* Intermediate multiplication that could overflow */
    short _Fract prod = a * b;
    
    /* Left shift operation - triggers FIXED_LSHIFT_EXPR */
    _Accum shifted = (_Accum)prod << shift;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Convert with potential overflow */
    short _Fract result = (short _Fract)shifted;
    
    return result;
}

/* Another function with different fixed-point types */
_Accum accumulate_fixed(_Fract *arr, int size, int shift_amount) {
    _Accum total = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Mix fixed-point with integer promotion */
        _Accum temp = (_Accum)arr[i] * (i + 1);
        
        /* Left shift that may overflow */
        temp = temp << shift_amount;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        total += temp;
    }
    
    return total;
}

/* Function that specifically triggers saturation checks */
void test_saturation(void) {
    /* Values close to bounds */
    short _Fract near_max = 0.999r;  /* Close to maximum */
    short _Fract near_min = -0.999r; /* Close to minimum */
    
    /* Operations that may overflow */
    short _Fract prod1 = near_max * near_max;  /* May exceed 1.0 */
    short _Fract prod2 = near_min * near_min;  /* May exceed -1.0 */
    
    /* Left shifts that definitely overflow */
    _Accum shifted1 = (_Accum)near_max << 2;
    _Accum shifted2 = (_Accum)near_min << 2;
    
    /* Assign to narrower types to force range checking */
    short _Fract narrow1 = (short _Fract)shifted1;
    short _Fract narrow2 = (short _Fract)shifted2;
    
    /* Memory barriers between operations */
    asm volatile ("" : : : "memory");
    
    /* Use results to prevent dead code elimination */
    volatile short _Fract sink1 = prod1;
    volatile short _Fract sink2 = prod2;
    volatile short _Fract sink3 = narrow1;
    volatile short _Fract sink4 = narrow2;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 10) iterations = 10;
    
    /* Initialize arrays with various fixed-point values */
    _Fract f_array[10];
    short _Fract sf_array[10];
    _Accum accum_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        f_array[i] = (i % 10) * 0.1r;
        sf_array[i] = (i % 10) * 0.1r;
        accum_array[i] = (i % 10) * 0.1k;
    }
    
    /* Test saturation logic */
    test_saturation();
    
    /* Main processing loop with varying shift amounts */
    _Accum total_result = 0.0k;
    
    for (int i = 0; i < iterations; i++) {
        /* Get volatile value to prevent constant propagation */
        long _Fract vol_val = volatile_source * (i + 1);
        
        /* Convert to different fixed-point types */
        short _Fract a = (short _Fract)vol_val;
        short _Fract b = sf_array[i % 10];
        
        /* Perform operation that triggers fixed-value analysis */
        short _Fract result = process_fixed(a, b, i % 4);
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Accumulate results */
        total_result += (_Accum)result;
        
        /* Additional overflow-prone operations */
        _Accum temp = (_Accum)a * (_Accum)b;
        temp = temp << (i % 3 + 1);  /* Left shift 1-3 bits */
        
        /* Force assignment to narrower type */
        short _Fract narrowed = (short _Fract)temp;
        total_result += (_Accum)narrowed;
    }
    
    /* More complex fixed-point operations */
    _Accum array_result = accumulate_fixed(f_array, iterations, 2);
    total_result += array_result;
    
    /* Use result to prevent dead code elimination */
    volatile _Accum final_sink = total_result;
    
    /* Print something to ensure code runs */
    printf("Result: %ld\n", (long)(total_result * 1000));
    
    return 0;
}
