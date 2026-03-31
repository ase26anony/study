/* fixed-point-test.c - Designed to trigger fixed-value.cc uncovered lines 264-277 */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.5r;
volatile _Accum volatile_acc = 0.5k;

/* Function with fixed-point parameters to force analysis */
_Accum process_fixed_point(short _Fract a, _Accum b, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;  /* Mixed-type multiplication */
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    _Accum temp2 = temp1 << shift;
    
    /* Another multiplication with potential overflow */
    long _Fract lf1 = 0.9lr;
    long _Fract lf2 = 0.95lr;
    short _Fract narrow_result = lf1 * lf2;  /* Potential overflow into narrower type */
    
    asm volatile ("" : : : "memory");
    
    /* Combine results */
    return temp2 + (_Accum)narrow_result;
}

/* Another function with saturation-prone operations */
short _Fract saturating_multiply(short _Fract a, short _Fract b) {
    /* This multiplication in wider intermediate precision */
    /* may trigger the max_r/max_s bounds checking */
    _Accum wider = (_Accum)a * (_Accum)b;
    
    /* Convert back with potential saturation */
    return (short _Fract)wider;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations > 100) iterations = 100;  /* Safety limit */
    
    /* Array of fixed-point values */
    _Accum accum_array[10];
    short _Fract fract_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        accum_array[i] = (_Accum)(i * 0.1k);
        fract_array[i] = (short _Fract)(i * 0.05r);
    }
    
    _Accum total_result = 0.0k;
    
    /* Main loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Read from volatile to prevent compile-time computation */
        short _Fract sf1 = volatile_sf + (short _Fract)(i * 0.01r);
        _Accum acc1 = volatile_acc + (_Accum)(i * 0.02k);
        
        /* Operation 1: Multiplication with potential overflow */
        /* Using values close to 1.0 to maximize overflow chance */
        short _Fract a = 0.9r - (short _Fract)(i * 0.01r);
        short _Fract b = 0.95r;
        short _Fract c = a * b;  /* FIXED_MULT_P */
        
        asm volatile ("" : : : "memory");
        
        /* Operation 2: Left shift of fixed-point (FIXED_LSHIFT_EXPR) */
        _Accum shifted = acc1 << (i % 4);  /* Variable shift amount */
        
        /* Operation 3: Mixed-width operations */
        long _Fract lf1 = 0.99lr;
        long _Fract lf2 = 0.98lr;
        /* Assignment to narrower type forces range check */
        short _Fract narrow = lf1 * lf2;
        
        /* Operation 4: Call function with complex operations */
        _Accum processed = process_fixed_point(sf1, acc1, (i % 3) + 1);
        
        /* Operation 5: Another saturation-prone multiplication */
        short _Fract sat_result = saturating_multiply(
            fract_array[i % 10], 
            (short _Fract)0.99r
        );
        
        /* Combine results in a way that prevents dead code elimination */
        total_result += (_Accum)c + shifted + (_Accum)narrow + processed + (_Accum)sat_result;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
        
        /* Update array elements to create data dependencies */
        accum_array[i % 10] = shifted * 0.5k;
        fract_array[i % 10] = sat_result;
    }
    
    /* Use the result to prevent optimization */
    volatile _Accum output = total_result;
    
    /* Print something to ensure code isn't optimized away */
    printf("Result: %f\n", (double)output);
    
    return 0;
}
